#ifndef HD2_GAME_SESSION_ROUND_RESET_H
#define HD2_GAME_SESSION_ROUND_RESET_H
#include "game/session_activation.h"
#include "game/session_match_peers.h"
namespace hd2::game {
//0069C390. The containing adapter supplies the SAME retained World/modules,
//process records, query wrappers and application. No reconstruction of a World
//and no pause toggle can substitute for any required reset receiver.
template<class Runtime>void reset_retained_session_round(SessionActivationState& state,bool restart,Runtime& r){
    if(state.status!=1&&state.words.template get<std::uint32_t>(0x1c)!=1)return;
    r.round_query_focus();
    for(auto offset:{0x10u,0x2cu,0x28u,0x30u})state.words.set(offset,0u);
    state.words.set(0x6f,std::uint8_t{0});state.words.set(0x70,std::uint8_t{0});state.words.set(0x90,0u);
    state.words.set(0xb4,0xffffffffu);state.words.set(0x35,std::uint8_t{0});
    state.words.set(0x88,(r.mode_flags()&7u)==7u?0u:3u);state.words.set(0xb8,1u);
    state.words.set(0x34,std::uint8_t{1});state.words.set(0x36,std::uint8_t{1});
    for(auto offset:{0x64u,0x94u,0x40u})state.words.set(offset,std::uint8_t{0});
    if(!r.round_shared_lives())prepare_session_match_peer_lives(r.round_peer_records(),r);
    else{const auto counts=live_session_match_team_counts(r.round_peer_records(),r);state.words.set(0x84,0u-(counts[0]+counts[1]));}
    if(!(r.mode_flags()&1u))state.words.set(0x78,0xffffffffu);
    while(!state.pending_frame_peers.empty())state.pending_frame_peers.pop_back();
    if(r.mode_flags()&2u){r.round_reset_hud();r.round_finish_dubbing();r.round_initialize_dubbing();}
    r.round_reset_navigation();
    if(!restart&&(r.mode_flags()&7u)==7u&&state.words.template get<std::uint32_t>(0x1c)!=1){
        for(std::int32_t identity=0;identity!=32;++identity)if(auto* actor=r.round_actor_identity(identity)){
            if(r.mode_flags()&2u)r.round_deactivate_actor(*actor,false);
            if(r.mode_flags()&1u)r.round_deactivate_actor(*actor,true);
            r.round_remove_actor(actor);
        }
        for(auto n=r.round_actor_count();n;)if(auto* actor=r.round_actor_identity(static_cast<std::int32_t>(--n));actor&&actor->base().type()==10){
            r.round_deactivate_actor(*actor,false);r.round_deactivate_actor(*actor,true);
            r.round_activate_spawn(*actor,true);r.round_activate_spawn(*actor,false);
        }
    }
    r.round_reset_modules(restart);
    if(!restart&&(r.mode_flags()&7u)==7u&&state.words.template get<std::uint32_t>(0x1c)!=1){
        for(std::size_t n=0;n<r.round_squad_count();++n)if(r.round_squad_active(n)){
            auto* spawn=r.round_loading_spawn();if(!spawn)break;r.round_spawn_soldier(*spawn,n);
        }
    }
    if(r.mode_flags()&2u){r.round_reset_first_person();r.round_reset_text_selection();}
    r.round_query_iconic();r.round_clear_transients();if(state.mission)r.round_reset_objectives();
    if((r.mode_flags()&7u)!=7u&&!(r.mode_flags()&1u)){r.round_begin_entry();r.round_reset_entry_packets();}
    if(restart){state.words.set(0x20,500u);state.status=5;/*0049B7C0 is genuine RET4.*/}
    if((r.mode_flags()&7u)!=7u&&(r.mode_flags()&2u)&&!r.round_local_peer_human()&&r.round_is_current_application())r.round_queue_entry();
    r.round_reset_music_references();state.words.set(0x38,2500u);r.round_select_music();
    r.round_reset_speech_clocks();
    if((r.mode_flags()&7u)==7u&&r.round_face_quality())r.round_initialize_faces();
}
}
#endif
