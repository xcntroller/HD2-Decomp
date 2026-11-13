#ifndef HD2_GAME_MISSION_PARTICLE_INSTANCE_H
#define HD2_GAME_MISSION_PARTICLE_INSTANCE_H
#include "game/mission_particle_point_pool.h"
#include "game/mission_particle_publication.h"
#include "frontend/cleanup.h"
#include <algorithm>
#include <optional>

namespace hd2::game {
//00440530/004406A0's actual EMTR visual and00440800/00440B20 instance. Native
//point/definition/scene borrowers are typed; the process owns the shared pool
//and publication scratch. Frames must outlive every active AND cached instance.
template<class Frames>class MissionParticleInstance {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().create_mission_particle_visual());
    using Parent=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    struct Timer {std::int32_t wait=0,accumulated=0,age=0,period=0;float rate=0;std::int32_t repeats=0;};
    MissionParticleInstance(Frames& frames,MissionParticlePointPool& points,MissionParticleScratch& scratch)
        :frames_(frames),pool_(points),scratch_(scratch),visual_(frames.create_mission_particle_visual()){
        if(!visual_)throw std::runtime_error("Mission particle requires an actual EMTR visual");
    }
    ~MissionParticleInstance(){try{close();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    MissionParticleInstance(const MissionParticleInstance&)=delete;
    MissionParticleInstance& operator=(const MissionParticleInstance&)=delete;
    template<class Light>void initialize(Frame scene,const HumanMatrix& matrix,Frame parent,
            const EffectDefinition& definition,bool orientation,Light&& light){ //00440800.
        if(definition_)throw std::logic_error("Particle instance initialization repeated");
        begin(scene,matrix,parent);definition_=&definition;
        const auto radius=particle_real(definition.fields18[0]);radius_squared_=radius*radius;
        lighting_=definition.flag10?static_cast<float>(light(position(matrix))):1.f;
        frames_.mission_particle_texture(visual_.get(),definition.texturec);
        timers_.reserve(definition.emitters.size());
        for(const auto& emitter:definition.emitters){Timer timer;reset_timer(timer,emitter,false);timers_.push_back(timer);}
        frames_.mission_particle_initial_mode(visual_.get(),definition.field14,definition.fields20);
        publish_start(orientation);
    }
    template<class Light>void reuse(Frame scene,const HumanMatrix& matrix,Frame parent,bool orientation,Light&& light){ //00440B20.
        if(!definition_||!points_.empty()||parent_)throw std::logic_error("Particle reuse requires its actual detached empty instance");
        if(timers_.size()!=definition_->emitters.size())throw std::logic_error("Particle reuse lost actual emitter timers");
        begin(scene,matrix,parent);lighting_=definition_->flag10?static_cast<float>(light(position(matrix))):1.f;
        for(std::size_t n=0;n!=timers_.size();++n)reset_timer(timers_[n],definition_->emitters[n],true);
        publish_start(orientation);
    }
    void retire_to_pool(){ //00440D80: unlink, release parent, hide. Point ownership stays on this instance.
        if(!visual_)return;
        frontend::CleanupErrors errors;errors.attempt([&]{frames_.human_equipment_link(visual_.get(),Frame{},0);});
        parent_.reset();errors.attempt([&]{frames_.object_visible(visual_.get(),false);});errors.rethrow();
    }
    void close(){ //004406A0, returns points in reverse BEFORE visual release and vector retirement.
        if(!visual_)return;
        frontend::CleanupErrors errors;errors.attempt([&]{retire_to_pool();});
        while(!points_.empty())points_.pop_back();
        timers_.clear();visual_.reset();
        std::vector<Timer>().swap(timers_);std::vector<MissionParticlePointPool::Lease>().swap(points_);
        errors.rethrow();
    }
    bool update(std::uint32_t elapsed){ //004427A0, cleanup follows publication, including its false-epoch path.
        require_bound();finished_=true;
        if(flags_&4u){(void)frames_.mission_particle_vertices(visual_.get(),0,0.f);return true;}
        const bool current=current_epoch();
        advance_points(elapsed);
        if(flags_&1u)advance_emitters(elapsed);
        prepare_geometry(current);
        if(current)publish_geometry();else (void)frames_.mission_particle_vertices(visual_.get(),0,0.f);
        for(std::size_t n=0;n<points_.size();)if(points_[n]->remaining==0)points_.erase(points_.begin()+static_cast<std::ptrdiff_t>(n));else ++n; //004412D0, stable erase and SAME pool.
        return !finished_;
    }
    void present(){ //00442860, no elapsed-dependent point/emitter/retirement calls.
        require_bound();if(flags_&4u){(void)frames_.mission_particle_vertices(visual_.get(),0,0.f);return;}
        const bool current=current_epoch();prepare_geometry(current);
        if(current)publish_geometry();else (void)frames_.mission_particle_vertices(visual_.get(),0,0.f);
    }
    Frame visual()const{return visual_.get();}Frame parent()const{return parent_?parent_->get():Frame{};}
    const auto& points()const{return points_;}const auto& timers()const{return timers_;}
private:
    static HumanVector position(const HumanMatrix& matrix){return {{matrix[12],matrix[13],matrix[14],matrix[15]}};}
    void require_bound()const{if(!scene_||!definition_||!visual_)throw std::logic_error("Mission particle lost its actual bound owners");}
    void begin(Frame scene,const HumanMatrix& matrix,Frame parent){
        if(!scene)throw std::logic_error("Particle requires actual module17 scene100C");
        scene_=scene;world_=matrix;clock94_=100;flags_=1;
        if(parent){local_=frames_.rigid_matrix_product(matrix,frames_.car_inverse_matrix(frames_.actor_world_matrix(parent)));
            parent_.emplace(frames_.retain_actor_frame(parent));}
        else local_=human_identity();
    }
    static void reset_timer(Timer& timer,const EffectEmitterDefinition& emitter,bool reused){
        timer.wait=timer.accumulated=timer.age=0;const auto rate=particle_integer(emitter.fields24[2]);timer.rate=static_cast<float>(rate);
        timer.period=timer.rate==0?0:particle_truncate((1.L/(reused?static_cast<long double>(timer.rate):static_cast<long double>(rate)))*1000.f);
        timer.repeats=particle_integer(emitter.fields24[6]);
    }
    void refresh_world(){if(parent_)world_=frames_.rigid_matrix_product(local_,frames_.actor_world_matrix(parent_->get()));}
    void publish_start(bool orientation){
        frames_.mission_particle_world(visual_.get(),world_);
        frames_.human_equipment_link(visual_.get(),parent(),parent()?1u:0u);
        if(!parent())frames_.mission_particle_place(scene_,visual_.get());
        frames_.update_scene_object(visual_.get());
        for(std::size_t n=0;n!=definition_->emitters.size();++n){ //00440FE0, signed burst count = base + engine draw*range.
            const auto& emitter=definition_->emitters[n];const auto base=particle_integer(emitter.fields24[0]),range=particle_integer(emitter.fields24[1]);
            if(!base&&!range)continue;
            auto count=particle_truncate(static_cast<long double>(frames_.volume_random_unit())*range+base);
            if(count<0)throw std::runtime_error("Particle burst reached native negative-count loop");
            for(;count;--count){refresh_world();create_point(emitter,0,static_cast<std::uint32_t>(n));}
        }
        //004432B0 is the actual three-byte RET4; no successful service stands in for it.
        frames_.object_visible(visual_.get(),true);orientation_=orientation;
    }
    void create_point(const EffectEmitterDefinition& definition,std::uint32_t elapsed,std::uint32_t index){ //00440DC0.
        auto point=pool_.acquire();
        if(point->initialize(world_,definition,elapsed,index,frames_))points_.push_back(std::move(point));
    }
    void advance_points(std::uint32_t elapsed){ //00441390, count captured before emitter creation.
        const auto count=points_.size();refresh_world();
        for(std::size_t n=0;n!=count;++n){auto& point=*points_[n];point.advance(elapsed,frames_);
            if(point.remaining==0){++point.phase;(void)point.select_phase(world_,frames_);}}
    }
    void advance_emitters(std::uint32_t elapsed){ //004410B0, single initial500 gate; bursts may exceed it.
        if(points_.size()>=500)return;
        for(std::size_t n=0;n!=definition_->emitters.size();++n){auto& timer=timers_.at(n);const auto& emitter=definition_->emitters[n];
            if(timer.wait>0){timer.wait=particle_integer(static_cast<std::uint32_t>(timer.wait)-elapsed);if(timer.wait>0)continue;}
            if(timer.period<1)continue;
            timer.age=particle_integer(static_cast<std::uint32_t>(timer.age)+elapsed);
            if(emitter.fields24[5]&&particle_integer(emitter.fields24[4])<timer.age){
                const auto product=particle_integer(emitter.fields24[5]*elapsed);
                const auto rate=static_cast<long double>(timer.rate)-static_cast<long double>(product)*MissionParticlePoint::seconds;
                timer.rate=static_cast<float>(rate);timer.period=rate==0?0:particle_truncate((1.L/rate)*1000.f);
            }
            if(timer.period<1){
                if(timer.repeats<2&&timer.repeats!=-1)continue;
                if(timer.repeats>1)--timer.repeats;
                timer.wait=particle_integer(emitter.fields24[7]);timer.rate=static_cast<float>(particle_integer(emitter.fields24[2]));
                timer.period=timer.rate==0?0:particle_truncate((1.L/particle_integer(emitter.fields24[2]))*1000.f);timer.age=timer.accumulated=0;
            }else{
                timer.accumulated=particle_integer(static_cast<std::uint32_t>(timer.accumulated)+elapsed);
                const auto count=timer.accumulated/timer.period;
                for(std::int32_t step=0;step<count;++step){refresh_world();create_point(emitter,static_cast<std::uint32_t>(timer.accumulated),static_cast<std::uint32_t>(n));
                    timer.accumulated=particle_integer(static_cast<std::uint32_t>(timer.accumulated)-static_cast<std::uint32_t>(timer.period));}
            }
            finished_=false;
        }
    }
    bool current_epoch(){
        const auto epoch=frames_.vertex_frame_epoch(visual_.get());
        if(epoch==0xffffffffu){frames_.mission_particle_epoch(visual_.get(),0);return true;}
        return epoch==frames_.mission_particle_scene_epoch(scene_);
    }
    void prepare_geometry(bool current){ //00441460, bounds are built even when the visual's epoch is stale.
        scratch_.order.clear();if(finished_&&points_.empty())return;
        if(!points_.empty())finished_=false;
        if(points_.empty())return;
        if(current)scratch_.order.reserve(points_.size());
        refresh_world();average_depth_=0;const auto inverse=frames_.mission_particle_inverse(visual_.get());
        std::array<float,12> bounds{{1e16f,1e16f,1e16f,0,-1e16f,-1e16f,-1e16f,0,0,0,0,0}};
        for(std::size_t n=0;n!=points_.size();++n){const auto& point=*points_[n];auto point_position=position(point.matrix);
            if(!(point.flags&1u))for(unsigned axis=0;axis!=3;++axis)point_position[axis]+=world_[12+axis];
            const float depth=frames_.mission_particle_depth(scene_,point_position);average_depth_+=depth;
            if(current)scratch_.order.push_back({human_loading_stat_word(static_cast<long double>(depth)*1000.f),static_cast<std::uint32_t>(n)});
            point_position=frames_.car_transform(point_position,inverse);
            const float radius=static_cast<float>(static_cast<long double>(point.size)*.5f*.8660255074501038f);
            for(unsigned axis=0;axis!=3;++axis){const float lower=point_position[axis]-radius,upper=point_position[axis]+radius;
                if(lower<bounds[axis])bounds[axis]=lower;
                if(bounds[4+axis]<upper)bounds[4+axis]=upper;}
        }
        if(current)std::stable_sort(scratch_.order.begin(),scratch_.order.end(),[](const auto& a,const auto& b){return a.depth<b.depth;}); //006E4910 stable unsigned radix order.
        average_depth_=static_cast<float>(static_cast<long double>(average_depth_)/static_cast<std::int32_t>(points_.size()));
        if(bounds[0]<bounds[4]){
            HumanVector half{};for(unsigned axis=0;axis!=3;++axis){const float extent=bounds[4+axis]-bounds[axis];half[axis]=extent*.5f;bounds[8+axis]=half[axis]+bounds[axis];}
            bounds[11]=static_cast<float>(std::sqrt((static_cast<long double>(half[2])*half[2]+static_cast<long double>(half[1])*half[1])+static_cast<long double>(half[0])*half[0]));
        }
        frames_.mission_particle_bounds(visual_.get(),bounds);frames_.update_scene_object(visual_.get());
    }
    void publish_geometry(){ //00441A00, first500 sorted rows in REVERSE, four24-byte vertices per row.
        const auto count=std::min<std::size_t>(500,scratch_.order.size());
        if(!count){(void)frames_.mission_particle_vertices(visual_.get(),0,0.f);return;}
        auto billboard=frames_.mission_particle_camera_matrix(scene_);
        if(!orientation_)frames_.formation_matrix_direction(billboard,{{-world_[8],-world_[9],-world_[10],0}});
        auto* vertices=frames_.mission_particle_vertices(visual_.get(),static_cast<std::uint32_t>(count),average_depth_);if(!vertices)return;
        refresh_world();
        constexpr std::array<HumanVector,4> corners{{{{-.5f,.5f,0,0}},{{.5f,.5f,0,0}},{{-.5f,-.5f,0,0}},{{.5f,-.5f,0,0}}}};
        for(auto n=count;n;){const auto& point=*points_.at(scratch_.order[--n].index);const auto colour=particle_vertex_colour(point,lighting_);
            auto matrix=billboard;std::uint32_t angle_word;std::memcpy(&angle_word,&point.angle,4);
            if((angle_word&0x7fffffffu)>=0x322bcc77u){auto roll=human_identity();const auto angle=static_cast<long double>(point.angle);
                roll[0]=roll[5]=static_cast<float>(std::cos(angle));roll[1]=static_cast<float>(std::sin(angle));roll[4]=-roll[1];
                matrix=frames_.mission_particle_rotate_matrix(roll,billboard);}
            for(unsigned row=0;row!=3;++row)for(unsigned axis=0;axis!=3;++axis)matrix[row*4+axis]*=point.size;
            for(unsigned axis=0;axis!=3;++axis)matrix[12+axis]=(point.flags&1u)?point.matrix[12+axis]:point.matrix[12+axis]+world_[12+axis];
            const auto& uv=point.definition->colour_keys.at(point.texture_index);
            for(unsigned corner=0;corner!=4;++corner){const auto position=frames_.car_transform(corners[corner],matrix);
                vertices[corner]={{{position[0],position[1],position[2]}},colour,particle_real(uv[corner<2?0u:2u]),particle_real(uv[corner%2?3u:1u])};}
            vertices+=4;
        }
    }
    Frames& frames_;MissionParticlePointPool& pool_;MissionParticleScratch& scratch_;Owner visual_;
    std::optional<Parent> parent_;Frame scene_{};const EffectDefinition* definition_=nullptr;
    HumanMatrix world_=human_identity(),local_=human_identity();
    std::uint32_t clock94_=0,flags_=1;float radius_squared_=0,lighting_=1,average_depth_=0;bool finished_=false,orientation_=false;
    std::vector<MissionParticlePointPool::Lease> points_;std::vector<Timer> timers_;
};
} //namespace hd2::game
#endif
