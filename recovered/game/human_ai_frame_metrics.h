#ifndef HD2_GAME_HUMAN_AI_FRAME_METRICS_H
#define HD2_GAME_HUMAN_AI_FRAME_METRICS_H
#include "game/human_joint_owners.h"
#include <cmath>

namespace hd2::game {
//005733E0: reverse actual hostile list, real model poses each iteration;
//extended differences, stored FLOAT square, SIGNED bit-pattern epsilon test.
template<class Human,class Frames>long double human_ai_closest_enemy_distance(Human& human,Frames& frames){
    float minimum=65535.f;const auto& enemies=human.perception.lists()[0];
    for(auto n=enemies.size();n;){
        auto* enemy=enemies[--n];if(!enemy)throw std::logic_error("AI distance requires actual hostile actor");
        const auto own=human.base().frame(),other=enemy->base().frame();
        if(!own||!other)throw std::runtime_error("AI distance requires actual model frames");
        const auto a=frames.actor_world_matrix(own),b=frames.actor_world_matrix(other);
        const auto x=static_cast<long double>(b[12])-a[12],y=static_cast<long double>(b[13])-a[13],z=static_cast<long double>(b[14])-a[14];
        const float square=static_cast<float>((z*z+y*y)+x*x);std::int32_t bits;std::memcpy(&bits,&square,4);
        const long double distance=bits<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(square));
        if(distance<minimum)minimum=static_cast<float>(distance);
    }
    return minimum;
}
//00584890 retains normalized X/Y extended until the final dot but spills Z.
//Its two normalization sums have different orders; no shared vector helper
//with earlier FLOAT stores is substituted here. Only native AL is consumed.
inline std::array<long double,3> human_ai_aim_metric_normalize(const HumanVector& input,bool displacement){
    std::array<long double,3> value{{input[0],input[1],input[2]}};
    const auto x=value[0]*value[0],y=value[1]*value[1],z=value[2]*value[2];
    const auto square=displacement?(z+x)+y:(z+y)+x;const float stored=static_cast<float>(square);
    constexpr float epsilon=9.99999993922529e-09f;
    if(!(std::abs(square-1)<epsilon)){
        if(!(stored<epsilon)){
            const auto scale=1/std::sqrt(static_cast<long double>(stored));value[0]*=scale;value[1]*=scale;
            value[2]=static_cast<float>(value[2]*scale);
        }else{const auto axis=value[0]!=0?0u:value[2]!=0?2u:1u;value[axis]=value[axis]<0?-1.L:1.L;}
    }
    return value;
}
template<class Observer,class State,class Target,class Frames>bool human_ai_aims_at(Observer& observer,State& state,Target& target,Frames& frames){
    const auto own=observer.base().frame(),other=target.base().frame();
    if(!own||!other)throw std::runtime_error("AI aim predicate requires actual model frames");
    const auto a=frames.actor_world_matrix(own),b=frames.actor_world_matrix(other);
    HumanVector delta{};for(unsigned axis=0;axis!=3;++axis)delta[axis]=static_cast<float>(static_cast<long double>(b[12+axis])-a[12+axis]);
    const auto aim=state.target_direction(); //Actual enemy B0 virtual4C, AFTER model queries.
    const auto d=human_ai_aim_metric_normalize(delta,true),v=human_ai_aim_metric_normalize(aim,false);
    return (v[2]*d[2]+v[0]*d[0])+v[1]*d[1]>.9848099946975708f; //008110C4, strict.
}
} //namespace hd2::game
#endif
