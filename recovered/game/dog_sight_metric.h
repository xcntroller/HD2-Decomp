#ifndef HD2_GAME_DOG_SIGHT_METRIC_H
#define HD2_GAME_DOG_SIGHT_METRIC_H
#include "game/human_ai_shot_direction.h"
namespace hd2::game {
//005CE740's two normalizations retain X extended in the planar pass.
namespace dog_sight_math {
inline long double planar_x(HumanVector& d){
    long double x=d[0];const auto square=static_cast<long double>(d[2])*d[2]+x*x;
    const float stored=static_cast<float>(square);constexpr float eps=9.99999993922529e-09f;
    if(!(std::abs(square-1)<eps)){
        if(!(stored<eps)){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));x*=inverse;d[1]=static_cast<float>(0.f*inverse);d[2]=static_cast<float>(d[2]*inverse);}
        else if(x!=0)x=x<0?-1:1;else if(d[2]!=0)d[2]=d[2]<0?-1:1;else d[1]=1;
    }return x;
}
inline long double edge(float ax,float az,float bx,float bz,float px,float pz){return (static_cast<long double>(bx)-ax)*(static_cast<long double>(pz)-az)-(static_cast<long double>(px)-ax)*(static_cast<long double>(bz)-az);}
}
//Queries must preserve actual target-state/model -> fresh Dog544 -> fresh544
//ordering. A masked controller returns without writing either output.
template<class Queries>void dog_controller_sight_metric(std::uint32_t flags,std::int32_t range,
        std::uint32_t& interval,float& score,Queries& queries){
    if(flags&0x40000800u)return;
    interval=99;
    const auto target=queries.target_flags();
    long double distance=target.second?static_cast<long double>(static_cast<float>(range)):static_cast<long double>(range);
    const auto f=target.first;
    if(f&1u)distance*=.6000000238418579f;else if(f&2u)distance*=.800000011920929f;
    if(f&0x1000u)distance*=1.0499999523162842f;
    else if(f&0x2000u)distance*=1.2000000476837158f;
    else if(f&0x4000u)distance*=1.399999976158142f;
    const float limit=static_cast<float>(distance);
    const auto position=queries.target_position();
    auto direction=queries.head_direction();
    normalize_human_ai_shot(direction,(static_cast<long double>(direction[1])*direction[1]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[0])*direction[0]);
    direction[1]=0;
    const auto x=dog_sight_math::planar_x(direction);
    const float zero=static_cast<float>(static_cast<long double>(direction[1])*0.f);
    const float crossx=static_cast<float>(static_cast<long double>(zero)-static_cast<long double>(direction[2])*-1.f);
    const float crossz=static_cast<float>(x*-1.f-zero);
    const float spread=static_cast<float>(std::tan(static_cast<long double>(.6981317202250164))*limit);
    const float dx=static_cast<float>(x*limit),dz=static_cast<float>(static_cast<long double>(direction[2])*limit);
    const auto origin=queries.head_position();
    const auto side_x=static_cast<long double>(crossx)*spread,side_z=static_cast<long double>(crossz)*spread;
    const float sx=static_cast<float>(side_x);
    const auto center_x=static_cast<long double>(dx)+origin[0],center_z=static_cast<long double>(dz)+origin[2];
    const float cx=static_cast<float>(center_x),cz=static_cast<float>(center_z);
    const float ax=static_cast<float>(center_x+sx),az=static_cast<float>(static_cast<long double>(cz)+side_z);
    const float bx=static_cast<float>(static_cast<long double>(cx)-sx),bz=static_cast<float>(static_cast<long double>(cz)-side_z);
    //Native parity tests reject only ordered negative edge values.
    score=!(dog_sight_math::edge(origin[0],origin[2],ax,az,position[0],position[2])<0)&&
          !(dog_sight_math::edge(ax,az,bx,bz,position[0],position[2])<0)&&
          !(dog_sight_math::edge(bx,bz,origin[0],origin[2],position[0],position[2])<0)?100.f:0.f;
}
} //namespace hd2::game
#endif
