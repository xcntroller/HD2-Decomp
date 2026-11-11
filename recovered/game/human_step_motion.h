#ifndef HD2_GAME_HUMAN_STEP_MOTION_H
#define HD2_GAME_HUMAN_STEP_MOTION_H

#include "game/human_animation_requests.h"
#include "game/human_joint_math.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {

template<class Control,class Human,class Services,class Frames>
void advance_human_step_motion(Control& control,Human& human,Services& services,Frames& frames,
        std::int32_t elapsed){ //00767D40, AC0 with SAME linked C00 and DD0.
    auto& words=control.physical().control_frame_words();
    auto& movement=control.movement().frame_words();
    auto& action=control.action().frame_words();
    action.set(0x5c,action.template get<std::uint32_t>(0x5c)+static_cast<std::uint32_t>(elapsed));
    words.set(0x228,0u);
    movement.set(0xe8,3u);
    const float duration=static_cast<float>(elapsed);
    const float time=static_cast<float>(static_cast<long double>(elapsed)+words.template get<float>(0x968));
    words.set(0x968,time);
    const auto start=words.template get<float>(0x960),end=words.template get<float>(0x964);
    const float offset=static_cast<float>(static_cast<long double>(time)-start);
    float rise=0;
    if(static_cast<long double>(duration)+offset>=0&&time<end){
        long double portion=static_cast<long double>(duration)+offset;
        if(static_cast<long double>(duration)+time<=end){
            if(offset>0)portion=duration;
        }else portion=static_cast<long double>(end)-(offset>=0?time:start);
        rise=static_cast<float>((static_cast<long double>(words.template get<float>(0x958))-
            words.template get<float>(0x95c))*portion/(static_cast<long double>(end)-start));
    }
    const auto marker=control.action().marker(); //Native C00+80, borrowed HD from00777A80.
    if(!marker)return;
    const auto trigger=action.template get<std::int32_t>(0x60);
    if(trigger!=-10000&&trigger<human_frame_signed_word(action.template get<std::uint32_t>(0x5c))){
        control.tracks().motion_impulse(action.template get<HumanVector>(0x90),
            human_animation_time_integer(1000.L/services.human_control_animation_scale()));
        action.set(0x60,std::int32_t{-10000});
    }
    const auto local=frames.actor_position(marker);
    auto displacement=movement.template get<HumanVector>(0x10); //Native temporary W is unwritten, retain its destination padding.
    for(unsigned axis=0;axis!=3;++axis)displacement[axis]=local[axis]-action.template get<float>(0x70+4*axis);
    const auto parent=frames.object_parent(marker);
    if(!parent)throw std::runtime_error("Human step motion requires the actual HD parent");
    auto matrix=frames.actor_world_matrix(parent);
    matrix[12]=matrix[13]=matrix[14]=0;
    frames.car_transform_in_place(displacement,matrix);
    action.set(0x70,frames.actor_position(marker)); //Native reads marker local position again after its parent transform.
    if((control.mode_flags()&7u)!=7u&&!control.human_is_player()&&!control.animation_authority_actor()&&
            words.template get<std::int32_t>(0x1ac)==1)return;
    for(unsigned axis=0;axis!=3;++axis)
        displacement[axis]=static_cast<float>(static_cast<long double>(displacement[axis])+words.template get<float>(0x4d0+axis*4));
    displacement[1]+=rise;
    //The LS3DF vector operator and native stack copies leave W unspecified.
    displacement[3]=movement.template get<float>(0x1c);
    movement.set(0x10,displacement);
    auto position=human.base().world_position();
    for(unsigned axis=0;axis!=3;++axis)position[axis]+=displacement[axis];
    human.base().world_position(position);
    movement.set(0x51,std::uint8_t{1});
}

} //namespace hd2::game
#endif
