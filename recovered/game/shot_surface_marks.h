#ifndef HD2_GAME_SHOT_SURFACE_MARKS_H
#define HD2_GAME_SHOT_SURFACE_MARKS_H

#include "game/human_joint_math.h"
#include <optional>
#include <memory>

namespace hd2::game {

//ShotMgr A0/A4/A8 payload004546E0/00454940, not a particle instance.
//The projection visual links into the actual Scene and renders through LS3DF.
template<class Frames>class ShotSurfaceMark {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    ShotSurfaceMark()=default;
    ShotSurfaceMark(const ShotSurfaceMark&)=delete;
    ShotSurfaceMark& operator=(const ShotSurfaceMark&)=delete;
    template<class Services>void create(Frames& frames,Services& services,Frame texture,
            const HumanVector& position,const HumanVector& normal,float limit){ //004546E0.
        if(!texture)return; //Original null-texture constructor stores only a null frame.
        frame_.emplace(frames.create_shot_surface());
        if(!frame_->get())throw std::runtime_error("Shot surface requires its actual driver projection visual");
        frames.shot_surface_texture(frame_->get(),texture);
        configure(frames,services,position,normal,limit,true);
    }
    template<class Services>void reuse(Frames& frames,Services& services,
            const HumanVector& position,const HumanVector& normal,float limit){ //00454940.
        if(!frame_||!frame_->get())throw std::runtime_error("Shot surface reuse requires the actual oldest projection visual");
        configure(frames,services,position,normal,limit,false);
    }
    unsigned tick(Frames& frames,std::uint32_t elapsed){ //00454B50, return2 is ignored by00451FB0's A0 pass.
        if(time_==0xffffffffu)return 0;
        if(!frame_||!frame_->get())return 2;
        time_+=elapsed;
        std::int32_t signed_time;std::memcpy(&signed_time,&time_,4);
        if(signed_time>100){
            size_+=.0010000000474974513f;
            frames.shot_surface_scale(frame_->get(),size_,size_,.1f);
            frames.shot_surface_mode(frame_->get(),3);frames.update_scene_object(frame_->get());
            time_-=100u; //ONE step, strict signed>100, no catch-up.
        }
        if(size_>=limit_){frames.update_scene_object(frame_->get());time_=0xffffffffu;} //Ordered: NaN stays live.
        return 0;
    }
    void clear(){frame_.reset();} //00454C40 releases the engine reference, then nulls it.
    Frame frame()const{return frame_?frame_->get():Frame{};}
    float size()const{return size_;}float limit()const{return limit_;}std::uint32_t time()const{return time_;}
private:
    template<class Services>void configure(Frames& frames,Services& services,const HumanVector& position,
            const HumanVector& normal,float limit,bool created){
        auto offset=position;
        for(unsigned axis=0;axis!=3;++axis){
            auto product=static_cast<long double>(normal[axis])*.05000000074505806f;
            if(axis==2)product=static_cast<float>(product);
            offset[axis]=static_cast<float>(product+position[axis]);
        }
        const auto frame=frame_->get();frames.object_position(frame,{offset[0],offset[1],offset[2]});
        frames.shot_surface_scale(frame,.1f,.1f,.1f);
        HumanVector facing=normal;for(unsigned axis=0;axis!=3;++axis)facing[axis]=-normal[axis];
        const float roll=static_cast<float>((static_cast<long double>(frames.volume_random_unit())-.5f)*6.2831854820251465f);
        frames.shot_surface_direction(frame,facing,roll);
        if(created){frames.shot_surface_enabled(frame,true);frames.shot_surface_link(frame,Frame{});}
        const auto sector=frames.human_scene_sector(offset);frames.shot_surface_link(frame,sector);
        const auto light=services.human_scene_light(offset,sector,true);
        const float stored=static_cast<float>(light);
        frames.shot_surface_colour(frame,light<.2f?.2f:stored);
        frames.shot_surface_mode(frame,3);frames.update_scene_object(frame);
        if(created){time_=0;size_=.009999999776482582f;}
        const auto draw=static_cast<long double>(frames.volume_random_unit())-.5f;
        if(!created){time_=0;size_=.009999999776482582f;}
        limit_=static_cast<float>((draw+draw)*.1+limit); //0080FB10 is DOUBLE0.1, independent engine random draw.
    }
    std::optional<Owner> frame_;
    float size_=0,limit_=0;std::uint32_t time_=0; //Defined source guards for the native null-texture branch's unwritten words.
};

//0044F2B0, over the SAME process ShotMgr owner and current selected profile.
template<class State,class Frames,class Services>void emit_shot_surface_mark(State& state,Frames& frames,Services& services,
        const HumanVector& position,const HumanVector& normal,float limit){
    if(!services.human_surface_quality_enabled())return;
    if(!state.world)throw std::logic_error("Shot surface requires its actual world-bound ShotMgr");
    using Record=typename State::SurfaceMark;
    std::unique_ptr<Record> mark;
    if(state.surfaces.size()<state.maximum150){
        mark=std::make_unique<Record>();
        mark->create(frames,services,state.blood_textures[5]?state.blood_textures[5]->get():typename Frames::Frame{},position,normal,limit);
    }else{
        if(state.surfaces.empty())throw std::runtime_error("Shot surface reuse requires a nonempty native pool");
        mark=std::move(state.surfaces.front());state.surfaces.erase(state.surfaces.begin());
        mark->reuse(frames,services,position,normal,limit);
    }
    state.surfaces.push_back(std::move(mark));
}
} //namespace hd2::game
#endif
