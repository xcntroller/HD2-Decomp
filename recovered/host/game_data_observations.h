#ifndef HD2_HOST_GAME_DATA_OBSERVATIONS_H
#define HD2_HOST_GAME_DATA_OBSERVATIONS_H
#include "host/game_world_loading.h"
namespace hd2::host {
//Synchronous snapshot of SAME live owners. Values alone enter the store; no
//retirement callbacks or pointers are retained. This does not execute a frame.
inline void observe_world_data(world_loading::World& world){
    auto& d=world.owners().dependencies();auto& frames=d.scene.frames();
    if(!frames.resource_observations())return;
    frames.observe_data("population","World snapshot begin");frames.observe_scene_population();
    for(auto* actor:d.world_actors.runtime().active_actors())if(actor)
        frames.observe_data("population","Runtime active actor","type",actor->base().type());
    const auto& shots=d.startup.shot_manager_process();
    frames.observe_data("population","ShotMgr","blood90",shots.blood_marks.size());
    frames.observe_data("population","ShotMgr","surfacesA0",shots.surfaces.size());
    frames.observe_data("population","ShotMgr render","80",shots.render.size());
    frames.observe_data("population","ShotMgr render","B0",shots.muzzle_flashes.size());
    for(unsigned n=0;n<shots.transient.size();++n)frames.observe_data("population","ShotMgr transient",n==0?"114":n==1?"124":"134",shots.transient[n].size());
    frames.observe_data("population","ShotMgr transient","134",shots.shells.size());
    const auto& particles=d.startup.mission_effects_process();
    frames.observe_data("population","Particles","active1014",particles.active_particles());
    frames.observe_data("population","Particles","placed1034",particles.placed_particles());
    const auto& volumes=world.modules().game_effects().volumes().values();
    frames.observe_data("population","GEffects","volumesC4",volumes.size());
    for(const auto& volume:volumes)if(volume){
        std::uint32_t bits;std::memcpy(&bits,&volume->field2c,4);
        frames.observe_data("population","Volume field2C","float bits",bits);
        frames.observe_data("population","Volume particles","count",volume->particles.size());
        frames.observe_data("population","Volume history3C40 previously written",volume->light_name.c_str(),volume->history_written());
    }
    const auto& clouds=d.startup.procedural_clouds();
    frames.observe_data("population","process0089B63C","texture0 present",clouds.has_texture(0));
    frames.observe_data("population","process0089B63C","texture4 present",clouds.has_texture(1));
    frames.observe_data("population","GEffects","clock224",d.startup.effects_loading_process().field224);
    const auto& special=d.scene.effects().special_model();
    frames.observe_data("population","GEffects special2E0","frame present",special.frame()!=nullptr);
    frames.observe_data("population","GEffects special2E0","deferred2EC",special.pending());
    frames.observe_data("population","GEffects special2E0","timer2F0 bits",static_cast<std::uint32_t>(special.remaining()));
    frames.observe_data("population","World snapshot end");
}
}
#endif
