#ifndef HD2_GAME_MOUNTED_WEAPON_CAPABILITY_H
#define HD2_GAME_MOUNTED_WEAPON_CAPABILITY_H
#include "game/mounted_weapon_aim.h"
namespace hd2::game {
namespace mounted_capability_detail {
inline void normalize(std::array<float,4>& value){
    const auto exact=(static_cast<long double>(value[1])*value[1]+static_cast<long double>(value[2])*value[2])+static_cast<long double>(value[0])*value[0];
    normalize_human_ai_shot(value,exact);
}
}
//004C5F60: X/Y remain extended; Z and the target pitch are FLOAT snapshots.
template<class Frames>bool mounted_vertical_capability(Frames& frames,typename Frames::Frame vertical,const StaticWeaponControl& control,const std::array<float,4>& target){
    const auto world=frames.actor_world_matrix(vertical);
    const auto normalize=[](std::array<long double,3> v){
        const auto exact=(v[0]*v[0]+v[2]*v[2])+v[1]*v[1];const float stored=static_cast<float>(exact);
        if(!(std::abs(exact-1)<9.99999993922529e-09f)){
            if(stored>=9.99999993922529e-09f){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));v[0]*=inverse;v[1]*=inverse;v[2]=static_cast<float>(v[2]*inverse);}
            else{const auto axis=v[0]!=0?0u:v[2]!=0?2u:1u;v[axis]=v[axis]<0?-1.L:1.L;}
        }
        return v;
    };
    auto direction=normalize({{static_cast<long double>(target[0])-world[12],static_cast<long double>(target[1])-world[13],static_cast<float>(static_cast<long double>(target[2])-world[14])}});
    const float target_pitch=static_cast<float>(std::atan2(direction[1],std::sqrt(direction[0]*direction[0]+direction[2]*direction[2])));
    const auto refreshed=frames.actor_world_matrix(vertical);direction=normalize({{refreshed[8],refreshed[9],refreshed[10]}});
    const auto current_pitch=std::atan2(direction[1],std::sqrt(direction[0]*direction[0]+direction[2]*direction[2]));
    if(!control.field118)throw std::logic_error("Mounted capability reached original unwritten118 angle; actual aim tick is required");
    const auto relative=target_pitch-(current_pitch-mounted_aim_detail::real(*control.field118));
    return relative<control.field128&&relative>control.field12c;
}
//004C5A30: three planar normalizations, strict open angle limits, no yaw wrapping.
template<class Frames>bool mounted_horizontal_capability(Frames& frames,typename Frames::Frame vertical,const StaticWeaponControl& control,const std::array<float,4>& target){
    if(control.field134==1000000.f)return true;
    const auto first=frames.actor_world_matrix(vertical);std::array<float,4> direction{{static_cast<float>(static_cast<long double>(target[0])-first[12]),0,static_cast<float>(static_cast<long double>(target[2])-first[14]),0}};
    mounted_capability_detail::normalize(direction);
    const auto second=frames.actor_world_matrix(vertical);std::array<float,4> forward{{second[8],second[9],second[10],second[11]}};
    mounted_capability_detail::normalize(forward);forward[1]=0;mounted_capability_detail::normalize(forward);
    float angle=frames.car_vector_angle(forward,direction);
    const auto third=frames.actor_world_matrix(vertical);std::array<float,4> right{{third[0],0,third[2],third[3]}};
    const auto exact=static_cast<long double>(right[2])*right[2]+static_cast<long double>(right[0])*right[0];const float stored=static_cast<float>(exact);
    auto x=static_cast<long double>(right[0]),z=static_cast<long double>(right[2]);
    if(!(std::abs(exact-1)<9.99999993922529e-09f)){
        if(stored>=9.99999993922529e-09f){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));x*=inverse;z*=inverse;right[1]=static_cast<float>(0.f*inverse);}
        else if(x!=0)x=x<0?-1:1;else if(z!=0)z=z<0?-1:1;else right[1]=1;
    }
    if((x*direction[0]+z*direction[2])+static_cast<long double>(right[1])*direction[1]<0)angle=-angle;
    if(!control.field11c)throw std::logic_error("Mounted capability reached original unwritten11C angle; actual aim tick is required");
    const auto relative=static_cast<long double>(angle)+mounted_aim_detail::real(*control.field11c);
    return relative<control.field134&&relative>control.field130;
}
} //namespace hd2::game
#endif
