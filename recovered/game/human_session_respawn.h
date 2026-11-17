#ifndef HD2_GAME_HUMAN_SESSION_RESPAWN_H
#define HD2_GAME_HUMAN_SESSION_RESPAWN_H
#include "game/session_match_peers.h"
#include "game/session_pending_spawn.h"
namespace hd2::game {
//0059F050. Reuses the SAME Human/model/inline owner; no replacement factory.
template<class Human,class Soldier,class Statistics,class Runtime>void respawn_session_human(Human& human,
        const HumanVector& position,const HumanVector& direction,Soldier* soldier,const Statistics* stats,Runtime& r){
    r.respawn_clear_storage(human);
    if((r.mode_flags()&7u)!=7u)r.respawn_enable_collision(human);
    if(r.mode_flags()&2u)human.deactivate(false);
    if(r.mode_flags()&1u)human.deactivate(true);
    human.activation.field254=0;human.activation.flag214=false;
    human.base().world_position(position);r.respawn_model_position(human,position);
    human.base().world_direction(direction);r.respawn_model_direction(human,direction);
    r.respawn_input_position(human,position);
    if(soldier)r.respawn_configure(human,*soldier,stats);
    if(r.mode_flags()&1u)human.activate(true);
    if(r.mode_flags()&2u)human.activate(false);
    if((r.mode_flags()&1u)&&human.field25c!=0xffffffffu)r.respawn_sight_target_modes(human);
    if(r.human_is_player(human))r.respawn_restore_player(human);
    if((r.mode_flags()&2u)&&r.human_current_player_id()==human.base().id10())r.respawn_refresh_visibility();
    human.field208=0;human.activation.flag20c=true;
}
//0069D780.418 covers life mutation, same-Human reset and Player/Remote flag.
//Voice and local map publication occur AFTER unlock, only for a named Human.
template<class State,class Runtime>void respawn_session_peer(State& session,std::uint32_t peer,
        const HumanVector& position,const HumanVector& direction,frontend::PreMissionSoldier& soldier,
        const std::array<std::uint32_t,12>& stats,frontend::ControlStartupState& records,Runtime& r){
    bool found=false;
    with_session_match_peers(records,r,[&](auto& peers){
        if(!(r.mode_flags()&1u)&&r.zone_style()==3){
            if(!r.pending_spawn_rule(0x27c))peers.at(peer).word(0xb0,peers.at(peer).word(0xb0)+1u);
            else session.words.set(0x84,session.words.template get<std::uint32_t>(0x84)+1u);
        }
        if(peer>=peers.size())return;
        auto* borrower=peers[peer].human_actor();if(!borrower||!peers[peer].byte(8).value())return;
        auto& human=r.respawn_peer_human(borrower);r.respawn_existing_human(human,position,direction,soldier,stats);
        if(r.zone_style()==2){const auto kind=r.spawn_state_kind(human);
            if(kind==1)r.spawn_player_flag(human,static_cast<std::uint8_t>(soldier.flag09));
            else if(kind==7)r.spawn_remote_flag(human,static_cast<std::uint8_t>(soldier.flag09));}
        found=true;
    });
    if(!found)return;
    if(r.spawn_voice_enabled())r.update_human_transmit_targets();
    if(peer==static_cast<std::uint32_t>(r.human_current_player_id())){session.words.set(0x90,0u);r.spawn_hide_map();}
}
}
#endif
