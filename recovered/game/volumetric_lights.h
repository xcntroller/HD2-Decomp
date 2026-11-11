#ifndef HD2_GAME_VOLUMETRIC_LIGHTS_H
#define HD2_GAME_VOLUMETRIC_LIGHTS_H

#include "game/serialized_bytes.h"
#include "frontend/cleanup.h"
#include "io/resource_observations.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <utility>

namespace hd2::game {

struct VolumetricVertex {
    std::array<float,3> position{};
    std::uint32_t colour=0;
    std::array<float,2> uv{};
};
static_assert(sizeof(VolumetricVertex)==24);

//00482A00/00481810/00481440 and their construction/refresh/cleanup owners.
//Frame/texture endpoints are mandatory real scene/graphics methods. The scene
//outlives these borrowed light references. Rendering uses the SAME geometry,
//textures and alpha records; the caller supplies actual visibility/phase state.
template<class Frames>
class VolumetricLights {
public:
    using Frame=typename Frames::Frame;
    using TextureOwner=decltype(std::declval<Frames&>().load_car_particle_texture(
        std::declval<const std::string&>(),std::declval<const std::string&>(),unsigned{}));
    struct Particle {
        std::string texture_name;
        float angle=0.7853981852531433f,radius=2,centre_alpha=.5f,edge_alpha=.5f,seed=0;
        bool ribbon=false;
        float field3c8=.1f,field3d0=3.1415927410125732f,field3d4=0,field3d8=0,uv_scale=1;
        std::int32_t segments=3;
        std::optional<TextureOwner> texture;
        std::vector<VolumetricVertex> vertices;
        std::vector<float> alphas;
        std::array<std::array<float,2>,3> ribbon_uv{};
    };
    struct Volume {
        Frame light{};
        std::string light_name; //Owning diagnostic copy of the actual serialized reference; no native field claim.
        float field18=10,field1c=0,field20=0,field28=0,field2c=0,field30=30,field34=25,plane38=0,distance44=0;
        //00481320 leaves3C/40 unwritten. No invented first-frame direction.
        std::optional<std::array<float,2>> history3c;
        float render_distance14=0;bool visible24=false;
        std::vector<std::unique_ptr<Particle>> particles;
        ~Volume() {while(!particles.empty())particles.pop_back();} //004813D0/00480D00.
    };
    explicit VolumetricLights(Frames& frames,std::function<void()> retire_procedural={})
        :frames_(frames),retire_procedural_(std::move(retire_procedural)) {}
    ~VolumetricLights() {clear();}
    VolumetricLights(const VolumetricLights&)=delete;
    VolumetricLights& operator=(const VolumetricLights&)=delete;
    const std::vector<std::unique_ptr<Volume>>& values() const {return volumes_;}
    //00480390, over the original retained texture and geometry. An alpha<0
    //preserves the previous vertex colours; it is not a fully opaque request.
    template<class Draw>void render_particle(Particle& particle,std::int32_t time,float phase,float alpha,Draw& draw){
        if(!particle.texture||!(*particle.texture))return;
        draw.volume_bind_texture(particle.texture->get());
        auto opacity=[&](std::size_t n){
            if(alpha>=0.f){
                const float scaled=static_cast<float>(static_cast<long double>(alpha)*particle.alphas.at(n)*255.f);
                const auto rounded=std::nearbyint(static_cast<long double>(scaled));
                const auto word=!std::isfinite(rounded)||rounded<-2147483648.L||rounded>2147483647.L?0x80000000u:
                    static_cast<std::uint32_t>(static_cast<std::int32_t>(rounded));
                particle.vertices.at(n).colour=(word<<24)|0x00ffffffu;
            }
        };
        if(particle.ribbon){
            const auto value=std::trunc(static_cast<long double>(particle.seed)*1000.f+time);
            if(!std::isfinite(value)||value<-2147483648.L||value>2147483647.L)throw std::runtime_error("Volumetric ribbon clock exceeds native integer range");
            const auto tick=static_cast<std::int32_t>(value);
            const float u=static_cast<float>(static_cast<long double>(tick)*particle.field3d4*.0010000000474974513f);
            const float v=static_cast<float>(static_cast<long double>(tick)*particle.field3d8*.0010000000474974513f);
            for(std::size_t n=3;n;){--n;auto& vertex=particle.vertices.at(n);
                vertex.uv={static_cast<float>(static_cast<long double>(u)+particle.ribbon_uv[n][0]),
                           static_cast<float>(static_cast<long double>(v)+particle.ribbon_uv[n][1])};opacity(n);
            }
            draw.volume_draw(particle.vertices.data(),1);return;
        }
        //The native field3D0 is multiplied by the FLOAT degree conversion,
        //including its default pi value. Do not reinterpret it as radians.
        const auto count=particle.segments;
        if(count<0||count>10)throw std::runtime_error("Invalid volumetric fan draw count");
        float angle=static_cast<float>((static_cast<long double>(phase)+particle.seed)*particle.field3c8);
        const float step=static_cast<float>(static_cast<long double>(particle.field3d0)*.01745329238474369f/count);
        for(std::int32_t n=0;n<count;++n){
            const auto offset=std::size_t(n)*3;
            auto uv=[](long double a){return std::array<float,2>{static_cast<float>((std::cos(a)+1.L)*.5),static_cast<float>((std::sin(a)+1.L)*.5)};};
            particle.vertices.at(offset+1).uv=uv(angle);
            const auto next=static_cast<long double>(step)+angle;particle.vertices.at(offset+2).uv=uv(next);angle=static_cast<float>(next);
            opacity(offset);opacity(offset+1);opacity(offset+2);
        }
        draw.volume_draw(particle.vertices.data(),static_cast<std::uint32_t>(count));
    }
    //00480D80. Distance and angular accumulator MUST come from00482260's
    //actual per-volume state; this leaf does not manufacture camera visibility.
    template<class Draw>bool render_volume(Volume& volume,std::int32_t time,const std::array<float,4>& camera,
            float distance,float angular_accumulator,float phase,Draw& draw){
        if(!volume.light||!draw.volume_visible(volume.light))return false;
        auto matrix=frames_.car_world_matrix(volume.light);
        const std::array<float,4> original_forward{{matrix[8],matrix[9],matrix[10],matrix[11]}};
        const std::array<float,4> relative{{static_cast<float>(static_cast<long double>(matrix[12])-camera[0]),
            static_cast<float>(static_cast<long double>(matrix[13])-camera[1]),static_cast<float>(static_cast<long double>(matrix[14])-camera[2]),0}};
        draw.volume_set_dir3(matrix,original_forward,relative);
        matrix[3]=matrix[7]=matrix[11]=0; //Native zeros homogeneous basis lanes, NOT world translation.
        draw.volume_world_transform(matrix);draw.volume_graphics_state(13,volume.field18<distance);
        const float distance_phase=static_cast<float>(static_cast<long double>(volume.field28)*distance);
        const float angle_phase=static_cast<float>(static_cast<long double>(angular_accumulator)*volume.field2c);
        if(distance>=volume.field30)return false;
        float alpha=-1.f;
        if(distance<=volume.field34){
            if(volume.field1c!=0){
                if(distance>=volume.field1c)alpha=1.f;
                else if(distance<=volume.field20)return false;
                else alpha=static_cast<float>((static_cast<long double>(distance)-volume.field20)/(static_cast<long double>(volume.field1c)-volume.field20));
            }
        }else alpha=static_cast<float>(1.L-(static_cast<long double>(distance)-volume.field34)/(static_cast<long double>(volume.field30)-volume.field34));
        if(alpha==0.f)return false;
        const float spin=static_cast<float>(static_cast<long double>(angle_phase)+distance_phase+phase);
        for(auto& particle:volume.particles)render_particle(*particle,time,spin,alpha,draw);
        return true;
    }
    //00482260 containing traversal. Procedural is the SEPARATE process0089B63C
    //receiver, called even with no volumes. Missing history is an explicit native
    //safety boundary, not original successful behavior or an initial-angle policy.
    template<class Draw,class Procedural>void render(std::int32_t time,Draw& draw,Procedural& procedural){
        const auto driver=std::uint32_t(draw.hud_driver_state(3)!=0);
        const auto graph14=std::uint32_t(draw.volume_graphics_state(14)!=0);
        const auto graph13=std::uint32_t(draw.volume_graphics_state(13)!=0);
        frontend::CleanupErrors errors;
        try{
            io::observe_bound_data(frames_,"execution","00482260 input","clock224 bits",static_cast<std::uint32_t>(time));
            draw.volume_graphics_state(14,0);
            procedural.render(time);
            draw.hud_driver_state(3,1);
            const auto camera=draw.volume_render_camera();
            const auto position=frames_.car_world_matrix(camera);
            observe_float("00482260 camera","position X bits",position[12]);
            observe_float("00482260 camera","position Y bits",position[13]);
            observe_float("00482260 camera","position Z bits",position[14]);
            const auto orientation=frames_.car_world_matrix(draw.volume_render_camera());
            auto forward=normalize({orientation[8],orientation[9],orientation[10]});
            forward=normalize({forward[0],0,forward[2]});
            if(!camera_history_)throw std::runtime_error("00482260 requires original owner18/1C angular history");
            observe_float("00482260 camera","previous horizontal X bits",(*camera_history_)[0]);
            observe_float("00482260 camera","previous horizontal Z bits",(*camera_history_)[1]);
            phase_=static_cast<float>(static_cast<long double>(phase_)+angle_delta(*camera_history_,{forward[0],forward[2]}));
            camera_history_=std::array<float,2>{forward[0],forward[2]};
            observe_float("00482260 camera","current horizontal X bits",forward[0]);
            observe_float("00482260 camera","current horizontal Z bits",forward[2]);
            observe_float("00482260 camera","phase10 bits",phase_);
            for(std::size_t n=0;n<volumes_.size();){
                auto& volume=*volumes_[n];
                io::observe_bound_data(frames_,"execution","00482260 selected volume",volume.light_name.c_str(),n);
                const auto relation=draw.volume_visibility_relation(volume.light);
                if(!relation){volumes_.erase(volumes_.begin()+n);continue;}
                volume.visible24=draw.volume_relation_visible(relation);
                if(volume.visible24){
                    const auto light=frames_.car_world_matrix(volume.light);
                    const float x=static_cast<float>(static_cast<long double>(light[12])-position[12]);
                    const float z=static_cast<float>(static_cast<long double>(light[14])-position[14]);
                    const auto direction=normalize({x,0,z});
                    if(static_cast<long double>(direction[0])*forward[0]+static_cast<long double>(direction[1])*forward[1]+static_cast<long double>(direction[2])*forward[2]<0)
                        volume.visible24=false;
                    else{
                        const float squared=static_cast<float>(static_cast<long double>(x)*x+static_cast<long double>(z)*z);
                        std::int32_t bits;std::memcpy(&bits,&squared,4);
                        volume.render_distance14=bits<0x322bcc77?0:std::abs(static_cast<long double>(squared)-1)<9.99999993922529e-09f?1:static_cast<float>(std::sqrt(static_cast<long double>(squared)));
                        if(volume.render_distance14<volume.field30){
                            observe_float("00482260 volume","distance14 bits",volume.render_distance14);
                            observe_float("00482260 volume","angular44 before update bits",volume.distance44);
                            observe_float("00482260 volume","field2C bits",volume.field2c);
                            observe_float("00482260 volume","current direction X bits",direction[0]);
                            observe_float("00482260 volume","current direction Z bits",direction[2]);
                            io::observe_bound_data(frames_,"execution","00482260 volume","history3C40 known",volume.history3c.has_value());
                            if(!volume.history3c){
                                io::observe_bound_data(frames_,"source-guard","00482260 unknown initial history",volume.light_name.c_str());
                                throw std::runtime_error("00482260 requires original volume3C/40 angular history");
                            }
                            observe_float("00482260 volume","previous direction3C bits",(*volume.history3c)[0]);
                            observe_float("00482260 volume","previous direction40 bits",(*volume.history3c)[1]);
                            volume.distance44=static_cast<float>(static_cast<long double>(volume.distance44)+angle_delta(*volume.history3c,{direction[0],direction[2]}));
                            volume.history3c=std::array<float,2>{direction[0],direction[2]};
                        }else volume.visible24=false;
                    }
                }
                ++n;
            }
            const std::array<float,4> eye{position[12],position[13],position[14],position[15]};
            //Native pairwise swapping mutates the retained owner order. Invisible
            //slots stay in place; using std::sort would change ties and later calls.
            for(std::size_t n=0;n<volumes_.size();++n)if(volumes_[n]->visible24){
                for(std::size_t j=n+1;j<volumes_.size();++j)
                    if(volumes_[j]->visible24&&volumes_[n]->render_distance14<volumes_[j]->render_distance14)
                        std::swap(volumes_[n],volumes_[j]);
                auto& volume=*volumes_[n];
                render_volume(volume,time,eye,volume.render_distance14,volume.distance44,phase_,draw);
            }
        }catch(...){errors=frontend::CleanupErrors(std::current_exception());}
        errors.attempt([&]{draw.hud_driver_state(3,driver);});
        errors.attempt([&]{draw.volume_graphics_state(14,graph14);});
        errors.attempt([&]{draw.volume_graphics_state(13,graph13);});errors.rethrow();
    }
    void clear() noexcept {if(retire_procedural_)retire_procedural_();while(!volumes_.empty())volumes_.pop_back();camera_history_={{1,0}};phase_=0;} //00482D70.
    bool read(SerializedBytes bytes) {
        clear();auto root=next_mission_chunk(bytes);
        if(!root||root->id!=1)return false;
        while(auto record=next_mission_chunk(root->bytes)) {
            auto volume=std::make_unique<Volume>();
            volume->distance44=static_cast<float>(static_cast<long double>(frames_.volume_random_unit())*6.2831854820251465f);
            read_volume(record->bytes,*volume);volumes_.push_back(std::move(volume));
        }
        refresh(true);return true; //00482A00; its module caller refreshes once more.
    }
    template<class Files>bool load(Files& files,const std::string& resource) {
        clear();const auto path=std::string("missions\\")+resource+"\\volumetrics.bin";
        const auto handle=files.open(path.c_str(),0);if(handle==0xffffffffu)return false;
        struct Close {Files& files;std::uint32_t handle;~Close(){files.close(handle);}} close{files,handle};
        const auto size=files.seek(handle,0,2);
        if(size==0xffffffffu||size>128u*1024u*1024u)throw std::runtime_error("Invalid volumetric light file size");
        if(files.seek(handle,0,0)!=0)throw std::runtime_error("Cannot rewind volumetric lights");
        std::vector<unsigned char> bytes(size);
        if(size&&files.read(handle,bytes.data(),size)!=size)throw std::runtime_error("Cannot read volumetric lights");
        return read(SerializedBytes(bytes));
    }
    void refresh(bool textures) { //00482980/00480FA0; preserve ascending removal order.
        for(std::size_t i=0;i<volumes_.size();) {
            auto& volume=*volumes_[i];
            if(!volume.light) {volumes_.erase(volumes_.begin()+i);continue;}
            volume.distance44=distance(volume.light);
            for(auto& particle:volume.particles)
                if(!textures||!particle->texture_name.empty())rebuild(*particle,textures,volume.plane38);
            ++i;
        }
    }
    void rebuild(Particle& particle,bool textures,float plane) { //00480740 RET8, not RET4.
        if(textures) {
            particle.texture.reset();
            if(!particle.texture_name.empty()) {
                auto name=particle.texture_name;
                if(name.find_last_of('.')==std::string::npos)name+=".bmp";
                particle.texture.emplace(frames_.load_car_particle_texture(name,name,0x404));
            }
        }
        particle.vertices.clear();particle.alphas.clear();
        if(!std::isfinite(particle.angle)||!std::isfinite(particle.radius)||
           !std::isfinite(particle.uv_scale)||!std::isfinite(plane))
            throw std::runtime_error("Non-finite volumetric geometry");
        const auto radians=static_cast<long double>(particle.angle)*0.01745329238474369f;
        auto emit=[&](std::array<float,3> position,float alpha,std::array<float,2> uv) {
            particle.vertices.push_back({position,colour(alpha),uv});particle.alphas.push_back(alpha);
        };
        if(particle.ribbon) {
            const float radius=static_cast<float>(std::tan(radians*.5f)*particle.radius);
            const float ratio=radius/particle.radius;
            if(!std::isfinite(radius)||!std::isfinite(ratio))throw std::runtime_error("Invalid volumetric ribbon geometry");
            const auto low=static_cast<float>(.5f-static_cast<long double>(ratio)*particle.uv_scale);
            const auto high=static_cast<float>(static_cast<long double>(ratio)*particle.uv_scale+.5f);
            const float end=static_cast<float>(static_cast<long double>(particle.radius)+plane);
            particle.ribbon_uv={{{.5f,0},{low,particle.uv_scale},{high,particle.uv_scale}}};
            emit({0,0,plane},particle.centre_alpha,particle.ribbon_uv[0]);
            emit({radius,0,end},particle.edge_alpha,particle.ribbon_uv[1]);
            emit({-radius,0,end},particle.edge_alpha,particle.ribbon_uv[2]);return;
        }
        if(particle.segments<=0)return; //Original signed loop emits no vertices.
        if(particle.segments>10)throw std::runtime_error("Volumetric fan exceeds original ten-segment storage");
        float angle=static_cast<float>(radians*-.5f+1.5707963705062866f);
        const float step=static_cast<float>(radians/particle.segments);
        auto edge=[&](long double value) {
            return std::array<float,3>{{static_cast<float>(std::cos(value)*particle.radius),0,
                static_cast<float>(std::sin(value)*particle.radius+plane)}};
        };
        for(std::int32_t i=0;i<particle.segments;++i) {
            emit({0,0,plane},particle.centre_alpha,{.5f,.5f});
            emit(edge(angle),particle.segments>1&&i==0?0:particle.edge_alpha,{0,0});
            const auto next=static_cast<long double>(step)+angle;
            emit(edge(next),particle.segments>1&&i+1==particle.segments?0:particle.edge_alpha,{0,0});
            angle=static_cast<float>(next); //Original FSTP at end of each segment.
        }
    }
private:
    void observe_float(const char* operation,const char* detail,float value)noexcept{
        std::uint32_t bits;std::memcpy(&bits,&value,sizeof bits);
        io::observe_bound_data(frames_,"execution",operation,detail,bits);
    }
    static long double angle_delta(const std::array<float,2>& old,const std::array<float,2>& next){
        auto delta=std::atan2(static_cast<long double>(next[1]),static_cast<long double>(next[0]))-
                   std::atan2(static_cast<long double>(old[1]),static_cast<long double>(old[0]));
        if(delta<-3.1415927410125732421875f)delta+=6.283185482025146484375f;
        if(delta>3.1415927410125732421875f)delta-=6.283185482025146484375f;
        return delta; //007885B0; no randomness.
    }
    static std::array<float,3> normalize(std::array<float,3> value){
        const float squared=static_cast<float>(static_cast<long double>(value[0])*value[0]+static_cast<long double>(value[1])*value[1]+static_cast<long double>(value[2])*value[2]);
        if(!std::isfinite(squared))throw std::runtime_error("Non-finite volumetric direction");
        if(std::abs(static_cast<long double>(squared)-1)>=9.99999993922529e-09f){
            if(squared>=9.99999993922529e-09f){
                const auto scale=1.L/std::sqrt(static_cast<long double>(squared));
                for(auto& lane:value)lane=static_cast<float>(lane*scale);
            }else if(value[0]!=0)value[0]=value[0]<0?-1:1;
            else if(value[2]!=0)value[2]=value[2]<0?-1:1;
            else value[1]=value[1]<0?-1:1;
        }
        return value;
    }
    static std::uint32_t colour(float alpha) {
        const float scaled=static_cast<float>(static_cast<long double>(alpha)*255.f);
        const auto rounded=std::nearbyint(static_cast<long double>(scaled)); //FISTP current rounding mode.
        if(!std::isfinite(rounded)||rounded<std::numeric_limits<std::int32_t>::min()||
           rounded>std::numeric_limits<std::int32_t>::max())throw std::runtime_error("Invalid volumetric vertex alpha");
        return (std::uint32_t(static_cast<std::int32_t>(rounded))<<24)|0x00ffffffu;
    }
    float distance(Frame light) {
        const auto matrix=frames_.car_world_matrix(light);
        const float squared=static_cast<float>(static_cast<long double>(matrix[14])*matrix[14]+
            static_cast<long double>(matrix[13])*matrix[13]+static_cast<long double>(matrix[12])*matrix[12]);
        if(!std::isfinite(squared))throw std::runtime_error("Non-finite volumetric light position");
        std::int32_t bits;std::memcpy(&bits,&squared,4);
        if(bits<0x322bcc77)return 0;
        return std::abs(static_cast<long double>(squared)-1)<9.99999993922529e-09f?1:
            static_cast<float>(std::sqrt(static_cast<long double>(squared)));
    }
    static std::string text(SerializedBytes& bytes) {
        return bytes.section(std::min<std::size_t>(bytes.remaining(),999)).text();
    }
    static void read_particle(SerializedBytes& bytes,Particle& particle) {
        while(auto chunk=next_mission_chunk(bytes)) {
            auto& value=chunk->bytes;
            switch(chunk->id) {
            case 0x3f2:particle.texture_name=text(value);break;
            case 0x3fc:particle.angle=value.real();break;
            case 0x406:particle.radius=value.real();break;
            case 0x410:particle.centre_alpha=value.real();break;
            case 0x41a:particle.edge_alpha=value.real();break;
            case 0x424:particle.ribbon=value.word()!=0;break;
            case 0x42e:particle.field3c8=value.real();break;
            case 0x438: {const auto word=value.word();std::memcpy(&particle.segments,&word,4);break;}
            case 0x442:particle.field3d0=value.real();break;
            case 0x44c:particle.field3d4=value.real();break;
            case 0x456:particle.field3d8=value.real();break;
            case 0x460:particle.uv_scale=value.real();break;
            default:break; //00481440 leaves unknown chunks.
            }
        }
    }
    void read_volume(SerializedBytes& bytes,Volume& volume) {
        float seed=0;
        while(auto chunk=next_mission_chunk(bytes)) {
            auto& value=chunk->bytes;
            switch(chunk->id) {
            case 10:
                volume.light_name=text(value);
                volume.light=frames_.find_scene_frame(volume.light_name,2);
                if(!volume.light)return; //Original leaves the volume; refresh deletes it.
                seed=distance(volume.light);break;
            case 0x14:volume.field18=value.real();break;
            case 0x19:volume.field1c=value.real();break;
            case 0x1c:volume.field20=value.real();break;
            case 0x1e:volume.field28=value.real();break;
            case 0x23:volume.field2c=value.real();break;
            case 0x26:volume.plane38=value.real();break;
            case 0x32:volume.field34=value.real();break;
            case 0x3c:volume.field30=value.real();break;
            case 0x28:
                while(auto record=next_mission_chunk(value)) {
                    auto particle=std::make_unique<Particle>();
                    particle->seed=static_cast<float>(static_cast<long double>(volume.particles.size())+seed);
                    read_particle(record->bytes,*particle);volume.particles.push_back(std::move(particle));
                }
                break;
            default:break; //00481810 leaves unknown chunks.
            }
        }
    }
    std::optional<std::array<float,2>> camera_history_{{{1,0}}};float phase_=0;
    Frames& frames_;
    std::function<void()> retire_procedural_; //Production00482D70 retires SAME process0089B63C first.
    std::vector<std::unique_ptr<Volume>> volumes_;
};

} //namespace hd2::game
#endif
