#ifndef HD2_GAME_HUMAN_JOINT_MATH_H
#define HD2_GAME_HUMAN_JOINT_MATH_H

#include "game/actor_activation_math.h"

namespace hd2::game {
using HumanMatrix=std::array<float,16>;
using HumanVector=std::array<float,4>;
inline HumanMatrix human_identity() {HumanMatrix m{};m[0]=m[5]=m[10]=m[15]=1;return m;}
inline HumanMatrix human_transpose(const HumanMatrix& m) { //007A2190.
    HumanMatrix result{};for(unsigned row=0;row!=4;++row)for(unsigned column=0;column!=4;++column)result[row*4+column]=m[column*4+row];return result;
}
inline HumanMatrix human_vertical_basis(const HumanVector& z) { //007A2E10.
    auto result=human_identity();
    if(z[0]==0&&z[2]==0){result[0]=1;result[1]=result[2]=0;}
    else {const auto scale=1/std::sqrt(static_cast<long double>(z[0])*z[0]+static_cast<long double>(z[2])*z[2]);result[0]=static_cast<float>(scale*z[2]);result[1]=0;result[2]=static_cast<float>(-scale*z[0]);}
    result[4]=static_cast<float>(static_cast<long double>(z[1])*result[2]);
    result[5]=static_cast<float>(static_cast<long double>(z[2])*result[0]-static_cast<long double>(result[2])*z[0]);
    result[6]=static_cast<float>(-static_cast<long double>(z[1])*result[0]);
    for(unsigned axis=0;axis!=3;++axis)result[8+axis]=z[axis];
    return result;
}
//007A3E60's shortest rotation, including nearly parallel/antiparallel vectors.
//The original writes nine entries only; preserve translation and homogeneous words.
inline void human_align_vectors(const HumanVector& a,const HumanVector& b,HumanMatrix& result) {
    const float x=static_cast<float>(static_cast<long double>(b[2])*a[1]-static_cast<long double>(a[2])*b[1]);
    const float y=static_cast<float>(static_cast<long double>(a[2])*b[0]-static_cast<long double>(a[0])*b[2]);
    const float z=static_cast<float>(static_cast<long double>(a[0])*b[1]-static_cast<long double>(b[0])*a[1]);
    const float dot=static_cast<float>((static_cast<long double>(a[1])*b[1]+static_cast<long double>(a[2])*b[2])+static_cast<long double>(a[0])*b[0]);
    constexpr double parallel=.999999;
    if(std::abs(dot)<=parallel) {
        const float scale=static_cast<float>((1-static_cast<long double>(dot))/((static_cast<long double>(x)*x+static_cast<long double>(y)*y)+static_cast<long double>(z)*z));
        const float sx=static_cast<float>(static_cast<long double>(scale)*x);
        const float syz=static_cast<float>(static_cast<long double>(scale)*z*y);
        result[0]=static_cast<float>(static_cast<long double>(sx)*x+dot);
        result[4]=static_cast<float>(static_cast<long double>(sx)*y-z);
        result[8]=static_cast<float>(static_cast<long double>(sx)*z+y);
        result[1]=static_cast<float>(static_cast<long double>(sx)*y+z);
        result[5]=static_cast<float>(static_cast<long double>(scale)*y*y+dot);
        result[9]=syz-x;
        result[2]=static_cast<float>(static_cast<long double>(sx)*z-y);
        result[6]=syz+x;
        result[10]=static_cast<float>(static_cast<long double>(scale)*z*z+dot);return;
    }
    HumanVector axis{};
    const auto ax=std::abs(a[0]),ay=std::abs(a[1]),az=std::abs(a[2]);
    axis[ay<=ax?(az<=ay?2u:1u):(ax<az?0u:2u)]=1;
    HumanVector u{},v{};for(unsigned i=0;i!=3;++i){u[i]=axis[i]-a[i];v[i]=axis[i]-b[i];}
    const float first=static_cast<float>(2/((static_cast<long double>(u[0])*u[0]+static_cast<long double>(u[1])*u[1])+static_cast<long double>(u[2])*u[2]));
    const float second=static_cast<float>(2/((static_cast<long double>(v[0])*v[0]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[2])*v[2]));
    const auto uv=(static_cast<long double>(v[0])*u[0]+static_cast<long double>(v[1])*u[1])+static_cast<long double>(v[2])*u[2];
    for(unsigned i=0;i!=3;++i)for(unsigned j=0;j!=3;++j)
        result[j*4+i]=static_cast<float>(uv*second*first*v[i]*u[j]-(static_cast<long double>(first)*u[i]*u[j]+static_cast<long double>(v[j])*v[i]*second)+(i==j?1:0));
}
} //namespace hd2::game
#endif
