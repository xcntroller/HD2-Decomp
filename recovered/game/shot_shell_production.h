#ifndef HD2_GAME_SHOT_SHELL_PRODUCTION_H
#define HD2_GAME_SHOT_SHELL_PRODUCTION_H
#include "game/human_ai_shot_direction.h"

namespace hd2::game {
//0044E5D0: C8/marker/CC gates BEFORE current-player lookup or any random draw.
//Acquire the SAME64 pool only after all three velocity draws; publish on134.
template<class State,class Frames,class World,class Human>
void produce_item_shell(State& state,Frames& frames,World& world,Human& human,typename Frames::Frame marker,std::uint32_t command){
    if(!state.shell_third||!marker||!state.shell_first)return;
    if(world.current_human_frame_player()!=&human)return;
    if(!state.world)throw std::logic_error("Shell requires actual world-bound ShotMgr");
    auto matrix=frames.actor_world_matrix(marker);HumanVector velocity{{matrix[4],matrix[5],matrix[6],matrix[7]}};
    velocity[0]=static_cast<float>(((static_cast<long double>(frames.volume_random_unit())-.5f)*.5f+1.5f)*velocity[0]);
    velocity[1]=static_cast<float>(((static_cast<long double>(frames.volume_random_unit())-.5f)+1.5f)*velocity[1]);
    const auto z=((static_cast<long double>(frames.volume_random_unit())-.5f)*.5f+1.5f)*velocity[2];
    const auto& human_velocity=world.shot_human_velocity(human);
    velocity[0]=static_cast<float>(static_cast<long double>(velocity[0])+human_velocity[0]);
    velocity[1]=static_cast<float>(static_cast<long double>(velocity[1])+human_velocity[1]);velocity[2]=static_cast<float>(z+human_velocity[2]);
    auto record=state.decals.acquire(); //Actual40 pool, not a new registry or particle owner.
    const bool first=world.human_view_offset_enabled();matrix=frames.actor_world_matrix(marker);
    HumanVector direction{{matrix[8],matrix[9],matrix[10],matrix[11]}};
    const float square=static_cast<float>((static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[1])*direction[1]);
    normalize_human_ai_shot(direction,square);matrix=frames.actor_world_matrix(marker);
    const HumanVector position{{matrix[12],matrix[13],matrix[14],matrix[15]}};
    record->initialize(frames,world,human,first?state.shell_first:state.shell_third,position,direction,velocity,command);
    state.shells.push_back(std::move(record));
}
} //namespace hd2::game
#endif
