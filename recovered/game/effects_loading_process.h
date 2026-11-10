#ifndef HD2_GAME_EFFECTS_LOADING_PROCESS_H
#define HD2_GAME_EFFECTS_LOADING_PROCESS_H

#include "game/effects_environment.h"
#include "game/effects_fade.h"
#include "game/effects_water_settings.h"
#include "game/effects_decorations_loading.h"

namespace hd2::game {

//GEffects process scalars persist across worlds. The containing module facade
//owns frame-bearing cold loading lifetimes; retract borrowers before scene
//retirement. This state supplies no successful update/render/model producer.
template<class Frame>struct EffectsLoadingProcess {
    EffectsEnvironment environment;EffectsFade fade;EffectsWaterSettings water;EffectsDecorationsLoading decorations;
    std::array<float,3> colour2f4{{1,1,1}};std::array<std::uint32_t,24> fields25c{};
    Frame sun1b0{},water_camera1ac{};std::array<Frame,4> water_frames238{};
    std::array<float,4> sun_position10{};
    std::uint32_t field224=0,field2d4=0;float field2d0=0;
    bool flag220=false,active248=false,flag24c=false;
    const void* world=nullptr; //Typed borrower guard, not native readiness.
};
} //namespace hd2::game
#endif
