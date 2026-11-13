#ifndef HD2_GAME_SHOT_SHELL_RECORD_H
#define HD2_GAME_SHOT_SHELL_RECORD_H
#include "game/human_joint_owners.h"
#include <functional>
#include <memory>
#include <optional>

namespace hd2::game {
template<class Frames>class SceneActor;
//0044DB10/00454190/004546C0, actual reusable40 record from ShotMgr pool64.
//Retirement hides/updates its visual; the pool retains that clone until final
//destruction. It does not release/reclone on every shell ejection.
template<class Frames>class alignas(16) ShotShellRecord {
public:
    using Frame=typename Frames::Frame;using Owner=decltype(std::declval<Frames&>().create_shot_model());
    ShotShellRecord(){for(auto offset:{0x20u,0x24u,0x28u})words.set(offset,0u);}
    ShotShellRecord(const ShotShellRecord&)=delete;ShotShellRecord& operator=(const ShotShellRecord&)=delete;
    Frame frame()const{return visual_?visual_->get():Frame{};}
    template<class World,class Human>void initialize(Frames& frames,World& world,Human& human,Frame source_model,
            const HumanVector& position,const HumanVector& direction,const HumanVector& velocity,std::uint32_t command){
        frames_=&frames;words.set(0,position);words.set(0x10,velocity);source=&human;words.set(0x28,0u);words.set(0x30,command);
        advance_=[this,&world](std::uint32_t elapsed){return world.advance_shot_shell(*this,elapsed);};
        if(!source_model)return; //Native2C remains unwritten; no fabricated motion state.
        if(!frame()){
            visual_.emplace(frames.create_shot_shell(source_model));
            if(!frame())throw std::runtime_error("Shell requires its actual object clone");
        }
        frames.human_sound_link(frame(),Frame{});frames.shot_shell_pose(frame(),position,direction);
        const bool first=world.human_is_player(human)&&world.human_view_offset_enabled();
        frames.place_shot_shell(frame(),first);frames.object_visible(frame(),true);frames.update_scene_object(frame());
        words.set(0x2c,0u);initialized_=true;
    }
    unsigned tick(std::uint32_t elapsed){
        if(!initialized_)throw std::runtime_error("Shell requires its actual initialized motion state");
        if(words.template get<std::uint32_t>(0x2c)==3u)return 2;
        if(!advance_)throw std::runtime_error("Shell requires actual00454360 collision/motion receiver");
        return advance_(elapsed);
    }
    void retire(){
        if(frame()){
            if(!frames_)throw std::logic_error("Shell visual outlived its Frames borrower");
            frames_->object_visible(frame(),false);frames_->update_scene_object(frame());
        }
        source=nullptr;advance_={};initialized_=false;
    }
    HumanJointWords<0x40> words;SceneActor<Frames>* source=nullptr;
private:
    Frames* frames_=nullptr;std::optional<Owner> visual_;std::function<unsigned(std::uint32_t)> advance_;bool initialized_=false;
};
} //namespace hd2::game
#endif
