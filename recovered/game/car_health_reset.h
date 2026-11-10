#ifndef HD2_GAME_CAR_HEALTH_RESET_H
#define HD2_GAME_CAR_HEALTH_RESET_H

#include "game/car_drivetrain.h"
#include "game/car_wheel_runtime.h"
#include "game/car_contact_callbacks.h"
#include "game/car_lighting.h"

namespace hd2::game {

struct CarHealthState {float body=0,drivetrain=0;}; //610/63C, separate from60C's torque multiplier.

//004FEEA0 resets the actual wheel/link/paired-projector bindings. Single-texture
//projectors keep their original texture; native5A0 contains only shared pairs.
//The runtime's paired_flags uses the SAME geometry projector indices.
template<class Frames>void reset_car_health(Frames& frames,const CarPhysics& physics,
        CarDrivetrainState& drive,CarHealthState& health,std::vector<CarWheelRuntime<Frames>>& wheels,
        std::vector<CarContactLink>& links,const CarLighting<Frames>& lighting,std::vector<std::uint16_t>& paired_flags){
    health.drivetrain=physics.scalars().get(0x640);health.body=physics.scalars().get(0x614);drive.health=1;
    for(auto& wheel:wheels)wheel.reset_loading_health();
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
