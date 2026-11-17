#ifndef HD2_GAME_SESSION_RECEIVED_MATCH_H
#define HD2_GAME_SESSION_RECEIVED_MATCH_H
#include "game/session_activation.h"
namespace hd2::game {
//00699590: received result is independent from authority rule evaluation.
template<class Runtime>void receive_session_match_result(SessionActivationState& state,std::uint32_t result,Runtime& r){
    if(r.frame_rule(0x224)==0||r.frame_rule(0x224)==3){
        state.words.set(0x88,2u);r.frame_show_info(true);r.frame_set_hint(-1);
    }else{
        state.words.set(0x88,1u);r.frame_hide_map();
        const auto second=r.frame_team_win(1),first=r.frame_team_win(0); //Original reads0097D4CC BEFORE D4C8.
        r.frame_result_hint(result?0x1cecu:0x1cebu,first,second);
    }
    state.words.set(0x8c,state.words.get<std::uint32_t>(0x28)); //AFTER UI receivers, fresh time.
}
//00728450 case6: complete configuration replacement precedes current-game
//lookup and0069B9D0; a decoder failure must not issue application status9.
template<class Packet,class Runtime>void receive_session_configuration(Packet& packet,Runtime& r){
    r.read_game_network_map(packet);if(r.has_application_game())r.receive_current_frame_control(6,0);
}
//0072DAC0: one literal-string formatting argument, no second percent pass.
template<class Runtime>void receive_session_vote_result(bool accepted,Runtime& r){
    const auto text=r.application_localize(accepted?0x22e3u:0x22e4u);
    r.append_application_vote_text(text,{{50,150,50}});
}
}
#endif
