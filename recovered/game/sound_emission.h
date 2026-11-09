#ifndef HD2_GAME_SOUND_EMISSION_H
#define HD2_GAME_SOUND_EMISSION_H

#include <array>
#include <cstdint>
#include <cstring>

namespace hd2::game {

//004374D0's real64-byte emission value, shared by actor constructors and spatial
//sound requests. Neutral word names avoid guessing meanings of unread fields.
//This value has no playback method; its actual manager consumer is required.
struct SoundEmission {
    std::array<std::uint32_t,16> words{};
    SoundEmission() {words[12]=4;words[13]=words[14]=words[15]=0x3f800000;}
    void position(const std::array<float,4>& value) {
        for(std::size_t i=0;i!=4;++i)std::memcpy(&words[i],&value[i],4);
    }
};

} //namespace hd2::game
#endif
