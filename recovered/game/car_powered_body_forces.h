#ifndef HD2_GAME_CAR_POWERED_BODY_FORCES_H
#define HD2_GAME_CAR_POWERED_BODY_FORCES_H

#include "game/car_body_forces.h"
#include "game/car_powered_wheel_forces.h"

namespace hd2::game {

//0050E95C..0050E9A0: aero precedes the second wheel pass.020 already contains
//first-pass springs, so publish each additional force into that SAME diagnostic.
template<class Frames>CarBodyForces<Frames> car_solver_aero(Frames& frames,const CarPhysics& physics,
        const CarBodyIntegration<Frames>& body,const std::array<float,3>& drag,std::uint32_t features,
        typename Frames::Vector& diagnostic){
    using Vector=typename Frames::Vector;CarBodyForces<Frames> result;const auto& pose=body.pose();const auto& fields=physics.scalars();
    if(features&8u)for(unsigned n=0;n!=2;++n){
        const unsigned base=0x1c0+n*0x10;
        const auto point=frames.car_transform(Vector{{fields.get(base),fields.get(base+4),fields.get(base+8),0}},pose.world);
        Vector force{},offset{};
        for(unsigned a=0;a!=3;++a){force[a]=-pose.up[a]*drag[n+1];offset[a]=point[a]-pose.centre[a];result.force[a]+=force[a];diagnostic[a]+=force[a];}
        for(unsigned a=0;a!=3;++a){const auto b=(a+1)%3,c=(a+2)%3;auto moment=static_cast<long double>(force[b])*offset[c]-static_cast<long double>(force[c])*offset[b];if(a!=2)moment=static_cast<float>(moment);result.torque[a]=static_cast<float>(result.torque[a]+moment);}
    }
    for(unsigned a=0;a!=3;++a){auto force=-static_cast<long double>(drag[0])*body.motion().direction[a];if(a)force=static_cast<float>(force);result.force[a]=static_cast<float>(result.force[a]+force);diagnostic[a]=static_cast<float>(diagnostic[a]+force);}
    return result;
}

//0050FDF9..0051028C. Diagnostic020 excludes010 drive and low-speed correction;
//the body receives the full forces. Remaining longitudinal grip subtracts the
//actual requested drive magnitude, including when the ellipse later limits it.
template<class Frames,class State>CarBodyForces<Frames> finish_car_solver_forces(Frames& frames,
        const CarPhysics& physics,State& state,const CarSolverWheelForces& wheel_forces){
    using Vector=typename Frames::Vector;CarBodyForces<Frames> result;
    auto& solver=state.solver;const auto& body=state.body();const auto& pose=body.pose();const auto& fields=physics.scalars();
    for(unsigned a=0;a!=3;++a){
        result.force[a]=wheel_forces.force[a]+solver.external_force[a];
        result.torque[a]=static_cast<float>((static_cast<long double>(wheel_forces.friction_torque[a])+wheel_forces.spring_torque[a])+solver.external_torque[a]);
        solver.published_net[a]+=solver.external_force[a];
    }
    bool contact=false;for(const auto& wheel:state.wheels)if((wheel.flags&0x40000408u)==8){contact=true;break;}
    if(solver.speed<3&&contact){
        Vector spring{};float longitudinal=0,transverse=0;
        for(auto n=state.wheels.size();n;){const auto& wheel=state.wheels[--n];if((wheel.flags&0x40000408u)!=8)continue;
            for(unsigned a=0;a!=3;++a)spring[a]+=wheel.vectors.at(0x70)[a];
            longitudinal+=wheel.scalars.get(0x1b0);transverse+=wheel.scalars.get(0x1b4);
        }
        longitudinal-=wheel_forces.drive_magnitude;
        frames.car_transform_in_place(spring,pose.inverse_rotation);
        auto gravity=frames.car_transform(Vector{{fields.get(0x200),fields.get(0x204),fields.get(0x208),0}},pose.inverse_rotation);
        gravity[0]+=spring[0];gravity[2]+=spring[2];
        if(transverse<std::abs(gravity[0]))gravity[0]=gravity[0]<=0?-transverse:transverse;
        Vector correction{};correction[0]=static_cast<float>(-(1-static_cast<long double>(solver.speed)*.3333333432674408f)*gravity[0]);
        if(state.drive.gear&&solver.speed<2&&state.control_requests.first<.30000001192092896f&&state.control_requests.second<.30000001192092896f){
            bool spinning=false;for(auto n=state.wheels.size();n;){const auto flags=state.wheels[--n].flags;if((flags&0x102u)&&!(flags&0x400u)&&(flags&8u)&&(flags&0x10u)){spinning=true;break;}}
            if(!spinning){if(longitudinal<std::abs(gravity[2]))gravity[2]=gravity[2]<=0?-longitudinal:longitudinal;
                correction[2]=static_cast<float>(-(1-static_cast<long double>(solver.speed)*.5f)*gravity[2]);}
        }
        frames.car_transform_in_place(correction,pose.rotation);
        for(unsigned a=0;a!=3;++a)result.force[a]+=correction[a];
        result.torque[1]=static_cast<float>(static_cast<long double>(result.torque[1])-(1-static_cast<long double>(solver.speed)*.3333333432674408f)*wheel_forces.spring_torque[1]);
    }
    return result;
}
} //namespace hd2::game
#endif
