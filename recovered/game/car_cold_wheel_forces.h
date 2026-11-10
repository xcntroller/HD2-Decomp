#ifndef HD2_GAME_CAR_COLD_WHEEL_FORCES_H
#define HD2_GAME_CAR_COLD_WHEEL_FORCES_H

#include "game/car_wheel_traction.h"
#include "game/car_drivetrain.h"
#include "game/actor_activation_math.h"

namespace hd2::game {

struct CarColdWheelForces {
    std::array<float,4> force{{0,0,0,0}},spring_torque{{0,0,0,0}},friction_torque{{0,0,0,0}},normals{{0,0,0,0}};
    bool driven_contact=false,clear_wheel_spin=false;
};

//Second wheel pass in0050DE60 when00509AA0 has selected engine-off1000000.
//Requires the preceding traction pass and actual wheel query outputs. Powered
//drive/brake control is an explicit separate branch, never a cold substitute.
template<class Frames>CarColdWheelForces car_cold_wheel_forces(Frames& frames,
        std::vector<CarWheelRuntime<Frames>>& wheels,const CarTyres& tyres,
        const CarPhysics& physics,CarDrivetrainState& drive,float speed,
        const typename Frames::Vector& centre){
    using Vector=typename Frames::Vector;
    if(drive.running||!(drive.body_flags&0x1000000u))throw std::runtime_error("Car cold wheel forces require the actual engine-off branch");
    CarColdWheelForces result;drive.engine_load=0;
    const auto squared=[](const Vector& v){return static_cast<float>((static_cast<long double>(v[0])*v[0]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[2])*v[2]);};
    const auto length=[&](const Vector& v){const float square=squared(v);std::int32_t bits;std::memcpy(&bits,&square,4);return bits<0x322bcc77?0.f:std::abs(square-1.f)<9.99999993922529e-09f?1.f:static_cast<float>(std::sqrt(static_cast<long double>(square)));};
    const auto add_torque=[](Vector& total,const Vector& force,const Vector& point,const Vector& origin){
        Vector offset{};for(unsigned axis=0;axis!=3;++axis)offset[axis]=point[axis]-origin[axis];
        //Native uses force CROSS offset (row-vector convention), not offset CROSS force.
        for(unsigned axis=0;axis!=3;++axis){const auto next=(axis+1)%3,last=(axis+2)%3;auto amount=static_cast<long double>(offset[last])*force[next]-static_cast<long double>(offset[next])*force[last];if(axis!=2)amount=static_cast<float>(amount);total[axis]=static_cast<float>(amount+total[axis]);}
    };
    unsigned side=0;
    for(auto index=wheels.size();index;side^=1u){
        auto& wheel=wheels[--index];const auto previous_flags=wheel.flags;wheel.flags&=0xfffffd4fu;
        if(!(previous_flags&8u)||(previous_flags&0x40000400u))continue;
        auto& fields=wheel.scalars;const auto& velocity=wheel.vectors.at(0xa0);const auto& direction=wheel.vectors.at(0xb0);
        auto lateral=wheel.vectors.at(0xe0);
        const auto angle=frames.car_vector_angle(direction,velocity);
        const auto slip=std::sin(static_cast<long double>(angle));fields.set(0x1d4,static_cast<float>(slip));
        if(squared(lateral)>1){
            if(fields.get(0x21c)<slip)wheel.flags|=0x10u;
            lateral=actor_activation_direction(lateral);
        }
        const CarTyreSurface* surface=nullptr;
        if(wheel.material_type!=0){
            if(wheel.material_type<0||wheel.material_surface<0)throw std::runtime_error("Car contact requires its actual tyre surface");
            surface=&tyres.types().at(static_cast<std::size_t>(wheel.material_type)).surfaces.at(static_cast<std::size_t>(wheel.material_surface));
        }
        const auto minimum=surface?surface->coefficients[1]:fields.get(0x194);
        if(fields.get(0x1d4)>=minimum)wheel.flags|=0x200u;
        //The earlier scaled lateral temporary is overwritten by this engine-off branch.
        const auto longitudinal=fields.get(0x1b0),transverse=fields.get(0x1b4);
        const auto friction=std::max(longitudinal,transverse);
        Vector force{};for(unsigned axis=0;axis!=3;++axis)force[axis]=-(velocity[axis]*friction);
        const auto force_angle=frames.car_vector_angle(force,direction);
        const auto tangent=std::tan(static_cast<long double>(force_angle)-1.5707963705062866f);
        const auto denominator=static_cast<long double>(longitudinal)*longitudinal+tangent*tangent*transverse*transverse;
        const auto ellipse=(static_cast<long double>(longitudinal)/denominator)*std::sqrt(denominator)*transverse;
        const float limit=static_cast<float>(std::sqrt(ellipse*ellipse+ellipse*tangent*ellipse*tangent));
        if(!std::isfinite(limit))throw std::runtime_error("Car contact produced an undefined friction ellipse");
        const auto force_length=length(force);fields.set(0x274,force_length/limit);
        if(limit<force_length){force=actor_activation_direction(force);for(unsigned axis=0;axis!=3;++axis)force[axis]*=limit;fields.set(0x274,1);}
        if(wheel.flags&0x102u)result.clear_wheel_spin=true; //The running-null gate is sufficient; no driving receiver selected.
        const bool sliding=(wheel.flags&0x80000000u)!=0;
        const auto resistance=surface?surface->coefficients[sliding?11:7]:fields.get(sliding?0x1d0:0x1c0);
        float rolling=static_cast<float>((static_cast<long double>(resistance)/fields.get(0x1d8))*fields.get(0x168));
        if(longitudinal<rolling)rolling=longitudinal;
        if(wheel.flags&0x10000u)rolling=-rolling;
        const float scaled=std::min(speed,1.f)*rolling;
        for(unsigned axis=0;axis!=3;++axis){const float addition=scaled*direction[axis];force[axis]+=addition;}
        wheel.vectors[0xe0]=force;
        if(wheel.flags&0x102u){
            result.driven_contact=true;
            auto amount=static_cast<long double>(longitudinal)*fields.get(0x1d8);
            const auto kind=static_cast<unsigned>(physics.scalars().get(0x550));
            if(kind==1)amount*=std::abs(side?drive.throttle:drive.second_throttle);
            else if(kind!=0&&kind!=2)amount=0;
            drive.engine_load=static_cast<float>(amount+drive.engine_load);
        }
        const auto& spring=wheel.vectors.at(0x70);
        add_torque(result.spring_torque,spring,wheel.vectors.at(0x50),centre);
        add_torque(result.friction_torque,force,wheel.vectors.at(0x40),centre);
        for(unsigned axis=0;axis!=3;++axis){result.force[axis]=static_cast<float>(static_cast<long double>(result.force[axis])+spring[axis]+force[axis]);result.normals[axis]+=wheel.vectors.at(0x80)[axis];}
    }
    return result;
}
} //namespace hd2::game
#endif
