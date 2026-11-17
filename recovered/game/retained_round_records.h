#ifndef HD2_GAME_RETAINED_ROUND_RECORDS_H
#define HD2_GAME_RETAINED_ROUND_RECORDS_H
#include <stdexcept>
namespace hd2::game {
//00452280 over the SAME ShotMgr containers. Resource textures, hit zones,
//models,124 queue,140 clock and both reuse pools survive this operation.
template<class State,class RecycleBullet>void reset_shot_round_records(State& state,RecycleBullet&& recycle_bullet){
    for(auto n=state.shells.size();n;){auto& shell=state.shells[--n];shell->retire();state.decals.recycle(shell);}
    for(auto n=state.render.size();n;)if(auto& row=state.render[--n]){row->clear();row.reset();}
    for(auto n=state.blood_marks.size();n;)if(auto& row=state.blood_marks[--n]){row->clear();row.reset();}
    for(auto n=state.surfaces.size();n;)if(auto& row=state.surfaces[--n]){row->clear();row.reset();}
    for(auto n=state.muzzle_flashes.size();n;)if(auto& row=state.muzzle_flashes[--n]){row->clear();row.reset();}
    state.render.clear();state.blood_marks.clear();state.surfaces.clear();state.muzzle_flashes.clear();state.shells.clear();
    for(auto n=state.transient[0].size();n;){auto& row=state.transient[0][--n];
        if(row->kind()==1u)recycle_bullet(row);
        else if(row->kind()==3u||row->kind()==4u)row.reset();
        else throw std::runtime_error("ShotMgr reset requires actual selected projectile kind");
    }
    state.transient[0].clear();
}
//00447730 differs from module17 finish: both live lists retire backwards;
//false retains definitions/pools/id/latch; true clears definitions BEFORE pools.
template<class Rows,class Definitions,class Identity,class Initialized,class Pools>
void reset_particle_round_records(Rows& active,Rows& placed,Definitions& definitions,Identity& identity,
        Initialized& initialized,bool restart,Pools&& clear_pools){
    for(auto n=active.size();n;){auto& instance=active[--n].instance;if(instance){instance->close();instance.reset();}}
    active.clear();
    for(auto n=placed.size();n;){auto& instance=placed[--n].instance;if(instance){instance->close();instance.reset();}}
    placed.clear();
    if(restart){definitions.clear();identity=-1;clear_pools();initialized=true;}
}
}
#endif
