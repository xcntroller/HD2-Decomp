#ifndef HD2_GAME_HUMAN_COMMON_COMMAND_H
#define HD2_GAME_HUMAN_COMMON_COMMAND_H

#include "game/human_packet_commands.h"
#include "game/car_targeted_placement.h"
#include "game/human_loading_stats.h"

namespace hd2::game {
//00586200 DC, shared Player/Remote/AI policy. All actual switch cases and
//default holes; World binds real Scene2/runtime identities and inline effects.
template<class State,class World>bool execute_human_common_command(State& state,
        const HumanPacketCommand& command,World& world){
    const auto& fields=command.fields;
    const auto subtype=fields.template get<std::uint32_t>(8);
    auto* bound=state.human();if(!bound)throw std::logic_error("Human common command requires actual bound Human");
    auto& receiver=*bound;
    const auto validate=world.human_common_receiver_guard(receiver);
    const auto progress=[&](auto& receiver,std::uint32_t caption){
        validate();const auto duration=state.words().template get<std::uint32_t>(0x64);
        receiver.activation.field17c=receiver.activation.field180=duration;receiver.activation.field184=caption;
    };
    const auto start=[&](std::uint32_t duration){state.words().set(0x60,subtype);state.words().set(0x64,duration);};
    const auto interaction=[&](auto& receiver,auto* target,std::uint32_t action){world.human_common_interaction(receiver,target,action);validate();};
    switch(subtype){
    case 0:case 1:
        if(world.mode_flags()&1u)world.human_command_key_door(receiver,fields.template get<std::int32_t>(0xc),subtype==0);
        validate();if(world.mode_flags()&2u)world.human_inline(receiver).action_command(0x67);
        return true;
    case 6:{
        auto* target=world.human_command_actor(fields.template get<std::int32_t>(0xc));
        if(!target||target->base().type()!=3)return false;
        start(1500);world.human_command_focus(receiver,target);return true;
    }
    case 0x11:
        state.refresh_item();
        if((world.mode_flags()&1u)&&world.human_command_item_kind(state.item())!=10)return false;
        start(2000);interaction(receiver,static_cast<typename State::ActionTarget*>(nullptr),0x16);progress(receiver,0x1b9e);return true;
    case 0x1e:world.human_command_ladder(receiver,world.human_command_actor(fields.template get<std::int32_t>(0xc)));return true;
    case 0x22:
        if(!(world.mode_flags()&1u))return true;
        return world.human_command_place_documents(receiver,fields.template get<std::int32_t>(0xc));
    case 0x23:
        if(human_frame_signed_word(receiver.properties.stats_ec[0])<=0||world.human_common_health_percent(receiver)==100||world.formation_direction_gate(receiver))return false;
        if(receiver.field160){const auto kind=receiver.field160->base().type();if(kind==4||kind==0x14||kind==0x15||kind==0x22)return false;}
        if(world.human_inline(receiver).frame_navigation_floor())return false;
        state.action_target(&receiver);start(3000);state.words().set(0x6c,fields.template get<std::uint32_t>(0xc));
        interaction(receiver,&receiver,0x24);progress(receiver,0x1ba2);return true;
    case 2:case 4:case 0xe:case 0xf:case 0x10:case 0x13:case 0x24:case 0x26:{
        //Subtype10 finds the actual ready kind19 holster item BEFORE target lookup.
        const auto item=world.human_common_ground_entry(receiver,subtype==0x10);
        if(subtype==0x10&&!item)return false;
        auto* target=(subtype==2||subtype==0x10)?
            world.human_common_runtime_actor(fields.template get<std::int32_t>(0xc)):
            world.human_command_actor(fields.template get<std::int32_t>(0xc));
        state.action_target(target);if(!target)return false;
        if(subtype==0x13&&!world.human_common_live_human(*target))return false;
        const auto required_target=[&]{validate();auto* live=state.action_target();if(!live)throw std::runtime_error("Human common callback retired its required target");return live;};
        switch(subtype){
        case 2:case 0x26:start(4000);interaction(receiver,target,0x14);progress(receiver,0x1ba3);break;
        case 4:world.human_common_alignment(receiver);start(3000);interaction(receiver,target,0x16);progress(receiver,0x1ba0);break;
        case 0xe:{
            world.human_common_alignment(receiver);state.words().set(0x60,subtype);
            const auto skill=human_loading_stat(receiver,8);
            const auto time=(1.L-skill)*5000.f+500.f;
            state.words().set(0x64,human_loading_stat_word(time));interaction(receiver,target,0x12);progress(receiver,0x1ba1);break;
        }
        case 0xf:start(3000);interaction(receiver,target,0xe);world.human_common_target_interaction(*required_target(),&receiver,0x10);validate();
            progress(receiver,0x1ba2);world.human_common_target_progress(*required_target(),state.words().template get<std::uint32_t>(0x64),0x1ba2);break;
        case 0x10:{start(3900);world.human_common_alignment(receiver);const auto model=world.human_common_item_model(item);validate();
            world.human_common_interaction_model(receiver,required_target(),0x1c,model);validate();progress(receiver,0x1ba6);break;}
        case 0x13:start(5000);interaction(receiver,&receiver,0x1a);interaction(receiver,required_target(),0x18);progress(receiver,0x1ba4);break;
        case 0x24:start(1000);interaction(receiver,target,0x1e);world.human_common_target_interaction(*required_target(),&receiver,0x20);validate();progress(receiver,0x1ba5);break;
        default:break;
        }
        return true;
    }
    default:break;
    }
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
    if(fields.template get<std::uint32_t>(8)==5u){ //00586200→virtualE4/00585A60, reached attached exit.
        auto* human=state.human();if(!human)throw std::logic_error("Seat exit command requires actual bound Human");
        world.human_command_exit_emplacement(*human);return true;
    }
    if(fields.template get<std::uint32_t>(8)==3u){ //00586200→virtualE0/00585D10, reached Car/StaticWeapon admission.
        auto* human=state.human();if(!human)throw std::logic_error("Seat entry command requires actual bound Human");
        return world.human_command_enter_emplacement(*human,world.human_command_actor(fields.template get<std::int32_t>(0xc)),fields.template get<std::int32_t>(0x10));
    }
    if(subtype==0x1bu||subtype==0x1cu){
        const bool direct=(state.words().template get<std::uint32_t>(0x50)&1u)&&(world.mode_flags()&1u);
        auto* target=world.human_command_actor(fields.template get<std::int32_t>(0xc));
        auto* human=state.human();if(!human)throw std::logic_error("Door command requires actual bound Human");
        world.human_command_door(*human,target,direct,subtype==0x1bu);return true;
    }
    if(subtype!=0x14u)return true; //Genuine native switch holes/default.
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

//00586200 subtype1E, actor identity resolution precedes actual inline id1F.
template<class State,class World,class Runtime>bool execute_human_common_command(State& state,
        const HumanPacketCommand& command,World& world,Runtime& runtime){
    if(command.fields.template get<std::uint32_t>(8)!=0x1eu)return execute_human_common_command(state,command,world);
    auto* target=world.human_command_actor(command.fields.template get<std::int32_t>(0xc));
    auto* human=state.human();if(!human)throw std::logic_error("Ladder command requires actual bound Human");
    (void)world.human_inline(*human).request_frame_ladder(target,runtime);
    return true; //Native ignores NULL/rejected/busy setup's signed result.
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
