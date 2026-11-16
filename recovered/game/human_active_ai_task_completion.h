#ifndef HD2_GAME_HUMAN_ACTIVE_AI_TASK_COMPLETION_H
#define HD2_GAME_HUMAN_ACTIVE_AI_TASK_COMPLETION_H
#include "game/human_joint_owners.h"
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
//00535C00.202 publishes BEFORE the real pending gate. Selected standalone
//ordinary/default and attachment/follow stops are bound; combat/cover policy
//retains its actual required call boundary and earlier198 store.
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
        throw std::runtime_error("Active task completion requires actual005276E0 decisions/cover posture and00537E50 policy");
    case 41:throw std::runtime_error("Active task completion requires actual00537E50 policy");
    default:resume_human_active_task_follow(ai);break;
    }
    auto* human=ai.human();if(!human)throw std::logic_error("Active task completion log lost Human44");
    world.active_task_completion_debug_name(*human); //005A81C0 precedes genuine retail0069BFE0 RET.
}
} //namespace hd2::game
#endif
