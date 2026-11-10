#ifndef HD2_GAME_CAR_WHEEL_TRACTION_H
#define HD2_GAME_CAR_WHEEL_TRACTION_H

#include "game/car_wheel_runtime.h"

namespace hd2::game {

struct CarWheelTractionTotals {
    std::array<float,4> spring_force{{0,0,0,0}};
    std::array<float,2> driven_limits{{0,0}};
};

//First wheel pass in0050DE60. Actual preceding wheel queries must produce
//normal80, contact velocityA0/directionB0, load168, angle1D4 and slip flag200.
//Cold no-contact wheels follow the original explicit clear/skip branch.
//No surface or contact is fabricated when the actual tyre registry lacks one.
template<class Frames>CarWheelTractionTotals prepare_car_wheel_traction(Frames& frames,
        std::vector<CarWheelRuntime<Frames>>& wheels,const CarTyres& tyres,
        const typename Frames::Vector& up,float additional_grip){
    using Vector=typename Frames::Vector;CarWheelTractionTotals totals;unsigned side=0;
    for(auto index=wheels.size();index;side^=1u){
        auto& wheel=wheels[--index];auto& scalars=wheel.scalars;
        if(!(wheel.flags&8u)||(wheel.flags&0x40000400u)){
            scalars.set(0x1b0,0);scalars.set(0x1b4,0);scalars.set(0x1d4,0);wheel.vectors[0x70]=Vector{};continue;
        }
        const auto& velocity=wheel.vectors.at(0xa0);const auto& direction=wheel.vectors.at(0xb0);
        const auto projection=(static_cast<long double>(direction[0])*velocity[0]+static_cast<long double>(direction[1])*velocity[1])+static_cast<long double>(direction[2])*velocity[2];
        Vector lateral{};
        for(unsigned axis=0;axis!=3;++axis){
            auto amount=projection*direction[axis];if(axis)amount=static_cast<float>(amount);
            lateral[axis]=static_cast<float>(amount-velocity[axis]);
        }
        wheel.vectors[0xe0]=lateral;scalars.set(0x1ac,1);
        const auto world=frames.actor_world_matrix(wheel.frame);const auto& normal=wheel.vectors.at(0x80);
        const auto angle=frames.car_vector_angle(Vector{{world[0],world[1],world[2],world[3]}},normal);
        const float alignment=static_cast<float>(std::abs((static_cast<long double>(angle)-1.5707963705062866f)*5.729577541351318f)); //00810994.
        const float amount=1.f-std::min(alignment,1.f);scalars.set(0x1a8,amount);
        const CarTyreSurface* surface=nullptr;
        if(wheel.material_type!=0){
            if(wheel.material_type<0||wheel.material_surface<0)throw std::runtime_error("Car contact requires its actual tyre type and surface");
            surface=&tyres.types().at(static_cast<std::size_t>(wheel.material_type)).surfaces.at(static_cast<std::size_t>(wheel.material_surface));
        }
        const auto coefficient=[&](unsigned builtin,unsigned field){return surface?surface->coefficients.at(field):scalars.get(builtin);};
        const auto blend=coefficient(0x190,4);
        const auto grip=(1-static_cast<long double>(blend))*amount+blend;
        const bool slipping=(wheel.flags&0x80000000u)!=0;
        scalars.set(0x1b0,static_cast<float>(grip*coefficient(slipping?0x1c8:0x1b8,slipping?9:5)));
        scalars.set(0x1b4,static_cast<float>(grip*coefficient(slipping?0x1cc:0x1bc,slipping?10:6)));
        const auto square=(static_cast<long double>(lateral[2])*lateral[2]+static_cast<long double>(lateral[1])*lateral[1])+static_cast<long double>(lateral[0])*lateral[0];
        if((wheel.flags&0x200u)&&square>1){
            const auto low=coefficient(0x194,1),high=coefficient(0x198,2);
            auto scale=(static_cast<long double>(scalars.get(0x1d4))-low)/(static_cast<long double>(high)-low);
            if(scale>1)scale=1; //Original does not clamp the lower end.
            scalars.set(0x1ac,static_cast<float>((1-(1-static_cast<long double>(coefficient(0x19c,3)))*scale)*scalars.get(0x1ac)));
        }
        const auto load=scalars.get(0x168),reduction=scalars.get(0x1ac);
        const auto longitudinal=static_cast<long double>(load)*scalars.get(0x1b0)*reduction;
        const auto transverse=static_cast<long double>(load)*reduction*scalars.get(0x1b4);
        scalars.set(0x1b0,static_cast<float>(longitudinal+additional_grip));scalars.set(0x1b4,static_cast<float>(transverse+additional_grip));
        Vector spring{};for(unsigned axis=0;axis!=3;++axis)spring[axis]=load*normal[axis];
        const auto slope=(static_cast<long double>(up[0])*normal[0]+static_cast<long double>(normal[1])*up[1])+static_cast<long double>(normal[2])*up[2];
        if(slope>=.5f){const auto scale=1/slope;for(unsigned axis=0;axis!=3;++axis)spring[axis]=static_cast<float>(spring[axis]*scale);}
        else for(unsigned axis=0;axis!=3;++axis)spring[axis]+=spring[axis];
        wheel.vectors[0x70]=spring;
        for(unsigned axis=0;axis!=3;++axis)totals.spring_force[axis]+=spring[axis];
        const auto limit=coefficient(slipping?0x1c4:0x18c,slipping?8:0);
        scalars.set(0x1b0,std::min(scalars.get(0x1b0),limit));scalars.set(0x1b4,std::min(scalars.get(0x1b4),limit));
        if((wheel.flags&0x102u)&&!(wheel.flags&0x400u))totals.driven_limits[side]+=scalars.get(0x1b0);
    }
    return totals;
}
} //namespace hd2::game
#endif
