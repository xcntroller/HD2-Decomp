#ifndef HD2_GAME_HUMAN_ACTIVE_AI_GROUP_H
#define HD2_GAME_HUMAN_ACTIVE_AI_GROUP_H
#include "game/human_active_ai_commands.h"
#include "game/human_active_ai_target_modes.h"
namespace hd2::game {
namespace human_active_group {
inline HumanVector add(HumanVector a,const HumanVector& b){for(unsigned n=0;n!=3;++n)a[n]=static_cast<float>(static_cast<long double>(a[n])+b[n]);return a;}
inline HumanVector scale(HumanVector a,float b){for(unsigned n=0;n!=3;++n)a[n]=static_cast<float>(static_cast<long double>(a[n])*b);return a;}
inline HumanVector subtract(HumanVector a,const HumanVector& b){for(unsigned n=0;n!=3;++n)a[n]=static_cast<float>(static_cast<long double>(a[n])-b[n]);return a;}
inline long double square(const HumanVector& a){return (static_cast<long double>(a[2])*a[2]+static_cast<long double>(a[1])*a[1])+static_cast<long double>(a[0])*a[0];}
inline HumanVector cross_up(const HumanVector& a){return {{static_cast<float>(static_cast<long double>(a[1])*0.f-a[2]),static_cast<float>(static_cast<long double>(a[2])*0.f-static_cast<long double>(a[0])*0.f),static_cast<float>(static_cast<long double>(a[0])-static_cast<long double>(a[1])*0.f),a[3]}};}
template<class Position>HumanVector grenade_destination(const HumanMatrix& model,const HumanVector& previous,Position&& current){ //00457396..004574F4.
    long double x=static_cast<long double>(model[12])-previous[0];float y=0,z=static_cast<float>(static_cast<long double>(model[14])-previous[2]);
    const auto extended=x*x+static_cast<long double>(z)*z;
    if(!(std::abs(extended-1.f)<9.99999993922529e-9f)){
        const float stored=static_cast<float>(extended);
        if(!(stored<9.99999993922529e-9f)){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));x*=inverse;y=static_cast<float>(0.L*inverse);z=static_cast<float>(static_cast<long double>(z)*inverse);}
        else if(x!=0)x=x<0?-1.f:1.f;else if(z!=0)z=z<0?-1.f:1.f;else y=1.f;
    }
    //Only Z*7 spills before origin addition; X/Y retain extended products.
    const float z_offset=static_cast<float>(static_cast<long double>(z)*7.f);auto position=current();
    position[0]=static_cast<float>(x*7.f+position[0]);position[1]=static_cast<float>(static_cast<long double>(y)*7.f+position[1]);position[2]=static_cast<float>(static_cast<long double>(z_offset)+position[2]);return position;
}
}
//0045F280 selects from the FIRST qualifying reverse roster member, not the
//combined enemy set0045F3A0. A singleton still consumes CRT rand.
template<class Human,class Frames,class World>auto* select_active_group_shared_threat(Human& human,Frames& frames,World& world){
    using Actor=std::remove_reference_t<decltype(*human.perception.lists()[0].front())>;
    auto& roster=world.human_roster();
    for(auto n=roster.size();n;){auto* other=roster.at(static_cast<std::int32_t>(--n));
        if(!other)throw std::runtime_error("Group posture lost actual roster Human");
        if(other==&human||human_frame_signed_word(other->properties.stats_ec[0])<=0)continue;
        const auto a=frames.actor_world_matrix(human.base().frame()),b=frames.actor_world_matrix(other->base().frame());
        const auto x=static_cast<long double>(b[12])-a[12],y=static_cast<long double>(b[13])-a[13],z=static_cast<long double>(b[14])-a[14];
        if(!((z*z+y*y)+x*x>1600.f)){const auto& enemies=other->perception.lists()[0];if(!enemies.empty())return enemies.at(world.human_frame_random_word()%enemies.size());} //Native AND4100 also admits unordered.
    }
    return static_cast<Actor*>(nullptr);
}
//00537B60: real own enemies before shared roster lookup; postures BEFORE5C.
template<class AI,class Frames,class World>void select_active_group_move_posture(AI& ai,Frames& frames,World& world){
    const auto validate=world.human_common_receiver_guard(*ai.human());
    bool danger=!ai.human()->perception.lists()[0].empty();
    if(!danger)danger=select_active_group_shared_threat(*ai.human(),frames,world)!=nullptr;
    validate();auto& state=ai.state();const auto flags=ai.words().template get<std::uint32_t>(0x50);
    const auto timer=state.template get<std::int32_t>(0x19c);std::uint32_t posture=0;
    if(flags&1u){
        const auto& target=ai.resources().target();const auto delta=target.state.template get<HumanVector>(0x10);
        const auto square=(static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[1])*delta[1])+static_cast<long double>(delta[2])*delta[2];
        if(danger&&(state.template get<std::uint32_t>(0x174)&6u)&&!target.actor&&square<1.e-16f)posture=1;
        else posture=timer>999||danger?2u:4u;
    }else if(flags&2u){if(timer<=999&&!danger)posture=4;}
    else if((flags&4u)&&(timer>=1001||danger))posture=2;
    if(posture){ai.pending_posture(posture);validate();}
    ai.words().set(0x5c,state.template get<std::uint8_t>(0x188)?0x1000u:0x4000u);
}
//005354F0: the task's208/214/204 stores precede mode and posture callbacks.
template<class AI,class Frames,class World>void start_active_group_move(AI& ai,const HumanVector& position,std::uint32_t delay,bool notify,Frames& frames,World& world){
    const auto validate=world.human_common_receiver_guard(*ai.human());
    ai.initialize_task(nullptr,52u,position,ai.human()->base().world_direction(),true,.5f);validate();
    auto& state=ai.state();state.set(0x208,std::uint8_t(notify));state.set(0x214,delay);state.set(0x204,std::uint8_t{1});
    ai.pending_first_mode(state.template get<std::uint8_t>(0x188)?5u:4u);validate();select_active_group_move_posture(ai,frames,world);
}
//00535910 queries ready holster kind9; it does NOT equip it. The far_position task
//reuses actual ranged aim/animation in00528950, then notifies empty orders.
template<class AI,class World>void start_active_group_grenade(AI& ai,const HumanVector& position,World& world){
    const auto validate=world.human_common_receiver_guard(*ai.human());
    if(!world.active_group_grenade_available(*ai.human())){world.reject_active_command(*ai.human());return;}
    //Native differences remain extended through this comparison, no FLOAT spills.
    const auto own=ai.human()->base().world_position();const auto x=static_cast<long double>(own[0])-position[0],y=static_cast<long double>(own[1])-position[1],z=static_cast<long double>(own[2])-position[2];
    if(!((z*z+y*y)+x*x>36.f)){world.reject_active_command(*ai.human());return;}
    ai.initialize_task(nullptr,37u,position,ai.human()->base().world_direction(),false,1.5f);
    validate();
    auto& state=ai.state();state.set(0x200,std::uint8_t{1});state.set(0x174,(state.template get<std::uint32_t>(0x174)&~0x10u)|8u);
    delay_active_focused_task(ai,world);validate();notify_human_active_ai_empty_orders(ai,world);validate();
}
//00457820 owns DISTINCT F8 command members. It never borrows a group4C..8C
//buffer as storage. Native temporary vector W lanes are unproduced: typed math
//retains its input lane without assigning native W-value equivalence.
template<class Process,class Humans,class Actor,class Frames,class World>void produce_active_group_tasks(Process& process,const Humans& incoming,
        std::uint32_t action,Actor* actor,bool immediate,Frames& frames,World& world){
    using namespace human_active_group;auto& members=process.command_members();process.clear_command_members();members.insert(members.end(),incoming.begin(),incoming.end());
    if(members.empty())return;
    auto& state=process.words();state.set(0xa0,action);if(action!=3&&action!=18&&action!=21)return;
    const auto limit=action==18?1u:action==21?2u:members.size();while(members.size()>limit)members.pop_back();
    state.set(0xa4,0u);state.set(0xa8,static_cast<std::uint32_t>(members.size()));
    process.capture_command_members(world);
    if(!actor)throw std::runtime_error("Group producer dereferences native NULL target after member publication");
    auto& target=*actor;
    const auto live=world.active_event_actor_guard(target);(void)frames.actor_world_matrix(target.base().frame());live();
    if(action!=3)process.command_target(&target,live); //Native F0 store only in18/21.
    auto& navigation=world.active_task_navigation();const auto references=navigation.actor_references(&target);
    if(action!=3&&references.size()!=2)return;
    if(references.size()<2)throw std::runtime_error("Group3 reaches native missing navigation-reference dereference");
    const auto point=[&](std::uint16_t index){const auto p=navigation.position(index);return HumanVector{{p[0],p[1],p[2],0}};};
    const auto first=point(references[0]),second=point(references[1]),origin=world.active_group_reference_origin();
    const float a=static_cast<float>(square(subtract(origin,first))),b=static_cast<float>(square(subtract(origin,second)));
    const bool near_first=a<b;const auto near_position=near_first?first:second,far_position=near_first?second:first;
    auto direction=subtract(near_position,far_position);if(action==3)direction[1]=0;direction=active_target_unit(direction,square(direction));
    auto side=cross_up(direction);side=active_target_unit(side,square(side));
    if(action==3){state.set(0xe0,scale(direction,-1.f));state.set(0xd0,far_position);}
    else {state.set(0xb0,far_position);state.set(0xc0,scale(direction,-1.f));}
    if(action==18&&state.template get<std::uint32_t>(0xa8)>1u)state.set(0xa8,2u);
    for(auto n=members.size();n;){const auto index=--n;process.validate_command_member(index);auto* member=members.at(index);if(!member)throw std::runtime_error("Group producer lost actual command member");
        auto& ai=world.active_command_state(*member);ai.state().set(0x188,std::uint8_t(immediate));
        if(index>2)continue;
        HumanVector position=near_position;
        if(action==3){
            if(index==1)position=add(near_position,scale(direction,2.5f));
            else if(index==2)position=add(near_position,scale(subtract(direction,side),2.5f));
            else position=add(add(near_position,scale(direction,1.5f)),scale(side,1.600000023841858f));
        }else if(action==21){
            position=add(near_position,index?scale(index==1?add(side,direction):subtract(direction,side),1.5f):direction);
        }else{
            position=index?add(near_position,scale(direction,1.5f)):add(add(near_position,scale(side,.6000000238418579f)),scale(direction,.5f));
        }
        start_active_group_move(ai,position,0,true,frames,world);live();if(action==18&&!index)ai.state().set(0x21c,.20000000298023224f);
    }
}
//00456F00 after native prefix publishes A4 (or A0=-1 for18). Called on the
//SAME retained command-member owner; a callback may not silently erase borrowers.
template<class Process,class Frames,class World>void finish_active_group_arrival(Process& process,std::uint32_t action,Frames& frames,World& world){
    using namespace human_active_group;auto& state=process.words();auto& members=process.command_members();
    if(action==18){
        for(auto n=members.size();n;){const auto index=--n;process.validate_command_member(index);auto* human=members.at(index);if(!human)throw std::runtime_error("Group arrival lost actual command member");auto& ai=world.active_command_state(*human);
            if(index>1)continue;
            if(!index)ai.state().set(0x202,std::uint8_t{1});
            auto& target=process.require_command_target();const auto phase=world.active_group_door_phase(target);
            if(phase==1||phase==2){
                ai.state().set(0x202,std::uint8_t{1});const auto own=human->base().world_position();const auto model=frames.actor_world_matrix(target.base().frame());process.validate_command_target();
                const auto position=grenade_destination(model,own,[&]{return human->base().world_position();});
                start_active_group_grenade(ai,position,world);
            }else if(!index){
                auto position=ai.state().template get<HumanVector>(0x1e0);for(unsigned axis=0;axis!=3;++axis)position[axis]=0; //Native temporary W unproduced.
                ai.initialize_task(&target,27u,position,human->base().world_direction(),false,.5f);ai.state().set(0x200,std::uint8_t{1});ai.state().set(0x208,std::uint8_t{1});state.set(0xa0,18u);state.set(0xa8,1u);
            }else world.reject_active_command(*human);
        }
        return;
    }
    const auto origin=state.template get<HumanVector>(action==3?0xd0:0xb0),direction=state.template get<HumanVector>(action==3?0xe0:0xc0),side=cross_up(direction);
    for(auto n=members.size();n;){const auto index=--n;process.validate_command_member(index);auto* human=members.at(index);if(!human)throw std::runtime_error("Group arrival lost actual command member");if(index>2)continue;
        auto delta=index?(index==1?add(direction,side):subtract(direction,side)):direction;
        delta=scale(delta,action==3?2.f:1.5f);const auto position=add(origin,delta);
        const std::uint32_t delay=!index?0u:action==3?(index==1?4000u:8000u):(index==1?3000u:4000u);
        start_active_group_move(world.active_command_state(*human),position,delay,false,frames,world);
    }
    state.set(0xa0,0xffffffffu);
}
} //namespace hd2::game
#endif
