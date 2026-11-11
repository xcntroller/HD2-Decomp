#ifndef HD2_GAME_MISSION_PARTICLE_EMISSION_H
#define HD2_GAME_MISSION_PARTICLE_EMISSION_H
#include "game/human_joint_owners.h"
#include <cmath>

namespace hd2::game {
//00445150 builds the native Identity/SetDir3 transform before004451B0.
template<class Process,class Frames>std::int32_t emit_mission_particle(Process& process,Frames& frames,
        std::uint32_t definition,const HumanVector& position,const HumanVector& direction,
        typename Frames::Frame parent,bool orientation,bool flag){
    auto matrix=human_identity();frames.particle_point_direction(matrix,direction);
    for(unsigned axis=0;axis!=3;++axis)matrix[12+axis]=position[axis];
    return process.emit(definition,matrix,parent,orientation,flag);
}
//00451C00: camera68 BEFORE64, and the caller supplies their actual outputs.
//Native keeps X/Y extended, spills Z and the squared length, then retains the
//inverse in x87. The final strict dot comparison suppresses unordered values.
inline bool damage_material_particle_visible(const HumanVector& point,const HumanVector& camera_position,
        const HumanVector& camera_direction){
    auto x=static_cast<long double>(point[0])-camera_position[0];
    auto y=static_cast<long double>(point[1])-camera_position[1];
    const auto z_difference=static_cast<long double>(point[2])-camera_position[2];
    float z=static_cast<float>(z_difference);
    const auto square=(z_difference*z+y*y)+x*x;const float stored=static_cast<float>(square);
    constexpr float epsilon=9.99999993922529e-09f;
    if(!(std::abs(square-1.L)<epsilon)){
        if(!(stored<epsilon)){
            const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));x*=inverse;y*=inverse;z=static_cast<float>(z*inverse);
        }else if(x!=0)x=x<0?-1.f:1.f;
        else if(z!=0)z=z<0?-1.f:1.f;
        else y=y<0?-1.f:1.f;
    }
    const auto dot=(static_cast<long double>(camera_direction[2])*z+camera_direction[1]*y)+camera_direction[0]*x;
    return dot>-.05000000074505806f;
}
} //namespace hd2::game
#endif
