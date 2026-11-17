#ifndef HD2_GAME_SESSION_TARGETED_PACKETS_H
#define HD2_GAME_SESSION_TARGETED_PACKETS_H
#include "game/session_peer_records.h"
#include "game/session_packet_stream.h"
namespace hd2::game {
//0072BF90: target is an external transport identity, NOT a peer slot. Clear
//active BEFORE the terminator/lookup/submission; reset only after submission.
template<class Backend>void finish_session_application_packet(SessionPacketStream& packet,
        frontend::ControlStartupState& records,Backend& backend){
    packet.word1c=0;if(!packet.word24)return;
    packet.write(19,5);
    const auto slot=find_session_peer_transport(records,backend,packet.word24);
    if(slot==static_cast<std::uint32_t>(backend.application_local_peer()))backend.enqueue_application_presentation(packet.written_bytes());
    else backend.send_control_connection_payload(slot,packet.written_bytes(),0x42);
    packet.position(0,0);
}
}
#endif
