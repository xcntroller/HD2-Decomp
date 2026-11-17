#ifndef HD2_GAME_HUMAN_CAMERA_STATE_H
#define HD2_GAME_HUMAN_CAMERA_STATE_H

#include "game/human_loading_stats.h"
#include "game/human_joint_math.h"

namespace hd2::game {
//00465AD0, camera virtual70. The second current-player query is native and
//may differ after a callback; an attached current Human preserves the old280.
template<class Control>void apply_human_camera_distance_hold(std::uint32_t& timer,std::uint32_t duration,std::uint32_t mode,Control& players){
    if(mode&2u){auto* current=players.current_player();if(current){
        current=players.current_player();if(!current)throw std::runtime_error("Camera distance hold lost native second current Human");
        if(current->field160)return;
    }}
    timer=duration;
}
template<class Human>bool human_camera_is_player(const Human& human,std::uint32_t mode,std::int32_t player_id) { //005A0890.
    if(!human.loading_state)return false;
    if((mode&7u)==7u)return human.loading_state->words().template get<std::uint32_t>(0x48)==1;
    return human.base().id10()==player_id;
}
template<class Human>std::uint32_t human_camera_action(const Human& human,std::uint32_t mode,std::int32_t player_id) { //005A8170.
    return human_camera_is_player(human,mode,player_id)?human.loading_state->derived_loading_word(0x52c):0;
}
template<class Human>bool human_camera_focus(const Human& human) { //005A6430's low byte consumed by00466970.
    if(!human.loading_state)throw std::runtime_error("Camera focus requires actual Human B0");
    return human.loading_state->words().template get<std::uint32_t>(0x48)==1&&
        (human.loading_state->derived_loading_word(0x4d4)&0xffu)!=0;
}
template<class Human>HumanVector human_camera_target_direction(const Human& human) { //005A80E0, B0 virtual4C.
    if(!human.loading_state)throw std::runtime_error("Camera direction requires actual Human B0");
    return human.loading_state->target_direction(); //Player00811848+4C=00584B50;AI tables=0056FC00.
}
} //namespace hd2::game
#endif
