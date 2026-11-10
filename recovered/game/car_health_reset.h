#ifndef HD2_GAME_CAR_HEALTH_RESET_H
#define HD2_GAME_CAR_HEALTH_RESET_H

#include "game/car_drivetrain.h"
#include "game/car_wheel_runtime.h"
#include "game/car_contact_callbacks.h"
#include "game/car_lighting.h"

namespace hd2::game {

struct CarHealthState {float body=0,drivetrain=0;bool destroyed=false;}; //610/63C/22A0, separate from60C's torque multiplier.

inline void damage_car_health(const CarPhysics& physics,CarDrivetrainState& drive,
        CarHealthState& health,std::uint32_t& dirty,float impulse){ //00500EB0.
    const auto& fields=physics.scalars();
    if(fields.get(0x620)<impulse&&!(dirty&4u)){
        health.body=static_cast<float>(static_cast<long double>(health.body)-
            ((static_cast<long double>(impulse)-static_cast<long double>(fields.get(0x618))*.009999999776482582f*impulse)/fields.get(0x61c))*fields.get(0x614));
        if(health.body<0)health.body=0;
        dirty|=4u;
        drive.health=static_cast<float>((1.L-fields.get(0x608))*(static_cast<long double>(health.body)/fields.get(0x614))+fields.get(0x608));
    }
    if(health.body<=0)health.destroyed=true;
    if(fields.get(0x64c)<impulse&&!(dirty&8u)){
        health.drivetrain=static_cast<float>(static_cast<long double>(health.drivetrain)-
            ((static_cast<long double>(impulse)-static_cast<long double>(fields.get(0x644))*.009999999776482582f*impulse)/fields.get(0x648))*fields.get(0x640));
        if(health.drivetrain<0)health.drivetrain=0;
        dirty|=8u;
    }
}

//004FEEA0 resets the actual wheel/link/paired-projector bindings. Single-texture
//projectors keep their original texture; native5A0 contains only shared pairs.
//The runtime's paired_flags uses the SAME geometry projector indices.
template<class Frames>void reset_car_health(Frames& frames,const CarPhysics& physics,
        CarDrivetrainState& drive,CarHealthState& health,std::vector<CarWheelRuntime<Frames>>& wheels,
        CarLighting<Frames>& lighting,std::vector<std::uint16_t>& paired_flags){
    health.drivetrain=physics.scalars().get(0x640);health.body=physics.scalars().get(0x614);drive.health=1;
    for(auto& wheel:wheels)wheel.reset_loading_health();
    auto& links=lighting.lights();
    for(auto n=links.size();n;){auto& link=links[--n];link.field14=physics.scalars().get(0x560);link.flags10&=~1u;}
    const auto& projectors=lighting.projectors();paired_flags.resize(projectors.size());
    std::vector<bool> shared(projectors.size(),false);
    for(const auto& light:lighting.lights())if(light.shared)shared.at(light.projector)=true;
    for(auto n=projectors.size();n;){const auto index=--n;if(!shared[index])continue;
        const auto& projector=projectors[index];const auto visual=projector.visual.get(),light=projector.light.get();
        if(!visual||!light)throw std::runtime_error("Car health reset lost an actual paired projector/light");
        paired_flags[index]=3;frames.car_projector_texture(visual,projector.textures[0].get());frames.car_light_power(light,2.f);
    }
}
} //namespace hd2::game
#endif
