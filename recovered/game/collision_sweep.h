#ifndef HD2_GAME_COLLISION_SWEEP_H
#define HD2_GAME_COLLISION_SWEEP_H

#include "game/static_collision_sweep.h"
#include "game/dynamic_collision_sweep.h"
#include "game/actor_activation_math.h"

namespace hd2::game {

//00715510/00718530 composition for the actual loaded static tree and its SAME
//dynamic buckets. Output identities borrow descriptors/tree records. Unsupported
//primitive receivers throw; no synthetic hit/no-hit backend is supplied.
template<class Frame,class Object,class Resources,class Math>
class CollisionSweep {
public:
    using Tree=CollisionTree<Frame>;
    using Grid=typename Tree::DynamicGrid;
    using Buckets=DynamicCollisionBuckets<Object,Grid>;
    using Vector=CollisionSweepState::Vector;
    struct Hit {
        Vector position{},normal{};float distance_squared=0;
        std::optional<CollisionCellReference> static_identity;
        const Object* dynamic_identity=nullptr;
    };
    CollisionSweep(const Tree* tree,const Buckets* buckets,Resources& resources,Math& math)
        :tree_(tree),buckets_(buckets),resources_(resources),static_(math),dynamic_(math) {}
    std::optional<Hit> sweep(const Vector& start,const Vector& displacement,float radius,
        std::uint32_t excluded_layers=0,std::uint32_t excluded_flags=0,bool normal=true) {
        if(!tree_)return std::nullopt;
        if(!std::isfinite(radius))throw std::runtime_error("Non-finite collision sweep radius");
        CollisionSweepState state;state.start=start;state.displacement=displacement;state.radius=radius;
        state.excluded_layers=excluded_layers|8u;state.excluded_flags=(excluded_flags==0xffffffffu?0:excluded_flags)|0x20u;
        const auto square=length_squared(displacement);
        if(square<9.99999993922529e-09f)return std::nullopt;
        set_travel(state,square);set_bounds(state);
        bool hit=false;
        if(excluded_layers!=0xffffffffu&&buckets_) {
            hit=visit_dynamic_collision_sweep(tree_->dynamic,*buckets_,state,dynamic_);
            if(hit) {
                for(unsigned axis=0;axis!=3;++axis)state.displacement[axis]=state.hit_position[axis]-start[axis];
                const auto shortened=length_squared(state.displacement);
                state.best_distance_squared=shortened;
                if(shortened<9.99999993922529e-09f)return finish(state,normal);
                set_travel(state,shortened);set_bounds(state);
            }
        }
        if(excluded_flags!=0xffffffffu) {
            state.side_axis={{state.direction[2],0,-state.direction[0],0}};
            if(length_squared(state.side_axis)<.001)state.side_axis={{1,0,0,0}}; //008106B8 double.
            state.side_axis=actor_activation_direction(state.side_axis);
            const auto& axis=state.side_axis;
            auto plane=[&](bool positive) {
                const auto sign=positive?1.f:-1.f;
                const auto x=static_cast<long double>(axis[0])*radius*sign+start[0];
                const auto y=static_cast<long double>(axis[1])*radius*sign+start[1];
                const auto z=static_cast<long double>(axis[2])*radius*sign+start[2];
                return static_cast<float>(-((static_cast<long double>(axis[1])*y+static_cast<long double>(axis[2])*z)+static_cast<long double>(x)*axis[0]));
            };
            state.side_minimum=plane(true);state.side_maximum=plane(false);
            if(state.side_maximum<state.side_minimum)std::swap(state.side_minimum,state.side_maximum);
            hit=visit_collision_sweep_cells(*tree_,state,resources_,static_)||hit;
        }
        return hit?std::optional<Hit>(finish(state,normal)):std::nullopt;
    }
private:
    static float length_squared(const Vector& value) {
        const float result=static_cast<float>((static_cast<long double>(value[2])*value[2]+static_cast<long double>(value[1])*value[1])+static_cast<long double>(value[0])*value[0]);
        if(!std::isfinite(result))throw std::runtime_error("Non-finite collision sweep displacement");
        return result;
    }
    static void set_travel(CollisionSweepState& state,float square) {
        state.best_distance_squared=square;state.length=static_cast<float>(std::sqrt(static_cast<long double>(square)));
        const auto inverse=1/std::sqrt(static_cast<long double>(square));
        for(unsigned axis=0;axis!=3;++axis)state.direction[axis]=static_cast<float>(state.displacement[axis]*inverse);
    }
    static void set_bounds(CollisionSweepState& state) {
        for(unsigned axis=0;axis!=3;++axis) {
            const float end=state.start[axis]+state.displacement[axis];
            state.minimum[axis]=std::min(state.start[axis],end)-state.radius;
            state.maximum[axis]=std::max(state.start[axis],end)+state.radius;
        }
    }
    static Hit finish(const CollisionSweepState& state,bool normal) {
        Hit hit;hit.position=state.hit_position;hit.distance_squared=state.best_distance_squared;
        hit.static_identity=state.hit_static;hit.dynamic_identity=static_cast<const Object*>(state.hit_dynamic);
        if(normal) {
            hit.normal=actor_activation_direction(state.hit_normal);
            for(unsigned axis=0;axis!=3;++axis)hit.normal[axis]=-hit.normal[axis];
        }
        return hit;
    }
    const Tree* tree_;const Buckets* buckets_;Resources& resources_;
    StaticCollisionSweepNarrowPhase<Frame,Math> static_;
    DynamicCollisionSweepNarrowPhase<Object,Math> dynamic_;
};
} //namespace hd2::game
#endif
