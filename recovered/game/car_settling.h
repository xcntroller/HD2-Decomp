#ifndef HD2_GAME_CAR_SETTLING_H
#define HD2_GAME_CAR_SETTLING_H

#include "game/car_cold_solver.h"

namespace hd2::game {

//004F67B0. Update and Finalize are mandatory00509AA0/0050B180 receivers.
//The actual host clock is timeGetTime, distinct from the QPC Car actor clock.
//No iteration cap or successful finalizer is supplied here. Partial failure
//leaves mutations with the containing runtime owner, which must discard it.
template<class Frames,class Update,class Finalize>void settle_car_loading(Frames& frames,
        CarBodyIntegration<Frames>& body,const std::vector<CarWheelRuntime<Frames>>& wheels,
        CarDrivetrainState& drive,CarColdSolverState<Frames>& state,std::int32_t& update_hold,
        std::uint32_t duration,float initial_speed,std::int32_t iterations,Update&& update,Finalize&& finalize){
    const auto features=state.substeps.flags,body_flags=drive.body_flags;
    const auto maximum_delta=state.substeps.maximum_delta;const auto hold=update_hold;const auto audible=drive.audible;
    drive.audible=false;state.substeps.maximum_delta=.02800000086426735f;state.substeps.flags=0x80000801u;
    state.mass_changed=true;drive.body_flags=1;state.substeps.previous_delta=.019999999552965164f;
    body.motion().velocity={};body.motion().angular_velocity={};state.speed=0;update_hold=0;
    const auto started=frames.car_settling_ticks();
    while(static_cast<std::uint32_t>(frames.car_settling_ticks()-started)<duration||iterations>0){
        update(std::uint32_t{83});drive.body_flags=1;--iterations;state.stable_milliseconds=0;
        auto& velocity=body.motion().velocity;bool clamp=false;
        if(velocity[1]>.10000000149011612f){velocity={{0,.10000000149011612f,0,0}};clamp=true;}
        else{
            bool contact=false;for(auto n=wheels.size();n;){const auto flags=wheels[--n].flags;if((flags&8u)&&!(flags&0x400u)){contact=true;break;}}
            if(contact&&velocity[1]<-.4000000059604645f){velocity={{0,-.4000000059604645f,0,0}};clamp=true;}
        }
        if(clamp)for(unsigned axis=0;axis!=3;++axis)body.motion().angular_velocity[axis]*=.5f;
    }
    drive.body_flags=body_flags;update_hold=hold;finalize(std::uint32_t{83});
    drive.audible=audible;body.motion().angular_velocity={};
    for(unsigned axis=0;axis!=3;++axis)body.motion().velocity[axis]=initial_speed*body.pose().forward[axis];
    auto direction=body.motion().velocity;
    const float square=static_cast<float>((static_cast<long double>(direction[2])*direction[2]+static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[0])*direction[0]);
    if(std::abs(static_cast<long double>(square)-1)>=9.99999993922529e-09f){
        if(square>=9.99999993922529e-09f){const auto inverse=1/std::sqrt(static_cast<long double>(square));for(unsigned axis=0;axis!=3;++axis)direction[axis]=static_cast<float>(direction[axis]*inverse);}
        else{const auto axis=direction[0]!=0?0u:direction[2]!=0?2u:1u;direction[axis]=direction[axis]<0?-1.f:1.f;}
    }
    body.motion().direction=direction;state.speed=std::abs(initial_speed);
    state.substeps.flags=features;state.substeps.maximum_delta=maximum_delta;
}
} //namespace hd2::game
#endif
