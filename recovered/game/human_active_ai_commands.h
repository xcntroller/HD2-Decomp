#ifndef HD2_GAME_HUMAN_ACTIVE_AI_COMMANDS_H
#define HD2_GAME_HUMAN_ACTIVE_AI_COMMANDS_H
#include "game/human_active_ai_task_state.h"
#include "game/human_inventory_lookup.h"
#include "game/human_inventory_counts.h"
#include <vector>
#include <utility>
namespace hd2::game {
template<class Human,class Items>bool active_attack_special_available(Human& human,Items& items){ //0045BD10, all lookups BEFORE quantity tests.
    const auto first=find_human_inventory_definition(human,items,0x16),second=find_human_inventory_definition(human,items,7),third=find_human_inventory_definition(human,items,0x104);
    for(auto entry:{first,second,third})if(entry){auto& item=items.runtime_item(entry);
        if(item.amount()!=0.f||count_human_ammunition(human,items,item.definition().ammunition(),false))return true;
    }
    return false;
}
template<class Human,class Items,class Inventory>bool prepare_active_attack_special(Human& human,Items& items,Inventory& inventory){ //00535110, FIXED native source2.
    auto* storage=human.base().storage();if(!storage||!storage->holder(3))throw std::runtime_error("Active attack26 requires actual hand holder");
    const auto hand=storage->holder(3)->entry();auto selected=find_human_inventory_definition(human,items,0x16);
    if(!selected)selected=find_human_inventory_definition(human,items,7);
    if(!selected)selected=find_human_inventory_definition(human,items,0x104);
    if(!selected)return false;
    if(selected!=hand){
        if(hand)(void)inventory.swap(hand,3,human,selected,2);else (void)inventory.move(selected,human,2,3,0);
    }
    return true; //Native deliberately ignores false move/swap results.
}
//0045BD10 attack cases: prefer either direction of actual hostile membership,
//then retry all candidates.26 tests special ammunition on EACH pass.
template<class Humans,class Actor,class World>auto select_active_attack_candidate(Humans& humans,
        std::uint32_t action,Actor& target,World& world) -> typename Humans::value_type {
    for(unsigned phase=0;phase!=2;++phase){
        if(!phase&&target.base().type()!=3)continue;
        for(auto* human:humans){
            if(!phase&&!world.active_command_attack_pair(*human,target))continue;
            if(action!=26||world.active_command_special_available(*human))return human;
        }
    }
    return nullptr;
}
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
        //Native0045BD10 accepts ANY nonzero capacity result, including a
        //negative failure.005346C0 subsequently requires a positive result.
        for(auto* human:humans)if(world.active_loose_target_capacity(*human,target)!=0)return human;
        return nullptr;
    case 18:
        for(auto* human:humans)if(world.active_group_grenade_available(*human))return human;
        return nullptr;
    case 25:case 26:case 37:return select_active_attack_candidate(humans,action,target,world);
    case 24:return world.select_active_handoff_candidate(humans,target);
    case 22:
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
//005346C0, item/focus/supply producer. Occupied exit is AFTER task publication;
//its actual common-command/Car receiver remains owned by H8.4.
template<class AI,class Actor,class Frames,class World>void produce_active_pickup_task(AI& ai,Actor& target,std::uint32_t secondary,Frames& frames,World& world){
    auto* human=ai.human();if(!human)throw std::logic_error("Active pickup producer lost Human44");
    const auto kind=target.base().type();
    if(!((kind==11&&world.active_loose_target_capacity(*human,target)>0)||(kind==3&&!human->field174)||kind==9)){
        world.reject_active_command(*human);return;
    }
    const auto validate=world.human_common_receiver_guard(*human);
    start_active_target_task(ai,target,6u,false,1.f,true,frames,world);validate();
    if(human->field160){world.active_command_exit_attachment(ai);validate();ai.state().set(0x214,1000u);}
    if(kind==9)ai.state().set(0x210,secondary);
}
//00535050/00535110. The alternate producer is selected by saved188 even for
//command25. Inventory requests ignore their boolean; task publication follows.
template<class AI,class Actor,class Frames,class World>void produce_active_attack_task(AI& ai,Actor& target,
        bool alternate,Frames& frames,World& world){
    const auto validate=world.human_common_receiver_guard(*ai.human());
    if(world.active_task_actor_unavailable(target)){world.reject_active_command(*ai.human());return;}
    if(alternate){const bool found=world.prepare_active_attack_special_weapon(ai);validate();if(!found){world.reject_active_command(*ai.human());return;}}
    start_active_target_task(ai,target,alternate?26u:25u,false,1.f,true,frames,world);validate();
    auto& state=ai.state();state.set(0x174,state.template get<std::uint32_t>(0x174)&~4u);state.set(0x204,std::uint8_t{1});
    if(alternate){ai.active_event_action_mode(17);validate();}
    ai.pending_first_mode(10);validate();delay_active_focused_task(ai,world);validate();
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
    case 25:case 26:
        if(!target)throw std::runtime_error("Active attack producer dereferences native NULL target");
        produce_active_attack_task(ai,*target,action==26||ai.state().template get<std::uint8_t>(0x188),frames,world);return;
    case 6:
        if(!target)throw std::runtime_error("Active pickup producer dereferences native NULL target");
        produce_active_pickup_task(ai,*target,secondary,frames,world);return;
    case 23:
        if(!target)throw std::runtime_error("Active loot producer dereferences native NULL target");
        if(!world.active_loot_target_dead(*target)){reject();return;}start();return;
    case 4:
        if(!target)throw std::runtime_error("Active explosive producer dereferences native NULL target");
        if(!world.active_command_explosive_allowed(*ai.human(),*target)){reject();return;}
        start();delay_active_focused_task(ai,world);return;
    case 19:
        if(!target)throw std::runtime_error("Active release producer dereferences native NULL target");
        if(!world.active_command_release_allowed(*target)){reject();return;}
        start();delay_active_focused_task(ai,world);return;
    case 32:world.produce_active_formation_task(ai,target);return;
    case 37:
        if(!target)throw std::runtime_error("Active grenade producer dereferences native NULL target");
        world.produce_active_target_grenade(ai,*target);return;
    case 24:
        if(!target)throw std::runtime_error("Active handoff producer dereferences native NULL target");
        //00534BF0 really REJECTS type3 before005A6380; do not repair it.
        if(target->base().type()==3){reject();return;}
        world.produce_special_active_player_command(ai,action,target,secondary);return;
    case 8:case 9:case 10:case 11:case 12:case 13:case 16:case 22:
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
