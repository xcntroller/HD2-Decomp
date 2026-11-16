#ifndef HD2_GAME_CAR_COLD_FINALIZER_H
#define HD2_GAME_CAR_COLD_FINALIZER_H

#include "game/car_cold_particles.h"
#include "game/car_finalizer_particles.h"
#include "game/car_gear_control.h"
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
    using Frame=typename Frames::Frame;using Vector=typename Frames::Vector;
    CarLoadingLightState lights;CarStripUpdateState strips;
    std::vector<CarDoorRuntime<Frames>> doors;CarColdParticleSlots particles;
    std::uint32_t previous_features=0x8000dff9u,dirty_flags=0,light_elapsed=0;
    std::uint16_t puncture_elapsed=30000;
    float light_factor=1,slip_distance=0,road_enabled=1,field5d0=0;
    bool live=true,inside=false,air_requested=false,debug_selected=false;
    bool engine_transition=false,field2287=false;
    Vector steering_target{}; //004EA780 core40/44/48, consumed only with2287.
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
template<class Frames,class Geometry,class Collisions,class Queries,class BodyController,class Skids,class Random,class Callbacks>
struct CarFinalizerContext {
    using Frame=typename Frames::Frame;using Descriptor=typename CarCollisionLoading<Frames,Collisions>::Descriptor;
    Frames& frames;Geometry& geometry;CarBodyIntegration<Frames>& body;
    std::vector<CarWheelRuntime<Frames>>& wheels;CarDrivetrainState& drive;CarColdSolverState<Frames>& solver;
    CarSteeringState& steering;CarSettlingControllerState<Frames>& controller;
    CarCollisionLoading<Frames,Collisions>& collisions;Queries& queries;DynamicCollisionExclusions<Descriptor>& exclusions;
    BodyController& body_collision;CarSoundUpdates<Frames>& sounds;CarHealthState& health;
    const CarMaterialRegistry& materials;std::vector<CarParticleState<Frame>>& pool;Skids& skids;Random& random;
    Callbacks& callbacks;const std::vector<CarParticleDefinition>& definitions;
    const bool& reset;const bool& saved_transition;const bool& signal_first;const bool& signal_second;const bool& automatic_gears;
    float& brake_amount;float& engine_timeout;float& gear_direction_timer;
    std::vector<std::uint16_t>& paired_flags;
};

//0050B180, on the SAME initialized owners. The native debug-selected drawing
//path remains required. This source composition does not publish app readiness.
template<class Frames,class Geometry,class Collisions,class Queries,class BodyController,class Skids,class Random,class Callbacks>
unsigned finalize_car(CarFinalizerContext<Frames,Geometry,Collisions,Queries,BodyController,Skids,Random,Callbacks>& context,
        CarColdFinalizerState<Frames>& state,std::uint32_t milliseconds,unsigned game_kind){
    using Frame=typename Frames::Frame;
    auto& c=context;auto& drive=c.drive;auto& solver=c.solver;const auto& physics=c.geometry.physics();
    if(c.reset)state.engine_transition=c.saved_transition;
    if(!milliseconds)milliseconds=1;
    const float elapsed=static_cast<float>(static_cast<long double>(static_cast<float>(milliseconds))*.0010000000474974513f);
    if(c.body.motion().velocity[1]<-85.f)return 0;
    const auto model=c.geometry.model();c.frames.update_scene_object(model);c.frames.car_place_in_scene(model,c.body.pose().centre);
    if(state.debug_selected)throw std::runtime_error("Car selected debug finalizer requires actual debug drawing receivers");
    if(!state.engine_transition){
        if(!c.reset){if(drive.running)c.sounds.stop_engine_transition(drive,state.inside,state.engine_transition,c.health.destroyed);}
        else c.sounds.pause_engine_loops();
    }else if(drive.audible)c.sounds.engine(physics,drive,solver.substeps.flags,state.engine_transition,state.inside,state.engine_inside2298,c.engine_timeout,elapsed);
    if((solver.substeps.flags&0x20u)&&c.brake_amount<=0)state.air_requested=false;
    if(drive.audible){
        if((solver.substeps.flags&0x20u)&&c.brake_amount>0)c.sounds.air_brake(solver.substeps.flags,state.inside,state.air_requested);
        c.sounds.signal(false,c.signal_first,state.inside);c.sounds.signal(true,c.signal_second,state.inside);
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
        const auto& direction=c.wheels.front().vectors.at(0xb0);const auto& velocity=c.body.motion().direction;
        state.slip_distance=static_cast<float>(((static_cast<long double>(direction[0])*velocity[0]+
            static_cast<long double>(direction[1])*velocity[1])+static_cast<long double>(direction[2])*velocity[2])*c.body.motion().travel+state.slip_distance);
    }
    struct DoorSounds {CarSoundUpdates<Frames>& sounds;const CarAuxiliaryGeometry<Frames>& geometry;bool audible,inside;
        void door(std::size_t seat,bool opening){sounds.door(geometry,seat,opening,audible,inside);}} door_sounds{c.sounds,c.geometry.auxiliary(),drive.audible,state.inside};
    update_car_doors(c.frames,c.geometry.auxiliary(),state.doors,elapsed,door_sounds);
    update_car_windows(c.frames,c.geometry.auxiliary(),state.doors,c.geometry.deformables().parts(),elapsed);
    if(solver.substeps.flags&0x40u)for(auto n=c.wheels.size();n;){const auto index=--n;auto& wheel=c.wheels[index];
        if(wheel.flags&0x10u)c.skids.update(index,wheel,c.materials);
        else c.skids.retire(index,wheel.skid_cursor);
    }
    c.collisions.pose(c.body.pose().world,c.body.pose().inverse);
    if(drive.body_flags&1u){c.collisions.refresh_active();update_car_road_displacement(physics,c.materials,c.wheels,state.road_enabled,solver.substeps.flags,c.random);}
    const auto kind=static_cast<unsigned>(physics.scalars().get(0x550));
    if(kind==0)update_car_steering(c.frames,model,physics,c.body,c.wheels,c.steering,elapsed,solver.speed);
    else if(kind==2)reset_car_tracked_steering(c.frames,c.wheels);
    update_car_wheel_rotation(c.frames,physics,c.body,c.wheels,drive,solver.queries,c.controller.frame_simulated,elapsed);
    const CarLoadingLightInputs light_inputs{drive.gear,drive.brake,c.steering.current};
    update_car_loading_lights(c.frames,c.geometry.lighting(),state.lights,elapsed,solver.substeps.flags,state.previous_features,
        c.paired_flags,state.sector,c.body.pose().forward,&light_inputs);
    update_car_steering_wheel(c.frames,c.geometry.auxiliary(),physics,c.steering);
    if(state.live||solver.substeps.flags!=state.previous_features)
        c.geometry.axes().apply(c.frames,c.geometry.wheels().wheels(),solver.substeps.flags,state.previous_features);
    if(state.live)update_car_strips(c.frames,c.geometry.strips(),physics,c.body,c.wheels,drive,c.controller.frame_simulated,elapsed,state.strips);
    CarFinalizerParticles<Frames,Callbacks> particles(c.frames,c.callbacks,c.definitions,c.pool);
    bool quiet=particles.damage(c.geometry,c.body,c.health,state.particles,solver.substeps.flags,state.live,solver.speed,state.light_factor);
    particles.wheels(c.materials,c.wheels,solver.substeps.flags,state.live,solver.speed,game_kind,state.light_factor);
    particles.exhaust(c.geometry,c.body,drive,state.particles,solver.substeps.flags,state.live,state.engine_transition,solver.speed,state.light_factor);
    particles.impacts(c.materials,c.body_collision.state().records,c.body,state.particles,solver.substeps.flags,state.live,state.light_factor);
    for(unsigned axis=0;axis<3;++axis){solver.external_force[axis]=0;solver.external_torque[axis]=0;}
    state.field5d0=0;c.controller.requests_update=false;c.controller.frame_simulated=true;
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
    if(!(drive.body_flags&1u)&&quiet&&!c.controller.controlled&&!state.engine_transition&&!solver.mass_changed)return 2;
    if(solver.mass_changed||(drive.brake>.8999999761581421f&&drive.throttle>0)){
        if((!(solver.substeps.flags&0x4000u)||c.health.drivetrain>0)&&state.engine_transition&&c.automatic_gears&&drive.gear&&
                (drive.flag2293||c.controller.has_strip_connections)&&drive.requested_gear>0){
            const auto rolling=(static_cast<long double>(solver.speed)/physics.scalars().get(0x8d8))*car_gear_ratio(physics,drive.gear);
            if(physics.gear_count()>=2&&rolling<physics.scalars().get(0x818))
                request_car_gear(physics,drive,solver.substeps.flags,c.health.drivetrain,c.gear_direction_timer,1);
            else if(physics.gear_count()>2&&rolling<physics.scalars().get(0x81c))
                request_car_gear(physics,drive,solver.substeps.flags,c.health.drivetrain,c.gear_direction_timer,2);
        }
    }
    solver.mass_changed=false;solver.queries.moving_primitive=false;c.body_collision.state().moving_primitive=false;state.field2287=false;
    return 1;
}
} //namespace hd2::game
#endif
