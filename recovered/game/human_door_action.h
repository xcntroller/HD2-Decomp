#ifndef HD2_GAME_HUMAN_DOOR_ACTION_H
#define HD2_GAME_HUMAN_DOOR_ACTION_H

#include "game/human_animation_tracks.h"

namespace hd2::game {

template<class Look,class Services,class Frames>
std::int32_t human_primary_animation_duration(const Look& look,Services& services,Frames& frames,
        std::int32_t id,std::int32_t posture,std::int32_t variant){ //0075CBA0, raw signed engine duration.
    if(id>=206)return -1;
    const auto property=look.state().template get<std::int32_t>(0xa0);
    if(property!=1&&property!=2)throw std::runtime_error("Human animation duration requires its actual animation table");
    const auto index=id<190?static_cast<std::int64_t>(id)*3+posture:
        498+(static_cast<std::int64_t>(id)-190)*18+static_cast<std::int64_t>(posture)*6+variant;
    if(index<0||index>=786)throw std::runtime_error("Human animation duration exceeds its actual table");
    const auto clip=services.human_animation_row(static_cast<unsigned>(property-1),static_cast<unsigned>(index)).animation.get();
    return clip?frames.human_clip_duration(clip):-1;
}

template<class Control,class Look,class Human,class Services,class Frames,class Actor>
bool begin_human_door_action(Control& control,Look& look,Human& human,Services& services,Frames& frames,
        Actor* target,std::uint32_t flags,std::uint32_t argument){ //00751480 case1.
    auto& physical=control.physical();auto& words=physical.control_frame_words();auto& request=look.frame_words();
    words.set(0xa44,0u);request.set(0xd4,request.template get<std::uint32_t>(0xd4)|0x4000u);
    words.set(0xa40,argument);words.set(0xa38,std::int32_t{-10000});words.set(0xa34,flags);
    physical.control_action_frame(typename Control::Frame{});
    physical.control_interaction_target(target);words.set(0xa38,0u);
    auto& door=services.human_navigation_door(target);
    const auto side=door.navigation_side(human.base().world_position());
    if(side==2||words.template get<std::int32_t>(0x988)!=-10000){
        if(control.mode_flags()&1u)services.human_action_request_door(target,argument?2u:3u);
        return true;
    }
    if(side==0||side==1){
        const auto animation=static_cast<std::int32_t>((argument?0u:2u)+side);
        words.set(0x1fc,animation);normalize_human_primary_animation(words,0x1fc);
        words.set(0xa38,human_primary_animation_duration(look,services,frames,animation,0,1)); //751593/7515CE push variant1 and posture0.
    }
    return true;
}

template<class Control,class Look,class Runtime>
void advance_human_control_actions(Control& control,Look& look,std::uint32_t elapsed,Runtime& runtime){ //00755500, inactive and case1.
    auto& words=control.physical().control_frame_words();
    const auto flags=words.template get<std::uint32_t>(0xa34);
    const auto timer=words.template get<std::int32_t>(0xa38);
    if(!flags||timer==-10000)return;
    const auto action=flags&0x3ffu;
    if(action==2||action==3||action==4||action==5||action==6||action==7||action==11||action==12||action==13){control.advance_seat_action(look,elapsed,runtime);return;}
    if((flags&0x3ffu)>=8u&&(flags&0x3ffu)<=10u){control.advance_model_action(look,elapsed);return;}
    if((flags&0x3ffu)!=1u){runtime.advance_human_control_motion(control,elapsed);return;} //Other required positive action cases.
    if(timer>0)words.set(0xa38,static_cast<std::uint32_t>(timer)-elapsed);
    else{
        words.set(0xa38,std::int32_t{-10000});words.set(0xa34,0u);
        auto& request=look.frame_words();request.set(0xd4,request.template get<std::uint32_t>(0xd4)&~0x4000u);
    }
}

} //namespace hd2::game
#endif
