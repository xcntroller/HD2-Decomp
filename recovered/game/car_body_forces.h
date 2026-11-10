#ifndef HD2_GAME_CAR_BODY_FORCES_H
#define HD2_GAME_CAR_BODY_FORCES_H

#include "game/car_cold_wheel_forces.h"
#include "game/car_body_integration.h"

namespace hd2::game {

template<class Frames>struct CarBodyForces {
    using Vector=typename Frames::Vector;
    Vector force{},torque{},published_net{};
};

//0050DE60's aero/external-force accumulation and low-speed contact stabilization
//around the actual engine-off wheel pass. The result is input to integration,
//not a substitute for model publication, collision acceptance or wheel queries.
template<class Frames>CarBodyForces<Frames> car_cold_body_forces(Frames& frames,
        const CarPhysics& physics,const CarDrivetrainState& drive,
        const CarBodyIntegration<Frames>& body,const std::vector<CarWheelRuntime<Frames>>& wheels,
        const CarColdWheelForces& wheel_forces,const std::array<float,3>& drag,
        const typename Frames::Vector& external_force,const typename Frames::Vector& external_torque,
        std::uint32_t features,float speed,float first_input,float second_input){
    if(drive.running||!(drive.body_flags&0x1000000u))throw std::runtime_error("Car cold body forces require the actual engine-off branch");
    using Vector=typename Frames::Vector;CarBodyForces<Frames> result;const auto& pose=body.pose();
    const auto& fields=physics.scalars();
    result.force=wheel_forces.force;
    const auto torque=[](const Vector& force,const Vector& offset){Vector result{};for(unsigned axis=0;axis!=3;++axis){const auto n=(axis+1)%3,l=(axis+2)%3;result[axis]=static_cast<float>(static_cast<long double>(force[n])*offset[l]-static_cast<long double>(force[l])*offset[n]);}return result;};
    if(features&8u){
        for(unsigned n=0;n!=2;++n){
            const unsigned base=0x1c0+n*0x10;
            const auto point=frames.car_transform(Vector{{fields.get(base),fields.get(base+4),fields.get(base+8),0}},pose.world);
            Vector force{},offset{};for(unsigned axis=0;axis!=3;++axis){force[axis]=-pose.up[axis]*drag[n+1];offset[axis]=point[axis]-pose.centre[axis];result.force[axis]+=force[axis];}
            const auto moment=torque(force,offset);for(unsigned axis=0;axis!=3;++axis)result.torque[axis]+=moment[axis];
        }
    }
    for(unsigned axis=0;axis!=3;++axis){
        const float air=-drag[0]*body.motion().direction[axis];
        result.force[axis]=static_cast<float>(static_cast<long double>(result.force[axis])+air+external_force[axis]);
        result.torque[axis]=static_cast<float>((static_cast<long double>(result.torque[axis])+wheel_forces.friction_torque[axis])+wheel_forces.spring_torque[axis]+external_torque[axis]);
        result.published_net[axis]=result.force[axis]+fields.get(0x200+axis*4);
    }
    bool contact=false;for(const auto& wheel:wheels)if((wheel.flags&0x40000408u)==8){contact=true;break;}
    if(speed<3.f&&contact){ //0080F538=3, not the separate speed10 constant.
        Vector spring{};float longitudinal=0,transverse=0;
        for(auto n=wheels.size();n;){const auto& wheel=wheels[--n];if((wheel.flags&0x40000408u)!=8)continue;
            for(unsigned axis=0;axis!=3;++axis)spring[axis]+=wheel.vectors.at(0x70)[axis];
            longitudinal+=wheel.scalars.get(0x1b0);transverse+=wheel.scalars.get(0x1b4);
        }
        frames.car_transform_in_place(spring,pose.inverse_rotation);
        auto gravity=frames.car_transform(Vector{{fields.get(0x200),fields.get(0x204),fields.get(0x208),0}},pose.inverse_rotation);
        gravity[0]+=spring[0];gravity[2]+=spring[2];
        if(transverse<std::abs(gravity[0]))gravity[0]=gravity[0]<=0?-transverse:transverse;
        Vector correction{};correction[0]=static_cast<float>(-(1-static_cast<long double>(speed)*.3333333432674408f)*gravity[0]);
        if(drive.gear&&speed<2.f&&first_input<.30000001192092896f&&second_input<.30000001192092896f){
            bool spinning=false;for(auto n=wheels.size();n;){const auto flags=wheels[--n].flags;if((flags&0x102u)&&!(flags&0x400u)&&(flags&8u)&&(flags&0x10u)){spinning=true;break;}}
            if(!spinning){
                if(longitudinal<std::abs(gravity[2]))gravity[2]=gravity[2]<=0?-longitudinal:longitudinal;
                correction[2]=static_cast<float>(-(1-static_cast<long double>(speed)*.5f)*gravity[2]);
            }
        }
        frames.car_transform_in_place(correction,pose.rotation);
        for(unsigned axis=0;axis!=3;++axis)result.force[axis]+=correction[axis];
        result.torque[1]=static_cast<float>(static_cast<long double>(result.torque[1])-(1-static_cast<long double>(speed)*.3333333432674408f)*wheel_forces.spring_torque[1]);
        //Native diagnostic20 remains the uncorrected net force above.
    }
    return result;
}
} //namespace hd2::game
#endif
