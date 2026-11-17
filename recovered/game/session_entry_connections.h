#ifndef HD2_GAME_SESSION_ENTRY_CONNECTIONS_H
#define HD2_GAME_SESSION_ENTRY_CONNECTIONS_H
#include "frontend/control_startup.h"
namespace hd2::game {

//00730A40. This is entry generation publication, not a transport-ready test or
//queue reset. An absent actual wrapper fails AFTER the peer AC mutation, just
//where the native pointer dereference occurs. Re-entry toggles again.
template<class Locks>void toggle_session_entry_connections(frontend::ControlStartupState& state,
        std::uint32_t mode,std::int32_t local_peer,Locks& locks){
    if(!(mode&1u)){
        const auto slot=static_cast<std::uint32_t>(local_peer);
        auto& peer=state.records.at(slot);
        peer.word(0xac,peer.word(0xac)^1u);
        if(!state.client334)throw std::logic_error("Client entry requires its actual334 connection owner");
        state.client334->generation(state.records.at(slot).word(0xac));
        return; //No418 lock on the client branch.
    }
    locks.enter_control_lock(0);
    struct Unlock{Locks& locks;~Unlock(){locks.leave_control_lock(0);}}unlock{locks};
    for(std::size_t slot=0;slot<state.records.size();++slot){
        auto& peer=state.records.at(slot);
        if(!peer.byte(8).value()||peer.word(0x74)!=2)continue;
        peer.word(0xac,peer.word(0xac)^1u);
        auto* connection=state.records.at(slot).connection(); //Fresh vector/record lookup, native+6C.
        if(!connection)throw std::logic_error("Authority entry requires its actual peer6C connection owner");
        connection->generation(state.records.at(slot).word(0xac));
    }
}
} //namespace hd2::game
#endif
