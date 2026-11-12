#ifndef HD2_GAME_HUMAN_ACTIVE_AI_TASK_STATE_H
#define HD2_GAME_HUMAN_ACTIVE_AI_TASK_STATE_H
#include "game/human_joint_owners.h"

namespace hd2::game {
//00534340 receives ECX=Active+1E0, NOT the whole Active.224 remains a typed
//borrower.208/209 are BYTE stores; neighboring unwritten lanes must survive.
template<class AI,class Actor>void initialize_human_active_ai_task(AI& ai,Actor* target,
        std::uint32_t action,const HumanVector& position,const HumanVector& direction,bool enabled,float radius){
    auto& state=ai.state();state.set(0x220,action);
    for(auto offset:{0x201u,0x202u,0x200u})state.set(offset,std::uint8_t{0});
    ai.task_target=target;state.set(0x1e0,position);state.set(0x1f0,direction);
    state.set(0x214,0u);state.set(0x20c,std::int32_t{-1});state.set(0x205,std::uint8_t{0});
    if(target)state.set(0x20c,target->base().id10());
    state.set(0x206,std::uint8_t{0});state.set(0x203,std::uint8_t(enabled));state.set(0x21c,radius);
    for(auto offset:{0x207u,0x204u,0x208u})state.set(offset,std::uint8_t{0});
    state.set(0x218,0u);state.set(0x210,0u);state.set(0x209,std::uint8_t{0});
}
//00535730. The action37/512 masks REPLACE174 when nonzero, rather than
//merely clearing an individual action flag. Formation restore runs even idle.
template<class AI,class World>void cancel_human_active_ai_task(AI& ai,World& world){
    auto& state=ai.state();
    if(!state.template get<std::uint8_t>(0x202)){
        auto& query=ai.resources().query().state;
        const auto flags=query.template get<std::uint32_t>(0x50);
        if(flags&0x10000000u)query.set(0x50,flags|0x2000u);
        auto* human=ai.human();if(!human)throw std::logic_error("Active task cancellation requires Human44");
        if(human->field160&&human->field164==0)ai.resources().path().stop_loading();
        auto active=state.template get<std::uint32_t>(0x174)&~1u;state.set(0x174,active);
        const auto action=state.template get<std::uint32_t>(0x220);
        const auto mask=action==37?active&8u:action==512?active&0x200u:0u;
        if(mask)state.set(0x174,mask);
        state.set(0x202,std::uint8_t{1});state.set(0x206,std::uint8_t{1});
    }
    ai.words().set(0x50,ai.words().template get<std::uint32_t>(0x50)&~0x700u);
    world.restore_active_order_formation(ai);
}
//00538390: the empty-list notification owns actual process delayed speech.
template<class AI,class World>void notify_human_active_ai_empty_orders(AI& ai,World& world){
    if(!ai.has_pending_orders_loading())world.active_order_completion(ai.human());
}
//005377F0, including full positive follow. Native squared distance retains
//x87 differences and sums (Z*Z+Y*Y)+X*X, accepting unordered as well as >=100 (native TEST AH,5/JNP).
template<class AI,class World>void follow_human_active_ai(AI& ai,bool enabled,bool immediate,World& world){
    auto& state=ai.state();const auto old=state.template get<std::uint8_t>(0x189);
    state.set(0x188,std::uint8_t(immediate));state.set(0x189,std::uint8_t(enabled));
    if(!enabled){
        state.set(0x1b0,0u);state.set(0x160,ai.current_direction());ai.pending_first_mode(immediate?3u:2u);return;
    }
    if(!state.template get<std::uint8_t>(0x202)){
        const auto action=state.template get<std::uint32_t>(0x220);
        bool cancel=action==22||action==25||action==26||action==40||action==41||action==42;
        if(action==3&&!ai.has_pending_orders_loading()&&ai.task_target){
            const auto kind=ai.task_target->base().type();cancel=kind==4||kind==21;
        }
        if(cancel)cancel_human_active_ai_task(ai,world);
    }
    ai.pending_first_mode(state.template get<std::uint8_t>(0x188)!=0);
    state.set(0x174,state.template get<std::uint32_t>(0x174)&~0x21u);
    if(state.template get<std::uint8_t>(0x202)&&!ai.has_pending_orders_loading()&&!immediate&&old)state.set(0x1b0,4500u);
    auto* human=ai.human();if(!human)throw std::logic_error("Active follow requires Human44");
    if(auto* leader=human->formation_leader){
        if(state.template get<std::uint8_t>(0x188)){
            ai.words().set(0x5c,0x1000u);notify_human_active_ai_empty_orders(ai,world);return;
        }
        const auto a=leader->base().world_position(),b=human->base().world_position();
        const auto x=static_cast<long double>(a[0])-b[0],y=static_cast<long double>(a[1])-b[1],z=static_cast<long double>(a[2])-b[2];
        if(!((z*z+y*y)+x*x<100.f)){
            ai.words().set(0x5c,0x4000u);notify_human_active_ai_empty_orders(ai,world);return;
        }
    }
    ai.words().set(0x5c,0x2000u);notify_human_active_ai_empty_orders(ai,world);
}
//005357E0 selects Player28 BEFORE cancellation/reset. Unlike0052F0E0, its
//formation setter is called even when selected is NULL. Tail notification is
//unconditional, in addition to the one reached inside positive follow.
template<class AI,class World>void regroup_human_active_ai(AI& ai,bool reset,World& world){
    auto* selected=world.active_selected_player();auto& state=ai.state();
    if(!reset){
        const auto action=state.template get<std::uint32_t>(0x220);
        if(!state.template get<std::uint8_t>(0x202)&&(action==40||action==41||action==42))cancel_human_active_ai_task(ai,world);
        world.set_active_order_formation(ai,selected);
    }else if(ai.has_pending_orders_loading()){
        ai.reset_orders_loading();cancel_human_active_ai_task(ai,world);ai.refresh_pending_action_mode();
        ai.pending_first_mode(state.template get<std::uint8_t>(0x188)!=0);ai.pending_first_mode(19);
        world.set_active_order_formation(ai,selected);
    }else{
        follow_human_active_ai(ai,selected!=nullptr,state.template get<std::uint8_t>(0x188)!=0,world);
        notify_human_active_ai_empty_orders(ai,world);return;
    }
    auto* human=ai.human();if(!human)throw std::logic_error("Active regroup requires Human44");
    if(auto* leader=human->formation_leader)if(leader->formation)leader->formation->mark_member_changed(human);
    follow_human_active_ai(ai,selected!=nullptr,state.template get<std::uint8_t>(0x188)!=0,world);
    notify_human_active_ai_empty_orders(ai,world);
}
} //namespace hd2::game
#endif
