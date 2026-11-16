#ifndef HD2_GAME_CAR_SOLVER_H
#define HD2_GAME_CAR_SOLVER_H

#include "game/car_cold_solver.h"
#include "game/car_powered_body_forces.h"
#include "game/car_steering.h"

namespace hd2::game {

//0050DE60 on actual initialized Car owners. Wheel query/notification and
//BODYCOL are mandatory existing receivers. Plane-cache and absent active
//particle producers retain their own downstream boundaries; none are replaced.
template<class Frames,class State,class Queries,class Receivers,class Collision>void car_solver_step(Frames& frames,
        typename Frames::Frame model,const CarPhysics& physics,const CarTyres& tyres,State& state,
        Queries& queries,Receivers& receivers,float elapsed,const std::array<float,3>& drag,
        float first_drive,float second_drive,std::uint32_t layers,std::uint32_t flags,Collision& collision){
    using Vector=typename Frames::Vector;auto& body=state.body();auto& wheels=state.wheels;auto& solver=state.solver;auto& drive=state.drive;
    ++solver.substeps.index;
    for(unsigned a=0;a!=3;++a){solver.published_net[a]=0;solver.drive_force[a]=0;}
    const float engine_road=static_cast<float>((static_cast<long double>(drive.engine_speed)/car_gear_ratio(physics,drive.gear))*physics.scalars().get(0x8d8));
    const auto traction=prepare_car_wheel_traction(frames,wheels,tyres,body.pose().up,solver.additional_grip,&solver.published_net);
    const auto aero=car_solver_aero(frames,physics,body,drag,solver.substeps.flags,solver.published_net);
    solver.differential_mode=1; //0050E9A0, unconditional original publication.
    const auto forces=car_solver_wheel_forces(frames,physics,tyres,state,traction,first_drive,second_drive,engine_road,aero.force,aero.torque);
    const auto net=finish_car_solver_forces(frames,physics,state,forces);
    const auto candidate=body.predict(elapsed,net.force,net.torque,&solver.published_net);
    body.publish(candidate);
    solver.queries.moving_primitive=collision(body,solver.substeps,candidate,elapsed,solver.speed,solver.mass_changed,
        drive.audible,drive.gear!=0,drive.body_flags)||solver.queries.moving_primitive;
    body.refresh();
    if(state.finalizer.field2287){
        Vector toward{};for(unsigned a=0;a!=3;++a)toward[a]=state.finalizer.steering_target[a]-body.physical_position()[a];
        const auto& forward=body.pose().forward;const auto& right=body.pose().right;
        float requested=frames.car_vector_angle(toward,forward)/state.steering.maximum;
        const auto dot=(static_cast<long double>(toward[0])*right[0]+static_cast<long double>(toward[1])*right[1])+static_cast<long double>(toward[2])*right[2];
        if(dot<=0){requested=-requested;if(requested< -1)requested=-1;}else if(requested>1)requested=1;
        request_car_steering(state.steering,requested);update_car_steering(frames,model,physics,body,wheels,state.steering,elapsed*1000.f,solver.speed);
    }
    solver.speed=car_solver_length(body.motion().velocity);solver.rolling_speed=0;
    Vector down{};for(unsigned a=0;a!=3;++a)down[a]=-body.pose().up[a];
    auto sum=car_cold_wheel_contacts(frames,model,body,wheels,tyres,queries,receivers,solver.queries,elapsed,down,true,
        solver.substeps.flags,drive.body_flags,solver.speed,drive.audible,layers,flags,solver.substeps.count);
    const float square=static_cast<float>((static_cast<long double>(sum[0])*sum[0]+static_cast<long double>(sum[2])*sum[2])+static_cast<long double>(sum[1])*sum[1]);
    if(square<=.5f)solver.rolling_speed=solver.speed;
    else{sum=car_solver_normalize(sum);const auto& direction=body.motion().direction;
        solver.rolling_speed=static_cast<float>(std::abs((static_cast<long double>(sum[0])*direction[0]+static_cast<long double>(sum[2])*direction[2])+static_cast<long double>(sum[1])*direction[1])*solver.speed);}
    const auto& angular=body.motion().angular_velocity;
    const auto angular_square=(static_cast<long double>(angular[0])*angular[0]+static_cast<long double>(angular[2])*angular[2])+static_cast<long double>(angular[1])*angular[1];
    if(car_solver_square(body.motion().velocity)<=.0010000000474974513f&&angular_square<=.0010000000474974513f&&drive.flag2293&&!solver.mass_changed){
        if(state.control_requests.first<.30000001192092896f&&solver.stable_milliseconds>0){
            for(unsigned a=0;a!=3;++a){body.motion().velocity[a]=0;body.motion().angular_velocity[a]=0;body.motion().direction[a]=0;}solver.speed=0;
        }
        if(!(drive.body_flags&0x20000000u))drive.body_flags|=2u;
    }
    const auto kind=static_cast<unsigned>(physics.scalars().get(0x550));
    if(kind==0||kind==2)update_car_drivetrain(physics,drive,elapsed,first_drive+second_drive,forces.brake_reaction,solver.speed,solver.speed_angle,body.pose().forward,body.motion().velocity,receivers);
    else if(kind==1){if(drive.throttle<=0)first_drive=-first_drive;update_car_drivetrain(physics,drive,elapsed,first_drive+first_drive,forces.brake_reaction,solver.speed,solver.speed_angle,body.pose().forward,body.motion().velocity,receivers);}
    solver.speed_angle=frames.car_vector_angle(body.pose().forward,body.motion().velocity);
}
} //namespace hd2::game
#endif
