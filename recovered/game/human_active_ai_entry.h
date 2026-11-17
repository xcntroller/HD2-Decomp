#ifndef HD2_GAME_HUMAN_ACTIVE_AI_ENTRY_H
#define HD2_GAME_HUMAN_ACTIVE_AI_ENTRY_H
#include "game/human_active_ai_commands.h"
namespace hd2::game {
template<class AI>void publish_active_task_second_mode(AI&,std::uint32_t);
template<class AI,class Actor,class Frames,class World>bool active_task_target_arrived(AI&,Actor*,Frames&,World&);
//00528950 initial motion. Return2 waits in an existing gun seat,1 starts the
//actual driver path,0 continues on foot. Exit callbacks precede the205 store.
template<class AI,class World>unsigned prepare_active_task_attachment(AI& ai,World& world){
    auto* human=ai.human();if(!human)throw std::logic_error("Active task attachment lost Human44");
    auto* actor=human->field160;const auto action=ai.state().template get<std::uint32_t>(0x220);
    const bool attack=action==25||action==26||action==22;
    if(actor){const auto kind=actor->base().type();
        if(kind==4&&human->field164!=0){
            if(attack&&world.human_pose_actor_has_control(actor,human->field164))return 2;
            world.active_command_exit_focus(ai,0);
        }else if(kind==21){if(attack)return 2;world.active_command_exit_focus(ai,0);}
    }
    human=ai.human();if(!human)throw std::logic_error("Active task exit lost Human44");
    const bool driver=human->field160&&human->field160->base().type()==4&&human->field164==0;
    ai.state().set(0x205,std::uint8_t{1});
    if(driver){auto& path=ai.resources().path();
        path.bind_vehicle(human->field160,[&world](void* car){world.human_vehicle_governor(car,world.human_vehicle_maximum(car));});
        path.request_vehicle_position(ai.state().template get<HumanVector>(0x1e0),1.f);
        path.movement_flags(ai.state().template get<std::uint8_t>(0x188)?0x1000u:0x2000u);
    }
    return driver?1u:0u;
}
//00534510: command8..13 is a seat approach producer whose actual task is3.
//The supplied position is used directly; no fresh target-model read occurs.
template<class AI,class Actor,class World>void produce_active_seat_approach(AI& ai,Actor& target,
        std::uint32_t command,const HumanVector& position,World& world){
    const auto kind=target.base().type();
    if(kind==4){if(!world.active_car_entry_available(target)){ai.reject_pending_task();return;}}
    if((kind==4||kind==21)&&world.active_command_seat_occupied(target,static_cast<std::int32_t>(command-8u))){ai.reject_pending_task();return;}
    auto* human=ai.human();if(!human)throw std::logic_error("Active seat approach lost Human44");
    const auto direction=human->base().world_direction();ai.initialize_task(&target,3,position,direction,true,2.5f);
    ai.state().set(0x210,command-8u);delay_active_focused_task(ai,world);notify_human_active_ai_empty_orders(ai,world);
}
//00528950 selects an unCLAIMED native6C seat, not an unoccupied virtual78.
//All-claimed failure clears203 and speaks before the204 group-completion gate.
template<class AI,class World>bool select_active_task_entry_seat(AI& ai,World& world,std::uint32_t& seat){
    ai.require_live_task_target();auto* actor=ai.task_target;if(!actor)return true;
    const auto kind=actor->base().type();if(kind!=4&&kind!=20&&kind!=21)return true;
    const auto wanted=ai.state().template get<std::uint32_t>(0x210);
    if(!world.human_emplacement_claimed(actor,static_cast<std::int32_t>(wanted))){seat=wanted;return true;}
    for(auto count=world.active_task_seat_count(*actor);count;){const auto index=--count;
        if(!world.human_emplacement_claimed(actor,static_cast<std::int32_t>(index))){seat=static_cast<std::uint32_t>(index);return true;}}
    ai.state().set(0x203,std::uint8_t{0});ai.reject_pending_task();return false;
}
//00528950 Car entry: paired exit/approach pose queries use the requested210
//first, then ascending alternatives. The chosen free seat is a separate value.
template<class AI,class Frames,class World>bool advance_active_task_car_entry(AI& ai,std::uint32_t seat,Frames& frames,World& world){
    ai.require_live_task_target();auto* actor=ai.task_target;auto* human=ai.human();
    if(!human)throw std::logic_error("Active Car entry lost Human44");
    if(!actor||actor->base().type()!=4||human->field160==actor)return false;
    const auto wanted=ai.state().template get<std::uint32_t>(0x210);HumanVector position{},direction{};
    bool available=world.formation_seat_pose(*actor,wanted,position,direction);
    if(!available)for(std::size_t n=0;n<world.active_task_seat_count(*actor);++n)
        if(n!=wanted&&world.formation_seat_pose(*actor,n,position,direction)){available=true;break;}
    if(available){
        const auto matrix=frames.actor_world_matrix(human->base().frame());
        const auto x=static_cast<long double>(position[0])-matrix[12],y=static_cast<long double>(position[1])-matrix[13],z=static_cast<long double>(position[2])-matrix[14];
        const float square=static_cast<float>((x*x+y*y)+z*z);std::int32_t bits;std::memcpy(&bits,&square,4);
        const auto distance=bits<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-9f?1.L:std::sqrt(static_cast<long double>(square));
        if(distance<=4.5f){publish_active_task_second_mode(ai,seat+8u);return false;}
        if(distance<=20.f){ai.state().set(0x214,1000u);produce_active_seat_approach(ai,*actor,wanted+8u,position,world);return true;}
    }
    //Retain a hold for the first pending order before trimming/cancellation.
    if(ai.has_pending_orders_loading()){
        const auto facing=human->base().world_direction();auto& first=ai.first_pending_order();first.require_live_target();
        auto point=first.state.template get<HumanVector>(0x30);
        if(first.target){const auto m=frames.actor_world_matrix(first.target->base().frame());point={{m[12],m[13],m[14],m[15]}};}
        ai.append_hold_order(point,facing);
        while(ai.pending_orders().size()>2)ai.retire_pending_order(0,true);
    }
    cancel_human_active_ai_task(ai,world);follow_human_active_ai(ai,true,false,world);ai.reject_pending_task();ai.pending_first_mode(18);return true;
}
//00536120 occupied follow policy. The attachment pointers are native borrows:
//a different-seat exit still returns using the PRE-callback own attachment.
template<class AI,class Formation,class Human,class Frames,class World,class Validate>
bool advance_active_follow_attachment(AI& ai,Formation& formation,Human& leader,float distance,
        Frames& frames,World& world,Validate& validate){
    auto* human=ai.human();if(!human)throw std::logic_error("Occupied follow lost Human44");
    const auto occupied=[](auto* actor){if(!actor)return false;const auto kind=actor->base().type();return kind==4||kind==20||kind==21||kind==34;};
    auto* leader_actor=leader.field160;auto* own_actor=human->field160;
    if(occupied(leader_actor)){
        if(own_actor&&own_actor!=leader_actor&&own_actor->base().type()!=20){world.active_command_exit_focus(ai,0);validate();}
        if(own_actor)return true;
        if(leader_actor->base().type()==4&&human->field174){world.active_command_exit_focus(ai,ai.state().template get<std::int32_t>(0x20c));validate();}
        if(distance<2.25f||(!(ai.resources().query().state.template get<std::uint32_t>(0x50)&0x10000000u)&&
                leader_actor->base().type()==4&&active_task_target_arrived(ai,leader_actor,frames,world))){
            std::int32_t identity=-1;const auto seat=formation.member_attachment(human,identity);if(identity<0)return true;
            auto* target=world.active_follow_identity(identity);if(!target)throw std::runtime_error("Occupied follow reached native NULL Scene2 identity");
            if(static_cast<std::int64_t>(world.active_task_seat_count(*target))<=seat)return true;
            if(world.active_command_seat_occupied(*target,seat))return true;
            if(!human->field174){world.execute_active_task_command(ai,3,identity,static_cast<std::uint32_t>(seat));validate();publish_active_task_second_mode(ai,static_cast<std::uint32_t>(seat)+8u);}
            else{world.active_command_exit_focus(ai,ai.state().template get<std::int32_t>(0x20c));validate();}
            return true;
        }
        return false;
    }
    if(occupied(own_actor)){world.active_command_exit_focus(ai,0);validate();}
    return false;
}

}
#endif
