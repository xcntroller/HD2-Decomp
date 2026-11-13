#ifndef HD2_GAME_CAR_DEFORM_DAMAGE_H
#define HD2_GAME_CAR_DEFORM_DAMAGE_H
#include "game/car_part_variants.h"
#include "game/car_light_receivers.h"
#include "game/car_wheel_runtime.h"

namespace hd2::game {
//004FF8B0's reached damage tails, sharing the actual mutable part/light/wheel
//records. Callback bodies and glass effects belong to the containing runtime.
template<class Frames,class Geometry,class Runtime>void damage_car_deformed_light(Frames&,Geometry& geometry,
        std::size_t index,float amount,Runtime& runtime){
    const auto& fields=geometry.physics().scalars();auto& light=geometry.lighting().lights().at(index);
    light.field14-=amount; //Common FLOAT amount was produced BEFORE the light loop.
    if(light.field14<fields.get(0x564))light.flags10|=1u;
    if(light.field14<0)light.field14=0;
    runtime.variant_light(light);
}
template<class Frames,class Geometry,class Runtime>void damage_car_deformed_part(Frames& frames,Geometry& geometry,
        CarBodyIntegration<Frames>& body,std::uint32_t& dirty,std::size_t index,float absorbed,
        const CarContactVector& direction,const CarContactVector* impulse,Runtime& runtime){
    auto& part=geometry.deformables().parts().at(index);const auto& fields=geometry.physics().geometry().parts.at(index).scalars;
    if(!(part.flags&2u)){
        part.health=static_cast<float>(static_cast<long double>(part.health)-(static_cast<long double>(absorbed)/fields.get(0x14))*fields.get(0x1c));
        if(part.health<0)part.health=0;
        update_car_part_variant(frames,geometry,body,index,impulse,true,runtime);
    }
    part.flags|=2u;dirty|=2u;
    if(part.type==3)runtime.variant_glass(index,direction);
    if(!(part.health<=0)){
        if(part.type==5)for(auto n=geometry.lighting().lights().size();n;){auto& light=geometry.lighting().lights()[--n];
            if(light.part1a!=index)continue;
            light.field14=part.health;
            if(light.field14<geometry.physics().scalars().get(0x564)){light.flags10|=1u;runtime.variant_light(light);}
        }
    }else{
        runtime.variant_detach(index,car_part_damage_velocity(frames,body,part.frame,false),impulse);
        for(auto n=geometry.lighting().lights().size();n;){auto& light=geometry.lighting().lights()[--n];
            if(light.part1a!=index)continue;
            light.field14=0;light.flags10|=1u;runtime.variant_light(light);
        }
    }
}
template<class Frames,class Runtime>void damage_car_deformed_wheel(Frames& frames,CarBodyIntegration<Frames>& body,
        std::vector<CarWheelRuntime<Frames>>& wheels,std::uint32_t& dirty,std::size_t index,float impulse,Runtime& runtime){
    auto& wheel=wheels.at(index);dirty|=2u;if(wheel.flags&0x10000000u)return;
    const auto absorption=static_cast<long double>(wheel.scalars.get(0x238))*.009999999776482582f;
    wheel.flags|=0x10000000u;
    const auto health=static_cast<long double>(wheel.scalars.get(0x230))-
        ((static_cast<long double>(impulse)-absorption*impulse)/wheel.scalars.get(0x23c))*wheel.scalars.get(0x234);
    wheel.scalars.set(0x230,static_cast<float>(health));
    if(health<0){ //Native strict negative, unlike packet2's <=0 branch.
        wheel.scalars.set(0x230,0);const auto& position=wheel.vectors.at(0x40);std::array<long double,3> offset{},cross{};
        for(unsigned a=0;a<3;++a)offset[a]=static_cast<long double>(position[a])-body.pose().centre[a];
        const auto& angular=body.motion().angular_velocity;
        for(unsigned a=0;a<3;++a){const auto n=(a+1)%3,l=(a+2)%3;cross[a]=angular[n]*offset[l]-angular[l]*offset[n];if(a<2)cross[a]=static_cast<float>(cross[a]);}
        auto velocity=body.motion().velocity;for(unsigned a=0;a<3;++a)velocity[a]=static_cast<float>(velocity[a]-cross[a]);
        runtime.variant_wheel(index,velocity);
    }
    (void)frames;
}
} //namespace hd2::game
#endif
