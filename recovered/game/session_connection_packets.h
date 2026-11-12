#ifndef HD2_GAME_SESSION_CONNECTION_PACKETS_H
#define HD2_GAME_SESSION_CONNECTION_PACKETS_H
#include "frontend/control_startup.h"
namespace hd2::game {
//0072C360: ignored context, payload/length, logical channel and queue flags.
//Copy into the SAME process430 FIFO; decoder/World dispatch happens later.
template<class LocalPackets>void deliver_session_connection_packet(std::uint32_t mode,
        const std::vector<unsigned char>& bytes,std::uint32_t channel,std::uint32_t flags,LocalPackets& packets){
    packets.enqueue((mode&1u)!=0,bytes,(channel<<5)|flags);
}

//0072D0E0. Queue submission is separate from worker flush/socket delivery. The
//authority branch reads6C BEFORE74, and has no active08 or local-peer filter.
template<class Pump,class Locks>void submit_session_connection_packet(frontend::ControlStartupState& state,
        std::uint32_t mode,std::uint32_t target,const std::vector<unsigned char>& bytes,
        std::uint32_t flags,Pump& pump,Locks& locks){
    const auto channel=(flags>>5)|((flags&0x80000000u)?0xf8000000u:0u),index=flags&3u;
    if(!(mode&1u)){
        if(state.client334)(void)state.client334->submit(bytes,channel,index,pump);
        return;
    }
    locks.enter_control_lock(0);
    struct Unlock{Locks& locks;~Unlock(){locks.leave_control_lock(0);}}unlock{locks};
    for(std::size_t slot=0;slot<state.records.size();++slot){
        if(target!=0xffffffffu&&slot!=target)continue;
        auto* connection=state.records.at(slot).connection();
        if(connection&&state.records.at(slot).word(0x74)==2)(void)connection->submit(bytes,channel,index,pump);
    }
}
} //namespace hd2::game
#endif
