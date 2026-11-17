#ifndef HD2_GAME_HUMAN_SCRIPT_NAVIGATION_OUTPUT_H
#define HD2_GAME_HUMAN_SCRIPT_NAVIGATION_OUTPUT_H
#include "game/human_joint_owners.h"
namespace hd2::game {
template<class State,class StopAim>void stop_human_script_commands(State& state,StopAim&& stop_aim){ //0058F640.
    auto& words=state.state();if(words.template get<std::uint8_t>(0x198))words.set(0x19c,0x80u);
    words.set(0x1c0,0u);
    if(state.words().template get<std::uint32_t>(0x50)&0x200000u)stop_aim();
    if(auto* query=state.resources().query_if_present())query->state.set(0x50,query->state.template get<std::uint32_t>(0x50)|0x2000u);
}
template<class State,class Inline>void request_human_script_stand(State& state,bool fast,Inline& control){ //0058F390/0058F420.
    auto& words=state.state();
    if(words.template get<std::uint8_t>(0x1d1)){
        words.set(0x1d1,std::uint8_t{0});control.action_command(fast?0x26:0x1d);return;
    }
    words.set(0x1b0,(words.template get<std::uint32_t>(0x1b0)&0xfffffffcu)|4u);
}
template<class State,class Inline,class World>void request_human_script_navigation(
        State& state,std::uint16_t point,std::int32_t pc,Inline& control,World& world){ //0058E780.
    auto& words=state.state();
    if(words.template get<std::uint8_t>(0x198))words.set(0x19c,0x80u); //00591ED0 BEFORE current-point read.
    auto* navigation=state.navigation();if(!navigation)throw std::logic_error("Script Move requires its actual navigation owner");
    const auto current=state.point()?navigation->index_of(*state.point()):std::uint16_t{0xffff};
    auto& query=state.resources().query();
    if(point!=current){
        if(point!=0xffff){query.state.set(0x54,(words.template get<std::uint32_t>(0x1b0)&0x1f000u)|8u);query.request_point(point);}
        words.set(0x1d8,pc);
        if(!(words.template get<std::uint32_t>(0x1b0)&0x1000u))world.clear_script_formation_item_ready(state,control);
        return;
    }
    const auto flags=query.state.template get<std::uint32_t>(0x50);
    if(flags&0x10000000u)query.state.set(0x50,flags|0x2000u);
    state.finish_script_attack_context(pc); //SAME optional1AC ->00416D50 PC-match callback.
    words.set(0x1d8,std::int32_t{-1});
}
} //namespace hd2::game
#endif
