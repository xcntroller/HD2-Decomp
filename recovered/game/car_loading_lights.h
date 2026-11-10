#ifndef HD2_GAME_CAR_LOADING_LIGHTS_H
#define HD2_GAME_CAR_LOADING_LIGHTS_H

#include "game/car_lighting.h"

namespace hd2::game {

struct CarLoadingLightState {
    std::uint32_t flags=0,previous=0; //004EA780's5B0/5B4.
    float blink_elapsed=0,hazard_elapsed=0; //004E2900's5C0/5CC.
    float steering_threshold=.20000000298023224f; //5C4, replaced by004D3110 before each presentation update.
};
struct CarLoadingLightInputs {std::int32_t gear;float brake,steering;};

//00501CB0's cold, unoccupied lights: no indicator/headlight/brake/reverse
//request. Even this path advances the original blink state and retires actual
//billboard/projector/light visibility when80 is present in current OR previous
//features. The reference marker at native entry+0 remains visible.
template<class Frames>void update_car_loading_lights(Frames& frames,const CarLighting<Frames>& lighting,
        CarLoadingLightState& state,float elapsed,std::uint32_t features,std::uint32_t previous_features,
        const CarLoadingLightInputs* inputs=nullptr){
    if(!((features|previous_features)&0x80u))return;
    constexpr std::uint32_t automatic=0x8808u; //004D16A0's actual unoccupied light command bits.
    if((state.flags|state.previous)&~(automatic|0x10000000u))throw std::runtime_error("Car active lighting requires its actual command receiver");
    if((state.flags&automatic)&&!inputs)throw std::runtime_error("Car automatic lights require actual gear/brake/steering inputs");
    state.hazard_elapsed+=elapsed;state.blink_elapsed+=elapsed;
    if(!(state.flags&0x10000000u)){
        if(state.blink_elapsed>=.30000001192092896f){state.blink_elapsed=0;state.flags|=0x10000000u;}
    }else if(state.blink_elapsed>=.4000000059604645f){state.blink_elapsed=0;state.flags&=~0x10000000u;}
    if(state.flags&0x8000u){if(inputs->gear<0)state.flags|=0x1000u;else state.flags&=~0x1000u;}
    if(state.flags&0x800u){if(inputs->brake<=0)state.flags&=~0x100u;else state.flags|=0x100u;}
    if(state.flags&8u){
        if(std::abs(inputs->steering)<=state.steering_threshold)state.flags&=~3u;
        else if(inputs->steering<=0)state.flags=(state.flags&~2u)|1u;
        else state.flags=(state.flags&~1u)|2u;
    }
    if(state.flags&~(automatic|0x10000000u))throw std::runtime_error("Car positive automatic lights require their actual active projector renderer");
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
