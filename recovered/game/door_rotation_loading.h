#ifndef HD2_GAME_DOOR_ROTATION_LOADING_H
#define HD2_GAME_DOOR_ROTATION_LOADING_H

#include <array>
#include <cmath>

namespace hd2::game {
//005FF700's quaternion product before actual engine Normalize. Preserve the
//client's FLOAT cross/scaled-vector/sum/scalar temporaries; dot stays extended.
//The engine supplies axis-angle Make and the final Normalize/SetRot receivers.
inline std::array<float,4> door_rotation_product(const std::array<float,4>& rotation,const std::array<float,4>& saved) {
    std::array<float,4> result{};
    std::array<float,3> cross{{
        static_cast<float>(static_cast<long double>(rotation[1])*saved[2]-static_cast<long double>(rotation[2])*saved[1]),
        static_cast<float>(static_cast<long double>(rotation[2])*saved[0]-static_cast<long double>(rotation[0])*saved[2]),
        static_cast<float>(static_cast<long double>(rotation[0])*saved[1]-static_cast<long double>(rotation[1])*saved[0])}};
    for(unsigned a=0;a!=3;++a) {
        const float left=static_cast<float>(static_cast<long double>(saved[a])*rotation[3]);
        const float right=static_cast<float>(static_cast<long double>(rotation[a])*saved[3]);
        const float sum=static_cast<float>(static_cast<long double>(left)+right);
        result[a]=static_cast<float>(static_cast<long double>(sum)+cross[a]);
    }
    const float scalar=static_cast<float>(static_cast<long double>(rotation[3])*saved[3]);
    const auto dot=(static_cast<long double>(rotation[0])*saved[0]+static_cast<long double>(rotation[1])*saved[1])+static_cast<long double>(rotation[2])*saved[2];
    result[3]=static_cast<float>(static_cast<long double>(scalar)-dot);return result;
}
inline std::array<float,4> door_handle_direction(std::array<float,4> value) {
    const auto square=(static_cast<long double>(value[1])*value[1]+static_cast<long double>(value[2])*value[2])+static_cast<long double>(value[0])*value[0];
    const float stored=static_cast<float>(square);
    if(std::abs(square-1.f)>=9.99999993922529e-09f) {
        if(stored>=9.99999993922529e-09f) {
            const auto reciprocal=1.L/std::sqrt(static_cast<long double>(stored));
            for(unsigned a=0;a!=3;++a)value[a]=static_cast<float>(static_cast<long double>(value[a])*reciprocal);
        }else {
            const auto axis=value[0]!=0?0u:value[2]!=0?2u:1u;
            value[axis]=value[axis]<0?-1.f:1.f; //Other components remain intact.
        }
    }
    value[3]=0;return value; //Native reused/unwritten W is never a direction component.
}
} //namespace hd2::game
#endif
