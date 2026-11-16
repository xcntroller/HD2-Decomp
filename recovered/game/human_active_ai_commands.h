#ifndef HD2_GAME_HUMAN_ACTIVE_AI_COMMANDS_H
#define HD2_GAME_HUMAN_ACTIVE_AI_COMMANDS_H
#include "game/human_active_ai_task_state.h"
#include <vector>
#include <utility>
namespace hd2::game {
//00459180. Native swaps ONLY Human pointers; the original distance array is
//never swapped. A conventional sort changes the order for three+ candidates.
template<class Human,class Frames>void order_active_command_candidates(std::vector<Human*>& humans,
        const HumanVector& target,Frames& frames){
    std::vector<float> distances;distances.reserve(humans.size());
    for(auto* human:humans){
        if(!human)throw std::logic_error("Players command selection lost Human");
        const auto m=frames.actor_world_matrix(human->base().frame());
        const auto x=static_cast<long double>(target[0])-m[12],y=static_cast<long double>(target[1])-m[13],z=static_cast<long double>(target[2])-m[14];
        distances.push_back(static_cast<float>((z*z+y*y)+x*x));
    }
    for(std::size_t at=0;at<humans.size();++at){
        auto best=at;auto distance=distances[at];
        for(auto n=at;n<humans.size();++n)if(distances[n]<distance){distance=distances[n];best=n;}
        std::swap(humans[at],humans[best]);
    }
}
//0045BD10 selected ordinary policies. Sort/model reads precede action policy,
//even for unsupported/default commands. Reached specialized policies are
//required; they must not masquerade as an ineligible Human.
template<class Human,class Actor,class Frames,class World>Human* select_active_command_candidate(
        std::vector<Human*>& humans,std::uint32_t action,Actor& target,std::uint32_t secondary,Frames& frames,World& world){
    if(humans.empty())return nullptr;
    const auto m=frames.actor_world_matrix(target.base().frame());
    order_active_command_candidates(humans,{{m[12],m[13],m[14],m[15]}},frames);
    switch(action){
    case 0:case 1:
        for(auto* human:humans)if(world.active_command_has_definition(*human,world.active_command_definition(target,action)))return human;
        return nullptr;
    case 4:case 0x22:{
        const auto definition=world.active_command_definition(target,action);
        for(auto* human:humans)if(world.active_command_has_definition(*human,definition))return human;
        return nullptr;
    }
    case 0x10:
        for(auto* human:humans)if(world.active_command_has_definition(*human,0x3bu))return human;
        return nullptr;
    case 2:case 5:case 8:case 9:case 10:case 11:case 12:case 13:case 14:
    case 0x13:case 0x1b:case 0x1c:case 0x21:case 0x26:return humans.front();
    case 3:
        if(target.base().type()!=21||!world.active_command_seat_occupied(target,0))return humans.front();
        return nullptr;
    case 15:
        for(auto* human:humans)if(static_cast<Actor*>(human)!=&target&&world.active_self_treatment_item(*human))return human;
        return nullptr;
    case 6:
        if(target.base().type()==3){for(auto* human:humans)if(!human->field174)return human;return nullptr;}
        if(target.base().type()==9)return humans.front();
        if(target.base().type()!=11)return nullptr;
        [[fallthrough]];
    case 18:case 22:case 24:case 25:case 26:case 37:
        return world.select_special_active_command_candidate(humans,action,target,secondary);
    default:return nullptr; //Native switch has no accepted candidate.
    }
}
//0045BB20. Car3 uses forward erase with unconditional increment (a shifted
//neighbor is skipped). Exit5 filters backwards;21/23/32 preserve the whole list.
template<class Human,class Actor,class Frames,class World>void filter_active_command_candidates(
        std::vector<Human*>& humans,std::uint32_t action,Actor* target,std::uint32_t secondary,Frames& frames,World& world){
    const auto require_target=[&]() -> Actor& {if(!target)throw std::runtime_error("Players command dereferences missing native target");return *target;};
    if(action==3){
        const auto kind=require_target().base().type();if(kind==8)return;
        if(kind==4){for(std::size_t n=0;n<humans.size();++n)if(humans[n]->field160==target)humans.erase(humans.begin()+static_cast<std::ptrdiff_t>(n));return;}
    }
    if(action==3||action==6)if(require_target().base().type()==9&&!world.active_command_supply_kind(*target))return;
    if(action==5){
        const bool focus=require_target().base().type()==3;
        for(auto n=humans.size();n;){--n;if((focus?humans[n]->field174:humans[n]->field160)!=target)humans.erase(humans.begin()+static_cast<std::ptrdiff_t>(n));}
        return;
    }
    if(action==21||action==23||action==32)return;
    if(humans.empty())return; //0045BD10 tests the list before dereferencing target20.
    auto* selected=select_active_command_candidate(humans,action,require_target(),secondary,frames,world);
    humans.clear();if(selected)humans.push_back(selected);
}
//00534620. Target model position is read before the Human's cached direction;
//00534340 publishes the actual task before optional empty-order notification.
template<class AI,class Actor,class Frames,class World>void start_active_target_task(AI& ai,Actor& target,
        std::uint32_t action,bool enabled,float radius,bool notify,Frames& frames,World& world){
    const auto m=frames.actor_world_matrix(target.base().frame());
    auto* human=ai.human();if(!human)throw std::logic_error("Active target task lost Human44");
    const auto direction=human->base().world_direction();
    ai.initialize_task(&target,action,{{m[12],m[13],m[14],m[15]}},direction,enabled,radius);
    if(notify)notify_human_active_ai_empty_orders(ai,world);
}
template<class AI,class World>void delay_active_focused_task(AI& ai,World& world){
    auto* human=ai.human();if(!human)throw std::logic_error("Active task focus phase lost Human44");
    if(!human->field174)return;
    world.active_command_exit_focus(ai,ai.state().template get<std::int32_t>(0x20c));
    ai.state().set(0x214,1000u); //Only AFTER actual00587540 returns.
}
template<class AI,class Actor,class Frames,class World>void produce_active_medical_task(AI& ai,Actor& target,Frames& frames,World& world){ //00535630.
    if(world.active_medical_target_health(target)<=0||!world.active_self_treatment_item(*ai.human())){
        world.reject_active_command(*ai.human());return;
    }
    start_active_target_task(ai,target,15u,false,1.5f,true,frames,world);delay_active_focused_task(ai,world);
}
//Selected ordinary00458E50 task producers. Full task execution is separate;
//unsupported producers throw before claiming publication or completion.
template<class AI,class Actor,class Frames,class World>void produce_active_player_command(AI& ai,
        std::uint32_t action,Actor* target,std::uint32_t secondary,Frames& frames,World& world){
    (void)secondary;
    const auto reject=[&]{world.reject_active_command(*ai.human());};
    const auto start=[&](bool enabled=false,float radius=1.5f){
        if(!target)throw std::runtime_error("Active task producer dereferences missing native target");
        start_active_target_task(ai,*target,action,enabled,radius,true,frames,world);
    };
    switch(action){
    case 0:case 1:case 28:case 33:start();return; //00534840/4860/48F0/43E0.
    case 2:case 14:case 38:start();delay_active_focused_task(ai,world);return;
    case 3:{
        if(!target)throw std::runtime_error("Active entry task requires native target");
        const auto kind=target->base().type();const bool enabled=kind==4||kind==8||kind==20||kind==21;
        start(enabled,kind==20?3.5f:enabled?2.5f:1.5f);delay_active_focused_task(ai,world);return;
    }
    case 5:
        if(!ai.human()->field160&&!ai.human()->field174){reject();return;}
        start();ai.state().set(0x200,std::uint8_t{1});ai.state().set(0x207,std::uint8_t{1});return;
    case 27:
        if(!target)throw std::runtime_error("Active Door task requires native target");
        if(!world.active_command_door_allowed(*ai.human(),*target)){reject();return;}start();return;
    case 34:
        if(!target)throw std::runtime_error("Active document task requires native target");
        if(!world.active_command_document_allowed(*ai.human(),*target)){reject();return;}start();return;
    case 15:
        if(!target)throw std::runtime_error("Active medical task dereferences native NULL target");
        produce_active_medical_task(ai,*target,frames,world);return;
    case 4:case 6:case 8:case 9:case 10:case 11:case 12:case 13:case 16:
    case 19:case 22:case 23:case 24:case 25:case 26:case 32:case 37:
        world.produce_special_active_player_command(ai,action,target,secondary);return;
    default:return; //Native unhandled command still publishes188 in the caller.
    }
}
//00458E50 after actual explicit-Human/group4 selection. Save first BEFORE
//filtering; rejected speech uses that Human, not the filtered/sorted receiver.
template<class Human,class Actor,class Frames,class World>void dispatch_active_player_commands(std::vector<Human*>& humans,
        std::uint32_t action,Actor* target,bool immediate,std::uint32_t secondary,Frames& frames,World& world){
    auto* first=humans.empty()?nullptr:humans.front();
    filter_active_command_candidates(humans,action,target,secondary,frames,world);
    if(humans.empty()){if(first)world.reject_active_command(*first);return;}
    if(action==18||action==21||(action==3&&target&&target->base().type()==8)){
        world.produce_active_group_command(humans,action,target,immediate);return;
    }
    for(auto n=humans.size();n;){auto& ai=world.active_command_state(*humans[--n]);ai.state().set(0x188,std::uint8_t(immediate));
        produce_active_player_command(ai,action,target,secondary,frames,world);
    }
}
} //namespace hd2::game
#endif
