#ifndef HD2_GAME_COLLISION_SWEEP_CANDIDATES_H
#define HD2_GAME_COLLISION_SWEEP_CANDIDATES_H

#include "game/collision_tree.h"
#include <cmath>
#include <optional>

namespace hd2::game {

//Named native00715510 query state consumed by007151D0. The concrete hit
//receivers must implement00704D60 and007101B0; this filter does not turn a
//candidate into a hit. Static/dynamic sweep composition is still required.
struct CollisionSweepState {
    using Vector=std::array<float,4>;
    Vector start{},displacement{};
    float radius=0;
    std::array<float,3> minimum{},maximum{};
    Vector direction{};
    std::uint32_t excluded_flags=0,excluded_layers=0;
    float length=0,best_distance_squared=0;
    Vector side_axis{};
    float side_minimum=0,side_maximum=0;
    Vector hit_position{},hit_normal{};
    std::optional<CollisionCellReference> hit_static;
    const void* hit_dynamic=nullptr;
};

//007151D0. Cell entries arrive in original order. The first triangle whose
//first vertex exceeds maxX terminates this CELL, preserving the native sorted
//early exit. Shape candidates are passed to their actual family receiver.
template<class Frame,class Resources,class NarrowPhase>
bool filter_collision_sweep_cell(const CollisionTree<Frame>& tree,
    const std::vector<CollisionCellReference>& cell,CollisionSweepState& state,
    Resources& resources,NarrowPhase& narrow) {
    bool hit=false;
    for(const auto& entry:cell) {
        if(entry.kind) {
            if(entry.kind<0x80||entry.kind>0x87)throw std::runtime_error("Unknown collision sweep shape family");
            const auto& shape=tree.shapes[entry.kind-0x80].at(entry.index);
            if(!(state.excluded_flags&shape.flags))hit=narrow.sweep_shape(entry,shape,state)||hit;
            continue;
        }
        const auto& triangle=tree.triangles.at(entry.index);
        if(state.excluded_flags&triangle.flags)continue;
        if(triangle.flags&0xa0)throw std::runtime_error("Collision sweep triangle references a shape/rejected record without matching exclusion");
        const auto vertices=collision_triangle_vertices(tree,triangle,resources);
        if(!(state.minimum[0]<=vertices[2][0]))continue;
        if(!(vertices[0][0]<=state.maximum[0]))return hit;
        auto minimum=vertices[0][2],maximum=vertices[2][2];
        if(maximum<minimum)std::swap(minimum,maximum);
        if(vertices[1][2]<minimum)minimum=vertices[1][2];
        else if(maximum<vertices[1][2])maximum=vertices[1][2];
        if(!(minimum<state.maximum[2]&&state.minimum[2]<=maximum))continue;
        auto projection=[&](const CollisionPoint& point) {
            return static_cast<float>(-((static_cast<long double>(state.side_axis[0])*point[0]+
                static_cast<long double>(state.side_axis[2])*point[2])+static_cast<long double>(state.side_axis[1])*point[1]));
        };
        const auto a=projection(vertices[0]),b=projection(vertices[2]),c=projection(vertices[1]);
        const bool overlaps=state.side_maximum<=a?(b<state.side_maximum||c<state.side_maximum):
            (state.side_minimum<b||state.side_minimum<c||state.side_minimum<a);
        if(overlaps)hit=narrow.sweep_triangle(entry,triangle,vertices,state)||hit;
    }
    return hit;
}

struct CollisionSweepCellRange {std::int32_t begin=0,end=0;};
inline CollisionSweepCellRange collision_sweep_cell_range(float low,float high,float origin,float step,std::uint32_t count) {
    if(!std::isfinite(step)||!step||count>std::uint32_t(std::numeric_limits<std::int32_t>::max()))
        throw std::runtime_error("Invalid collision sweep grid");
    const auto reciprocal=1/static_cast<long double>(step);
    auto truncate=[](long double value) {
        if(!std::isfinite(value)||value<std::numeric_limits<std::int32_t>::min()||value>std::numeric_limits<std::int32_t>::max())
            throw std::runtime_error("Collision sweep grid coordinate exceeds native integer range");
        return static_cast<std::int32_t>(value); //Native __ftol truncates, including negative fractions.
    };
    return {std::max<std::int32_t>(0,truncate((static_cast<long double>(low)-origin)*reciprocal)),
        std::min<std::int32_t>(std::int32_t(count),truncate((static_cast<long double>(high)-origin)*reciprocal+1))};
}

//00715510's static grid traversal. Its caller must construct the full query
//state and execute the actual dynamic sweep first. Cell+08/+0C survive file
//loading unchanged and filter Y before any candidate/primitive receiver runs.
template<class Frame,class Resources,class NarrowPhase>
bool visit_collision_sweep_cells(const CollisionTree<Frame>& tree,CollisionSweepState& state,
    Resources& resources,NarrowPhase& narrow) {
    if(tree.cell_fields8.size()!=tree.cells.size())throw std::runtime_error("Collision sweep cell limits are missing");
    const auto x=collision_sweep_cell_range(state.minimum[0],state.maximum[0],tree.bounds_and_steps[0],tree.bounds_and_steps[4],tree.count_x);
    const auto z=collision_sweep_cell_range(state.minimum[2],state.maximum[2],tree.bounds_and_steps[1],tree.bounds_and_steps[5],tree.count_z);
    const auto y=collision_sweep_cell_range(state.minimum[1],state.maximum[1],tree.fields20[1],tree.fields20[3],tree.count_y);
    bool hit=false;
    for(auto iz=z.begin;iz<z.end;++iz)for(auto iy=y.begin;iy<y.end;++iy)for(auto ix=x.begin;ix<x.end;++ix) {
        const auto index=tree.cell_index(std::size_t(ix),std::size_t(iy),std::size_t(iz));
        const auto limits=tree.cell_fields8.at(index);
        if(state.minimum[1]<=limits[0]&&limits[1]<state.maximum[1])
            hit=filter_collision_sweep_cell(tree,tree.cells.at(index),state,resources,narrow)||hit;
    }
    return hit;
}
} //namespace hd2::game
#endif
