#ifndef HD2_GAME_CAR_GEOMETRY_MATH_H
#define HD2_GAME_CAR_GEOMETRY_MATH_H

#include <array>
#include <cmath>
#include <stdexcept>

namespace hd2::game {

//Inlined S_vector normalization in004E2E30 and00500FD0. The tiny-vector
//branch replaces one component; it does not clear the other components.
inline std::array<float,4> car_normalize(std::array<float,4> value) {
    constexpr float epsilon=9.99999993922529e-09f; //0080F520.
    const auto sum=value[0]*value[0]+value[1]*value[1]+value[2]*value[2];
    if(!std::isfinite(sum))throw std::runtime_error("Non-finite Car geometry direction");
    if(std::abs(sum-1.f)>=epsilon) {
        if(sum>=epsilon) {
            const auto scale=1.f/std::sqrt(sum);
            for(std::size_t i=0;i!=3;++i)value[i]*=scale;
        } else {
            const auto index=value[0]!=0?0u:value[2]!=0?2u:1u;
            value[index]=value[index]<0?-1.f:1.f;
        }
    }
    return value;
}

} //namespace hd2::game
#endif
