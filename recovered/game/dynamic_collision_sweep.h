#ifndef HD2_GAME_DYNAMIC_COLLISION_SWEEP_H
#define HD2_GAME_DYNAMIC_COLLISION_SWEEP_H

#include "game/collision_sweep_box.h"
#include "game/actor_activation_math.h"
#include "game/dynamic_collision_grid.h"

namespace hd2::game {

//00702810 required40/41/42 descriptor consumers. Object is the SAME retained
//descriptor registered in DynamicCollisionBuckets, not a copied collider view.
//Other primitive families require their actual producers before acceptance.
template<class Object,class Math>
class DynamicCollisionSweepNarrowPhase {
public:
    explicit DynamicCollisionSweepNarrowPhase(Math& math):math_(math) {}
    bool sweep(const Object& object,CollisionSweepState& state) {
        using namespace collision_sweep_math;
        const auto delta=subtract(object.position,state.start);const auto square=dot(delta,delta);
        const auto projection=dot(delta,state.direction);const float radius=object.radius+state.radius;
        const auto perpendicular=subtract(scale(state.direction,projection),delta);
        const float radius_square=radius*radius;
        if(!(dot(perpendicular,perpendicular)<=radius_square))return false;
        const float reach=projection<=0?radius:radius+state.length;
        if(!(square<=reach*reach))return false;
        auto publish=[&] {state.hit_dynamic=&object;state.hit_static.reset();return true;};
        switch(object.flags&0xff) {
        case 0x40: {
            if(!object.matrix||!object.inverse)throw std::runtime_error("Dynamic box has no actual transforms");
            auto local=state;local.start=math_.car_transform(state.start,*object.inverse);
            local.displacement=math_.collision_sweep_rotate(state.displacement,*object.inverse);
            local.direction=actor_activation_direction(local.displacement);
            CollisionPoint low{},high{};for(unsigned axis=0;axis!=3;++axis){low[axis]=object.minimum[axis];high[axis]=object.maximum[axis];}
            if(!collision_sweep_box(low,high,local))return false;
            const auto position=math_.car_transform(local.hit_position,*object.matrix),normal=math_.collision_sweep_rotate(local.hit_normal,*object.matrix);
            state.hit_position=position;state.hit_normal=normal;state.best_distance_squared=local.best_distance_squared;return publish();
        }
        case 0x41: {
            CollisionPoint low{},high{};for(unsigned axis=0;axis!=3;++axis){low[axis]=object.minimum[axis];high[axis]=object.maximum[axis];}
            return collision_sweep_box(low,high,state)?publish():false;
        }
        case 0x42: {
            if(!(projection>0))return false;
            if(square<radius_square&&9.99999993922529e-09f<=square) {
                const float scale=static_cast<float>(radius/std::sqrt(static_cast<long double>(square)));
                state.hit_position=subtract(object.position,collision_sweep_math::scale(delta,scale));state.hit_position[1]=state.start[1];
                state.hit_normal={{delta[0],.01f,delta[2],0}};state.best_distance_squared=0;return publish();
            }
            const float discriminant=radius_square-(square-projection*projection);
            if(discriminant<0)return false;
            const float distance=static_cast<float>(projection-std::sqrt(static_cast<long double>(discriminant)));
            if(distance<-state.radius)return false;
            const float travel_square=distance*distance;if(!(travel_square<state.best_distance_squared))return false;
            state.hit_position=add(state.start,scale(state.direction,distance));state.hit_normal=subtract(object.position,state.hit_position);
            if(state.hit_normal[1]<0)state.hit_normal[1]=0;
            state.best_distance_squared=travel_square;return publish();
        }
        default:throw std::runtime_error("Required dynamic sweep primitive is not reconstructed");
        }
    }
private:
    Math& math_;
};

//00702810's Z/X traversal and actual descriptor mask filters. The +8 layer
//bits added by006EFA70/006EFB10 exclude the current Human's own volumes.
template<class Object,class Grid,class NarrowPhase>
bool visit_dynamic_collision_sweep(const Grid& grid,const DynamicCollisionBuckets<Object,Grid>& buckets,
    CollisionSweepState& state,NarrowPhase& narrow) {
    const auto x=collision_sweep_cell_range(state.minimum[0],state.maximum[0],grid.origin[0],grid.step[0],grid.count_x);
    const auto z=collision_sweep_cell_range(state.minimum[2],state.maximum[2],grid.origin[1],grid.step[1],grid.count_z);
    bool hit=false;
    for(auto iz=z.begin;iz<z.end;++iz)for(auto ix=x.begin;ix<x.end;++ix) {
        const auto* bucket=buckets.bucket(std::uint32_t(ix),std::uint32_t(iz));if(!bucket)continue;
        for(const auto* object:bucket->objects) {
            if(!object)throw std::runtime_error("Null dynamic collision descriptor");
            if(!(state.excluded_layers&object->mask)&&!(state.excluded_flags&object->flags))hit=narrow.sweep(*object,state)||hit;
        }
    }
    return hit;
}
} //namespace hd2::game
#endif
