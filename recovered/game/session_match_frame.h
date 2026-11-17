#ifndef HD2_GAME_SESSION_MATCH_FRAME_H
#define HD2_GAME_SESSION_MATCH_FRAME_H
#include "game/human_state_frame_timers.h"
#include <cstdint>
namespace hd2::game {
//Original process BSS, not a per-session counter copy. Startup owns the single
//instance; loading/replacing a World must not restart these accumulators.
struct SessionMatchFrameProcess {
    std::uint32_t hazard_008ae588=0,rule_008ae5d4=0,sync_008ae5d8=0,sequence_008ae5d0=0;
};
template<class State,class Services>void advance_nonoffline_fixed_frame(State& state,Services& q,std::uint32_t elapsed){
    if(q.frame_console_enabled())while(auto command=q.take_frame_console_command())q.process_frame_console_command(*command);
    auto& w=state.words;
    if(w.template get<std::uint32_t>(0x88)==3){
        if(q.frame_rule(0x238)*1000u<w.template get<std::uint32_t>(0x28)){
            if(q.frame_rule(0x224)!=2u&&!q.frame_remote_ai_enabled())q.frame_reset_scores();
            w.set(0x88,0u);
            if(q.mode_flags()&2u)q.frame_reset_local_suppression();
            q.frame_clear_flag_actor_states();
            w.set(0x30,0u);
            if(!q.frame_rule(0x27c))q.frame_prepare_peer_lives();
            else{const auto counts=q.frame_live_team_counts();w.set(0x84,0u-counts[0]-counts[1]);}
            //00731C00 is genuine RET.
            q.frame_queue_command(11,0,0,0);q.frame_choose_objective_actor();
        }
        //Still the originally entered phase3 arm, even after88 changed above.
        if(w.template get<std::uint32_t>(0x28)==100u)q.frame_begin_match_countdown();
    }else{
        if(w.template get<std::uint32_t>(0x88)==2u&&w.template get<std::uint32_t>(0x8c)+5000u<w.template get<std::uint32_t>(0x28)){
            //00731C10 is genuine RET; DWORD round increment precedes signed compare.
            const auto round=w.template get<std::uint32_t>(0x74)+1u;w.set(0x74,round);
            if(human_frame_signed_word(round)<human_frame_signed_word(q.frame_rule(0x234)))q.frame_restart_round();
            else q.frame_rotate_map(nullptr);
        }
        q.frame_evaluate_rules(elapsed);q.frame_advance_sync(elapsed);
    }
}
template<class State,class Services>void advance_nonoffline_variable_frame(State& state,Services& q,std::uint32_t elapsed){
    q.frame_advance_hazards(elapsed);
    auto& w=state.words;
    if(w.template get<std::uint32_t>(0x88)==3){
        const auto remaining=q.frame_rule(0x238)*1000u-w.template get<std::uint32_t>(0x28);
        q.frame_countdown_hint(0x1ceau,human_frame_signed_word(remaining)<0?0u:remaining);
    }else{
        if(q.frame_rule(0x224)&&human_frame_signed_word((q.frame_rule(0x23c)+1u)*1000u)<w.template get<std::int32_t>(0x7c)&&q.frame_rule(0x240)){
            w.set(0x7c,w.template get<std::uint32_t>(0x30)%1000u);
            q.frame_queue_command(16,0,0,0);
        }
        if(!(q.mode_flags()&1u)&&q.frame_connection_status()==9u)q.frame_connection_warning(0x92bu,1000);
    }
    if(w.template get<std::uint32_t>(0x88)==4u&&w.template get<std::uint32_t>(0x8c)+4000u<w.template get<std::uint32_t>(0x28)){
        w.set(0x88,0u);q.frame_defer_network_close();q.frame_current_application_status(11);
    }
    if(w.template get<std::uint32_t>(0x88)==1u&&w.template get<std::uint32_t>(0x8c)+4000u<w.template get<std::uint32_t>(0x28)){
        w.set(0x8c,w.template get<std::uint32_t>(0x28));w.set(0x88,2u);
        if(q.mode_flags()&2u){q.frame_show_info(true);q.frame_set_hint(-1);} //0049F020 is INFO6B3A0, not scoreboard.
    }
    q.frame_control_heartbeat(elapsed);
    const auto pending=w.template get<std::uint32_t>(0x90);
    if(pending&&q.frame_first_prepared_player_valid()){
        if(pending==1u)q.frame_queue_request(0,0,0);
        else if(pending==2u)q.frame_queue_request(1,0,0);
        w.set(0x90,0u); //Unknown nonzero pending values also clear; invalid first player retains it.
    }
    if((q.mode_flags()&1u)&&!state.pending_frame_peers.empty()){
        const auto pending_spawn=state.pending_frame_peers.back();
        if(!pending_spawn)throw std::logic_error("Session pending spawn lost its original transferred record");
        q.frame_queue_pending_spawn(pending_spawn);
        state.pending_frame_peers.pop_back(); //AFTER successful enqueue, exactly one LIFO entry.
    }
}
template<class Services>void advance_session_match_sync(SessionMatchFrameProcess& process,Services& q,std::uint32_t elapsed){ //00699320.
    process.sync_008ae5d8+=elapsed;
    if(human_frame_signed_word(process.sync_008ae5d8)>4999){process.sync_008ae5d8=0;q.frame_queue_command(8,0,0,0);}
}
}
#endif
