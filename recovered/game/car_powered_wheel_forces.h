#ifndef HD2_GAME_CAR_POWERED_WHEEL_FORCES_H
#define HD2_GAME_CAR_POWERED_WHEEL_FORCES_H

#include "game/car_cold_wheel_forces.h"

namespace hd2::game {

struct CarSolverWheelForces:CarColdWheelForces {float drive_magnitude=0,brake_reaction=0;};
inline std::int32_t car_solver_float_bits(float value){std::int32_t bits;std::memcpy(&bits,&value,4);return bits;}
template<class Vector>float car_solver_square(const Vector& v){return static_cast<float>((static_cast<long double>(v[2])*v[2]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[0])*v[0]);}
template<class Vector>float car_solver_length(const Vector& v){const auto s=car_solver_square(v);return car_solver_float_bits(s)<0x322bcc77?0.f:
    std::abs(static_cast<long double>(s)-1)<9.99999993922529e-09f?1.f:static_cast<float>(std::sqrt(static_cast<long double>(s)));}
template<class Vector>Vector car_solver_normalize(Vector v){
    const auto square=car_solver_square(v);
    if(std::abs(static_cast<long double>(square)-1)>=9.99999993922529e-09f){
        if(square>=9.99999993922529e-09f){const auto inverse=1/std::sqrt(static_cast<long double>(square));for(unsigned a=0;a!=3;++a)v[a]=static_cast<float>(v[a]*inverse);}
        else{const auto axis=v[0]!=0?0u:v[2]!=0?2u:1u;v[axis]=v[axis]<0?-1.f:1.f;}
    }return v;
}

//0050EAD3..0050EC71 keeps normalized X/Y extended, spills Z, then
//stores the directional projection. Rounding the reciprocal to FLOAT can
//change the later wheel-lock comparison at the friction limit.
template<class Vector>float car_solver_velocity_projection(const Vector& velocity,const Vector& direction){
    long double x=velocity[0],y=velocity[1];float z=velocity[2];
    const auto sum=(x*x+y*y)+static_cast<long double>(z)*z;const float square=static_cast<float>(sum);
    if(std::abs(sum-1)>=9.99999993922529e-09f){
        if(square>=9.99999993922529e-09f){const auto inverse=1/std::sqrt(static_cast<long double>(square));x*=inverse;y*=inverse;z=static_cast<float>(z*inverse);}
        else if(x==0){if(z==0)y=y<0?-1.f:1.f;else z=z<0?-1.f:1.f;}else x=x<0?-1.f:1.f;
    }
    return static_cast<float>((y*direction[1]+static_cast<long double>(z)*direction[2])+x*direction[0]);
}

//0050DE60 second wheel pass, including1000000's engine-off arm. Native F7C
//is unconditionally written1 immediately before this pass, so its old0/2
//dispatch arms have no producer here. No alternate differential is invented.
//Inputs and diagnostic010/020,2293/229E/229F belong to the SAME initialized Car.
template<class Frames,class State>CarSolverWheelForces car_solver_wheel_forces(Frames& frames,
        const CarPhysics& physics,const CarTyres& tyres,State& state,const CarWheelTractionTotals& traction,
        float first_drive,float second_drive,float engine_road,const typename Frames::Vector& aero_force,
        const typename Frames::Vector& aero_torque){
    using Vector=typename Frames::Vector;auto& drive=state.drive;auto& solver=state.solver;auto& wheels=state.wheels;
    const auto& fields=physics.scalars();auto& controller=state.controller();
    CarSolverWheelForces result;result.force=aero_force;result.friction_torque=aero_torque;
    const float distribution=static_cast<float>(static_cast<long double>(traction.driven_limits[0])+traction.driven_limits[1]+1.f);
    const float sum=first_drive+second_drive;
    const float capacity=static_cast<float>(static_cast<long double>(car_gear_ratio(physics,drive.gear))*drive.clutch*fields.get(0x8f8)*drive.transfer);
    const float torque=capacity<std::abs(sum)?(sum<0?-capacity:capacity):sum;
    const auto saved_brake=drive.brake;const bool saved_limiter=solver.brake_limiter;
    drive.engine_load=0;
    if(drive.governor<solver.speed&&solver.speed<static_cast<long double>(drive.governor)+2.799999952316284f){solver.brake_limiter=true;drive.brake=1;} //00810990.
    auto angular=state.body().motion().angular_velocity;frames.car_transform_in_place(angular,state.body().pose().inverse_rotation);
    drive.flag2293=false;unsigned side=0;
    const auto add_torque=[](Vector& total,const Vector& force,const Vector& point,const Vector& centre){
        Vector offset{};for(unsigned a=0;a!=3;++a)offset[a]=point[a]-centre[a];
        for(unsigned a=0;a!=3;++a){const auto b=(a+1)%3,c=(a+2)%3;auto amount=static_cast<long double>(offset[c])*force[b]-static_cast<long double>(offset[b])*force[c];if(a!=2)amount=static_cast<float>(amount);total[a]=static_cast<float>(amount+total[a]);}
    };
    for(auto remaining=wheels.size();remaining;side^=1u){
        const auto index=--remaining;auto& wheel=wheels[index];const auto old=wheel.flags;wheel.flags&=0xfffffd4fu;
        if(!(old&8u)||(old&0x40000400u))continue;
        auto& s=wheel.scalars;const auto& velocity=wheel.vectors.at(0xa0);const auto& direction=wheel.vectors.at(0xb0);
        auto lateral=wheel.vectors.at(0xe0);const float projection=car_solver_velocity_projection(velocity,direction);
        const auto angle=frames.car_vector_angle(direction,velocity);const auto slip=std::sin(static_cast<long double>(angle));s.set(0x1d4,static_cast<float>(slip));
        if(car_solver_square(lateral)>1){if(s.get(0x21c)<slip)wheel.flags|=0x10u;lateral=car_solver_normalize(lateral);}
        const CarTyreSurface* surface=nullptr;
        if(wheel.material_type!=0){if(wheel.material_type<0||wheel.material_surface<0)throw std::runtime_error("Car solver requires actual tyre surface");surface=&tyres.types().at(static_cast<std::size_t>(wheel.material_type)).surfaces.at(static_cast<std::size_t>(wheel.material_surface));}
        const auto coefficient=[&](unsigned builtin,unsigned field){return surface?surface->coefficients.at(field):s.get(builtin);};
        const auto minimum=coefficient(0x194,1);
        if(minimum<=s.get(0x1d4))wheel.flags|=0x200u;
        else{const float scale=s.get(0x1d4)/minimum;for(unsigned a=0;a!=3;++a)lateral[a]*=scale;}
        const auto longitudinal=s.get(0x1b0),transverse=s.get(0x1b4),radius=s.get(0x1d8);
        for(unsigned a=0;a!=3;++a)lateral[a]*=transverse;
        const float lateral_length=car_solver_length(lateral);
        float braking=0,requested_force=0;
        const auto friction_force=[&]{Vector value{};const auto limit=longitudinal<=transverse?transverse:longitudinal;for(unsigned a=0;a!=3;++a)value[a]=-(velocity[a]*limit);return value;};
        const auto remaining_capacity=[&]{const auto cross=static_cast<long double>(transverse)*transverse,square=static_cast<long double>(longitudinal)*longitudinal;
            return std::sqrt((square*cross-square*lateral_length*lateral_length)/cross);};
        if(!(drive.body_flags&0x1000000u)){
            if(state.brake_amount>0&&(wheel.flags&4u))braking=solver.handbrake_full?fields.get(0x708):static_cast<float>(static_cast<long double>(fields.get(0x708))*state.brake_amount);
            bool yaw_brake=false;const auto previous=drive.brake;
            if(physics.flags().at(0x2290)&&solver.speed>1){
                const auto target=angular[1]>0?1u:0u;
                if(index==target){const auto& other=wheels.at(wheels.size()-(target?2u:1u));
                    if(other.scalars.get(0x1d4)>static_cast<long double>(other.scalars.get(0x194))+.017453279346227646f){lateral[0]=lateral[1]=lateral[2]=0;drive.brake=1;yaw_brake=true;}
                }
            }
            if(drive.brake>0){
                auto force=(static_cast<long double>(drive.brake)/radius)*s.get(0x1f4);
                if(solver.brake_limiter||yaw_brake){const float limit=static_cast<float>(remaining_capacity());if(limit<force)force=limit;}
                braking=static_cast<float>(static_cast<long double>(force)*radius+braking);
            }
            drive.brake=previous;
            if((wheel.flags&0x102u)&&drive.gear){
                if((!(drive.body_flags&4u)||state.control_requests.first!=0||state.control_requests.second!=0)&&state.finalizer.engine_transition){
                    result.brake_reaction+=braking;
                    const float share=static_cast<float>((static_cast<long double>(torque)/distribution)*longitudinal);
                    const float difference=braking-share;
                    if(!(difference<=0))braking=difference;
                    else{requested_force=static_cast<float>((static_cast<long double>(share)-braking)/radius);braking=0;}
                    //674 is an integer word copied through serialized FLOAT storage.
                    if(car_solver_float_bits(fields.get(0x674))<=drive.gear){const auto limit=remaining_capacity();if(limit<requested_force)requested_force=static_cast<float>(limit);}
                    const auto kind=static_cast<unsigned>(fields.get(0x550));
                    const float force=(kind==0||kind==2)&&drive.gear<0?-requested_force:requested_force;
                    for(unsigned a=0;a!=3;++a){const float change=force*direction[a];solver.drive_force[a]+=change;lateral[a]+=change;}
                    result.drive_magnitude+=std::abs(requested_force);
                }else{lateral=friction_force();braking=0;}
            }
            const auto resistance=(static_cast<long double>(projection)/radius)*braking;
            if(std::abs(resistance)<=longitudinal){
                for(unsigned a=0;a!=3;++a){auto amount=resistance*direction[a];if(a==2)amount=static_cast<float>(amount);lateral[a]=static_cast<float>(static_cast<long double>(lateral[a])-amount);}
            }else{lateral=friction_force();wheel.flags|=0x20u;if(car_solver_square(velocity)>1)wheel.flags|=0x10u;}
        }else lateral=friction_force();
        const auto force_angle=frames.car_vector_angle(lateral,direction);const auto tangent=std::tan(static_cast<long double>(force_angle)-1.5707963705062866f);
        const auto denominator=static_cast<long double>(longitudinal)*longitudinal+tangent*tangent*transverse*transverse;
        const auto ellipse=(static_cast<long double>(longitudinal)/denominator)*std::sqrt(denominator)*transverse;
        const float limit=static_cast<float>(std::sqrt(ellipse*ellipse+ellipse*tangent*ellipse*tangent));
        if(!std::isfinite(limit))throw std::runtime_error("Car solver produced an undefined friction ellipse");
        const auto length=car_solver_length(lateral);s.set(0x274,length/limit);
        if(limit<length){lateral=car_solver_normalize(lateral);for(unsigned a=0;a!=3;++a)lateral[a]*=limit;s.set(0x274,1);}
        if(wheel.flags&0x102u){
            const auto dot=(static_cast<long double>(lateral[0])*direction[0]+static_cast<long double>(lateral[1])*direction[1])+static_cast<long double>(lateral[2])*direction[2];
            if(!drive.gear||((drive.body_flags&4u)&&state.control_requests.first==0&&state.control_requests.second==0)||!state.finalizer.engine_transition||
                    std::abs(static_cast<long double>(engine_road)-solver.speed)<=2.0||std::abs(requested_force)<=std::abs(dot))controller.frame_simulated=false;
            else{if(car_solver_square(velocity)>1)wheel.flags|=0x10u;wheel.flags|=0x80u;controller.requests_update=true;}
        }
        const auto rolling_coefficient=coefficient((wheel.flags&0x80000000u)?0x1d0:0x1c0,(wheel.flags&0x80000000u)?11:7);
        float rolling=static_cast<float>((static_cast<long double>(rolling_coefficient)/radius)*s.get(0x168));if(longitudinal<rolling)rolling=longitudinal;
        if(wheel.flags&0x10000u)rolling=-rolling;
        const float scaled=(solver.speed<=1?solver.speed:1.f)*rolling;
        for(unsigned a=0;a!=3;++a)lateral[a]+=scaled*direction[a];wheel.vectors[0xe0]=lateral;
        if(wheel.flags&0x102u){
            drive.flag2293=true;const auto kind=static_cast<unsigned>(fields.get(0x550));
            auto amount=static_cast<long double>(longitudinal)*radius;
            if(kind==1)amount*=std::abs(side?drive.throttle:drive.second_throttle);else if(kind!=0&&kind!=2)amount=0;
            drive.engine_load=static_cast<float>(amount+drive.engine_load);
        }
        const auto& spring=wheel.vectors.at(0x70);const auto& centre=state.body().pose().centre;
        add_torque(result.spring_torque,spring,wheel.vectors.at(0x50),centre);add_torque(result.friction_torque,lateral,wheel.vectors.at(0x40),centre);
        for(unsigned a=0;a!=3;++a){result.force[a]=static_cast<float>(static_cast<long double>(result.force[a])+spring[a]+lateral[a]);solver.published_net[a]+=lateral[a];result.normals[a]+=wheel.vectors.at(0x80)[a];}
    }
    for(unsigned a=0;a!=3;++a)solver.published_net[a]-=solver.drive_force[a];
    solver.brake_limiter=saved_limiter;drive.brake=saved_brake;
    return result;
}
} //namespace hd2::game
#endif
