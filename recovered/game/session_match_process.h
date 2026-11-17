#ifndef HD2_GAME_SESSION_MATCH_PROCESS_H
#define HD2_GAME_SESSION_MATCH_PROCESS_H
#include "game/human_state_frame_timers.h"
namespace hd2::game {
//0072D960 constructs the SAME process0097D540 (control+4D0), not a World
//counter. Unwritten native04 padding is excluded; only the written byte exists.
struct SessionMatchHeartbeat {
    std::uint32_t phase=0x42,counter08=0,word0c=0;
    std::uint8_t flag04=0;
};
template<class Queue>void stop_session_match_heartbeat(SessionMatchHeartbeat& state,std::uint32_t reason,Queue&& queue){ //0072DCA0.
    state.phase=0x42;queue(23,0,reason&0xffu,0); //Publication precedes process FIFO allocation/failure.
}
template<class Queue>void advance_session_match_heartbeat(SessionMatchHeartbeat& state,std::uint32_t mode,std::uint32_t elapsed,Queue&& queue){ //0072DCD0.
    if(state.phase==0x42)return;
    state.counter08+=elapsed;
    if((mode&1u)&&human_frame_signed_word(state.counter08)>15000)
        stop_session_match_heartbeat(state,0,queue); //Raw tail writes stack argument0 before jumping to DCA0.
}
inline bool session_connection_recent(std::uint32_t now,std::uint32_t worker_clock){ //007316F0: signed DWORD subtraction, equality600 remains recent.
    return human_frame_signed_word(now-worker_clock)<601;
}
}
#endif
