#ifndef HD2_GAME_HUMAN_COMMON_COMMAND_H
#define HD2_GAME_HUMAN_COMMON_COMMAND_H

#include "game/human_packet_commands.h"
#include "game/car_targeted_placement.h"

namespace hd2::game {
//00586200 DC, shared by the proven Player/Remote/AI vtables. Only the reached
//subtype14 and1F bodies are supplied here. Subtype14 refreshes A8 before resolving
//the target, then short-circuits target/item/definition guards in native order.
template<class State,class World>bool execute_human_common_command(State& state,
        const HumanPacketCommand& command,World& world){
    const auto& fields=command.fields;
    if(fields.template get<std::uint32_t>(8)==0x1fu){
        auto* human=state.human();
        if(!human)throw std::logic_error("Human climb command has no actual bound Human");
        if(state.input()&&world.human_current_player_id()!=human->base().id10()&&!(world.mode_flags()&1u)){
            const auto position=state.climb_history_position(); //Actual7C history1 / position-value+2C.
            world.replay_human_climb_position(*human,position); //00759E20, SAME DD0.
        }
        world.begin_human_climb_animation(state); //E0 kind4, movement1E, changed1 AFTER replay.
        return true;
    }
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

//Actual virtualD8 policies: Player0053D340 queues first80 only; Remote00587540
//tail-dispatches DC; AI00571C10 executes DC first, ignores its boolean, THEN
//copies the three live parameter words to kind3 in second90. Allocation failure
//is a native source exception guard, not an original successful null command.
template<class State,class Execute>void dispatch_human_deferred_common_command(
        State& state,const HumanPacketCommand& command,Execute&& execute){
    const auto kind=state.words().template get<std::uint32_t>(0x48);
    if(kind!=1&&kind!=7&&(kind<3||kind>8))
        throw std::runtime_error("Human deferred common command has no proven B0 receiver");
    if(kind!=1)(void)execute(state,command);
    if(kind==7)return;
    HumanPacketCommand copy;copy.kind=3;
    for(const auto offset:{8u,0xcu,0x10u})copy.fields.set(offset,command.fields.template get<std::uint32_t>(offset));
    state.queue_frame_command(std::move(copy),kind!=1);
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
