#ifndef HD2_GAME_HUMAN_SCRIPT_FORMATION_OUTPUT_H
#define HD2_GAME_HUMAN_SCRIPT_FORMATION_OUTPUT_H
#include "game/human_joint_owners.h"
namespace hd2::game {
template<class State,class Formation,class Finish>
void bind_human_script_formation(State& state,Formation*& borrower,Formation* formation,Finish&& finish){ //0058D900.
    auto& words=state.state();
    if(words.template get<std::uint8_t>(0x198))words.set(0x19c,0x80u); //00591ED0.
    borrower=formation;if(!formation)return;
    auto& query=state.resources().query().state;
    if(query.template get<std::uint32_t>(0x50)&0x10000000u){
        query.set(0x50,query.template get<std::uint32_t>(0x50)|0x2000u);
        finish(words.template get<std::int32_t>(0x1d8)); //Actual1AC context gate supplied by state.
    }
    words.set(0x1d8,std::int32_t{-1});state.resources().flag().request(1);
}
//0058D970. Native1E0 is the actual Players subgroup formation borrower; it is
//distinct from Human2C8 and must be retired by its actual producer before owner
//destruction.00598320 consumes changed88 BEFORE the model cache/query callbacks.
template<class State,class Inline,class World,class Frames>
bool advance_human_script_formation(State& state,Inline& control,World& world,Frames& frames){
    auto* formation=state.script_formation();if(!formation)return false;
    auto* human=state.human();if(!human)throw std::logic_error("Script formation has no actual bound Human");
    if(human==formation->leader())return false;
    HumanVector position;std::uint16_t point;
    const bool changed=formation->consume_member_position(human,position,&point);
    (void)frames.actor_world_matrix(human->base().frame()); //Even when no changed output is consumed.
    if(!changed)return true;
    auto& query=state.resources().query();
    auto stance=query.state.template get<std::uint32_t>(0x54)&0x1f000u;
    if(!control.frame_navigation_floor())query.request_point_position(point,position);
    else query.request_position(position);
    const auto origin=human->base().world_position();
    const auto x=static_cast<long double>(origin[0])-position[0];
    const auto y=static_cast<long double>(origin[1])-position[1];
    const auto z=static_cast<long double>(origin[2])-position[2];
    const float square=static_cast<float>((z*z+y*y)+x*x); //Raw58DA16..3A keeps differences extended, one FLOAT square.
    if(square>144.f){
        world.clear_script_formation_item_ready(state,control); //0058C400 before query54 store.
        query.state.set(0x54,0x10008u);
    }else if(square>64.f)query.state.set(0x54,0x4008u);
    else if(square<16.f&&square>4.f)query.state.set(0x54,0x2008u);
    else{if(square<1.f)stance=0x1000u;query.state.set(0x54,stance|8u);}
    return true;
}
}
#endif
