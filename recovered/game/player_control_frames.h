#ifndef HD2_GAME_PLAYER_CONTROL_FRAMES_H
#define HD2_GAME_PLAYER_CONTROL_FRAMES_H
#include "game/session_frame_input.h"
namespace hd2::game {
//004DD090. Read the live identity's flags, not a wheel's cached query_flags.
template<class Wheels,class Read>std::uint32_t car_control_surface_flags(const Wheels& wheels,Read&& read){
    std::uint32_t flags=0;
    for(auto n=wheels.size();n;){const auto& wheel=wheels[--n];
        if((wheel.flags&8u)&&wheel.query_identity)flags|=read(wheel)&0x6000u;
    }
    return flags;
}
//0045EF00: SAME roster and cached control34, after fixed selection tick
//and the actual004622C0 scan. Surface flags come from Human/DD0 or occupied Car.
template<class Control,class Mission,class Queries>
void update_player_control_frame_flags(Control& control,const Mission* mission,std::uint32_t& previous,Queries& queries){
    if((queries.mode_flags()&7u)!=7u||!mission_frame_input_allowed(mission))return;
    std::uint32_t flags=0;
    for(auto n=control.human_roster().size();n;){
        auto* human=control.human_roster().at(static_cast<std::int32_t>(--n));
        if(!human)continue;
        std::int32_t health;const auto bits=human->properties.stats_ec[0];std::memcpy(&health,&bits,4);
        if(health>0)flags|=queries.player_control_surface_flags(*human);
    }
    if(flags==previous)return;
    previous=flags; //Native publishes34 BEFORE result/hint callbacks, even on failure.
    if(flags&0x2000u)queries.fail_player_control_objective(true);
    else queries.player_control_hint(flags&0x4000u?0x1cefu:0xffffffffu);
}
} //namespace hd2::game
#endif
