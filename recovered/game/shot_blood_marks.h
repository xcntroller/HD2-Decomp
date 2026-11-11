#ifndef HD2_GAME_SHOT_BLOOD_MARKS_H
#define HD2_GAME_SHOT_BLOOD_MARKS_H
#include "io/resource_observations.h"
#include "game/human_ai_shot_direction.h"
#include "game/human_state_frame_timers.h"
#include <functional>
#include <optional>
#include <memory>

namespace hd2::game {
//Actual ShotMgr90 record00454C60/00454E20/00454FF0/00455240/00455350.
//World services are borrowed for the record's module14 lifetime. Native module
//finish retires these records before the containing query/light owners unbind.
template<class Frames>class ShotBloodMark {
public:
    using Frame=typename Frames::Frame;using Vector=HumanVector;
    using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    ShotBloodMark()=default;ShotBloodMark(const ShotBloodMark&)=delete;ShotBloodMark& operator=(const ShotBloodMark&)=delete;
    template<class Services>void create(Frames& frames,Services& services,Frame texture,const Vector& position,const Vector& direction){
        bind(services);reset_motion(frames,position,direction);
        if(!texture)return;
        frame_.emplace(frames.create_shot_surface());require_frame();
        frames.shot_surface_texture(frame(),texture);frames.object_visible(frame(),false);frames.update_scene_object(frame());
    }
    template<class Services>void reuse(Frames& frames,Services& services,Frame texture,const Vector& position,const Vector& direction){
        bind(services);
        if(!frame()){
            if(!texture){placed_=true;return;}
            frame_.emplace(frames.create_shot_surface());require_frame();
        }
        frames.shot_surface_texture(frame(),texture);frames.object_visible(frame(),false);frames.update_scene_object(frame());
        reset_motion(frames,position,direction);
    }
    unsigned tick(Frames& frames,std::uint32_t elapsed){
        if(placed_)return 0;
        if(!frame())return 2;
        time_+=elapsed;if(human_frame_signed_word(time_)>1000)return 2; //Containing90 pass ignores this return.
        if(!advance_)throw std::logic_error("Shot blood mark requires its actual World query/light borrower");
        return advance_(frames,elapsed);
    }
    void clear(){frame_.reset();advance_={};}
    Frame frame()const{return frame_?frame_->get():Frame{};}
    const Vector& position()const{return position_;}const Vector& velocity()const{return velocity_;}
    bool placed()const{return placed_;}std::uint32_t time()const{return time_;}
private:
    void require_frame(){if(!frame())throw std::runtime_error("Shot blood mark requires the actual projection visual");}
    void reset_motion(Frames& frames,const Vector& position,const Vector& direction){
        position_=position;velocity_=direction;
        const auto square=(static_cast<long double>(velocity_[2])*velocity_[2]+static_cast<long double>(velocity_[1])*velocity_[1])+static_cast<long double>(velocity_[0])*velocity_[0];
        normalize_human_ai_shot(velocity_,square);
        const auto draw=static_cast<long double>(frames.volume_random_unit())-.5f;
        const auto speed=draw+draw+3.f;
        for(unsigned axis=0;axis!=3;++axis)velocity_[axis]=static_cast<float>(speed*velocity_[axis]);
        placed_=false;time_=0;
    }
    template<class Services>void bind(Services& services){
        advance_=[this,&services](Frames& frames,std::uint32_t elapsed){return advance(frames,services,elapsed);};
    }
    template<class Services>unsigned advance(Frames& frames,Services& services,std::uint32_t elapsed){
        Vector delta{};const auto time=static_cast<float>(human_frame_signed_word(elapsed));
        for(unsigned axis=0;axis!=3;++axis)delta[axis]=static_cast<float>((static_cast<long double>(time)*velocity_[axis])*.0010000000474974513f);
        const auto hit=services.human_geometry_ray(position_,delta,0xffffffffu,0,true);
        if(hit){publish(frames,services,hit->position,hit->normal);return 0u;}
        for(unsigned axis=0;axis!=3;++axis)position_[axis]+=delta[axis];
        if(human_frame_signed_word(time_)>400)velocity_[1]=static_cast<float>(static_cast<long double>(velocity_[1])-
            static_cast<long double>(time)*.0010000000474974513f*9.800000190734863f);
        return 0u;
    }
    template<class Services>void publish(Frames& frames,Services& services,const Vector& point,const Vector& normal){
        auto position=point;for(unsigned axis=0;axis!=3;++axis){
            auto product=static_cast<long double>(normal[axis])*.019999999552965164f;
            if(axis==2)product=static_cast<float>(product);
            position[axis]=static_cast<float>(product+point[axis]);
        }
        frames.shot_surface_mode(frame(),0);frames.object_position(frame(),{{position[0],position[1],position[2]}});
        const auto size=static_cast<float>((static_cast<long double>(frames.volume_random_unit())-.5f)*.10000000149011612f+.20000000298023224f);
        frames.shot_blood_scale(frame(),size,normal[3]);
        auto facing=normal;for(unsigned axis=0;axis!=3;++axis)facing[axis]=-facing[axis];
        const auto roll=static_cast<float>((static_cast<long double>(frames.volume_random_unit())-.5f)*6.2831854820251465f);
        frames.car_steering_wheel_direction(frame(),facing,roll);frames.shot_surface_enabled(frame(),true);
        frames.object_visible(frame(),true);frames.update_scene_object(frame());frames.shot_surface_link(frame(),Frame{});
        const auto sector=frames.human_scene_sector(position);frames.shot_surface_link(frame(),sector);
        const auto light=services.human_scene_light(position,sector,true);
        frames.shot_surface_colour(frame(),light<.2f?.2f:static_cast<float>(light));
        frames.shot_surface_mode(frame(),3);frames.update_scene_object(frame());placed_=true;
    }
    Vector position_{},velocity_{};std::optional<Owner> frame_;bool placed_=false;std::uint32_t time_=0;
    std::function<unsigned(Frames&,std::uint32_t)> advance_;
};

template<class State,class Frames,class Services>void emit_shot_blood_mark_texture(State& state,Frames& frames,Services& services,
        const HumanVector& position,const HumanVector& direction,std::size_t texture_index){ //0044F520.
    if(!services.human_surface_quality_enabled()||!state.blood_textures.at(texture_index)||!state.blood_textures[texture_index]->get())return;
    if(!state.world)throw std::logic_error("Shot blood mark requires the actual world-bound ShotMgr");
    const auto texture=state.blood_textures[texture_index]->get();std::unique_ptr<typename State::BloodMark> mark;
    if(state.blood_marks.size()<state.maximum14c){
        mark=std::make_unique<typename State::BloodMark>();mark->create(frames,services,texture,position,direction);
    }else{
        if(state.blood_marks.empty())throw std::runtime_error("Shot blood reuse requires the actual oldest record");
        mark=std::move(state.blood_marks.front());state.blood_marks.erase(state.blood_marks.begin());mark->reuse(frames,services,texture,position,direction);
    }
    state.blood_marks.push_back(std::move(mark));
    io::observe_bound_data(frames,"execution","ShotMgr blood_marks allocated","actual owner size",state.blood_marks.size());
}
template<class State,class Frames,class Services>void emit_shot_blood_mark(State& state,Frames& frames,Services& services,
        const HumanVector& position,const HumanVector& direction){ //0044F4D0, texture4C is x_blood_00.
    emit_shot_blood_mark_texture(state,frames,services,position,direction,0);
}
} //namespace hd2::game
#endif
