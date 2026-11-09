#ifndef HD2_GAME_COLLISION_SWEEP_BOX_H
#define HD2_GAME_COLLISION_SWEEP_BOX_H

#include "game/collision_sweep_triangle.h"

namespace hd2::game {

//00706900, factored from its six expanded face branches. Preserve X/Y/Z
//selection, per-face gate/edge bits, twelve-edge order and vertex fallback.
//This is the rounded swept-sphere box test, not an expanded-AABB ray shortcut.
inline bool collision_sweep_box(const CollisionPoint& low,const CollisionPoint& high,CollisionSweepState& state) {
    using namespace collision_sweep_math;
    const auto end=add(state.start,state.displacement);
    for(auto axis:{0u,2u,1u}) {
        if(std::max(state.start[axis],end[axis])<low[axis]-state.radius||
           high[axis]+state.radius<std::min(state.start[axis],end[axis]))return false;
    }
    std::array<Vector,8> vertices{{
        {{low[0],low[1],low[2],0}},{{low[0],low[1],high[2],0}},
        {{low[0],high[1],low[2],0}},{{high[0],low[1],low[2],0}},
        {{high[0],high[1],low[2],0}},{{high[0],low[1],high[2],0}},
        {{low[0],high[1],high[2],0}},{{high[0],high[1],high[2],0}}
    }};
    struct Face {unsigned axis;float normal;unsigned shift;std::uint32_t gate;std::array<unsigned,4> vertices;};
    const std::array<Face,3> faces{{
        state.direction[0]<=0?Face{0,-1,4,0,{{3,4,7,5}}}:Face{0,1,0,0,{{0,1,6,2}}},
        state.direction[1]<0?Face{1,-1,8,0x24,{{2,6,7,4}}}:Face{1,1,12,0x81,{{0,3,5,1}}},
        state.direction[2]<0?Face{2,-1,20,0x4242,{{1,5,7,6}}}:Face{2,1,16,0x1818,{{0,2,4,3}}}
    }};
    std::uint32_t mask=0;
    for(const auto& face:faces) {
        if(face.gate&&!(mask&face.gate))continue;
        Vector normal{};normal[face.axis]=face.normal;
        const auto& first=vertices[face.vertices[0]];
        const auto signed_distance=dot(subtract(state.start,first),normal);
        if(signed_distance<0&&-state.radius<signed_distance) {
            const auto moment=cross(normal,state.start);bool inside=true;
            for(unsigned edge=0;edge!=4;++edge) {
                const auto& a=vertices[face.vertices[edge]];const auto& b=vertices[face.vertices[(edge+1)%4]];
                if(!(dot(subtract(a,b),moment)+dot(cross(a,b),normal)<0)){inside=false;break;}
            }
            if(inside) {
                state.best_distance_squared=0;state.hit_normal=normal;
                state.hit_position=add(state.start,scale(normal,-signed_distance-state.radius));return true;
            }
        }
        const auto moment=cross(state.displacement,add(state.start,scale(normal,state.radius)));
        for(unsigned edge=0;edge!=4;++edge) {
            const auto& a=vertices[face.vertices[edge]];const auto& b=vertices[face.vertices[(edge+1)%4]];
            if(dot(subtract(a,b),moment)+dot(cross(a,b),state.displacement)>0)mask|=1u<<(face.shift+edge);
        }
        const auto denominator=dot(normal,state.displacement);
        if(mask&(0xfu<<face.shift))continue;
        if((face.axis<2&&face.normal>0)?denominator<9.99999993922529e-09f:std::abs(static_cast<long double>(denominator))<1.e-8)continue;
        const float parameter=static_cast<float>(-(static_cast<long double>(signed_distance)+state.radius)/denominator);
        if(parameter<0)continue;
        const auto position=add(state.start,scale(state.displacement,parameter));
        const auto travel=subtract(position,state.start);const auto square=dot(travel,travel);
        if(square<state.best_distance_squared) {
            state.best_distance_squared=square;state.hit_position=position;state.hit_normal=normal;return true;
        }
    }
    struct Edge {std::uint32_t mask;unsigned from,to;};
    constexpr std::array<Edge,12> edges{{
        {0x8001,0,1},{0x10008,0,2},{0x81000,0,3},{0x40010,3,4},
        {0x2080,3,5},{0x104000,1,5},{0x800002,1,6},{0x200040,5,7},
        {0x20800,2,4},{0x420,4,7},{0x400200,7,6},{0x104,6,2}
    }};
    std::uint32_t vertex=0;
    for(const auto& edge:edges)if(mask&edge.mask) {
        if(collision_sweep_triangle_edge(vertices[edge.from],vertices[edge.to],state,vertex,1u<<edge.from,1u<<edge.to))return true;
    }
    if(!vertex)return false;
    unsigned index=0;while(index!=8&&(1u<<index)!=vertex)++index;
    if(index==8)throw std::runtime_error("Invalid native box vertex fallback");
    const auto delta=subtract(vertices[index],state.start);const auto projection=dot(delta,state.direction);
    if(projection<0)return false;
    const auto discriminant=static_cast<long double>(state.radius)*state.radius-
        (static_cast<long double>(dot(delta,delta))-static_cast<long double>(projection)*projection);
    if(discriminant<0)return false;
    const auto extended=static_cast<long double>(projection)-std::sqrt(discriminant);
    const float distance=static_cast<float>(extended);
    const auto square=extended*distance;
    if(!(square<state.best_distance_squared))return false;
    state.best_distance_squared=static_cast<float>(square);state.hit_position=add(state.start,scale(state.direction,distance));
    state.hit_normal=subtract(vertices[index],state.hit_position);return true;
}
} //namespace hd2::game
#endif
