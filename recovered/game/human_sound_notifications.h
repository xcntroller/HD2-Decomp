#ifndef HD2_GAME_HUMAN_SOUND_NOTIFICATIONS_H
#define HD2_GAME_HUMAN_SOUND_NOTIFICATIONS_H

#include "game/human_perception_records.h"
#include <cmath>
#include <cstring>

namespace hd2::game {

//Typed SOUND observer payload. Native pointers10/1C stay typed on host64.
//The containing SOUND owner, not a raw float-array cast, constructs this view.
template<class Frame,class Actor>struct HumanSoundEvent {
    HumanVector position{};Frame receiver{};float field14=0;
    std::uint32_t kind=0;Actor source{};
};

//005A5430 after actual receiver-frame→Human resolution. Bindings must dispatch
//real B0+A8/AC/B0/B4 source; registering an empty successful receiver is forbidden.
template<class Human,class Event,class Frames,class Bindings>
void notify_human_sound(Human& human,const Event& event,Frames& frames,Bindings& bindings) {
    if(event.source==&human)return;
    auto* state=human.loading_state;
    if(!state)throw std::runtime_error("Human sound notification has no actual B0 state");
    const auto flags=state->words().template get<std::uint32_t>(0x50);
    if(flags&0x40000000u)return;
    auto timer_bits=state->words().template get<std::uint32_t>(0xb0);std::int32_t timer;std::memcpy(&timer,&timer_bits,4);
    if(timer<=250&&static_cast<long double>(bindings.human_sound_random())*.7f>event.field14)return;
    const auto matrix=frames.actor_world_matrix(human.base().frame());
    const auto x=static_cast<long double>(event.position[0])-matrix[12];
    const auto y=static_cast<long double>(event.position[1])-matrix[13];
    const auto z=static_cast<long double>(event.position[2])-matrix[14];
    const float square=static_cast<float>((y*y+z*z)+x*x); //Extended differences, then native float stores14/10.
    std::int32_t bits;std::memcpy(&bits,&square,4);long double distance=0;
    if(bits>=0x322bcc77)distance=std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?
        1.L:std::sqrt(static_cast<long double>(square));
    const auto hearing_bits=human.properties.stats_ec[11];std::int32_t hearing;std::memcpy(&hearing,&hearing_bits,4);
    if(distance>hearing)return; //Native unordered comparison continues.
    const auto require_source=[&]{if(!event.source)throw std::runtime_error("Human sound kind requires its actual source actor");};
    const auto friendly=[&]{require_source();return bindings.human_sound_friendly(human,event.source);};
    switch(event.kind) {
    case 0:
        if(friendly()||bindings.human_sound_in_car(human))return;
        //005A5CC0's ignored source-Human result is a PURE B0 flags/E0+50 getter;
        //005AC220 only reads inline0C+50. It has no mutation or receiver call.
        break;
    case 1:case 3:case 6:case 8:
        if(event.source&&friendly())return;
        break;
    case 2:case 13:
        if(friendly()){bindings.human_sound_state_ac(human,event.source);return;}
        break;
    case 4:
        if(event.field14>.95f)bindings.human_sound_state_b0(human,event.source);
        return;
    case 5:case 7:case 10:case 11:return;
    case 9:
        require_source();
        if(bindings.human_sound_health_ec(event.source)<1)bindings.human_sound_state_a8(human,event.source);
        break; //Native still appends/refreshes the kind9 record AFTER+A8.
    default:break;
    }
    human.list2ac.append_or_refresh(event.kind,event.position,event.source,
        [&]{return bindings.human_sound_ticks();},[&](auto& record){bindings.human_sound_state_b4(human,record);});
}
} //namespace hd2::game
#endif
