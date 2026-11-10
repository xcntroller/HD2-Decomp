#ifndef HD2_GAME_CAR_LOADING_LIGHTS_H
#define HD2_GAME_CAR_LOADING_LIGHTS_H

#include "game/car_lighting.h"

namespace hd2::game {

struct CarLoadingLightState {
    std::uint32_t flags=0,previous=0; //004EA780's5B0/5B4.
    float blink_elapsed=0,hazard_elapsed=0; //004E2900's5C0/5CC.
};

//00501CB0's cold, unoccupied lights: no indicator/headlight/brake/reverse
//request. Even this path advances the original blink state and retires actual
//billboard/projector/light visibility when80 is present in current OR previous
//features. The reference marker at native entry+0 remains visible.
template<class Frames>void update_car_loading_lights(Frames& frames,const CarLighting<Frames>& lighting,
        CarLoadingLightState& state,float elapsed,std::uint32_t features,std::uint32_t previous_features){
    if(!((features|previous_features)&0x80u))return;
    if((state.flags|state.previous)&0xefffffffu)throw std::runtime_error("Car active lighting requires its actual command receiver");
    state.hazard_elapsed+=elapsed;state.blink_elapsed+=elapsed;
    if(!(state.flags&0x10000000u)){
        if(state.blink_elapsed>=.30000001192092896f){state.blink_elapsed=0;state.flags|=0x10000000u;}
    }else if(state.blink_elapsed>=.4000000059604645f){state.blink_elapsed=0;state.flags&=~0x10000000u;}
    const auto& projectors=lighting.projectors();
    for(auto n=lighting.lights().size();n;){
        const auto& light=lighting.lights()[--n];
        if(light.billboard)frames.object_visible(light.billboard,false);
        if(light.projector!=CarLighting<Frames>::none){
            const auto& projector=projectors.at(light.projector);
            if(projector.visual){
                frames.object_visible(projector.visual.get(),false);
                //00506689 dereferences the companion light with no native
                //null guard. Surface the failed dependency rather than omit it.
                if(!projector.light)throw std::runtime_error("Car projector has no actual companion light");
                frames.object_visible(projector.light.get(),false);
            }
        }
        if(light.billboard)frames.update_scene_object(light.billboard);
    }
    state.previous=state.flags;
}
} //namespace hd2::game
#endif
