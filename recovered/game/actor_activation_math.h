#ifndef HD2_GAME_ACTOR_ACTIVATION_MATH_H
#define HD2_GAME_ACTOR_ACTIVATION_MATH_H

#include <array>
#include <cmath>
#include <stdexcept>

namespace hd2::game {

//00610240's inline normalizer. Z*Z + Y*Y + X*X is kept extended for
//the unit-length comparison; its stored float is reloaded before sqrt.
inline std::array<float,4> actor_activation_direction(std::array<float,4> value) {
    constexpr float epsilon=9.99999993922529e-09f;
    const auto extended=static_cast<long double>(value[2])*value[2]+
        static_cast<long double>(value[1])*value[1]+static_cast<long double>(value[0])*value[0];
    const float stored=static_cast<float>(extended);
    if(!std::isfinite(extended)||!std::isfinite(stored))throw std::runtime_error("Non-finite actor direction");
    if(std::abs(extended-1)>=epsilon) {
        if(stored>=epsilon) {
            const auto scale=1/std::sqrt(static_cast<long double>(stored));
            for(unsigned i=0;i!=3;++i)value[i]=static_cast<float>(value[i]*scale);
        } else {
            const auto i=value[0]!=0?0u:value[2]!=0?2u:1u;
            value[i]=value[i]<0?-1.f:1.f;
        }
    }
    return value;
}

} //namespace hd2::game
#endif
