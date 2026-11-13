#ifndef HD2_GAME_HUMAN_SCRIPT_EVENT_OUTPUT_H
#define HD2_GAME_HUMAN_SCRIPT_EVENT_OUTPUT_H
#include "game/human_ai_event_queue.h"
#include "game/human_perception_records.h"
namespace hd2::game {
//0058FD30, one selected FIRST event from the containing reverse traversal.
//00582740 produces a perception record, not an AI action.00591F00 appends it to
//SAME Human2AC before B4 and reverse group broadcasts. No new list/pool/clock.
template<class State,class Queue,class Services>
void dispatch_human_script_event(State& state,Queue& queue,std::size_t index,Services& services){
    const auto* event=queue.at(index);
    if(!event)throw std::logic_error("Script event output lost its actual FIRST event");
    if(event->words[0]!=0x10000017u)return;
    auto* human=state.human();if(!human)throw std::logic_error("Script event output has no actual bound Human");
    const auto actor=services.human_ai_resolve_actor(event->words[4]); //005ACA50, signed handle.
    HumanVector position;std::memcpy(position.data(),event->words.data()+8,sizeof position);
    const auto stamp=services.human_script_event_ticks(); //timeGetTime BEFORE pool acquisition.
    auto record=services.human_perception_record_pool().acquire(2,position,actor,stamp);
    queue.retire_at(index); //SAME process event pool BEFORE erase, then actual2AC append.
    auto& actual=human->list2ac.append(std::move(record));
    services.human_script_event_own_callback(state,actual); //Original virtualB4 on THIS state.
    auto* command=state.command();
    if(!command)throw std::logic_error("Script event output lost its actual group command owner");
    const auto& members=command->lists[0]; //Existing command78 actual actor-identity list.
    for(auto n=members.size();n;)services.human_script_event_group_callback(members.at(--n),actual);
}
}
#endif
