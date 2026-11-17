#ifndef HD2_GAME_HUMAN_AI_TARGET_OUTPUT_H
#define HD2_GAME_HUMAN_AI_TARGET_OUTPUT_H
#include "game/human_ai_shot_direction.h"
#include <array>
#include <stdexcept>
namespace hd2::game {
//005A3AB0. Relation mask comes from the observer's actual Sight258 and this
//target's25C; no observer/zero mask uses existing005A3CC0 random body point.
//The actual visual214 table is borrowed BEFORE either model-world update.
template<class Frames,class Human,class World>
HumanVector select_human_ai_target_point(Frames& frames,Human& target,Human* observer,World& world){
    if(!observer)return world.human_damage_random_position(target);
    const auto mask=world.human_hostile_relation_mask(*observer,target);
    if(!mask)return world.human_damage_random_position(target);
    const auto visual=frames.actor_find(target.base().frame(),"a",1);
    const auto* bones=frames.human_target_bone_table(visual);
    const auto* target_matrix=frames.human_target_world_matrix(target.base().frame());
    const auto* observer_matrix=frames.human_target_world_matrix(observer->base().frame());
    const auto x=static_cast<long double>(observer_matrix[12])-target_matrix[12];
    const auto y=static_cast<long double>(observer_matrix[13])-target_matrix[13];
    const auto z=static_cast<long double>(observer_matrix[14])-target_matrix[14];
    const auto square=(z*z+x*x)+y*y; //5A3B50..84, extended through strict thresholds.
    std::array<unsigned,13> choices{};unsigned count=0;
    const auto append=[&](unsigned bone){choices.at(count++)=bone;};
    if(mask&1u){append(12);append(8);append(3);append(1);}
    if((mask&16u)&&!(observer->properties.field19c==1u&&(mask&1u))){
        if(observer->properties.field19c==1u){if(square>225.f)append(21);append(20);if(square>225.f)append(17);append(16);}
        else{
            if(square>225.f)append(22);
            if(square>100.f)append(21);
            append(20);if(square>225.f)append(18);if(square>100.f)append(17);append(16);
        }
    }
    if(mask&2u)append(24);
    if(mask&4u)append(14);
    if(mask&8u)append(10);
    const auto draw=world.human_frame_random_word(); //Native draws BEFORE the required nonzero divisor.
    if(!count)throw std::logic_error("Human target mask has no original bone candidates");
    const auto bone=bones[choices[draw%count]];
    if(!bone)throw std::logic_error("Human target point requires actual skeletal frame");
    const auto matrix=frames.actor_world_matrix(bone);
    return {{matrix[12],matrix[13],matrix[14],matrix[15]}};
}
//00575450. The Human target branch passes the source Human as its observer to
//005A3AB0; the non-Human branch uses target20. Fourth scratch lane is unwritten
//in the native return, represented only as a safety guard here.
template<class State,class Actor,class Frames,class World>
HumanVector human_ai_actor_target_direction(State& state,Actor& actor,Frames& frames,World& world){
    auto* human=state.human();if(!human||!human->head_marker)throw std::logic_error("AI target direction requires actual source260 marker");
    const auto* origin=frames.human_target_world_matrix(human->head_marker);
    HumanVector destination{};
    if(actor.base().type()==3u)destination=select_human_ai_target_point(frames,world.human_ai_target_human(actor),human,world);
    else{const auto matrix=frames.actor_world_matrix(actor.base().frame());destination={{matrix[12],matrix[13],matrix[14],matrix[15]}};}
    HumanVector result{};
    for(unsigned n=0;n!=3;++n)result[n]=static_cast<float>(static_cast<long double>(destination[n])-origin[12+n]);
    normalize_human_ai_shot(result,(static_cast<long double>(result[1])*result[1]+static_cast<long double>(result[2])*result[2])+static_cast<long double>(result[0])*result[0]);
    return result;
}
//00575670, actual arbitrary target frame; normalizes stored differences using
//(Z squared + Y squared) + X squared, preserving the native FLOAT sqrt reload.
template<class State,class Frames>
HumanVector human_ai_frame_target_direction(State& state,typename Frames::Frame target,Frames& frames){
    auto* human=state.human();if(!human||!human->head_marker||!target)throw std::logic_error("AI frame direction requires actual source/target frames");
    const auto* origin=frames.human_target_world_matrix(human->head_marker);
    const auto* destination=frames.human_target_world_matrix(target);HumanVector result{};
    for(unsigned n=0;n!=3;++n)result[n]=static_cast<float>(static_cast<long double>(destination[12+n])-origin[12+n]);
    normalize_human_ai_shot(result,(static_cast<long double>(result[2])*result[2]+static_cast<long double>(result[1])*result[1])+static_cast<long double>(result[0])*result[0]);
    return result;
}
}
#endif
