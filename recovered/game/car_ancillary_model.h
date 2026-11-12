#ifndef HD2_GAME_CAR_ANCILLARY_MODEL_H
#define HD2_GAME_CAR_ANCILLARY_MODEL_H

#include "game/car_damage_packets.h"
#include <cmath>
#include <functional>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace hd2::game {

//004E19C0: first case-sensitive substring, then atoi after the key. A missing
//key/frame returns -1; a present nonnumeric suffix returns0.
inline std::int32_t car_attribute_number(const std::string& text,const std::string& key) {
    const auto offset=text.find(key);
    if(offset==std::string::npos)return -1;
    const auto value=std::strtoll(text.c_str()+offset+key.size(),nullptr,10);
    if(value<std::numeric_limits<std::int32_t>::min()||value>std::numeric_limits<std::int32_t>::max())
        throw std::runtime_error("Car numeric attribute overflow");
    return static_cast<std::int32_t>(value);
}

//004E0930/004E09D0's DM model and metadata owners. The real optional load is
//attempted; a missing/rejected model leaves this owner empty as original.
template<class Frames>
class CarAncillaryModel {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().create_car_ancillary_model());
    struct Zone {Frame frame{};std::int32_t hit_points=-1,material=-1;};
    struct Explosion {Frame frame{};std::int32_t delay=0,part=-1;bool armed=false;std::optional<std::int32_t> instance;};
    ~CarAncillaryModel(){
        try{clear_metadata();}catch(...){if(frames_)frames_->record_actor_cleanup_failure(std::current_exception());}
    }
    template<class Particles>void bind_particles(Particles& particles){
        if(stop_particle_||active_particles_)throw std::logic_error("Car DM particle borrowers already bound");
        stop_particle_=[&particles](std::int32_t id){particles.car_retire_particle(id);};
        active_particles_=[&particles]{return particles.car_active_particles();};
    }
    void load(Frames& frames,const std::string& resource,std::size_t seat_count) {
        if(model_)throw std::logic_error("Car ancillary model loaded twice");
        frames_=&frames;
        auto model=frames.create_car_ancillary_model();
        if(!model)return;
        if(!frames.load_car_ancillary_model(model.get(),"DM"+resource))return;
        model_.emplace(std::move(model));
        parse(frames,seat_count);
        frames.object_visible(model_->get(),false);
    }
    //004E0EA0 ->004E0DC0/004E09D0. Particle identities borrow the SAME
    //process and retire in native order; no independent effect pool is made.
    void reset_loading(Frames& frames,std::size_t seat_count){
        clear_metadata();
        if(model_)parse(frames,seat_count); //004E09D0 genuinely returns for absent optional DM.
        countdown_=0;triggered_=false; //004E0EA0 AFTER004E0DC0/004E09D0, not004E08C0's initial-1.
        if(fire_instance_!=-1)retire_particle(fire_instance_);
        if(smoke_instance_!=-1)retire_particle(smoke_instance_);
    }
    void update_cold(std::uint32_t){ //004E1990's actual post-reset cold gate.
        if(triggered_||countdown_==0)return;
        throw std::runtime_error("Car timed DM destruction requires its actual004E1FE0/004DBF00 receiver");
    }
    //004E1DC0: borrow the actual Car frame twice, then align HOR/VERT markers
    //by their short names to the matching Car local quaternion. No frame clone.
    void align_damage_model(Frames& frames,Frame car) {
        if(!model())return;
        const auto rotation=frames.human_matrix_rotation(frames.actor_world_matrix(car));
        const auto world=frames.actor_world_matrix(car);
        frames.car_damage_model_pose(model(),{{world[12],world[13],world[14],world[15]}},rotation);
        frames.link_scene_object(model(),frames.car_damage_root());
        for(auto n=planes_.size();n;){const auto marker=planes_[--n];
            const auto original=frames.car_find(car,frames.car_short_name(marker).c_str(),0xffff);
            if(!original)throw std::runtime_error("Car DM plane has no matching actual Car frame");
            frames.car_damage_plane_rotation(marker,original);
            frames.update_scene_object(marker);
        }
        frames.update_scene_object(model());
    }
    //004E1A20 RET20; the containing actor owns the source identity and6F8/FF4
    //events. This is its separate24E0 DM receiver, never physics00500EB0.
    template<class Runtime,class Actor>void damage(Frames& frames,Frame car,Runtime& runtime,Actor self,
            std::uint32_t kind,float amount,const CarContactVector& point,const CarContactVector& direction,Actor source) {
        if(triggered_||!model()||(kind!=4&&kind!=1&&kind!=7&&kind!=2))return;
        align_damage_model(frames,car);
        CarContactVector start=point;
        for(unsigned a=0;a!=3;++a)start[a]-=direction[a];
        //Native28 is the EXPL_D record-pointer array;38 is HOR/VERT.
        for(auto n=explosions_.size();n;)frames.object_visible(explosions_[--n].frame,false);
        for(auto n=seats_.size();n;){const auto seat=--n;if(seats_[seat])
            frames.object_visible(seats_[seat],runtime.car_damage_occupant(self,seat)!=nullptr);}
        Frame hit{};
        if(kind==2){
            if(zones_.empty())return;
            float nearest=1000000.f;
            for(auto n=zones_.size();n;){const auto frame=zones_[--n].frame;
                const auto world=frames.actor_world_matrix(frame);
                const auto x=static_cast<long double>(world[12])-point[0],y=static_cast<long double>(world[13])-point[1],z=static_cast<long double>(world[14])-point[2];
                const auto square=(z*z+y*y)+x*x;
                if(!(square>nearest)){nearest=static_cast<float>(square);hit=frame;}
            }
        }else{
            frames.object_visible(model(),true);
            auto extended=start;for(unsigned a=0;a!=3;++a)extended[a]-=direction[a];
            hit=frames.car_damage_pick(extended,direction,model(),0x40000);
            frames.object_visible(model(),false);
        }
        if(!hit)return;
        for(auto n=seats_.size();n;){const auto seat=--n;if(hit==seats_[seat]){
            const auto occupant=runtime.car_damage_occupant(self,seat);
            if(occupant)runtime.car_damage_human(occupant,1,amount,point,direction,source);
            return;
        }}
        Zone* zone=nullptr;for(auto n=zones_.size();n;){auto& candidate=zones_[--n];if(candidate.frame==hit){zone=&candidate;break;}}
        if(!zone){damage_seats(frames,runtime,self,start,direction,amount,source);return;}
        const auto reduced=static_cast<long double>(amount)-runtime.car_damage_absorption(zone->material);
        const float stored=static_cast<float>(reduced);
        if(!(reduced>0))return;
        zone->hit_points=car_damage_integer(static_cast<long double>(zone->hit_points)-stored);
        if(zone->hit_points>=0)return;
        zone->hit_points=0;
        damage_seats(frames,runtime,self,start,direction,stored,source);
        if(kind==1){
            const auto index=find_explosion(frames,start,direction);
            if(index<0)return;
            auto& explosion=explosions_.at(static_cast<std::size_t>(index));
            if(explosion.armed)return;
            if(explosion.delay!=0){
                if(countdown_==0||explosion.delay<countdown_)countdown_=explosion.delay;
                explosion.armed=true; //Publication precedes wire; a wire failure keeps both.
                runtime.car_damage_event(self,CarDamageEvent::explosion(static_cast<std::uint32_t>(index)));return;
            }
        }
        destroy(runtime,self);
    }
    template<class Runtime,class Actor>void destroy(Runtime& runtime,Actor self){ //004E1FE0,64 AFTER004DBF00.
        if(!triggered_){runtime.car_damage_destroy(self,false);triggered_=true;}
    }
    //004E1F70: the EXPL_D particle is retained by identity in native record8.
    //It is independent of record10's authority countdown/armed state.
    template<class Runtime>void present_explosion(Frames& frames,Frame car,Runtime& runtime,std::size_t index){
        auto& explosion=explosions_.at(index);if(explosion.part==-1)return;
        align_damage_model(frames,car);
        const auto matrix=frames.actor_world_matrix(explosions_.at(index).frame);
        const auto identity=runtime.car_emit_particle(explosions_.at(index).part,matrix,car);
        explosions_.at(index).instance=identity; //Reread native slot AFTER the callback.
    }
    template<class Runtime>void present_destruction(Frames& frames,Frame car,Runtime& runtime){ //004E2000.
        for(auto n=explosions_.size();n;){auto& explosion=explosions_[--n];
            retire_explosion(explosion); //Record8=-1 only follows actual stop/no-owner proof.
        }
        align_damage_model(frames,car);
        auto marker=particle_?particle_:car;
        auto matrix=frames.actor_world_matrix(marker);
        fire_instance_=runtime.car_emit_particle(fire_,matrix,car);
        marker=particle_?particle_:car;
        matrix=frames.actor_world_matrix(marker);
        smoke_instance_=runtime.car_emit_particle(smoke(),matrix,car);
        if(sound_>0)runtime.car_emit_dm_sound(sound_,particle_?particle_:car);
    }
    std::int32_t fire_instance()const{return fire_instance_;}
    std::int32_t smoke_instance()const{return smoke_instance_;}
    std::int32_t countdown()const{return countdown_;}
    Frame model() const {return model_?model_->get():Frame{};}
    const std::vector<Frame>& seats() const {return seats_;}
    const std::vector<Frame>& planes() const {return planes_;}
    const std::vector<Zone>& zones() const {return zones_;}
    const std::vector<Explosion>& explosions() const {return explosions_;}
    Frame particle() const {return particle_;}
    std::int32_t sound() const {return sound_;}
    std::int32_t fire() const {return fire_;}
    std::int32_t smoke() const {if(!smoke_)throw std::runtime_error("Car DM smoke definition14 has no native producer");return *smoke_;}
private:
    void retire_particle(std::int32_t identity){
        if(!stop_particle_)throw std::logic_error("Car DM requires its SAME process particle retirement borrower");
        stop_particle_(identity);
    }
    void retire_explosion(Explosion& explosion){
        //004E09D0 allocates0x14 bytes and writes0/4/C/10, but leaves8 unwritten.
        //An empty actual active list makes EVERY possible native value a miss.
        //With populated records, do not invent an ID/ownership for that read.
        if(!explosion.instance){
            if(!active_particles_||active_particles_()!=0)
                throw std::runtime_error("Car EXPL_D particle8 has no native producer while particle records may be active");
        }else if(*explosion.instance>=0)retire_particle(*explosion.instance);
        explosion.instance=-1;
    }
    void clear_metadata(){ //004E0DC0: planes, zones, seats THEN reverse explosion stop/free, markerNULL.
        planes_.clear();zones_.clear();seats_.clear();
        while(!explosions_.empty()){retire_explosion(explosions_.back());explosions_.pop_back();}
        particle_=Frame{};
    }
    template<class Runtime,class Actor>void damage_seats(Frames& frames,Runtime& runtime,Actor self,
            const CarContactVector& start,const CarContactVector& direction,float amount,Actor source){ //004E18C0.
        frames.object_visible(model(),true);
        for(auto n=seats_.size();n;){const auto seat=--n;const auto marker=seats_[seat];if(!marker)continue;
            const auto occupant=runtime.car_damage_occupant(self,seat);if(!occupant)continue;
            frames.object_visible(marker,true);
            if(frames.car_damage_pick(start,direction,marker,0x40000)==marker)
                runtime.car_damage_human(occupant,1,amount,start,direction,source);
        }
        frames.object_visible(model(),false);
    }
    std::int32_t find_explosion(Frames& frames,const CarContactVector& start,const CarContactVector& direction){ //004E1820.
        frames.object_visible(model(),true);
        for(auto n=explosions_.size();n;){const auto index=--n;const auto marker=explosions_[index].frame;
            frames.object_visible(marker,true);
            //The native successful early return deliberately leaves model visible.
            if(frames.car_damage_pick(start,direction,marker,0x40000)==marker)return static_cast<std::int32_t>(index);
        }
        frames.object_visible(model(),false);return -1;
    }
    void parse(Frames& frames,std::size_t seat_count){
        for(std::size_t seat=0;seat<seat_count;++seat) {
            const auto number=std::to_string(seat);
            const auto name=std::string("SEAT")+(number.size()<2?"0":"")+number;
            seats_.push_back(frames.car_find(model_->get(),name.c_str(),1));
        }
        const auto children=frames.car_model_frames(model_->get());
        for(auto frame=children.rbegin();frame!=children.rend();++frame) {
            const auto name=frames.car_short_name(*frame);
            if(name.find("HOR")!=std::string::npos||name.find("VERT")!=std::string::npos) {
                planes_.push_back(*frame);
            } else if(name.find("D_ZONE")!=std::string::npos) {
                const auto user=frames.car_user_data(*frame);
                zones_.push_back({*frame,car_attribute_number(user,"HP"),car_attribute_number(user,"matid")});
            } else if(name.find("EXPL_D")!=std::string::npos) {
                const auto user=frames.car_user_data(*frame);
                const auto bits=static_cast<std::uint32_t>(car_attribute_number(user,"EXPL"))*1000u;
                std::int32_t delay;std::memcpy(&delay,&bits,sizeof(delay));
                explosions_.push_back({*frame,delay,car_attribute_number(user,"PARTID"),false,std::nullopt});
            }
        }
        particle_=frames.car_find(model_->get(),"particle",0x10020);
        const auto user=particle_?frames.car_user_data(particle_):std::string{};
        sound_=car_attribute_number(user,"SID");fire_=car_attribute_number(user,"PFIRE");smoke_=car_attribute_number(user,"PSMOKE");
        if(fire_==-1)fire_=0x60;
        if(smoke_==-1)smoke_=0x5f;
    }
    Frames* frames_=nullptr;
    std::function<void(std::int32_t)> stop_particle_;std::function<std::size_t()> active_particles_;
    std::int32_t fire_instance_=-1,smoke_instance_=-1; //Native1C/20; process owns instances, not this model.
    std::optional<Owner> model_;
    std::vector<Frame> seats_,planes_;
    std::vector<Zone> zones_;
    std::vector<Explosion> explosions_;
    Frame particle_{};
    std::int32_t sound_=-1,fire_=-1;std::optional<std::int32_t> smoke_; //Native14 is unwritten by004E08C0; only004E09D0 produces it.
    std::int32_t countdown_=-1;bool triggered_=false; //004E08C0 C/64, before actor activation's reset.
};

} //namespace hd2::game
#endif
