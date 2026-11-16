#ifndef HD2_GAME_HUMAN_ACTIVE_AI_PENDING_H
#define HD2_GAME_HUMAN_ACTIVE_AI_PENDING_H
#include "game/human_ai_shot_direction.h"
#include "game/human_state_frame_timers.h"
#include "game/human_active_ai_task_execution.h"
namespace hd2::game {
//00529F60. The first order is borrowed across callbacks; kind4 can reset the
//owning sequence, so its caller must reread native begin[0] afterwards.
template<class AI,class Frames,class World>void start_human_active_pending_order(AI& ai,
        std::uint32_t elapsed,Frames& frames,World& world){
    auto& order=ai.first_pending_order();auto& record=order.state;auto& state=ai.state();
    const auto kind=record.template get<std::uint32_t>(0xc);
    if(record.template get<std::uint8_t>(0x49)){
        if(kind!=2&&kind!=6&&kind!=7){
            if(!ai.advance_pending_task(elapsed)||state.template get<std::uint8_t>(0x206))record.set(0x48,std::uint8_t{1});
        }else if(!state.template get<std::uint32_t>(0x198)&&!state.template get<std::uint32_t>(0x19c)){
            ai.set_target_direction(record.template get<HumanVector>(0x20));
            ai.set_current_direction(record.template get<HumanVector>(0x20));
            state.set(0x160,record.template get<HumanVector>(0x20));
        }
        return;
    }
    state.set(0x18e,std::uint8_t{0});
    if(kind==1||kind==2){
        state.set(0x202,std::uint8_t{1});
        order.require_live_target();auto position=record.template get<HumanVector>(0x30);
        if(order.target){
            const auto matrix=frames.actor_world_matrix(order.target->base().frame());
            position={{matrix[12],matrix[13],matrix[14],matrix[15]}};
        }
        ai.initialize_task(nullptr,52,position,record.template get<HumanVector>(0x20),true,.5f);
        state.set(0x204,std::uint8_t{1});state.set(0x201,std::uint8_t{1});
        state.set(0x174,state.template get<std::uint32_t>(0x174)&~1u);ai.pending_first_mode(kind==1?10u:11u);
    }else if(kind==3){
        order.require_live_target();world.dispatch_active_pending_command(ai,record.template get<std::uint32_t>(0x10),order.target,record.template get<std::uint32_t>(0x70));
    }else if(kind==4){ai.regroup(true);return;}
    else if(kind==6||kind==7){state.set(0x202,std::uint8_t{1});ai.pending_first_mode(kind==6?13u:14u);}
    auto flags=record.template get<std::uint32_t>(0x44);
    if(!(flags&7u)){flags|=4u;record.set(0x44,flags);}state.set(0x178,flags&7u);
    if(!(flags&0x1f000u)){flags|=0x4000u;record.set(0x44,flags);}state.set(0x180,flags&0x1f000u);
    if(!(flags&0x700u)){flags|=0x200u;record.set(0x44,flags);}
    const auto mind=flags&0x700u;
    ai.words().set(0x50,(ai.words().template get<std::uint32_t>(0x50)&0xfffe08ffu)|mind);
    ai.resources().path().movement_flags(state.template get<std::uint32_t>(0x180));
    ai.pending_second_mode(mind&0x100u?11u:mind&0x200u?10u:9u);
    const auto posture=state.template get<std::uint32_t>(0x178);
    if(posture!=(ai.words().template get<std::uint32_t>(0x50)&posture)){
        ai.pending_first_mode(posture&1u?16u:posture&2u?17u:18u);ai.pending_posture(posture);
    }
}
//005282E0. Completion appends its hold BEFORE retiring the first order; kind4
//suppresses hold. Process pool scalar storage outlives returned order leases.
template<class AI,class Frames,class World>bool process_human_active_pending_orders(AI& ai,
        std::uint32_t elapsed,Frames& frames,World& world){
    auto& state=ai.state();
    if(!ai.has_pending_orders_loading()){state.set(0x178,0u);state.set(0x180,0u);return false;}
    if(state.template get<std::uint8_t>(0x18e)){
        ai.first_pending_order().state.set(0x49,std::uint8_t{0});state.set(0x18e,std::uint8_t{0});
    }
    start_human_active_pending_order(ai,elapsed,frames,world);
    auto& order=ai.first_pending_order();auto& record=order.state;
    if(!record.template get<std::uint8_t>(0x49)){
        ai.activate_pending_order(order,ai.pending_orders());record.set(0x49,std::uint8_t{1});
    }
    if(record.template get<std::uint8_t>(0x48)){
        const auto kind=record.template get<std::uint32_t>(0xc);
        if(ai.pending_orders().size()==1&&kind!=4){
            auto* human=ai.human();if(!human)throw std::logic_error("Pending completion lost Human44");
            HumanVector direction;
            if(kind==1)direction=record.template get<HumanVector>(0x20);
            else{
                const auto matrix=frames.actor_world_matrix(human->base().frame());direction={{matrix[8],matrix[9],matrix[10],matrix[11]}};
                normalize_human_ai_shot(direction,(static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[2])*direction[2]);
            }
            //A fresh second model read follows normalization, including W.
            const auto matrix=frames.actor_world_matrix(human->base().frame());
            ai.append_hold_order({{matrix[12],matrix[13],matrix[14],matrix[15]}},direction);
        }
        ai.retire_pending_order(0,false);
    }
    return true;
}
//00528950's selected ordinary on-foot tasks, plus delayed/inactive gates.
//Occupied and specialized action families remain required. This does not
//activate the containing00526550 authority frame.
template<class AI>bool advance_human_active_pending_move(AI& ai){
    auto& state=ai.state();auto* human=ai.human();
    if(!human)throw std::logic_error("Active move task lost Human44");
    switch(state.template get<std::uint32_t>(0x220)){
    case 0:case 1:case 2:case 3:case 4:case 5:case 14:case 15:case 19:
    case 27:case 28:case 33:case 34:case 38:case 52:break;
    default:throw std::runtime_error("Active task requires its specialized00528950 action-family receivers");
    }
    if(human->field160)throw std::runtime_error("Active task requires its H8.4 occupied00528950 path");
    if(!state.template get<std::uint8_t>(0x205)&&!state.template get<std::uint8_t>(0x200)){
        state.set(0x205,std::uint8_t{1});ai.pending_clear_item_ready(); //0058C400, before query/174/direction.
        ai.resources().query().request_position(state.template get<HumanVector>(0x1e0));
        state.set(0x174,state.template get<std::uint32_t>(0x174)&~1u);
        const auto position=ai.human()->base().world_position();const auto destination=state.template get<HumanVector>(0x1e0);
        auto direction=state.template get<HumanVector>(0x160);
        for(unsigned axis=0;axis!=3;++axis){direction[axis]=static_cast<float>(static_cast<long double>(position[axis])-destination[axis]);state.set(0x160+axis*4,direction[axis]);}
        //Original temporary W is unwritten; retain its source lane without an
        //invented native value. XYZ length spills BEFORE the unit comparison.
        const float square=static_cast<float>((static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[2])*direction[2]);
        normalize_human_ai_shot(direction,square);
        for(unsigned axis=0;axis!=3;++axis)state.set(0x160+axis*4,direction[axis]);
        ai.pending_first_mode(state.template get<std::uint8_t>(0x188)?5u:4u);return true;
    }
    if(state.template get<std::uint8_t>(0x205)&&!state.template get<std::uint8_t>(0x200)){
        const auto position=ai.pending_model_position();const auto destination=state.template get<HumanVector>(0x1e0);
        const auto x=static_cast<long double>(destination[0])-position[0],y=static_cast<long double>(destination[1])-position[1],z=static_cast<long double>(destination[2])-position[2];
        const float square=static_cast<float>((y*y+z*z)+x*x);
        const auto radius=state.template get<float>(0x21c);const float radius_square=static_cast<float>(static_cast<long double>(radius)*radius);
        ai.require_live_task_target();
        if((ai.task_target&&ai.pending_target_arrived())||square<radius_square){
            auto& query=ai.resources().query().state;query.set(0x50,query.template get<std::uint32_t>(0x50)|0x2000u);state.set(0x200,std::uint8_t{1});
        }
        return true; //Arrival and completion are separate native frames.
    }
    if(!state.template get<std::uint8_t>(0x201))return turn_and_start_active_task(ai);
    if(state.template get<std::uint8_t>(0x206))return true;
    if(const auto posture=state.template get<std::uint32_t>(0x218);posture){
        if(posture!=(state.template get<std::uint32_t>(0x17c)&ai.words().template get<std::uint32_t>(0x50)))ai.pending_posture(posture);
        state.set(0x218,0u);
    }
    for(const auto mask:{8u,0x200u})if(state.template get<std::uint32_t>(0x174)&mask)state.set(0x174,state.template get<std::uint32_t>(0x174)&~mask);
    if(!state.template get<std::uint8_t>(0x203))state.set(0x202,std::uint8_t{1});
    state.set(0x206,std::uint8_t{1});
    ai.complete_pending_task(state.template get<std::uint32_t>(0x220)); //Actual00535C00, including pending/standalone gates.
    return true;
}
//00528950, delayed/inactive prefix before the selected move-task decision.
template<class AI>bool advance_human_active_pending_task(AI& ai,std::uint32_t elapsed){
    auto& state=ai.state();
    if(state.template get<std::uint8_t>(0x18d))return true;
    if(const auto old=state.template get<std::uint32_t>(0x214);old){
        const auto next=old-elapsed;state.set(0x214,next);
        if(human_frame_signed_word(next)>0)return true;
        if(human_frame_signed_word(next)<0)state.set(0x214,0u);
    }
    if(!state.template get<std::uint8_t>(0x202))return advance_human_active_pending_move(ai);
    const auto posture=state.template get<std::uint32_t>(0x17c);
    if(posture&&posture!=(ai.words().template get<std::uint32_t>(0x50)&posture)){
        ai.pending_posture(posture);state.set(0x17c,0u);
    }
    if(!(state.template get<std::uint32_t>(0x174)&0x20u))return false;
    if(!state.template get<std::uint32_t>(0x19c)&&!state.template get<std::uint32_t>(0x198)){
        ai.set_target_direction(state.template get<HumanVector>(0x160));ai.set_current_direction(state.template get<HumanVector>(0x160));
    }
    return true;
}
} //namespace hd2::game
#endif
