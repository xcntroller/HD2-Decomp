#ifndef HD2_GAME_HUMAN_ACTIVE_AI_TASK_COMPLETION_H
#define HD2_GAME_HUMAN_ACTIVE_AI_TASK_COMPLETION_H
#include "game/human_joint_owners.h"
#include "game/human_active_ai_cautious.h"
#include "game/human_ai_cover.h"
namespace hd2::game {
//005376F0 is NOT005377F0: no follow enable, delay, direction, notification
//or formation change. Missing leader preserves B05C after mode/174 publication.
template<class AI>void resume_human_active_task_follow(AI& ai){
    auto& state=ai.state();if(!state.template get<std::uint8_t>(0x189))return;
    ai.pending_first_mode(state.template get<std::uint8_t>(0x188)!=0);
    state.set(0x174,state.template get<std::uint32_t>(0x174)&~0x21u);
    auto* human=ai.human();if(!human)throw std::logic_error("Active task follow lost Human44");
    auto* leader=human->formation_leader;if(!leader)return;
    if(state.template get<std::uint8_t>(0x188)){ai.words().set(0x5c,0x1000u);return;}
    const auto a=leader->base().world_position(),b=human->base().world_position();
    const auto x=static_cast<long double>(a[0])-b[0],y=static_cast<long double>(a[1])-b[1],z=static_cast<long double>(a[2])-b[2];
    //No FLOAT spill before the native ordered <100 test; equality/NaN are far.
    ai.words().set(0x5c,((z*z+y*y)+x*x)<100.f?0x2000u:0x4000u);
}
//00537E50: held174 bit20 is an actual no-op gate. Direct output loans from
//00525C90 publish BEFORE its final leader query, even when that query fails.
template<class AI,class World>void hold_human_active_task(AI& ai,bool immediate,bool notify,World& world){
    auto& state=ai.state();const auto flags=state.template get<std::uint32_t>(0x174);if(flags&0x20u)return;
    const auto validate=world.human_common_receiver_guard(*ai.human());state.set(0x174,(flags&~1u)|0x20u);
    const auto action=state.template get<std::uint32_t>(0x220);
    if(!state.template get<std::uint8_t>(0x202)&&(action==40||action==41||action==42)){ai.cancel_task();validate();}
    auto direction=state.template get<HumanVector>(0x160);auto posture=state.template get<std::uint32_t>(0x17c);
    select_active_cautious_posture(ai,direction,posture,world,[&](const auto& value,std::uint32_t selected){state.set(0x17c,selected);state.set(0x160,value);});validate();
    if(state.template get<std::uint8_t>(0x189)){ai.disable_follow_loading(immediate);validate();}
    ai.pending_first_mode(state.template get<std::uint8_t>(0x188)?7u:6u);validate();
    state.set(0x160,ai.current_direction());ai.pending_posture(state.template get<std::uint32_t>(0x17c));validate();
    auto* human=ai.human();if(human->field160&&human->field164==0){ai.resources().path().stop_loading();validate();}
    if(notify)notify_human_active_ai_empty_orders(ai,world);
}
//00535C00 completion cover/posture. Without leader, selected Player is queried;
//without either AND without A4, native retains the previous17C value.
template<class AI,class World>void select_active_completion_posture(AI& ai,World& world){
    auto& state=ai.state();auto* leader=ai.human()->formation_leader;if(!leader)leader=world.active_selected_player();
    if(leader){
        if((world.active_follow_state(*leader).words().template get<std::uint32_t>(0x50)&1u)&&ai.words().template get<std::int32_t>(0xb0)>5000){state.set(0x17c,1u);return;}
        if(world.active_follow_state(*leader).words().template get<std::uint32_t>(0x50)&2u){state.set(0x17c,2u);return;}
    }
    if(const auto* point=ai.point()){
        const auto mask=human_ai_cover::direction(state.template get<HumanVector>(0x160));
        state.set(0x17c,(point->reserved17[1]&mask)?4u:(point->reserved17[0]&mask)?2u:1u);
    }
}
//00535C00.202 publishes BEFORE the real pending gate; cover decisions precede
//direction snapshot, posture, then hold. These reuse the containing-frame owners.
template<class AI,class World>void complete_human_active_task(AI& ai,std::uint32_t action,World& world){
    auto& state=ai.state();state.set(0x202,std::uint8_t{1});
    if(ai.has_pending_orders_loading())return;
    const auto stop=[&]{ai.disable_follow_loading(state.template get<std::uint8_t>(0x188)!=0);};
    switch(action){
    case 3:
        ai.require_live_task_target();
        if(ai.task_target){const auto kind=ai.task_target->base().type();if(kind==4||kind==20||kind==21){stop();break;}}
        resume_human_active_task_follow(ai);break;
    case 8:case 9:case 10:case 11:case 12:case 13:case 30:stop();break;
    case 32:case 40:case 42:
        state.set(0x198,0u);
        world.active_completion_cover(ai);
        state.set(0x160,ai.current_direction());select_active_completion_posture(ai,world);
        {const auto validate=world.human_common_receiver_guard(*ai.human());ai.pending_posture(state.template get<std::uint32_t>(0x17c));validate();}
        [[fallthrough]];
    case 41:hold_human_active_task(ai,state.template get<std::uint8_t>(0x188)!=0,false,world);break;
    default:resume_human_active_task_follow(ai);break;
    }
    auto* human=ai.human();if(!human)throw std::logic_error("Active task completion log lost Human44");
    world.active_task_completion_debug_name(*human); //005A81C0 precedes genuine retail0069BFE0 RET.
}
} //namespace hd2::game
#endif
