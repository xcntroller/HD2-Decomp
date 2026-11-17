#ifndef HD2_GAME_HUMAN_ACTIVE_AI_EVENTS_H
#define HD2_GAME_HUMAN_ACTIVE_AI_EVENTS_H
#include "game/human_ai_event_queue.h"
#include "game/human_joint_owners.h"
namespace hd2::game {
//00528580, actual Active virtualF4. A returned lease retires through the same
//pool; a NULL return transfers it to the actual second processed-event queue.
template<class AI,class Frames,class World>void dispatch_active_frame_event(AI& ai,HumanAIEventPool::Lease event,Frames& frames,World& world){
    if(!event)throw std::logic_error("Active F4 lost its popped event");
    auto& state=ai.state();auto* human=ai.human();if(!human)throw std::logic_error("Active F4 lost Human44");
    const auto validate=world.human_common_receiver_guard(*human);
    const auto resolve=[&]{return world.human_ai_resolve_actor(event->words[4]);};
    const auto merge=[&](std::uint32_t lifetime){
        auto* command=ai.command();if(!command)throw std::logic_error("Active F4 lost actual command40");
        command->second.events.merge(std::move(event),lifetime,false);
    };
    const auto ready=[&]{if(ai.active_event_mode()==0){ai.active_event_action_mode(16);validate();}};
    const auto remember=[&]{
        if(!resolve())return;
        auto* actor=resolve();if(!actor)throw std::runtime_error("Active F4 repeated identity query lost Actor");
        if(!world.active_event_hostile(*human,actor))merge(10000u);
    };
    switch(event->words[0]){
    case 0x10000002u:state.set(0x19c,10000u);merge(3000u);return;
    case 0x10000003u:state.set(0x19c,5000u);event->words[3]=event->words[4]=0xffffffffu;merge(2000u);return;
    case 0x20000004u:case 0x20000012u:state.set(0x19c,10000u);return;
    case 0x30000001u:
        state.set(0x19c,16000u);if(resolve())(void)resolve();return;
    case 0x3000000bu:state.set(0x19c,5000u);remember();return;
    case 0x3000000cu:{
        if(resolve()){
            auto* actor=resolve();if(!actor)throw std::runtime_error("Active F4 repeated identity query lost Actor");
            if(actor->base().type()==3){
                actor=resolve();if(!actor)throw std::runtime_error("Active F4 health query lost Actor");
                if(world.active_medical_target_health(*actor)>0)if(auto* selected=world.active_event_selected(*actor)){
                    const auto selected_live=world.active_event_actor_guard(*selected);
                    const auto& own=frames.active_task_borrow_world_matrix(human->base().frame());validate();selected_live();
                    const auto& other=frames.active_task_borrow_world_matrix(selected->base().frame());validate();selected_live();
                    const auto x=static_cast<long double>(other[12])-own[12],y=static_cast<long double>(other[13])-own[13],z=static_cast<long double>(other[14])-own[14];
                    if(((z*z+y*y)+x*x)<900.f&&world.active_event_roster_member(*selected))ready();
                }
            }
        }
        state.set(0x19c,10000u);remember();return;
    }
    case 0x40000010u:ready();state.set(0x19c,15000u);merge(1000u);return;
    case 0x40000011u:
        ready();remember();if(!event)state.set(0x19c,15000u);return; //19C store AFTER successful merge.
    case 0x40000015u:{
        if(resolve()){
            auto* actor=resolve();if(!actor)throw std::runtime_error("Active F4 repeated identity query lost Actor");
            if(actor->base().type()==3){
                actor=resolve();if(!actor)throw std::runtime_error("Active F4 friendship query lost Actor");
                if(world.active_event_attacker_hostile(*actor,*human)){validate();world.notify_nearby_player_damage(*human);validate();}
            }
        }
        const auto mode=ai.active_event_mode();if(mode==0||mode==2){ai.active_event_action_mode(16);validate();}
        world.queue_active_ai_speech(ai,0x6cu,true,2000u,false);validate();
        state.set(0x19c,15000u);merge(6000u);return;
    }
    default:return;
    }
}
//00528570 delegates005A8480. Pop BEFORE virtualF4; actual processed leases
//must not be returned a second time by the caller. Newly enqueued events drain.
template<class AI,class Frames,class World>void drain_active_frame_events(AI& ai,Frames& frames,World& world){
    auto* command=ai.command();if(!command)throw std::logic_error("Active event drain lost actual Human command owner");
    const auto validate=world.human_common_receiver_guard(*ai.human());
    while(command->first.events.size()){
        auto event=command->first.events.pop_front();dispatch_active_frame_event(ai,std::move(event),frames,world);validate();
    }
}
} //namespace hd2::game
#endif
