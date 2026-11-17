#ifndef HD2_GAME_SESSION_MATCH_PEERS_H
#define HD2_GAME_SESSION_MATCH_PEERS_H
#include "frontend/control_startup.h"
#include "game/human_state_frame_timers.h"
#include <array>
namespace hd2::game {
template<class Locks,class Call>decltype(auto) with_session_match_peers(frontend::ControlStartupState& state,Locks& locks,Call&& call){
    locks.enter_control_lock(0);
    struct Unlock{Locks& locks;~Unlock(){locks.leave_control_lock(0);}}unlock{locks};
    return call(state.records);
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
