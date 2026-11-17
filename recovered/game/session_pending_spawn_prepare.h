#ifndef HD2_GAME_SESSION_PENDING_SPAWN_PREPARE_H
#define HD2_GAME_SESSION_PENDING_SPAWN_PREPARE_H
#include "game/session_pending_spawn.h"
#include "game/human_state_frame_timers.h"
namespace hd2::game {
//0069D9C0. Caller owns the two original transferred allocations until either
//explicit rejection destroys them or a successfully appended9C/A0 record owns
//them. A life-limit RET leaves caller ownership intact (no invented spawn).
template<class State,class Runtime>void prepare_session_pending_spawn(State& state,std::uint32_t peer,
        std::unique_ptr<frontend::PreMissionSoldier>& soldier,
        std::unique_ptr<std::array<std::uint32_t,12>>& stats,Runtime& r){
    if(!soldier||!stats)throw std::logic_error("Pending spawn requires actual transferred soldier/stat allocations");
    const auto entry_mode=r.mode_flags();
    HumanVector position{{0,0,0,0}},direction{{0,0,1,0}}; //Native XYZ defaults; W is source padding, not a native value claim.
    const auto style=r.zone_style();
    if(style==0){
        //Native predecrements50 BEFORE the test: at most49 actual selections.
        for(unsigned left=49;left;--left){
            const auto side=r.zone_style()==0?0u:static_cast<std::uint32_t>(soldier->specialization);
            auto* spawn=r.select_authority_spawn_point((entry_mode&7u)!=7u,side);
            if(!spawn)throw std::logic_error("Pending spawn selection requires its actual SpawnPoint");
            position=spawn->base().world_position();direction=spawn->base().world_direction();
            if(!r.spawn_position_occupied(position))break;
        }
        soldier->specialization=0;
    }else if(style==1||style==2||style==3){
        if(style==3&&r.pending_spawn_rule(0x278))soldier->network_identity=r.pending_spawn_zone_by_group(0);
        auto* zone=r.pending_spawn_zone(soldier->network_identity);
        if(!zone||r.pending_spawn_zone_side(*zone)!=static_cast<std::uint32_t>(soldier->specialization)||
                !r.pending_spawn_pose(*zone,position,direction,r.pending_spawn_rule(0x240)!=0)){
            zone=r.pending_spawn_zone(r.pending_spawn_unique_zone(static_cast<std::uint32_t>(soldier->specialization)));
            if(!zone||!r.pending_spawn_pose(*zone,position,direction,r.pending_spawn_rule(0x240)!=0)){
                r.pending_spawn_rejected(peer);soldier.reset();stats.reset();return; //Free AFTER rejection FIFO allocation.
            }
            soldier->network_identity=zone->base().id10();
        }
        if(r.human_remote_ai_enabled()&&!r.pending_spawn_take_life(peer))return;
        if(r.zone_style()==2){
            auto& w=state.words;const auto side=r.pending_spawn_objective_side();soldier->flag09=false;
            if(peer==w.template get<std::uint32_t>(0x78)&&static_cast<std::uint32_t>(soldier->specialization)==side)w.set(0x78,0xffffffffu);
            if(w.template get<std::uint32_t>(0x88)!=3u&&w.template get<std::uint8_t>(0x71)&&!w.template get<std::uint8_t>(0x70)&&
                    (peer==w.template get<std::uint32_t>(0x78)||w.template get<std::uint32_t>(0x78)==0xffffffffu)&&
                    static_cast<std::uint32_t>(soldier->specialization)!=side){
                w.set(0x78,peer);soldier->flag09=true;w.set(0x70,std::uint8_t{1});
                r.pending_spawn_objective_clothing(*soldier);
            }
        }
        if(r.zone_style()==1)r.orient_spawn_toward_zone(direction,position,static_cast<std::uint32_t>(soldier->specialization));
    }
    if((r.mode_flags()&7u)!=7u)r.pending_spawn_reinitialize_peer(peer,position,direction,*soldier,*stats);
    //Native aligned record allocation happens AFTER the existing-Human reset.
    auto pending=std::make_shared<SessionPendingSpawn>();pending->peer=peer;pending->position=position;pending->direction=direction;
    pending->soldier=std::move(soldier);pending->stats=std::move(stats);
    state.pending_frame_peers.push_back(std::move(pending));
}
}
#endif
