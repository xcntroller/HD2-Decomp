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
//007A23F0 accepts an existing basis. X remains extended through projection
//and cross products while Y/Z and the projected-axis inverse are stored FLOATs.
inline HumanMatrix human_bone_direction_basis(const HumanMatrix& basis,const HumanVector& direction){
    using Axis=std::array<long double,3>;
    const auto normalize=[](Axis& value,long double square,bool stored_inverse){
        constexpr float epsilon=9.99999993922529e-09f;
        if(std::abs(square-1)<epsilon)return;
        const float stored=static_cast<float>(square);
        if(!(stored<epsilon)){
            auto inverse=1.L/std::sqrt(static_cast<long double>(stored));
            if(stored_inverse)inverse=static_cast<float>(inverse);
            value[0]*=inverse;
            value[1]=static_cast<float>(value[1]*inverse);
            value[2]=static_cast<float>(value[2]*inverse);
        }else{
            const auto index=value[0]!=0?0u:value[2]!=0?2u:1u;
            value[index]=value[index]<0?-1.L:1.L;
        }
    };
    Axis z{{direction[0],direction[1],direction[2]}};
    normalize(z,(z[0]*z[0]+z[2]*z[2])+z[1]*z[1],false);
    const auto project=[&](unsigned row,long double dot){
        Axis value;
        value[0]=static_cast<float>(basis[row]-z[0]*dot);
        const float y=static_cast<float>(z[1]*dot),other_z=static_cast<float>(z[2]*dot);
        value[1]=basis[row+1]-y;value[2]=basis[row+2]-other_z;
        return value;
    };
    auto x=project(0,(z[1]*basis[1]+z[0]*basis[0])+z[2]*basis[2]);
    const auto square=(x[2]*x[2]+x[1]*x[1])+x[0]*x[0];
    Axis y;
    if(square<1.0000000195414814e-16f){
        y=project(4,(z[2]*basis[6]+z[1]*basis[5])+z[0]*basis[4]);
        normalize(y,(y[2]*y[2]+y[1]*y[1])+y[0]*y[0],true);
        x={{y[1]*z[2]-z[1]*y[2],z[0]*y[2]-z[2]*y[0],z[1]*y[0]-z[0]*y[1]}};
    }else{
        normalize(x,square,true);
        y={{z[1]*x[2]-z[2]*x[1],z[2]*x[0]-z[0]*x[2],z[0]*x[1]-z[1]*x[0]}};
    }
    HumanMatrix result{};
    for(unsigned axis=0;axis!=3;++axis){
        result[axis]=static_cast<float>(x[axis]);result[4+axis]=static_cast<float>(y[axis]);result[8+axis]=static_cast<float>(z[axis]);
    }
    result[15]=1;return result;
}
inline void orthonormalize_human_bone_matrix(HumanMatrix& matrix){ //007A2310, native unchecked normalization.
    HumanVector z{};
    z[0]=static_cast<float>(static_cast<long double>(matrix[1])*matrix[6]-static_cast<long double>(matrix[5])*matrix[2]);
    z[1]=static_cast<float>(static_cast<long double>(matrix[4])*matrix[2]-static_cast<long double>(matrix[0])*matrix[6]);
    z[2]=static_cast<float>(static_cast<long double>(matrix[5])*matrix[0]-static_cast<long double>(matrix[4])*matrix[1]);
    const auto x_square=(static_cast<long double>(matrix[0])*matrix[0]+
        static_cast<long double>(matrix[2])*matrix[2])+static_cast<long double>(matrix[1])*matrix[1];
    const auto x_inverse=1.L/std::sqrt(x_square);
    for(unsigned axis=0;axis!=3;++axis)matrix[axis]=static_cast<float>(matrix[axis]*x_inverse);
    const auto z_square=(static_cast<long double>(z[0])*z[0]+static_cast<long double>(z[1])*z[1])+
        static_cast<long double>(z[2])*z[2];
    const auto z_inverse=1.L/std::sqrt(z_square);
    const std::array<long double,3> normal{{z[0]*z_inverse,z[1]*z_inverse,z[2]*z_inverse}};
    matrix[4]=static_cast<float>(normal[1]*matrix[2]-normal[2]*matrix[1]);
    matrix[5]=static_cast<float>(normal[2]*matrix[0]-normal[0]*matrix[2]);
    matrix[6]=static_cast<float>(normal[0]*matrix[1]-normal[1]*matrix[0]);
    for(unsigned axis=0;axis!=3;++axis)matrix[8+axis]=static_cast<float>(normal[axis]);
}
inline HumanVector interpolate_human_directions(const HumanVector& first,const HumanVector& last,float influence){ //007A4140.
    const auto dot=(static_cast<long double>(first[1])*last[1]+static_cast<long double>(first[0])*last[0])+
        static_cast<long double>(first[2])*last[2];
    float stored=static_cast<float>(dot);
    if(dot>1)stored=1;else if(stored<-1)stored=-1;
    const auto angle=std::acos(static_cast<long double>(stored));
    const float stored_angle=static_cast<float>(angle);
    constexpr float epsilon=9.99999993922529e-09f;
    if(!(angle>epsilon)||!(3.1415927410125732421875L-stored_angle>epsilon))return last;
    const auto inverse=1.L/std::sin(static_cast<long double>(stored_angle));
    const auto start=std::sin((1.L-influence)*stored_angle)*inverse;
    const auto finish=std::sin(static_cast<long double>(stored_angle)*influence)*inverse;
    auto result=last; //Native interpolated W is unwritten. Retain the known last lane.
    result[0]=static_cast<float>(static_cast<float>(start*first[0])+finish*last[0]);
    result[1]=static_cast<float>(finish*last[1])+static_cast<float>(start*first[1]);
    result[2]=static_cast<float>(static_cast<float>(finish*last[2])+start*first[2]);
    return result;
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
