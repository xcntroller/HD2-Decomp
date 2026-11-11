#ifndef HD2_GAME_SESSION_RECEIVED_APPLICATION_H
#define HD2_GAME_SESSION_RECEIVED_APPLICATION_H
#include "game/session_packet_stream.h"
#include "game/human_damage_messages.h"
namespace hd2::game {
//00728450 presentation cases reached by Human death/disconnect. Application
//envelopes precede the loaded-World gate. Authority00727800 has a DIFFERENT
//grammar; never replay these payloads through that switch.
template<class Runtime>void receive_session_application_packet(std::uint32_t kind,
        SessionPacketStream& packet,bool authority,Runtime& runtime){
    if(authority)throw std::runtime_error("Application packet requires actual authority request decoder");
    switch(kind){
    case 2:{
        const auto transport=packet.read(32);
        if(!(runtime.application_mode_flags()&1u)&&runtime.has_application_game()){
            if(runtime.application_peer_name(transport)){
                const auto label=runtime.application_localize(0x22cb);
                const auto name=runtime.application_peer_name(transport); //Native second lookup AFTER localization.
                if(!name)throw std::runtime_error("Departure peer name disappeared during localization");
                const auto message=format_human_damage_arguments("%s %s",{{*name,label}});
                runtime.emit_application_log(message);
            }
            runtime.receive_application_peer_departure(transport); //Late current-game lookup, then0069E720.
        }
        return;
    }
    case 10:{
        const auto identity=packet.read(5),status=packet.read(4);
        if(!(runtime.application_mode_flags()&1u)&&runtime.has_application_game())
            runtime.receive_application_objective(identity,status); //0069BC40, no score or outbound echo.
        return;
    }
    case 17:{
        const auto reason=packet.read(32),identity=packet.read(32);
        if(runtime.has_application_game()&&identity==static_cast<std::uint32_t>(runtime.application_local_peer()))
            runtime.receive_application_local_removal(reason); //00699A20 BEFORE current mode is queried again.
        if(!(runtime.application_mode_flags()&1u))runtime.receive_application_peer_removal(reason,identity);
        return;
    }
    default:throw std::runtime_error("Required application presentation command is not reconstructed");
    }
}
//00699A20 writes result/time even for unknown reason; localized hint is conditional.
template<class Session,class Hint>void receive_local_session_removal(Session& session,std::uint32_t reason,Hint& hint){
    session.words.set(0x88,std::uint32_t{4});session.words.set(0x8c,session.words.template get<std::uint32_t>(0x28));
    constexpr std::array<std::uint32_t,4> labels{{0x22a7,0x22a6,0x22a9,0x22a8}};
    if(reason<labels.size())hint.set_hint(static_cast<std::int32_t>(labels[reason]));
}
}
#endif
