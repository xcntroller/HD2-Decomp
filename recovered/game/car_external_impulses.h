#ifndef HD2_GAME_CAR_EXTERNAL_IMPULSES_H
#define HD2_GAME_CAR_EXTERNAL_IMPULSES_H

#include "game/car_loading_lifetime.h"

namespace hd2::game {

//004F2A30. Flags suppress linear(1) and angular(2) accumulation independently.
//These are SAME270/280 solver accumulators, not a second motion simulation.
//Installed loading actors keep native6A4 null; the containing producer must
//supply that notification if a later lifetime installs a positive callback.
template<class Frames,class Notify>void accumulate_car_contact_force(Frames& frames,CarActorColdState<Frames>& state,
        const CarContactVector& force,const CarContactVector& point,unsigned flags,
        const CarContactVector& scales,Notify&& notify){
    if(!(flags&1u))for(unsigned axis=0;axis!=3;++axis)state.solver.external_force[axis]+=force[axis];
    if(!(flags&2u)){
        std::array<long double,3> offset{};
        for(unsigned axis=0;axis!=3;++axis)offset[axis]=static_cast<long double>(point[axis])-state.body.pose().centre[axis];
        CarContactVector torque{};
        for(unsigned axis=0;axis!=3;++axis){const auto n=(axis+1)%3,l=(axis+2)%3;
            torque[axis]=static_cast<float>(offset[l]*force[n]-offset[n]*force[l]);}
        frames.car_transform_in_place(torque,state.body.pose().inverse_rotation);
        for(unsigned axis=0;axis!=3;++axis)torque[axis]*=scales[axis];
        frames.car_transform_in_place(torque,state.body.pose().rotation);
        for(unsigned axis=0;axis!=3;++axis)state.solver.external_torque[axis]+=torque[axis];
    }
    state.solver.mass_changed=true;notify(); //22C0, shared with mass/settling/finalizer.
}

//004F2B60. The ignored last native stack argument is not exposed. Deformation,
//damage and passengers run AFTER velocity mutation, BEFORE direction/22C0.
template<class Frames,class Deform,class Damage,class Passengers,class Notify>void apply_car_contact_impulse(
        Frames& frames,CarActorColdState<Frames>& state,float scale,const CarContactVector& point,
        const CarContactVector& incoming,const CarContactVector& normal,bool source_present,
        Deform&& deform,Damage&& damage,Passengers&& passengers,Notify&& notify){
    CarContactVector impulse{},angular{};
    std::array<long double,3> offset{};
    for(unsigned axis=0;axis!=3;++axis){
        offset[axis]=static_cast<long double>(point[axis])-state.body.pose().centre[axis];
        impulse[axis]=scale*incoming[axis];
    }
    const auto reciprocal=1.L/state.body.contact_state().mass918;
    for(unsigned axis=0;axis!=3;++axis){
        auto change=impulse[axis]*reciprocal;if(axis)change=static_cast<float>(change);
        state.body.motion().velocity[axis]=static_cast<float>(change+state.body.motion().velocity[axis]);
        const auto n=(axis+1)%3,l=(axis+2)%3;
        angular[axis]=static_cast<float>(offset[n]*impulse[l]-offset[l]*impulse[n]);
    }
    frames.car_transform_in_place(angular,state.body.pose().inverse_rotation);
    const auto inertia=state.body.contact_state().inertia220;
    for(unsigned axis=0;axis!=3;++axis)angular[axis]*=inertia[axis];
    frames.car_transform_in_place(angular,state.body.pose().rotation);
    for(unsigned axis=0;axis!=3;++axis)state.body.motion().angular_velocity[axis]-=angular[axis];
    const float square=static_cast<float>((static_cast<long double>(impulse[2])*impulse[2]+
        static_cast<long double>(impulse[1])*impulse[1])+static_cast<long double>(impulse[0])*impulse[0]);
    std::int32_t bits;std::memcpy(&bits,&square,4);
    const float magnitude=bits<0x322bcc77?0.f:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-9f?1.f:
        static_cast<float>(std::sqrt(static_cast<long double>(square)));
    if(state.solver.substeps.flags&0x8000u){auto reverse=normal;for(unsigned axis=0;axis!=3;++axis)reverse[axis]=-reverse[axis];
        deform(point,reverse,.44999998807907104f,magnitude);}
    if(state.solver.substeps.flags&0x4000u){damage(magnitude);passengers(impulse,point,source_present);}
    auto direction=state.body.motion().velocity;
    const float length2=static_cast<float>((static_cast<long double>(direction[0])*direction[0]+
        static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[2])*direction[2]);
    if(!std::isfinite(length2))throw std::runtime_error("Non-finite Car contact velocity");
    if(std::abs(static_cast<long double>(length2)-1)>=9.99999993922529e-9f){
        if(length2>=9.99999993922529e-9f){const auto inverse=1/std::sqrt(static_cast<long double>(length2));
            for(unsigned axis=0;axis!=3;++axis)direction[axis]=static_cast<float>(direction[axis]*inverse);
        }else{const auto axis=direction[0]!=0?0u:direction[2]!=0?2u:1u;direction[axis]=direction[axis]<0?-1.f:1.f;}
    }
    state.body.motion().direction=direction;state.solver.mass_changed=true;notify();
}

} //namespace hd2::game
#endif
