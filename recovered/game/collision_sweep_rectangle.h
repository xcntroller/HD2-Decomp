#ifndef HD2_GAME_COLLISION_SWEEP_RECTANGLE_H
#define HD2_GAME_COLLISION_SWEEP_RECTANGLE_H

#include "game/collision_sweep_triangle.h"

namespace hd2::game {

//Local-space geometry of007101B0 case83. Its containing shape receiver owns
//world/inverse transforms. The plane is localY=0; serialized Y bounds serve
//the initial coarse check. Edge flags/order and parallel-plane case are native.
inline bool collision_sweep_rectangle(const CollisionPoint& low,const CollisionPoint& high,CollisionSweepState& state) {
    using namespace collision_sweep_math;
    const auto end=add(state.start,state.displacement);
    for(auto axis:{0u,2u,1u})if(std::max(state.start[axis],end[axis])<low[axis]-state.radius||
        high[axis]+state.radius<std::min(state.start[axis],end[axis]))return false;
    const std::array<Vector,4> vertices{{
        {{low[0],0,low[2],0}},{{high[0],0,low[2],0}},
        {{high[0],0,high[2],0}},{{low[0],0,high[2],0}}
    }};
    std::uint32_t mask=0;
    if(std::abs(static_cast<long double>(state.displacement[1]))>=1.e-8) {
        const float sign=state.direction[1]<0?-1.f:1.f;const Vector normal{{0,sign,0,0}};
        const auto signed_distance=sign*state.start[1];
        const std::array<unsigned,4> order=sign<0?std::array<unsigned,4>{{0,3,2,1}}:std::array<unsigned,4>{{0,1,2,3}};
        if(signed_distance<0&&-state.radius<signed_distance) {
            const auto moment=cross(normal,state.start);bool inside=true;
            for(unsigned edge=0;edge!=4;++edge) {
                const auto& a=vertices[order[edge]];const auto& b=vertices[order[(edge+1)%4]];
                if(!(dot(subtract(a,b),moment)+dot(cross(a,b),normal)<0)){inside=false;break;}
            }
            if(inside) {
                state.best_distance_squared=0;state.hit_position=add(state.start,scale(normal,-signed_distance-state.radius));state.hit_normal=normal;return true;
            }
        }
        if(sign<0) {
            const auto moment=cross(state.displacement,add(state.start,scale(normal,state.radius)));
            for(unsigned edge=0;edge!=4;++edge) {
                const auto& a=vertices[order[edge]];const auto& b=vertices[order[(edge+1)%4]];
                if(dot(subtract(a,b),moment)+dot(cross(a,b),state.displacement)>0)mask|=1u<<(3-edge);
            }
        }else {
            const float reciprocal=1/state.direction[1];
            const float height=state.radius+state.start[1];
            const float x=static_cast<float>(state.start[0]-static_cast<long double>(reciprocal)*height*state.direction[0]);
            const float z=static_cast<float>(state.start[2]-static_cast<long double>(reciprocal)*height*state.direction[2]);
            if(x<low[0])mask=8;
            if(high[0]<x)mask|=2;
            if(z<low[2])mask|=1;
            if(high[2]<z)mask|=4;
        }
        if(!mask) {
            const float parameter=static_cast<float>(-(static_cast<long double>(state.start[1])+sign*state.radius)/state.displacement[1]);
            if(parameter>=0) {
                const auto position=add(state.start,scale(state.displacement,parameter));const auto travel=subtract(position,state.start);const auto square=dot(travel,travel);
                if(square<state.best_distance_squared){state.best_distance_squared=square;state.hit_position=position;state.hit_normal=normal;return true;}
            }
        }
    }else if(std::abs(state.start[1])<state.radius) {
        if(state.displacement[0]>0)mask=1;
        if(state.displacement[2]<0)mask|=2;
        if(state.displacement[0]<0)mask|=4;
        if(state.displacement[2]>0)mask|=8;
    }
    std::uint32_t vertex=0;
    for(unsigned edge=0;edge!=4;++edge)if(mask&(1u<<edge)) {
        if(collision_sweep_triangle_edge(vertices[edge],vertices[(edge+1)%4],state,vertex,1u<<edge,1u<<((edge+1)%4)))return true;
    }
    if(!vertex)return false;
    unsigned index=0;while(index!=4&&(1u<<index)!=vertex)++index;
    if(index==4)throw std::runtime_error("Invalid native rectangle vertex fallback");
    const auto delta=subtract(vertices[index],state.start);const auto projection=dot(delta,state.direction);
    if(projection<0)return false;
    const auto discriminant=static_cast<long double>(state.radius)*state.radius-
        (static_cast<long double>(dot(delta,delta))-static_cast<long double>(projection)*projection);
    if(discriminant<0)return false;
    const auto extended=static_cast<long double>(projection)-std::sqrt(discriminant);const float distance=static_cast<float>(extended);
    const auto square=extended*distance;
    if(!(square<state.best_distance_squared))return false;
    state.best_distance_squared=static_cast<float>(square);state.hit_position=add(state.start,scale(state.direction,distance));state.hit_normal=subtract(vertices[index],state.hit_position);return true;
}
} //namespace hd2::game
#endif
