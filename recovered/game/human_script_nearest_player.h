#ifndef HD2_GAME_HUMAN_SCRIPT_NEAREST_PLAYER_H
#define HD2_GAME_HUMAN_SCRIPT_NEAREST_PLAYER_H
#include "game/mission_script_player_predicates.h"
#include "game/human_state_frame_timers.h"
#include <limits>
namespace hd2::game {
//00421C00 op43. Caller selects SAME primary/auxiliary roster using006B66B0.
//Strict extended distance comparison and FLOAT best store retain reverse tie order.
template<class Frames,class Roster,class World>
typename Frames::Frame nearest_human_script_player(typename Frames::Frame source,Roster& roster,Frames& frames,World& world){
    typename Frames::Frame nearest{};float best=std::numeric_limits<float>::max();
    for(auto n=roster.size();n;){auto* human=roster.at(static_cast<std::int32_t>(--n));
        if(!human)throw std::runtime_error("TurnAtNearestPlayer reached native NULL roster Human");
        if(human_frame_signed_word(human->properties.stats_ec[0])<=0||world.human_selected_suppressed(*human)||!script_human_player_controlled(*human,world.mode_flags()))continue;
        const auto target=human->base().frame();if(!target||!source)continue;
        const auto first=frames.actor_world_matrix(source),second=frames.actor_world_matrix(target);
        const auto x=static_cast<long double>(second[12])-first[12],y=static_cast<long double>(second[13])-first[13],z=static_cast<long double>(second[14])-first[14];
        const auto square=(z*z+y*y)+x*x;if(square<best){nearest=target;best=static_cast<float>(square);}
    }
    return nearest;
}
} //namespace hd2::game
#endif
