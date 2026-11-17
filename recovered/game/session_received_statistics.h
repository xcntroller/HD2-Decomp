#ifndef HD2_GAME_SESSION_RECEIVED_STATISTICS_H
#define HD2_GAME_SESSION_RECEIVED_STATISTICS_H
#include "game/session_peer_statistics_wire.h"
#include "game/session_match_peers.h"
namespace hd2::game {
//0069AED0 retains whole signed seconds from existing7C, replaces the remainder
//with new30%1000; additions preserve x86 DWORD wrap rather than signed UB.
template<class State>void receive_session_round_clock(State& state,std::uint32_t value){
    auto& w=state.words;const auto seconds=w.template get<std::int32_t>(0x7c)/1000;
    w.set(0x30,value);w.set(0x7c,static_cast<std::uint32_t>(seconds)*1000u+
        static_cast<std::uint32_t>(human_frame_signed_word(value)%1000));
}
//0072F680.418 covers peer rows/time/lives only; team statistics follow unlock.
//No duplicated process records, World clock or rank/Human callback ownership.
template<class Packet,class Runtime>void read_session_match_statistics(frontend::ControlStartupState& state,
        Packet& packet,Runtime& r){
    std::uint32_t round_time=0,game_time=0,losses=0;
    with_session_match_peers(state,r,[&](auto& peers){
        for(auto peer=packet.read(6);peer!=0x21u;peer=packet.read(6)){
            std::array<std::uint32_t,9> discarded{};
            const bool apply=peer<peers.size()&&peers.at(peer).byte(8).value()&&peers.at(peer).human_actor()&&!(r.application_mode_flags()&1u);
            read_session_peer_statistics(packet,[&](unsigned index,std::uint32_t value){
                if(apply)peers.at(peer).word(0x40+index*4,value);else discarded.at(index)=value;
            });
        }
        round_time=packet.read(32);game_time=packet.read(32);
        if(r.received_statistics_remote_ai()){
            if(!r.received_statistics_shared_lives()){
                for(unsigned n=0;n!=6;++n){const auto value=packet.read(7);
                    if(!(r.application_mode_flags()&1u))peers.at(n).word(0xb0,value==127u?0xffffffffu:value);}
            }else{const auto value=packet.read(16);losses=(value&0x8000u)?value|0xffff0000u:value;}
        }
    });
    if(r.has_application_game()&&game_time&&round_time&&!(r.application_mode_flags()&1u))
        r.receive_application_statistics_clock(round_time,game_time,losses);
    for(unsigned team=0;team!=2;++team){
        std::array<std::uint32_t,9> discarded{};const bool apply=!(r.application_mode_flags()&1u);
        read_session_peer_statistics(packet,[&](unsigned index,std::uint32_t value){
            (apply?state.team_statistics464.at(team):discarded).at(index)=value;
        });
    }
    for(unsigned team=0;team!=2;++team){const auto value=packet.read(10);
        if(!(r.application_mode_flags()&1u))state.team_scores44c.at(team)=static_cast<float>(value);}
    for(unsigned team=0;team!=2;++team){const auto value=packet.read(8);
        if(!(r.application_mode_flags()&1u))state.team_wins458.at(team)=value;}
}
}
#endif
