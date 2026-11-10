#ifndef HD2_FRONTEND_CONTROL_ACTOR_RECORDS_H
#define HD2_FRONTEND_CONTROL_ACTOR_RECORDS_H

#include "frontend/control_startup.h"

namespace hd2::frontend {

//0072EC70/0072ED30 borrow SAME control1DC records under its418 critical
//section. No record copy, fabricated player roster or unlocked pointer escape.
//The callers retain original mode gates; bounds reject native invalid indexing.
template<class Locks,class Write>
void write_control_actor_record(ControlStartupState& state,Locks& locks,std::uint32_t identity,Write&& write) {
    locks.enter_control_lock(0);
    struct Unlock {Locks& locks;~Unlock(){locks.leave_control_lock(0);}} unlock{locks};
    if(identity>=state.records.size())throw std::out_of_range("Actor has no actual control record");
    write(state.records[identity]);
}
template<class Locks>
void clear_retired_actor_record(ControlStartupState& state,Locks& locks,std::int32_t identity) { //00476860.
    if(identity<0)throw std::out_of_range("Retired actor has a negative control identity");
    write_control_actor_record(state,locks,static_cast<std::uint32_t>(identity),[](ControlStartupRecord& record){record.byte(0xa2,0);});
}
template<class Locks>
void clear_human_player_slot(ControlStartupState& state,Locks& locks,std::uint32_t identity) { //0059E3E0.
    if(identity>=32)throw std::out_of_range("Human has no reserved control identity");
    write_control_actor_record(state,locks,identity,[](ControlStartupRecord& record){record.word(0x3c,0);});
}
} //namespace hd2::frontend
#endif
