#ifndef HD2_GAME_SESSION_HUMAN_SPAWN_H
#define HD2_GAME_SESSION_HUMAN_SPAWN_H
#include "game/prepared_human_spawn.h"
#include "game/human_frame_stats.h"
namespace hd2::game {
//00611C90 authority caller. Requery A8 after receivers, just as the original
//reloads its SpawnPoint borrower for each activation/flag/publication operation.
template<class Spawn,class Runtime>void spawn_session_human(Spawn& spawn,std::uint32_t peer,frontend::PreMissionSoldier* soldier,
        const std::array<std::uint32_t,12>* stats,Runtime& r){
    if(!(r.mode_flags()&1u))return;
    if((r.mode_flags()&7u)==7u)spawn.flag9c=false;
    const auto identity=(r.mode_flags()&7u)==7u?r.first_free_spawn_identity():human_frame_signed_word(peer);
    const auto clothing=(r.mode_flags()&7u)==7u?human_frame_signed_word(spawn.properties.fielda4):-1;
    HumanVector position,direction;r.initial_spawn_pose(spawn,position,direction);
    if((r.mode_flags()&7u)!=7u){
        const auto style=r.zone_style();
        if(style==1||style==2||style==3){
            if(!soldier)throw std::logic_error("Team spawn requires its actual prepared soldier");
            if(!r.zone_next_spawn_pose(soldier->network_identity,position,direction,r.spawn_cycle_enabled())){r.reject_session_spawn(peer);return;}
            if(r.zone_style()==1)r.orient_spawn_toward_zone(direction,position,static_cast<std::uint32_t>(soldier->specialization));
        }else{
            for(unsigned remaining=31;remaining;--remaining){
                const auto side=r.zone_style()==0?0u:(soldier?static_cast<std::uint32_t>(soldier->specialization):throw std::logic_error("Spawn side requires its actual soldier"));
                auto* selected=r.select_authority_spawn_point(1,side);if(!selected)throw std::logic_error("Authority spawn selection returned no actual SpawnPoint");
                position=selected->base().world_position();direction=selected->base().world_direction();
                if(!r.spawn_position_occupied(position))break;
            }
        }
    }
    if(soldier)r.prepare_authority_spawn_soldier(spawn,*soldier);
    auto* created=r.create_authority_spawn_human(peer,identity,position,direction,soldier,stats,clothing,spawn.properties.fielda0!=0);
    spawn.spawned_human=created;if(!created)return;
    const auto human=[&]{return r.require_authority_spawn_human(spawn);};
    if((r.mode_flags()&7u)==7u)r.bind_authority_spawn_record(peer);
    if((r.mode_flags()&7u)!=7u&&peer==static_cast<std::uint32_t>(r.human_current_player_id())&&r.spawn_result()!=3)r.set_spawn_match_phase(1);
    human()->activate(true);if(r.mode_flags()&2u)human()->activate(false);
    if((r.mode_flags()&7u)!=7u){
        if(r.spawn_state_kind(*human())==1){
            if(!soldier)throw std::logic_error("Spawn Player flags require its actual prepared soldier");
            r.spawn_human_side(*human(),static_cast<std::uint32_t>(soldier->specialization));
            r.spawn_selection_suppressed(*human(),static_cast<std::uint8_t>(soldier->flag08));
            r.spawn_player_flag(*human(),static_cast<std::uint8_t>(soldier->flag09));
        }
        if(r.spawn_voice_enabled())r.update_human_transmit_targets();
    }
    r.publish_authority_spawn(peer,identity,spawn,soldier,stats,position,direction);
    r.notify_authority_spawn_created(spawn); //Late frameEC+4/context gate after packet/direct presentation.
}
} //namespace hd2::game
#endif
