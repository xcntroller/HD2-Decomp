#ifndef HD2_GAME_HUMAN_ATTACHMENT_BLEND_H
#define HD2_GAME_HUMAN_ATTACHMENT_BLEND_H

#include "game/human_joint_math.h"
#include <cstdint>
#include <cstring>

namespace hd2::game {

struct HumanAttachmentBlend {unsigned pair=1;float horizontal=0,vertical=0;};

//007AF320's six-key selection. Limits are the actual virtualB8 up/down/left/
//right values. Their signs and zero divisions retain native floating behavior.
inline HumanAttachmentBlend human_attachment_blend(const HumanVector& direction,const std::array<float,4>& limits){
    HumanAttachmentBlend result;
    constexpr long double epsilon=9.99999993922529e-09;
    constexpr float half_pi=1.5707963705062866f;
    if(1.L-std::abs(static_cast<long double>(direction[1]))>epsilon&&std::abs(static_cast<long double>(direction[0]))>epsilon){
        const auto angle=half_pi-std::atan(std::abs(static_cast<long double>(direction[2]))/std::abs(static_cast<long double>(direction[0])));
        if(std::signbit(direction[0]))result.horizontal=static_cast<float>(-angle/limits[2]);
        else{result.pair=0;result.horizontal=static_cast<float>(1.L-angle/limits[3]);}
    }
    const auto angle=half_pi-std::acos(std::abs(static_cast<long double>(direction[1])));
    result.vertical=static_cast<float>(std::signbit(direction[1])?.5f-(-.5f/static_cast<long double>(limits[1]))*angle:
        .5f+(.5f/static_cast<long double>(limits[0]))*angle);
    for(auto* value:{&result.horizontal,&result.vertical}){
        if(std::signbit(*value))*value=0;
        else if(*value>1)*value=1;
    }
    return result;
}

inline bool human_attachment_key_present(float value){
    std::uint32_t bits;std::memcpy(&bits,&value,4);
    return bits!=0x7fc00000u&&bits!=0xffc00000u;
}

//007AF320's bilinear position path preserves each FLOAT product/sum boundary.
inline HumanVector blend_human_attachment_position(const HumanVector& first,const HumanVector& second,
        const HumanVector& third,const HumanVector& fourth,float horizontal,float vertical){
    const auto complement=1.L-horizontal;
    std::array<long double,3> lower{},upper{};
    for(unsigned axis=0;axis!=3;++axis){
        const float target=static_cast<float>(static_cast<long double>(horizontal)*third[axis]);
        const auto previous=complement*first[axis];
        const auto sum=(axis?static_cast<float>(previous):previous)+target;
        lower[axis]=axis?static_cast<float>(sum):sum;
        const auto upper_target=static_cast<long double>(horizontal)*fourth[axis];
        const auto upper_previous=complement*second[axis];
        const auto upper_sum=(axis?static_cast<float>(upper_previous):upper_previous)+
            (axis==2?static_cast<float>(upper_target):upper_target);
        upper[axis]=axis==0?static_cast<float>(upper_sum):upper_sum;
    }
    HumanVector result=first;
    const auto inverse=1.L-vertical;
    const float stored_inverse=static_cast<float>(inverse);
    for(unsigned axis=0;axis!=3;++axis){
        const float target=static_cast<float>(upper[axis]*vertical);
        const auto previous=lower[axis]*(axis?stored_inverse:inverse);
        result[axis]=static_cast<float>((axis==2?static_cast<float>(previous):previous)+target);
    }
    return result;
}

} //namespace hd2::game
#endif
