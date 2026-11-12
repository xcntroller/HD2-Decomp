#ifndef HD2_GAME_RIGID_BODY_MOTION_MATH_H
#define HD2_GAME_RIGID_BODY_MOTION_MATH_H
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace hd2::game::rigid_motion {
using Vector=std::array<float,4>;
inline float stored(long double value){return static_cast<float>(value);}
inline Vector add(Vector a,const Vector& b){ //004CD520, XYZ only.
    for(unsigned n=0;n!=3;++n)a[n]=stored(static_cast<long double>(a[n])+b[n]);
    return a;
}
inline Vector subtract(Vector a,const Vector& b){
    for(unsigned n=0;n!=3;++n)a[n]=stored(static_cast<long double>(a[n])-b[n]);
    return a;
}
inline Vector scale(Vector a,long double b){ //004066C0's pointer-scalar operation.
    for(unsigned n=0;n!=3;++n)a[n]=stored(static_cast<long double>(a[n])*b);
    return a;
}
inline Vector multiply(Vector a,const Vector& b){ //004CD4F0, componentwise XYZ product.
    for(unsigned n=0;n!=3;++n)a[n]=stored(static_cast<long double>(a[n])*b[n]);
    return a;
}
inline Vector cross(const Vector& a,const Vector& b){ //004CCC60; unwritten lane retains the input lane.
    return {{stored(static_cast<long double>(a[1])*b[2]-static_cast<long double>(a[2])*b[1]),
        stored(static_cast<long double>(a[2])*b[0]-static_cast<long double>(a[0])*b[2]),
        stored(static_cast<long double>(a[0])*b[1]-static_cast<long double>(a[1])*b[0]),a[3]}};
}
inline long double dot(const Vector& a,const Vector& b){ //00454660, Z/Y/X, no return spill.
    return (static_cast<long double>(a[2])*b[2]+static_cast<long double>(a[1])*b[1])+static_cast<long double>(a[0])*b[0];
}
inline long double square(const Vector& a){return dot(a,a);}
inline long double length_extended(const Vector& a){
    const float value=stored(square(a));std::int32_t bits;std::memcpy(&bits,&value,4);
    if(bits<0x322bcc77)return 0;
    return std::abs(static_cast<long double>(value)-1)<9.99999993922529e-9f?1.L:std::sqrt(static_cast<long double>(value));
}
inline float length(const Vector& a){return stored(length_extended(a));}
inline Vector normalize(Vector a){
    const float value=stored(square(a));
    if(std::abs(static_cast<long double>(value)-1)<9.99999993922529e-9f)return a;
    if(value>=9.99999993922529e-9f)return scale(a,1.L/std::sqrt(static_cast<long double>(value)));
    if(a[0]!=0)a[0]=a[0]<0?-1.f:1.f;
    else if(a[2]!=0)a[2]=a[2]<0?-1.f:1.f;
    else a[1]=a[1]<0?-1.f:1.f;
    return a;
}
//004CC520 explicitly multiplies axis quaternions twice. The second scalar
//product spills W*W before00454660, whose ST0 result is consumed directly.
inline Vector quaternion_product(const Vector& a,const Vector& b,bool spill_scalar){
    const auto c=cross(a,b);auto result=add(add(scale(b,a[3]),scale(a,b[3])),c);
    const float scalar=stored(static_cast<long double>(a[3])*b[3]);
    result[3]=stored((spill_scalar?static_cast<long double>(scalar):static_cast<long double>(a[3])*b[3])-dot(a,b));
    return result;
}
} //namespace hd2::game::rigid_motion
#endif
