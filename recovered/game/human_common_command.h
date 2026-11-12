#ifndef HD2_GAME_HUMAN_COMMON_COMMAND_H
#define HD2_GAME_HUMAN_COMMON_COMMAND_H

#include "game/human_packet_commands.h"
#include "game/car_targeted_placement.h"

namespace hd2::game {
//00586200 DC, shared by the proven Player/Remote/AI vtables. Only the reached
//subtype14 body is supplied here. The actual state refreshes A8 before resolving
//the target, then short-circuits target/item/definition guards in native order.
template<class State,class World>bool execute_human_common_command(State& state,
        const HumanPacketCommand& command,World& world){
    const auto& fields=command.fields;
    if(fields.template get<std::uint32_t>(8)!=0x14u)
        throw std::runtime_error("Human common DC requires its actual selected subtype receiver");
    state.refresh_item();
    auto* target=world.human_command_actor(fields.template get<std::int32_t>(0xc));
    if(!target||!state.item()||world.human_command_item_kind(state.item())!=0xbu)return false;
    auto* human=state.human();
    if(!human)throw std::logic_error("Human common command has no actual bound Human");
    auto& frames=world.human_command_frames();const auto head=world.human_command_head(*human);
    const auto matrix=frames.actor_world_matrix(head);
    const CarContactVector position{{matrix[12],matrix[13],matrix[14],matrix[15]}};
    //005A80E0 rereads the Human's current B0, not a cached direction/AI copy.
    const auto direction=world.human_command_direction(*human);
    const auto placement=car_targeted_placement(frames,target->base().frame(),position,direction);
    const bool authority=(world.mode_flags()&1u)!=0; //AFTER the actual engine pick.
    if(authority&&world.human_command_car_health(*target)==0.f)return false;
    //A miss may still reach an authority receiver that never reads placement
    //(mode2 clear), or returns false at its already-pending guard first.
    return world.human_command_targeted(*target,state.item(),*human,placement,authority);
}

//00540DB0/005406C0 command3's dispatch policy. Player presentation005418E0 only
//consumes; Remote presentation uses a fresh mode1 gate. Authority forwards the
//known command parameter only AFTER successful DC and a late non7 observation.
template<class State,class World,class Execute,class Forward>void receive_human_common_command(
        State& state,const HumanPacketCommand& command,bool authority,bool player,World& world,
        Execute&& execute,Forward&& forward){
    if(!authority){if(player||(world.mode_flags()&1u))return;(void)execute(state,command);return;}
    if(!execute(state,command))return;
    if((world.mode_flags()&7u)!=7u)forward(command);
}
} //namespace hd2::game
#endif
