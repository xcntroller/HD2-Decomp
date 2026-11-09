#ifndef HD2_GAME_RIGID_BODY_GEOMETRY_H
#define HD2_GAME_RIGID_BODY_GEOMETRY_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace hd2::game {

//004CD6E0's native geometry/scalar results. Offsets identify original fields;
//there is no dependency on client object layout or uninitialized fourth lanes.
struct RigidBodyGeometry {
    std::array<float,3> centre90{},inverse_inertia1f0{},gravity220{};
    std::array<std::array<float,3>,14> points230{};
    float mass56c=0,coefficient570=0,coefficient574=0,coefficient578=0;
    float coefficient57c=0,coefficient580=0,coefficient584=0,radius588=0;
    std::uint32_t mode560=0;
};

//Bounds are004C7DB0's actual visual hierarchy bounds, not a guessed unit box.
//The incoming coefficient580 is the SAME live body's field (cold10 from
//004C84A0). Motion and callback/state004C8270 are applied by the body owner
//before this geometry operation, in the original order.
template<class Bounds>
RigidBodyGeometry configure_rigid_body_geometry(const Bounds& bounds,float mass,float coefficient,float gravity_scale) {
    for(unsigned axis=0;axis!=3;++axis) {
        if(!std::isfinite(bounds.minimum[axis])||!std::isfinite(bounds.maximum[axis])||
           bounds.maximum[axis]<bounds.minimum[axis])throw std::runtime_error("Invalid rigid body geometry bounds");
    }
    if(!std::isfinite(mass)||mass<=0||!std::isfinite(coefficient)||!std::isfinite(gravity_scale))
        throw std::runtime_error("Invalid rigid body mass or coefficients");
    constexpr float third=.3333333432674407958984375f; //0080FD00.
    constexpr float margin=.0500000007450580596923828125f; //0080FAA4.
    RigidBodyGeometry result;
    result.mass56c=mass;result.coefficient570=coefficient;result.coefficient580=gravity_scale;
    std::array<long double,3> extent;
    for(unsigned axis=0;axis!=3;++axis)extent[axis]=static_cast<long double>(bounds.maximum[axis])-bounds.minimum[axis];
    //The original stores Z's extent before centre calculation, retaining X/Y
    //extended until their later stores. Keep the same x87 rounding boundary.
    extent[2]=static_cast<float>(extent[2]);
    for(unsigned axis=0;axis!=3;++axis)
        result.centre90[axis]=static_cast<float>(extent[axis]*.5f+bounds.minimum[axis]);
    const float x=static_cast<float>(extent[0]+margin),y=static_cast<float>(extent[1]+margin);
    const auto z=extent[2]+margin; //Still extended in004CD804..DA0B.
    const auto inverse_y=1.L/y;
    const float stored_inverse_y=static_cast<float>(inverse_y);
    const auto xy=inverse_y*x,xz=static_cast<long double>(x)/z,yz=static_cast<long double>(stored_inverse_y)*z;
    const auto outside=[](long double ratio){return ratio>2||ratio<.5f;};
    if(!outside(xy)&&!outside(xz)&&!outside(yz)) {
        const auto average=(static_cast<long double>(y)+z+x)*third;
        result.inverse_inertia1f0.fill(static_cast<float>(1.L/((average*average*mass)*third)));
    } else if(x>y&&x>z) {
        const auto squared=static_cast<long double>(x)*x;
        const auto value=1.L/((squared*mass)*third);
        result.inverse_inertia1f0[1]=result.inverse_inertia1f0[2]=static_cast<float>(value);
        const auto smaller=static_cast<long double>(y)<=z?z:y;
        result.inverse_inertia1f0[0]=static_cast<float>((squared/(smaller*smaller))*value);
    } else if(x<=y&&z<y) {
        const auto squared=static_cast<long double>(y)*y;
        const auto value=1.L/((squared*mass)*third);
        result.inverse_inertia1f0[0]=result.inverse_inertia1f0[2]=static_cast<float>(value);
        const auto smaller=static_cast<long double>(x)<=z?z:x;
        result.inverse_inertia1f0[1]=static_cast<float>((squared/(smaller*smaller))*value);
    } else {
        const auto squared=z*z;
        const auto value=1.L/((squared*mass)*third);
        result.inverse_inertia1f0[0]=result.inverse_inertia1f0[1]=static_cast<float>(value);
        const auto smaller=y<=x?x:y;
        result.inverse_inertia1f0[2]=static_cast<float>((squared/(static_cast<long double>(smaller)*smaller))*value);
    }
    const auto radius=std::min<long double>(x,std::min<long double>(y,z))*.5f;
    result.radius588=static_cast<float>(radius>.039999999105930328369140625f?.039999999105930328369140625f:radius);
    const auto& lo=bounds.minimum;const auto& hi=bounds.maximum;
    const auto midpoint=[&](unsigned axis) {return static_cast<float>((static_cast<long double>(hi[axis])-lo[axis])*.5f+lo[axis]);};
    result.points230={{{{lo[0],lo[1],lo[2]}},{{lo[0],lo[1],hi[2]}},{{hi[0],lo[1],hi[2]}},{{hi[0],lo[1],lo[2]}},
        {{hi[0],hi[1],hi[2]}},{{lo[0],hi[1],lo[2]}},{{lo[0],hi[1],hi[2]}},{{hi[0],hi[1],lo[2]}},
        {{midpoint(0),midpoint(1),lo[2]}},{{midpoint(0),midpoint(1),hi[2]}},
        {{midpoint(0),lo[1],midpoint(2)}},{{midpoint(0),hi[1],midpoint(2)}},
        {{lo[0],midpoint(1),midpoint(2)}},{{hi[0],midpoint(1),midpoint(2)}}}};
    const float zero=static_cast<float>(static_cast<long double>(mass)*0.f);
    const float negative=static_cast<float>(static_cast<long double>(mass)*-1.f);
    result.gravity220={{static_cast<float>(static_cast<long double>(zero)*gravity_scale),
        static_cast<float>(static_cast<long double>(negative)*gravity_scale),static_cast<float>(static_cast<long double>(zero)*gravity_scale)}};
    result.coefficient574=1.0099999904632568359375f; //3F8147AE.
    result.coefficient578=.039999999105930328369140625f; //3D23D70A.
    result.mode560=2;
    result.coefficient57c=static_cast<float>((static_cast<long double>(gravity_scale)*mass)*12.5f);
    result.coefficient584=static_cast<float>(static_cast<long double>(mass)*4.f);
    for(auto value:result.inverse_inertia1f0)if(!std::isfinite(value))throw std::runtime_error("Rigid body inertia exceeds bounds");
    if(!std::isfinite(result.coefficient57c)||!std::isfinite(result.coefficient584))throw std::runtime_error("Rigid body coefficients exceed bounds");
    return result;
}

} //namespace hd2::game
#endif
