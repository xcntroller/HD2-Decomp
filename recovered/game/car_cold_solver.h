#ifndef HD2_GAME_CAR_COLD_SOLVER_H
#define HD2_GAME_CAR_COLD_SOLVER_H

#include "game/car_body_forces.h"
#include "game/car_cold_wheel_contacts.h"
#include "game/car_substeps.h"

namespace hd2::game {

template<class Frames>struct CarColdSolverState {
    using Vector=typename Frames::Vector;
    CarSubstepState<Frames> substeps;
    CarWheelQueryState queries;
    Vector external_force{},external_torque{},published_net{}; //004EA780's270/280,0050DE60's20.
    float speed=0,rolling_speed=0,speed_angle=0,additional_grip=0;
    std::int32_t stable_milliseconds=0;
    bool mass_changed=false,driven_contact=false,wheel_spin=false;
    explicit CarColdSolverState(const CarPhysics& physics){queries.projected_radius=physics.scalars().get(0x8dc);} //004ED270's real initial8DC.
};

//0050DE60's actual engine-off branch. BODYCOL acceptance uses the mandatory
//containing controller, after publishing the candidate and before refreshing
//the cached pose.004F67B0 selects80000801, whose10 bit is clear.
//This is the solver step, not00509AA0/0050B180 or activation readiness.
template<class Frames,class Queries,class Receivers,class Collision>void car_cold_solver_step(Frames& frames,
        typename Frames::Frame model,const CarPhysics& physics,const CarTyres& tyres,
        CarBodyIntegration<Frames>& body,std::vector<CarWheelRuntime<Frames>>& wheels,
        CarDrivetrainState& drive,CarColdSolverState<Frames>& state,Queries& queries,Receivers& receivers,
        float elapsed,const std::array<float,3>& drag,float first_drive,float second_drive,
        float first_input,float second_input,std::uint32_t layers,std::uint32_t flags,Collision& collision){
    using Vector=typename Frames::Vector;
    if(drive.running||!(drive.body_flags&0x1000000u)||first_drive!=0||second_drive!=0)
        throw std::runtime_error("Car loading solver requires actual engine-off drive outputs");
    ++state.substeps.index;
    (void)prepare_car_wheel_traction(frames,wheels,tyres,body.pose().up,state.additional_grip);
    const auto wheel_forces=car_cold_wheel_forces(frames,wheels,tyres,physics,drive,state.speed,body.pose().centre);
    state.driven_contact=wheel_forces.driven_contact;if(wheel_forces.clear_wheel_spin)state.wheel_spin=false;
    const auto forces=car_cold_body_forces(frames,physics,drive,body,wheels,wheel_forces,drag,
        state.external_force,state.external_torque,state.substeps.flags,state.speed,first_input,second_input);
    state.published_net=forces.published_net;
    const auto candidate=body.predict(elapsed,forces.force,forces.torque);
    body.publish(candidate);
    state.queries.moving_primitive=collision(body,state.substeps,candidate,elapsed,state.speed,state.mass_changed,
        drive.audible,drive.gear!=0,drive.body_flags)||state.queries.moving_primitive;
    body.refresh();
    const auto square=[](const Vector& v){return static_cast<float>((static_cast<long double>(v[2])*v[2]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[0])*v[0]);};
    const auto length=[&](const Vector& v){const auto value=square(v);std::int32_t bits;std::memcpy(&bits,&value,4);return bits<0x322bcc77?0.f:std::abs(value-1.f)<9.99999993922529e-09f?1.f:static_cast<float>(std::sqrt(static_cast<long double>(value)));};
    state.speed=length(body.motion().velocity);state.rolling_speed=0;
    Vector down{};for(unsigned axis=0;axis!=3;++axis)down[axis]=-body.pose().up[axis];
    auto sum=car_cold_wheel_contacts(frames,model,body,wheels,tyres,queries,receivers,state.queries,
        elapsed,down,true,state.substeps.flags,drive.body_flags,state.speed,drive.audible,layers,flags);
    if(square(sum)<=.5f)state.rolling_speed=state.speed;
    else{
        const float squared=static_cast<float>((static_cast<long double>(sum[0])*sum[0]+static_cast<long double>(sum[1])*sum[1])+static_cast<long double>(sum[2])*sum[2]);
        if(std::abs(static_cast<long double>(squared)-1)>=9.99999993922529e-09f){
            const auto inverse=1/std::sqrt(static_cast<long double>(squared));for(unsigned axis=0;axis!=3;++axis)sum[axis]=static_cast<float>(sum[axis]*inverse);
        }
        const auto& direction=body.motion().direction;
        state.rolling_speed=static_cast<float>(std::abs((static_cast<long double>(sum[0])*direction[0]+static_cast<long double>(sum[2])*direction[2])+static_cast<long double>(sum[1])*direction[1])*state.speed);
    }
    if(square(body.motion().velocity)<=.0010000000474974513f&&square(body.motion().angular_velocity)<=.0010000000474974513f&&state.driven_contact&&!state.mass_changed){
        if(first_input<.30000001192092896f&&state.stable_milliseconds>0){body.motion().velocity={};body.motion().angular_velocity={};body.motion().direction={};state.speed=0;}
        if(!(drive.body_flags&0x20000000u))drive.body_flags|=2u;
    }
    const auto kind=static_cast<unsigned>(physics.scalars().get(0x550));
    if(kind==0||kind==1||kind==2)update_car_drivetrain(physics,drive,elapsed,0,0,state.speed,state.speed_angle,body.pose().forward,body.motion().velocity,receivers);
    state.speed_angle=frames.car_vector_angle(body.pose().forward,body.motion().velocity);
}

struct CarLoadingWithoutBodyCollision {
    template<class Body,class Substeps,class Candidate>bool operator()(Body&,Substeps& steps,const Candidate&,
            float,float,bool,bool,bool,std::uint32_t&)const{
        if(steps.flags&0x10u)throw std::runtime_error("Car BODYCOL acceptance/rollback requires its actual controller");
        return false;
    }
};
//Existing settling-only entry retains its explicit boundary. The containing
//ordinary controller must call the overload above with its SAME BODYCOL owner.
template<class Frames,class Queries,class Receivers>void car_cold_solver_step(Frames& frames,
        typename Frames::Frame model,const CarPhysics& physics,const CarTyres& tyres,
        CarBodyIntegration<Frames>& body,std::vector<CarWheelRuntime<Frames>>& wheels,
        CarDrivetrainState& drive,CarColdSolverState<Frames>& state,Queries& queries,Receivers& receivers,
        float elapsed,const std::array<float,3>& drag,float first_drive,float second_drive,
        float first_input,float second_input,std::uint32_t layers,std::uint32_t flags){
    //Reject before any mutation, including the candidate publication.
    if(state.substeps.flags&0x10u)throw std::runtime_error("Car BODYCOL acceptance/rollback requires its actual controller");
    CarLoadingWithoutBodyCollision collision;
    car_cold_solver_step(frames,model,physics,tyres,body,wheels,drive,state,queries,receivers,elapsed,
        drag,first_drive,second_drive,first_input,second_input,layers,flags,collision);
}
} //namespace hd2::game
#endif
