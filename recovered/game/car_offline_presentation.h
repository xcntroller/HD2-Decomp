#ifndef HD2_GAME_CAR_OFFLINE_PRESENTATION_H
#define HD2_GAME_CAR_OFFLINE_PRESENTATION_H

#include "game/car_actor_features.h"
#include "game/car_offline_frame.h"
#include <algorithm>
#include <cstdint>

namespace hd2::game {

//004D3110's offline branch, including004D38F0. Occupancy is produced by H3
//seat admission/input on these SAME owners; the original has no empty-seat
//precondition. Non7 entry remains the containing caller's required boundary.
template<class Owner>void present_car_offline_frame(Owner& owner,std::uint32_t elapsed){
    auto& state=*owner.initial_;auto& actor=owner.actor_;auto& frames=owner.frames_;
    elapsed=std::min(elapsed,std::uint32_t{83});
    state.finalizer.lights.steering_threshold=state.steering.maximum*.8999999761581421f;
    owner.same_epoch_=frames.vertex_scene_epoch()==frames.vertex_frame_epoch(actor.geometry->model());
    owner.update_core(elapsed);
    advance_car_mounted_controls(actor,false,elapsed);
    (void)owner.finalize_core(elapsed);
    //00611080(1/2), selected by finalizer result2/0, is genuine RET4.
    actor.base().world_direction(state.body().pose().forward);
    actor.base().world_position(state.body().physical_position());
    update_car_actor_features(frames,actor.geometry->model(),frames.active_game_camera(),owner.same_epoch_,actor.flag254a,
        owner.services_.mode_flags(),state.solver.substeps.flags,state.drive,state.audio);
    owner.seats_.update(owner.queries_,actor.base().world_direction(),state.body().motion().velocity,state.solver.speed,
        owner.services_.car_current_attachment(),owner.resources_.seat_camera);
    if(state.solver.speed!=0||owner.navigation_elapsed_>0){
        owner.navigation_elapsed_+=elapsed;
        if(owner.navigation_elapsed_>500){owner.navigation_elapsed_=state.solver.speed!=0;owner.refresh_navigation();}
    }
    owner.navigation_.observe_loading_marks();
    frames.supply_model_tick(actor.geometry->model(),elapsed);
}
} //namespace hd2::game
#endif
