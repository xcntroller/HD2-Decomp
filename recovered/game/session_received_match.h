#ifndef HD2_GAME_SESSION_RECEIVED_MATCH_H
#define HD2_GAME_SESSION_RECEIVED_MATCH_H
#include "game/session_activation.h"
#include "game/session_peer_records.h"
#include "game/session_match_application.h"
#include "game/human_state_frame_timers.h"
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
//0059F010 is manual and has different gates from automatic0059FE30.
template<class Human,class Runtime>void request_settled_human_respawn(Human& human,Runtime& r){
    if(human_frame_signed_word(human.field204)<=3000)return;
    auto* control=r.human_inline(human).control();if(!control)throw std::logic_error("Manual respawn lost actual Human DD0");
    auto& words=control->physical().control_frame_words();
    if(words.template get<std::int32_t>(0x1ac)!=0)return;
    if(words.template get<std::int32_t>(0x9c0)!=0)return;
    r.request_human_respawn();
}
//00699900, complete coordinator; actual menu objects remain on their process
//application owner. Two0069E620 calls have DIFFERENT receivers and ordering.
template<class Runtime>void finish_session_match_warmup(SessionActivationState& state,Runtime& r){
    state.words.set(0x30,0u);r.frame_disable_peer_bodies(false);r.frame_set_hint(-1);
    if(auto* human=r.frame_explicit_local_human()){
        auto* selected=r.frame_human_state(*human);if(!selected)throw std::logic_error("Warmup lost actual local Human B0");
        r.frame_suppress_state(*selected,false);
    }
    state.words.set(0x88,0u);state.words.set(0xb8,0u);
    if(r.mode_flags()&2u)if(r.frame_local_human()){
        if(r.frame_first_clothing_respawn_ready()){r.frame_queue_request(1,0,0);return;}
        const auto kind=r.frame_match_application().current_kind();if(!kind)throw std::logic_error("Warmup requires actual current application state");
        if(*kind!=11u){r.frame_match_application().install_retaining_game_menu(-2);r.frame_match_application().open_original_game_menu(state,-2);}
        r.frame_match_application().current_menu_respawn_pending(true);
    }
    //0069BFE0 retail logger is genuine RET; no new successful logger service.
}
//00699810, received side list finished. Local Human is kept across selection,
//but its B0 borrower is queried AFTER the Players callback as native does.
template<class Runtime>void receive_session_match_countdown(SessionActivationState& state,Runtime& r){
    r.frame_show_info(false);r.frame_set_hint(0x1cea);r.frame_disable_peer_bodies(true);
    if(auto* human=r.frame_local_human()){
        r.frame_select_human(*human);auto* selected=r.frame_human_state(*human);
        if(!selected)throw std::logic_error("Countdown lost actual local Human B0");
        r.frame_suppress_state(*selected,true);
    }
    r.frame_set_spawn_phase(2);state.words.set(0x88,3u);state.words.set(0x8c,state.words.get<std::uint32_t>(0x28));
}
//00728450 case16: clock update precedes absent/alive Human returns. A valid
//first clothing record chooses manual0059F010 even when that routine returns.
template<class Runtime>void receive_session_spawn_wave(SessionActivationState& state,Runtime& r){
    auto* human=r.frame_local_human();if(!(r.mode_flags()&1u))state.words.set(0x7c,state.words.get<std::int32_t>(0x30)%1000);
    if(!human||r.frame_human_health(*human)>0)return;
    if(r.frame_first_clothing_respawn_ready()){r.frame_manual_human_respawn(*human);return;}
    const auto kind=r.frame_match_application().current_kind();if(kind&&*kind==11u)r.frame_match_application().current_menu_respawn_pending(true);
}
//00728450 case12 is process/application-owned BEFORE its final current-World
//countdown. In particular, side changes cannot require a pre-existing Frame.
template<class Packet,class Runtime>void receive_session_match_sides(frontend::ControlStartupState& records,Packet& packet,Runtime& r){
    for(auto peer=packet.read(6);peer!=0x21u;peer=packet.read(6)){
        const auto side=packet.read(2);
        if(peer==static_cast<std::uint32_t>(r.application_local_peer())){
            if(r.application_live_squad().empty())throw std::logic_error("Side receiver requires actual first live soldier");
            if(side!=static_cast<std::uint32_t>(r.application_live_squad().front().specialization)){
                const auto kind=r.match_application().current_kind();if(!kind)throw std::logic_error("Side receiver requires actual current application state");
                if(*kind==1u){auto& soldier=r.application_live_squad().front();soldier.specialization=static_cast<std::int32_t>(side);soldier.clear_equipment();r.match_application().install_retaining_game_menu(-2);}
                else if(*kind==11u)r.match_application().change_current_menu_team(side);
            }
        }
        set_session_peer_side(records,r,peer,side,false);set_session_peer_side(records,r,peer,side,true);
    }
    if(r.has_application_game())r.receive_current_frame_control(12,0);
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
