#ifndef HD2_GAME_SESSION_MATCH_INPUT_H
#define HD2_GAME_SESSION_MATCH_INPUT_H
#include "game/session_frame_input.h"
#include "game/human_state_frame_timers.h"
namespace hd2::game {
//0069AAB0. The caller ignores native volatile return values; preserve effects,
//fresh local-Human lookups and original signed remainder, not a new key map.
template<class State,class Queries>void process_nonoffline_frame_input(State& session,Queries& q,std::uint32_t* second){
    auto& w=session.words;auto* const human=q.frame_local_human();
    auto* const state=human?q.frame_human_state(*human):nullptr;
    if(!state||w.template get<std::uint32_t>(0x88)==3u)w.set(0xb8,2u);
    else if(w.template get<std::uint32_t>(0xb8)==2u)w.set(0xb8,1u);
    if(((!state||!q.frame_state_suppressed(*state))&&w.template get<std::uint32_t>(0xb8)!=2u)||!(q.mode_flags()&2u)){
        if(!state||!(q.frame_state_flags(*state)&0x800u)||(q.mode_flags()&7u)==7u||q.frame_state_suppressed(*state)||
                !second||!session_input_active(second,55)||!q.frame_rule(0x224)||q.frame_human_age(*human)<3001)return;
        q.frame_suppress_state(*state,true);w.set(0xb4,0xffffffffu);w.set(0xb8,1u);return;
    }
    auto selected=w.template get<std::uint32_t>(0xb4);
    if(selected!=0xffffffffu&&selected>=32u)throw std::logic_error("Spectator cursor has no valid native reserved-slot producer");
    if((selected==0xffffffffu||(second&&session_input_active(second,55)))&&w.template get<std::uint32_t>(0xb8)!=2u){
        auto counter=selected+1u;
        auto index=static_cast<std::uint32_t>(human_frame_signed_word(counter)%32);
        if(index!=selected)for(;;){
            auto* candidate=q.frame_spectator_candidate(index);
            if(candidate&&candidate!=q.frame_local_human()&&q.frame_human_health(*candidate)>0&&
                    !q.frame_peer_spectator_only(q.frame_human_identity(*candidate))&&q.frame_human_controlled(*candidate)){
                const auto style=q.frame_rule(0x224);
                if(style==1u||style==2u||style==3u){
                    const auto local_side=q.frame_local_human_side();const auto target_side=q.frame_human_side(*candidate);
                    if(local_side==target_side||q.frame_local_human_side()==2u){
                        q.frame_override_player(candidate);q.frame_enable_first_person(false);q.frame_hud_spawn_mode(1);
                        q.frame_hud_peer_human(candidate);w.set(0xb4,index);break;
                    }
                }
            }
            if(w.template get<std::uint32_t>(0xb4)==0xffffffffu&&counter==32u)break;
            ++counter;index=static_cast<std::uint32_t>(human_frame_signed_word(counter)%32);
            if(index==w.template get<std::uint32_t>(0xb4))break;
        }
    }
    if(state&&second&&session_input_active(second,56)&&q.frame_current_match_phase()!=3u){
        const auto value=static_cast<std::uint32_t>(human_frame_signed_word(w.template get<std::uint32_t>(0xb8)+1u)%3);
        w.set(0xb8,value);
        if(value==2u){if(q.frame_local_human_side()==2u)q.frame_camera_command(2,1,1);
            else w.set(0xb8,static_cast<std::uint32_t>(human_frame_signed_word(value+1u)%3));}
    }
    if(w.template get<std::uint32_t>(0xb8)==2u&&second&&session_input_active(second,55))q.frame_camera_command(8,1,1);
}
}
#endif
