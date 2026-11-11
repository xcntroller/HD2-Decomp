#ifndef HD2_GAME_HUMAN_OBSTRUCTION_MOTION_H
#define HD2_GAME_HUMAN_OBSTRUCTION_MOTION_H

#include "game/human_state_frame_timers.h"

namespace hd2::game {

template<class Control,class Look,class Human,class Services,class Runtime,class Random>
void advance_human_obstruction(Control& control,Look& look,Human& human,Services& services,
        Runtime&,Random& random,std::uint32_t elapsed,bool obstructed){ //0075EBE0.
    auto& words=control.physical().control_frame_words();
    const auto timer=[&](unsigned offset){return words.template get<std::int32_t>(offset);};
    if(!obstructed){
        if(timer(0x1a4)>7499){control.play_presentation_track_sound(19);look.action_command(76,true);}
        words.set(0x1a4,0u);words.set(0x1a8,0u);
        return;
    }
    if(control.frame_special_movement_item()&&services.human_integrity_matches(39)){
        if(timer(0x1f4)==66)look.action_command(76,true);
        return;
    }
    const auto accumulated=static_cast<std::uint32_t>(timer(0x1a4))+elapsed;
    words.set(0x1a4,accumulated);
    words.set(0x1a8,static_cast<std::uint32_t>(timer(0x1a8))-elapsed);
    if(human_frame_signed_word(accumulated)>30000){
        if(control.mode_flags()&1u){
            const auto excess=human_frame_signed_word(accumulated)-30000;
            const auto amount=static_cast<float>(static_cast<long double>(excess)*.07999999821186066f);
            services.apply_human_damage(human,10,amount,HumanVector{},HumanVector{},static_cast<Human*>(nullptr),0); //Actual Human virtualD4.
        }
        if(control.human_is_player()&&(timer(0x1a8)<1||human_frame_signed_word(human.properties.stats_ec[0])<1)){
            words.set(0x1a8,static_cast<std::uint32_t>(random()%1900u)+2000u);
            control.play_presentation_track_sound(18);
        }
        words.set(0x1a4,30000u);
        if(timer(0x1f4)!=66)look.action_command(66,true);
    }
}

} //namespace hd2::game
#endif
