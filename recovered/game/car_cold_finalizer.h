#ifndef HD2_GAME_CAR_COLD_FINALIZER_H
#define HD2_GAME_CAR_COLD_FINALIZER_H

#include "game/car_cold_particles.h"
#include "game/car_collision_loading.h"
#include "game/car_door_update.h"
#include "game/car_loading_lights.h"
#include "game/car_road_displacement.h"
#include "game/car_settling_controller.h"
#include "game/car_sound_updates.h"
#include "game/car_strip_update.h"
#include "game/car_wheel_rotation.h"
#include "game/scene_light_sample.h"

namespace hd2::game {

template<class Frames>struct CarColdFinalizerState {
    using Frame=typename Frames::Frame;
    CarLoadingLightState lights;CarStripUpdateState strips;
    std::vector<CarDoorRuntime<Frames>> doors;CarColdParticleSlots particles;
    std::uint32_t previous_features=0x8000dff9u,dirty_flags=0,light_elapsed=0;
    std::uint16_t puncture_elapsed=30000;
    float air_pressure=0,light_factor=1,slip_distance=0,road_enabled=1,field5d0=0;
    bool live=true,inside=false,air_requested=false,debug_selected=false;
    bool engine_transition=false,engine_sound_pending=false,field2287=false;
    bool engine_inside2298=false; //004E2900 byte2298=0;00518710 publishes2297 after completed start.
    Frame sector{};
    template<class Geometry>explicit CarColdFinalizerState(const Geometry& geometry)
        :particles(geometry.auxiliary().exhausts().size()){
        for(const auto& door:geometry.auxiliary().doors())doors.emplace_back(door);
    }
};

//Every reference belongs to the actual containing Car/world. No backend or
//successful finalization callback is supplied by this aggregate. Geometry and
//skid types remain generic so native fixtures can double engine boundaries.
template<class Frames,class Geometry,class Collisions,class Queries,class BodyController,class Skids,class Random>
struct CarColdFinalizerContext {
    using Frame=typename Frames::Frame;using Descriptor=typename CarCollisionLoading<Frames,Collisions>::Descriptor;
    Frames& frames;Geometry& geometry;CarBodyIntegration<Frames>& body;
    std::vector<CarWheelRuntime<Frames>>& wheels;CarDrivetrainState& drive;CarColdSolverState<Frames>& solver;
    CarSteeringState& steering;CarSettlingControllerState<Frames>& controller;
    CarCollisionLoading<Frames,Collisions>& collisions;Queries& queries;DynamicCollisionExclusions<Descriptor>& exclusions;
    BodyController& body_collision;CarSoundUpdates<Frames>& sounds;const CarHealthState& health;
    const CarMaterialRegistry& materials;std::vector<CarParticleState<Frame>>& pool;Skids& skids;Random& random;
};

//0050B180's cold, engine-off/unoccupied path. All reached cold callees execute
//against the SAME owners. Running/debug transitions, active skid production
//and positive particle production remain explicit excluded receivers, not
//successful no-ops. This function is not Car actor activation/readiness.
template<class Frames,class Geometry,class Collisions,class Queries,class BodyController,class Skids,class Random>
unsigned finalize_car_cold(CarColdFinalizerContext<Frames,Geometry,Collisions,Queries,BodyController,Skids,Random>& context,
        CarColdFinalizerState<Frames>& state,std::uint32_t milliseconds,unsigned game_kind){
    using Frame=typename Frames::Frame;
    auto& c=context;auto& drive=c.drive;auto& solver=c.solver;const auto& physics=c.geometry.physics();
    if(drive.running||state.engine_transition||state.engine_sound_pending||state.debug_selected)
        throw std::runtime_error("Car cold finalizer requires actual unoccupied/quiescent engine and debug state");
    if(!milliseconds)milliseconds=1;
    const float elapsed=static_cast<float>(static_cast<long double>(milliseconds)*.0010000000474974513f);
    if(c.body.motion().velocity[1]<-85.f)return 0;
    const auto model=c.geometry.model();c.frames.update_scene_object(model);c.frames.car_place_in_scene(model,c.body.pose().centre);
    if((solver.substeps.flags&0x20u)&&state.air_pressure<=0)state.air_requested=false;
    if(drive.audible){
        if((solver.substeps.flags&0x20u)&&state.air_pressure>0)c.sounds.air_brake(solver.substeps.flags,state.inside,state.air_requested);
        c.sounds.signal(false,false,state.inside);c.sounds.signal(true,false,state.inside); //Cold2294/2295.
        c.sounds.skidding(physics,c.wheels,c.materials,drive,solver.speed,elapsed,state.inside);
        c.sounds.rolling(physics,c.body.motion().velocity,state.inside,solver.substeps.flags);
        state.puncture_elapsed=static_cast<std::uint16_t>(state.puncture_elapsed+milliseconds);
        for(auto n=c.wheels.size();n;){const auto flags=c.wheels[--n].flags;
            if((flags&0x80000008u)==0x80000008u&&!(flags&0x40000420u)){state.puncture_elapsed=0;break;}}
        c.sounds.puncture(physics,c.body.motion().velocity,c.body.pose().forward,state.inside,solver.substeps.flags,state.puncture_elapsed<1000);
        if(state.puncture_elapsed>=1000)state.puncture_elapsed=1000;
    }
    state.light_elapsed+=milliseconds;
    if(state.light_elapsed>100&&state.live){
        state.light_elapsed=0;c.exclusions.single(&c.collisions.secondary());
        //Both calls borrow the SAME exclusion window; clear on an endpoint failure too.
        try{
            const auto plain=static_cast<float>(sample_scene_light(c.frames,c.queries,c.body.pose().centre,Frame{},false,game_kind));
            const auto shadowed=static_cast<float>(sample_scene_light(c.frames,c.queries,c.body.pose().centre,Frame{},true,game_kind));
            c.exclusions.clear();state.light_factor=plain<=0?1.f:shadowed/plain;
        }catch(...){c.exclusions.clear();throw;}
    }
    if(!c.wheels.empty()){
        const auto& direction=c.wheels.front().vectors.at(0xb0);const auto& velocity=c.controller.previous_velocity;
        state.slip_distance=static_cast<float>(((static_cast<long double>(direction[0])*velocity[0]+
            static_cast<long double>(direction[1])*velocity[1])+static_cast<long double>(direction[2])*velocity[2])*c.body.motion().travel+state.slip_distance);
    }
    struct DoorSounds {CarSoundUpdates<Frames>& sounds;const CarAuxiliaryGeometry<Frames>& geometry;bool audible,inside;
        void door(std::size_t seat,bool opening){sounds.door(geometry,seat,opening,audible,inside);}} door_sounds{c.sounds,c.geometry.auxiliary(),drive.audible,state.inside};
    update_car_doors(c.frames,c.geometry.auxiliary(),state.doors,elapsed,door_sounds);
    update_car_windows(c.frames,c.geometry.auxiliary(),state.doors,c.geometry.deformables().parts(),elapsed);
    if(solver.substeps.flags&0x40u)for(auto n=c.wheels.size();n;){const auto index=--n;auto& wheel=c.wheels[index];
        if(wheel.flags&0x10u)throw std::runtime_error("Car sliding skid requires its actual active mesh update");
        c.skids.retire(index,wheel.skid_cursor);
    }
    c.collisions.pose(c.body.pose().world,c.body.pose().inverse);
    if(drive.body_flags&1u){c.collisions.refresh_active();update_car_road_displacement(physics,c.materials,c.wheels,state.road_enabled,solver.substeps.flags,c.random);}
    const auto kind=static_cast<unsigned>(physics.scalars().get(0x550));
    if(kind==0)update_car_steering(c.frames,model,physics,c.body,c.wheels,c.steering,elapsed,solver.speed);
    else if(kind==2)reset_car_tracked_steering(c.frames,c.wheels);
    update_car_wheel_rotation(c.frames,physics,c.body,c.wheels,drive,solver.queries,solver.wheel_spin,elapsed);
    const CarLoadingLightInputs light_inputs{drive.gear,drive.brake,c.steering.current};
    update_car_loading_lights(c.frames,c.geometry.lighting(),state.lights,elapsed,solver.substeps.flags,state.previous_features,&light_inputs);
    update_car_steering_wheel(c.frames,c.geometry.auxiliary(),physics,c.steering);
    if(state.live||solver.substeps.flags!=state.previous_features)
        c.geometry.axes().apply(c.frames,c.geometry.wheels().wheels(),solver.substeps.flags,state.previous_features);
    if(state.live)update_car_strips(c.frames,c.geometry.strips(),physics,c.body,c.wheels,drive,solver.wheel_spin,elapsed,state.strips);
    bool quiet=update_car_cold_damage_particle<Frames>(physics,c.health,state.particles,c.pool,solver.substeps.flags,state.live,solver.speed);
    update_car_cold_wheel_particles(c.materials,c.wheels,c.pool,solver.substeps.flags,state.live,solver.speed,game_kind);
    update_car_cold_exhaust_particles<Frames>(physics,drive,state.particles,c.pool,solver.substeps.flags,state.live,solver.speed);
    update_car_cold_impact_particles<Frames>(c.materials,c.body_collision.state().records,state.particles,c.pool,solver.substeps.flags,state.live);
    solver.external_force={};solver.external_torque={};state.field5d0=0;c.controller.requests_update=false;c.controller.frame_simulated=true;
    if(state.dirty_flags&2u){
        for(auto n=c.geometry.deformables().parts().size();n;)c.geometry.deformables().parts()[--n].flags&=0xfdu;
        for(auto n=c.wheels.size();n;){auto& wheel=c.wheels[--n];auto& fields=wheel.scalars;
            if((wheel.flags&0x10000000u)&&fields.get(0x230)<fields.get(0x244))
                fields.set(0x22c,static_cast<float>((1.L-static_cast<long double>(fields.get(0x230))/fields.get(0x244))*fields.get(0x228)));
            wheel.flags&=~0x10000000u;
        }
        state.dirty_flags&=~2u;
    }
    state.dirty_flags&=~0xcu;
    for(auto n=c.geometry.auxiliary().seats().size();n;){const auto index=c.geometry.auxiliary().seats()[--n].door;
        if(index!=CarAuxiliaryGeometry<Frames>::no_door&&(state.doors.at(index).flags&2u)){quiet=false;break;}}
    state.sector=c.frames.object_parent(model);if(quiet&&(state.lights.flags&0x83u))quiet=false;
    state.previous_features=solver.substeps.flags;
    if(!solver.queries.moving_primitive)drive.body_flags&=~0x2000000u;
    else{c.controller.inactive_milliseconds=0;drive.body_flags|=0x2000001u;solver.stable_milliseconds=std::min(solver.stable_milliseconds,std::int32_t{0});}
    if(!(drive.body_flags&1u)&&quiet&&!c.controller.controlled&&!solver.mass_changed)return 2;
    solver.mass_changed=false;solver.queries.moving_primitive=false;c.body_collision.state().moving_primitive=false;state.field2287=false;
    return 1;
}
} //namespace hd2::game
#endif
