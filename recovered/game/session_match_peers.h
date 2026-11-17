#ifndef HD2_GAME_SESSION_MATCH_PEERS_H
#define HD2_GAME_SESSION_MATCH_PEERS_H
#include "frontend/control_startup.h"
#include "game/human_state_frame_timers.h"
#include "frontend/profile_settings.h"
#include <array>
namespace hd2::game {
template<class Locks,class Call>decltype(auto) with_session_match_peers(frontend::ControlStartupState& state,Locks& locks,Call&& call){
    locks.enter_control_lock(0);
    struct Unlock{Locks& locks;~Unlock(){locks.leave_control_lock(0);}}unlock{locks};
    return call(state.records);
}
template<class Session,class Locks,class Rules>bool take_session_match_spawn_life(frontend::ControlStartupState& peers,Locks& locks,
        Session& session,std::uint32_t peer,Rules&& rules){ //0069D9C0 life branch, no active/name gate.
    return with_session_match_peers(peers,locks,[&](auto& records){
        const auto limit=rules(0x270);const bool shared=rules(0x27c)!=0; //Both AFTER418 acquisition.
        const auto current=shared?session.words.template get<std::uint32_t>(0x84):records.at(peer).word(0xb0);
        if(limit!=0xffffffffu&&human_frame_signed_word(limit)<=human_frame_signed_word(current))return false;
        if(shared)session.words.set(0x84,current+1u);else records.at(peer).word(0xb0,current+1u);
        return true;
    });
}
template<class Locks>std::array<std::uint32_t,2> count_session_match_teams(frontend::ControlStartupState& state,Locks& locks,bool pending,bool require_human){ //0072FF50.
    return with_session_match_peers(state,locks,[&](auto& peers){
        std::array<std::uint32_t,2> counts{};
        for(std::size_t n=0;n<peers.size();++n)if(peers[n].byte(8).value()){
            const auto side=peers[n].word(pending?0x68:0x64);
            if(side==0u){if(!require_human||peers[n].human_actor())++counts[0];}
            else if(side==1u&&(!require_human||peers[n].human_actor()))++counts[1];
        }
        return counts;
    });
}
//00730040 counts active08 only when transport00 is nonzero, under its OWN418.
//The following0072FF50 has a separate418 acquisition and no transport gate.
template<class Locks>std::uint32_t count_active_session_transports(frontend::ControlStartupState& state,Locks& locks){
    return with_session_match_peers(state,locks,[](auto& peers){
        std::uint32_t count=0;
        for(std::size_t n=0;n<peers.size();++n)if(peers[n].word(0)&&peers[n].byte(8).value())++count;
        return count;
    });
}
template<class Locks>void balance_session_match_peer_sides(frontend::ControlStartupState& state,Locks& locks,
        std::uint32_t unused,std::uint32_t percentage){ //007302D0. First stack argument is genuinely unread.
    (void)unused;
    const auto total=count_active_session_transports(state,locks);
    //Original x87: signed percentage * float0080F574, then signed total; __ftol EAX.
    const auto desired=frontend::settings_truncated_word(static_cast<long double>(human_frame_signed_word(total))*
        (static_cast<long double>(human_frame_signed_word(percentage))*0.01f));
    const auto counts=count_session_match_teams(state,locks,false,false);
    auto delta=desired-counts[0];
    if(delta==0u||delta==1u||delta==0xffffffffu)return;
    const bool positive=human_frame_signed_word(delta)>0;
    //No additional418 lock in native reverse mutation. Pending68 precedes fresh64.
    for(auto n=state.records.size();n;){--n;
        if(state.records.at(n).byte(8).value()&&state.records.at(n).word(0x64)==(positive?1u:0u)){
            const auto side=positive?0u:1u;state.records.at(n).word(0x68,side);
            delta+=positive?0xffffffffu:1u;state.records.at(n).word(0x64,side);
            if(positive?human_frame_signed_word(delta)<=0:human_frame_signed_word(delta)>=0)return;
        }
    }
}
template<class Locks>std::array<std::uint32_t,2> live_session_match_team_counts(frontend::ControlStartupState& state,Locks& locks){ //0072FE60, Human gate BEFORE side read.
    return with_session_match_peers(state,locks,[](auto& peers){
        std::array<std::uint32_t,2> counts{};
        for(std::size_t n=0;n<peers.size();++n)if(peers[n].byte(8).value()&&peers[n].human_actor()){
            const auto side=peers[n].word(0x64);if(side<2u)++counts[side];
        }
        return counts;
    });
}
template<class Locks>void prepare_session_match_peer_lives(frontend::ControlStartupState& state,Locks& locks){
    with_session_match_peers(state,locks,[](auto& peers){for(std::size_t n=0;n<peers.size();++n)peers[n].word(0xb0,0u-static_cast<std::uint32_t>(peers[n].human_actor()!=nullptr));});
}
template<class Locks>std::int32_t maximum_session_match_score(frontend::ControlStartupState& state,Locks& locks){ //0072ED40.
    return with_session_match_peers(state,locks,[](auto& peers){
        std::int32_t maximum=0;
        for(std::size_t n=0;n<peers.size();++n)if(peers[n].byte(8).value()&&peers[n].human_actor()){
            const auto score=human_frame_signed_word(peers[n].word(0x4c));if(maximum<score)maximum=score;
        }
        return maximum;
    });
}
template<class Locks>bool any_session_match_peer_below_limit(frontend::ControlStartupState& state,Locks& locks,std::uint32_t limit){
    return with_session_match_peers(state,locks,[&](auto& peers){
        bool found=false;for(std::size_t n=0;n<peers.size();++n)if(peers[n].byte(8).value()&&peers[n].human_actor()&&
            human_frame_signed_word(peers[n].word(0xb0))<human_frame_signed_word(limit))found=true;
        return found; //Native scans the remaining records even after finding one.
    });
}
template<class Locks>void publish_session_match_peer_sides(frontend::ControlStartupState& state,Locks& locks){ //007300D0.
    with_session_match_peers(state,locks,[](auto& peers){for(std::size_t n=0;n<peers.size();++n)if(peers[n].byte(8).value())peers[n].word(0x64,peers[n].word(0x68));});
}
}
#endif
