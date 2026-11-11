#ifndef HD2_GAME_SESSION_PEER_RECORDS_H
#define HD2_GAME_SESSION_PEER_RECORDS_H
#include "frontend/control_startup.h"
namespace hd2::game {
template<class Locks>void set_session_peer_side(frontend::ControlStartupState& state,Locks& locks,
        std::uint32_t slot,std::uint32_t side,bool pending){ //00730140, bounds BEFORE418.
    if(slot>=state.records.size())return;
    locks.enter_control_lock(0);
    struct Unlock{Locks& locks;~Unlock(){locks.leave_control_lock(0);}}unlock{locks};
    state.records.at(slot).word(pending?0x68:0x64,side);
}
inline void* local_session_peer_human(const frontend::ControlStartupState& state,std::int32_t local){ //00730460, unsigned bounds, no lock/retain.
    const auto slot=static_cast<std::uint32_t>(local);
    return slot<state.records.size()?state.records[slot].human_actor():nullptr;
}
//00612A60's late publication. Scene identity is NOT this peer slot. Setup has
//already completed; keep418 across3C, multimedia ticks/A8 and auxiliary virtual8.
template<class Human,class Runtime>void publish_session_peer_human(frontend::ControlStartupState& state,
        std::uint32_t slot,Human& human,Runtime& runtime){
    if((runtime.mode_flags()&7u)==7u)return;
    runtime.enter_control_lock(0);
    struct Unlock{Runtime& runtime;~Unlock(){runtime.leave_control_lock(0);}}unlock{runtime};
    state.records.at(slot).human_actor(&human);
    const auto ticks=runtime.peer_human_ticks();state.records.at(slot).word(0xa8,ticks);
    if((runtime.mode_flags()&1u)&&runtime.human_remote_ai_enabled())
        runtime.append_peer_auxiliary_human(static_cast<Human*>(state.records.at(slot).human_actor()));
}
}
#endif
