#ifndef HD2_GAME_SESSION_SPAWN_ADMISSION_H
#define HD2_GAME_SESSION_SPAWN_ADMISSION_H
#include "game/session_activation.h"
#include "game/human_state_frame_timers.h"
namespace hd2::game {
//0069E340: initial admission is NOT0069D9C0 pending respawn. Borrow caller's
//soldier/stats through actual00611C90; only the latter owns a transferred queue.
template<class Runtime>void admit_session_spawn(SessionActivationState& game,std::uint32_t peer,
        frontend::PreMissionSoldier& soldier,const std::array<std::uint32_t,12>* stats,Runtime& r){
    if((r.mode_flags()&7u)!=7u&&r.admission_actor_exists(peer))return;
    switch(r.zone_style()){
    case 0:soldier.specialization=0;break; //Native jumps over00730140 here.
    case 1:case 3:{
        if(r.pending_spawn_rule(0x278)&&r.human_remote_ai_enabled())soldier.network_identity=r.pending_spawn_zone_by_group(0);
        auto* zone=r.admission_zone(soldier.network_identity);
        if(!zone||r.pending_spawn_zone_side(*zone)!=static_cast<std::uint32_t>(soldier.specialization)){
            zone=r.admission_zone(r.pending_spawn_unique_zone(static_cast<std::uint32_t>(soldier.specialization)));
            if(!zone){r.reject_session_spawn(peer);return;}
            soldier.network_identity=zone->base().id10();
        }
        r.admission_publish_side(peer,static_cast<std::uint32_t>(soldier.specialization));break;
    }
    case 2:{
        auto* zone=r.admission_zone(soldier.network_identity);
        if(!zone||r.pending_spawn_zone_side(*zone)!=static_cast<std::uint32_t>(soldier.specialization)){r.reject_session_spawn(peer);return;}
        auto& w=game.words;
        if(w.template get<std::uint32_t>(0x88)!=3u&&w.template get<std::int32_t>(0x30)>30000&&
                r.admission_team_alive(static_cast<std::uint32_t>(soldier.specialization))&&!r.pending_spawn_rule(0x240))soldier.flag08=true;
        const auto side=r.pending_spawn_objective_side();soldier.flag09=false;
        if(peer==w.template get<std::uint32_t>(0x78)&&static_cast<std::uint32_t>(soldier.specialization)==side)w.set(0x78,0xffffffffu);
        if(w.template get<std::uint32_t>(0x88)!=3u&&w.template get<std::uint8_t>(0x71)&&!w.template get<std::uint8_t>(0x70)&&
                (peer==w.template get<std::uint32_t>(0x78)||w.template get<std::uint32_t>(0x78)==0xffffffffu)&&
                static_cast<std::uint32_t>(soldier.specialization)!=side){
            w.set(0x78,peer);soldier.flag09=true;w.set(0x70,std::uint8_t{1});r.pending_spawn_objective_clothing(soldier);
        }
        r.admission_publish_side(peer,static_cast<std::uint32_t>(soldier.specialization));break;
    }
    default:break;
    }
    if((r.mode_flags()&7u)!=7u)if(auto* spawn=r.admission_spawn_point())r.spawn_authority_human(*spawn,peer,&soldier,stats);
    if(peer==static_cast<std::uint32_t>(r.human_current_player_id()))r.spawn_hide_map(); //0049F040(false), even when no available SpawnPoint.
}
}
#endif
