#ifndef HD2_GAME_HUMAN_ACTIVE_AI_CAUTIOUS_H
#define HD2_GAME_HUMAN_ACTIVE_AI_CAUTIOUS_H
#include "game/human_joint_owners.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
namespace hd2::game {
//0052F160: reverse actual enemy list, each current B0 virtualA4. The true
//branch tests SELECTED Human's005A0AB0(own), so friendship is not reversed.
template<class AI,class World>auto active_cautious_attacker(AI& ai,bool friendly,World& world){
    auto* human=ai.human();const auto& enemies=human->perception.lists()[0];
    using Pointer=typename std::decay_t<decltype(enemies)>::value_type;
    for(auto n=enemies.size();n;){auto* enemy=enemies[--n];if(!enemy)throw std::runtime_error("Active cautious query reached native NULL enemy");
        auto* target=world.active_movement_selected(*enemy);
        if(friendly){if(target&&world.active_movement_target_friendly(*target,*human))return enemy;}
        else if(target==human)return enemy;
    }
    return Pointer{};
}
//00525150->00525160 installs these XYZ triples in process008AAC80. W is
//untouched BSS zero.00525700's raw code uses the normalized vector's OWN
//squared length in its pruning loop: it cannot be negative. No candidates are
//removed.525A1C consumes rand but525A25 replaces its result with count; the
//selected entry is count/2, NOT rand%count. Preserve that native behavior.
template<class World>bool choose_active_cautious_direction(std::uint32_t mask,HumanVector& direction,const HumanVector& position,World& world){
    if(!mask)return false;
    constexpr std::array<HumanVector,8> directions{{
        {{0,0,5,0}},{{5,0,5,0}},{{5,0,0,0}},{{5,0,-5,0}},
        {{0,0,-5,0}},{{-5,0,-5,0}},{{-5,0,0,0}},{{-5,0,5,0}}
    }};
    std::array<unsigned,8> candidates{};unsigned count=0;
    for(unsigned n=0;n!=8;++n)if(mask&(1u<<n))candidates[count++]=n;
    //Native selected-player query is mandatory even though the subsequent
    //normalized self-square cannot change the candidate list or output.
    if(auto* selected=world.active_avoidance_player()){
        const auto cached=selected->base().world_position();(void)cached;(void)position;
    }
    (void)world.human_frame_random_word();
    if(!count)throw std::runtime_error("Active cautious direction reached native empty low-byte mask dereference after rand");
    auto result=directions[candidates[count/2]];
    for(unsigned n=0;n!=3;++n)result[n]=-result[n];
    const float square=static_cast<float>((static_cast<long double>(result[2])*result[2]+static_cast<long double>(result[1])*result[1])+static_cast<long double>(result[0])*result[0]);
    const auto scale=1.L/std::sqrt(static_cast<long double>(square));
    for(unsigned n=0;n!=3;++n)result[n]=static_cast<float>(static_cast<long double>(result[n])*scale);
    direction=result;return true;
}
//00525C90: high-cover mask first, then crouched mask. Output publication
//precedes the final leader-state query, including on that receiver's failure.
template<class AI,class World,class Publish>void select_active_cautious_posture(AI& ai,HumanVector& direction,std::uint32_t& posture,World& world,Publish&& publish){
    auto value=ai.target_direction();const auto position=ai.human()->base().world_position();
    std::uint32_t selected_posture=2;
    if(const auto* point=ai.point()){
        if(choose_active_cautious_direction(point->reserved17[1],value,position,world))selected_posture=4;
        else (void)choose_active_cautious_direction(point->reserved17[0],value,position,world);
    }
    posture=selected_posture;direction=value;publish(direction,posture);auto* leader=ai.human()->formation_leader;
    if(ai.state().template get<std::uint8_t>(0x189)&&leader){
        const auto flags=world.active_follow_state(*leader).words().template get<std::uint32_t>(0x50);
        if((flags&1u)&&ai.words().template get<std::int32_t>(0xb0)>5000){posture=1;publish(direction,posture);}
    }
}
template<class AI,class World>void select_active_cautious_posture(AI& ai,HumanVector& direction,std::uint32_t& posture,World& world){
    select_active_cautious_posture(ai,direction,posture,world,[](const HumanVector&,std::uint32_t){});
}
} //namespace hd2::game
#endif
