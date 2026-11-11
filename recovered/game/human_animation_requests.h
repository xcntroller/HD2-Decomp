#ifndef HD2_GAME_HUMAN_ANIMATION_REQUESTS_H
#define HD2_GAME_HUMAN_ANIMATION_REQUESTS_H

#include <cstdint>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace hd2::game {

//The five-word007733D0/007724E0 request, with its frame reference outside
//native scalar words on hosts whose pointers are wider than32 bits.
template<class Frame>struct HumanAnimationRequest {
    Frame animation{};
    float weight=1;
    std::uint32_t flags=0;
    float blend=0;
    bool separate=false;
};

inline std::int32_t human_animation_time_integer(long double value) {
    if(!std::isfinite(value)||value<static_cast<long double>(std::numeric_limits<std::int64_t>::min())||
       value>=-static_cast<long double>(std::numeric_limits<std::int64_t>::min()))
        throw std::runtime_error("Human animation __ftol exceeds original integer range");
    const auto bits=static_cast<std::uint32_t>(static_cast<std::int64_t>(value));
    std::int32_t result;std::memcpy(&result,&bits,4);return result;
}

inline float human_animation_word_float(std::uint32_t bits){
    float value;std::memcpy(&value,&bits,4);return value;
}

} //namespace hd2::game
#endif
