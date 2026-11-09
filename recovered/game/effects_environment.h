#ifndef HD2_GAME_EFFECTS_ENVIRONMENT_H
#define HD2_GAME_EFFECTS_ENVIRONMENT_H

#include "game/serialized_bytes.h"
#include <array>

namespace hd2::game {

//GEffects004860B0's environment value and00489BD0's complete setter. These
//are actual stored parameters for shadow/effect consumers, not successful
//stand-ins for the GEffects module's model/particle/render services.
struct EffectsEnvironment {
    std::array<std::uint32_t, 3> vector_1c0{}, vector_1d0{};
    std::uint32_t scale_1e0 = 0x3f800000u, field_1e4 = 0x41000000u;
    std::uint32_t field_1e8 = 0x42c80000u, width_1ec = 64, height_1f0 = 64;

    //006CC760 B700 serializes eleven DWORDs. The temporary native vector
    //lanes at input indices3/7 of00489BD0 are padding and have no consumer.
    void read_scene_configuration(SerializedBytes& data) {
        scale_1e0 = data.word(); field_1e4 = data.word(); field_1e8 = data.word();
        width_1ec = data.word(); height_1f0 = data.word();
        for (auto& word : vector_1c0) word = data.word();
        for (auto& word : vector_1d0) word = data.word();
    }
};

} // namespace hd2::game
#endif
