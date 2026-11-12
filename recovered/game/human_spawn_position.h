#ifndef HD2_GAME_HUMAN_SPAWN_POSITION_H
#define HD2_GAME_HUMAN_SPAWN_POSITION_H
#include "game/human_joint_math.h"
#include <limits>
#include <type_traits>
namespace hd2::game {
//005ED550 keeps (Y²+X²)+Z² extended for its comparison, reloads the FLOAT
//square for sqrt, then keeps the inverse extended through all three stores.
inline HumanVector normalize_zone_spawn_direction(HumanVector direction){
    const auto square=(static_cast<long double>(direction[1])*direction[1]+static_cast<long double>(direction[0])*direction[0])+static_cast<long double>(direction[2])*direction[2];
    const float stored=static_cast<float>(square);constexpr float epsilon=9.99999993922529e-09f;
    if(!std::isfinite(square)||!std::isfinite(stored))throw std::runtime_error("Non-finite zone spawn direction");
    if(std::abs(square-1)>=epsilon){
        if(stored>=epsilon){const auto inverse=1/std::sqrt(static_cast<long double>(stored));for(unsigned n=0;n!=3;++n)direction[n]=static_cast<float>(direction[n]*inverse);}
        else{const auto n=direction[0]!=0?0u:direction[2]!=0?2u:1u;direction[n]=direction[n]<0?-1.f:1.f;}
    }
    return direction;
}
//0069D6A0 uses a downward0.2-radius sweep and continues past non-Human/Car
//hits. The original subtracts distance+.01 BEFORE checking hit identity.
template<class Runtime>bool human_spawn_position_occupied(Runtime& r,const HumanVector& position){
    float height=2.f;
    do{
        const HumanVector start{{position[0],static_cast<float>(static_cast<long double>(height)+position[1]),position[2],0}};
        const HumanVector displacement{{0,-height,0,0}};
        const auto hit=r.spawn_position_sweep(start,displacement,.20000000298023223876953125f);
        const float distance=hit&&hit->distance_squared>0?static_cast<float>(std::sqrt(static_cast<long double>(hit->distance_squared))):0.f;
        height=static_cast<float>(static_cast<long double>(height)-(static_cast<long double>(distance)+.00999999977648258209228515625f));
        if(!hit)return false;
        const auto flags=r.human_geometry_flags(*hit);
        if((flags&0x40u)&&(flags&0xc0u))if(auto* actor=r.human_ragdoll_contact_actor(*hit))
            if(actor->base().type()==3||actor->base().type()==4)return true;
    }while(height>.00999999977648258209228515625f);
    return false;
}
//00475840 captures Scene2 identity-slot count once, scans twice, and draws even when
//there are no candidates. This uses FTOL on extended product, no DOUBLE/floor.
template<class Spawn,class Entries,class Random>auto select_session_spawn_point(Entries&& entries,std::uint32_t kind,std::uint32_t side,Random&& random){
    using Actor=typename std::remove_reference_t<decltype(entries())>::value_type;
    const auto count=entries().size();if(count>static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))throw std::length_error("Spawn identity list exceeds native signed count");
    const auto eligible=[&](Actor actor){
        if(!actor||actor->base().type()!=10)return false;
        auto* spawn=dynamic_cast<Spawn*>(actor);if(!spawn)throw std::logic_error("Spawn identity entry lost its actual type10 owner");
        return spawn->properties.field90==kind&&spawn->flag9c&&(spawn->properties.field94==side||spawn->properties.field94==2);
    };
    std::int32_t candidates=0;
    for(std::size_t n=0;n<count;++n)if(eligible(entries().at(n)))++candidates;
    const auto word=static_cast<std::uint32_t>(random());std::int32_t draw;std::memcpy(&draw,&word,4);
    const std::uint32_t bits=0x38000100;float multiplier;std::memcpy(&multiplier,&bits,4);
    const auto selected=std::trunc(static_cast<long double>(draw)*multiplier*candidates);
    std::int32_t ordinal=0;
    for(std::size_t n=0;n<count;++n){auto* actor=entries().at(n);if(eligible(actor)){if(ordinal==selected)return actor;++ordinal;}}
    return Actor{};
}
} //namespace hd2::game
#endif
