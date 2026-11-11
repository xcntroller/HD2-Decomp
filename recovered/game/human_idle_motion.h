#ifndef HD2_GAME_HUMAN_IDLE_MOTION_H
#define HD2_GAME_HUMAN_IDLE_MOTION_H

#include "game/human_motion_state.h"
#include "game/human_animation_tracks.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {

template<class Control,class Look,class Human,class Services,class Frames,class Runtime>
void advance_human_idle_motion(HumanMotionState& motion,Control& control,Look& look,Human& human,
        Services& services,Frames& frames,Runtime&,std::uint32_t elapsed,bool winter){ //007507A0, actual CB0.
    auto& words=control.physical().control_frame_words();
    const auto& movement=control.movement().state();
    const auto& request=look.state();
    const auto active=words.template get<std::int32_t>(0x1d8);
    const auto queued=words.template get<std::int32_t>(0x1fc);
    const bool idle=active>=41&&active<=65&&human.properties.field19c==1&&
        ((queued>=41&&queued<=65)||queued==206);
    if(idle){
        if(!motion.field10)motion.field14=static_cast<std::int32_t>(services.human_frame_random_word()%25000u)+15500;
        motion.field10=human_frame_signed_word(static_cast<std::uint32_t>(motion.field10)+elapsed);
        if(motion.field10>100000000)motion.field10=1;
        motion.field14=human_frame_signed_word(static_cast<std::uint32_t>(motion.field14)-elapsed);
        if(motion.field14<1&&(!request.template get<std::uint32_t>(0x5c)||!request.template get<std::uint32_t>(0x54))){
            motion.field14=static_cast<std::int32_t>(services.human_frame_random_word()%10000u)+15500;
            const auto posture=words.template get<std::int32_t>(0x1dc);
            if(posture>=0&&posture<=2){
                const auto animation=posture==0?47u+services.human_frame_random_word()%7u:
                    posture==1?57u+services.human_frame_random_word()%5u:65u;
                words.set(0x1fc,animation);
                normalize_human_primary_animation(words,0x1fc);
            }
        }
    }else{
        motion.field14=-10000;
        motion.field10=0;
        if(request.template get<std::int32_t>(0xb8)==5&&(active==91||active==93)&&(queued==91||queued==93)){
            auto phase=words.template get<float>(0x234);
            const bool fast=movement.template get<float>(0x10c)>=.2f;
            if(!fast&&phase==0)phase=3.1415927410125732421875f;
            phase=static_cast<float>(phase+static_cast<long double>(human_frame_signed_word(elapsed))*
                (fast?.0008333333535119891f:.0012499999720603228f));
            if(phase>6.2831854820251465f)phase-=6.2831854820251465f; //Native subtracts once.
            words.set(0x234,phase);
        }else words.set(0x234,0.f);
    }
    if(!winter||movement.template get<float>(0x114)!=-10000.099609375f||
            !(words.template get<std::uint32_t>(0x22c)&0x20u)||services.human_hidden(human))return;
    if(services.human_view_offset_enabled()&&services.current_human_frame_player()==&human)return;
    motion.field18=human_frame_signed_word(static_cast<std::uint32_t>(motion.field18)-elapsed);
    if(motion.field18>0)return;
    const bool moving=words.template get<std::int32_t>(0x1d8)>=190;
    if(moving){
        const auto mode=request.template get<std::int32_t>(0x50);
        if(mode==1||mode==2||mode==5){
            motion.field18=static_cast<std::int32_t>(services.human_frame_random_word()%250u)+(mode==5?1300:1600);
            return;
        }
        motion.field18=static_cast<std::int32_t>(services.human_frame_random_word()%750u)+(mode==0?3500:3200);
    }else motion.field18=static_cast<std::int32_t>(services.human_frame_random_word()%1500u)+3500;
    if(frames.object_parent(human.base().frame())!=frames.camera_root_sector())return;
    const auto bone=control.physical().bones().at(4);
    if(!bone)throw std::runtime_error("Human breath requires its actual retained head bone");
    const auto position_matrix=frames.actor_world_matrix(bone);
    HumanVector position{{position_matrix[12],position_matrix[13],position_matrix[14],position_matrix[15]}};
    const auto direction_matrix=frames.actor_world_matrix(bone); //Native performs the second actual world-matrix query.
    const auto direction=actor_activation_direction({{direction_matrix[8],direction_matrix[9],direction_matrix[10],direction_matrix[11]}});
    position[0]=static_cast<float>(position[0]+static_cast<long double>(direction[0])*.15f);
    position[1]=static_cast<float>(position[1]+static_cast<long double>(direction[1])*.15f);
    const float z_offset=static_cast<float>(static_cast<long double>(direction[2])*.15f); //750CA2/750EE7 spill Z before addition.
    position[2]+=z_offset;
    services.emit_human_particle(moving?76u:17u,position,direction,moving?bone:typename Frames::Frame{},true,true); //SAME module17/process00445150→004451B0.
}

} //namespace hd2::game
#endif
