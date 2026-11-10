#ifndef HD2_GAME_PHYSICS_OBJECT_PIVOT_H
#define HD2_GAME_PHYSICS_OBJECT_PIVOT_H

#include "game/physics_object_motion.h"

namespace hd2::game {
struct PhysicsObjectPivot {
    //Native1B0 and1C0 are only written by actual phase transitions.
    std::optional<PhysicsObjectMotion::Vector> point,axis;
};
namespace physics_object_pivot_math {
using Vector=PhysicsObjectMotion::Vector;
inline Vector difference(const Vector& first,const Vector& second){auto result=first;
    for(unsigned a=0;a!=3;++a)result[a]=static_cast<float>(static_cast<long double>(first[a])-second[a]);
    return result;
}
inline Vector cross(const Vector& first,const Vector& second){return {{
    static_cast<float>(static_cast<long double>(first[1])*second[2]-static_cast<long double>(first[2])*second[1]),
    static_cast<float>(static_cast<long double>(first[2])*second[0]-static_cast<long double>(first[0])*second[2]),
    static_cast<float>(static_cast<long double>(first[0])*second[1]-static_cast<long double>(first[1])*second[0]),0}};}
inline long double magnitude(const Vector& v){return physics_object_motion_math::length(physics_object_motion_math::square(v));}
template<class Services,class Query>auto excluded(Services& services,Query&& query){
    services.physics_exclude_self();
    try{auto result=query();services.physics_reset_exclusions();return result;}
    catch(...){services.physics_reset_exclusions();throw;}
}
template<class Services>auto segment(Services& services,const Vector& first,const Vector& last){
    return services.physics_pivot_query(first,difference(last,first));
}
inline void damp(Vector& v,float amount){for(unsigned a=0;a!=3;++a)v[a]=static_cast<float>(static_cast<long double>(v[a])*amount);}
inline void support(PhysicsObjectMotion& state,PhysicsObjectPivot& pivot,const Vector& point){
    state.phase=1;pivot.point=point;for(unsigned a=0;a!=3;++a)state.angular[a]=0;
}
} //namespace physics_object_pivot_math

//0040BF20, native type3 phase0. Tentative frame translation is observable
//before the three static rays; common0040AB80 later publishes final owner pose.
template<class Services>bool advance_free_aggregate_physics(PhysicsObjectMotion& state,PhysicsObjectPivot& pivot,
        float seconds,Services& services){
    using namespace physics_object_pivot_math;
    if(!services.physics_frame_exists())return false;
    (void)services.physics_axis_world(0);(void)services.physics_axis_world(1);
    Vector delta{},tentative=state.position;
    for(unsigned a=0;a!=3;++a){delta[a]=static_cast<float>(static_cast<long double>(seconds)*state.velocity[a]);
        tentative[a]=static_cast<float>(static_cast<long double>(state.position[a])+delta[a]);}
    services.physics_publish_position(tentative);
    auto first=services.physics_axis_world(0),last=services.physics_axis_world(1);
    auto hit=excluded(services,[&]{
        auto value=segment(services,first,last);if(!value)value=segment(services,last,first);
        if(!value)value=services.physics_pivot_query(state.position,delta);
        return value;
    });
    if(hit){
        physics_object_motion_math::reflect(state.velocity,hit->normal,[&]{services.physics_collision_feedback();});
        damp(state.velocity,.4000000059604645f);const auto speed=magnitude(state.velocity);
        if(speed>=.10000000149011612f){
            if(speed>=.5f)return true;
            if(hit->normal[1]>.699999988079071f)for(unsigned a=0;a!=3;++a)state.velocity[a]=0;
        }
        support(state,pivot,hit->position);return true;
    }
    for(unsigned a=0;a!=3;++a)state.position[a]=static_cast<float>(static_cast<long double>(state.position[a])+delta[a]);
    const auto previous=services.physics_frame_rotation();
    advance_physics_object_rotation(state,seconds,services); //0040BEC0 delegates only while phase0.
    services.physics_publish_rotation(state.rotation);
    first=services.physics_axis_world(0);last=services.physics_axis_world(1);
    hit=excluded(services,[&]{auto value=segment(services,first,last);if(!value)value=segment(services,last,first);return value;});
    if(!hit){state.velocity[1]=static_cast<float>(static_cast<long double>(state.velocity[1])-static_cast<long double>(seconds)*9.8100004196167f);return true;}
    state.rotation=previous; //Frame itself stays tentative until common pose, matching native.
    physics_object_motion_math::reflect(state.velocity,hit->normal,[&]{services.physics_collision_feedback();});
    damp(state.velocity,.5f);
    if(magnitude(state.velocity)<.10000000149011612f)support(state,pivot,hit->position);
    return true;
}

//0040C670: normalized gravity cross normalized lever, angle=actual lever
//length times gravity-step length. Engine quaternion/rotation math is required.
//Only frame pose changes here; owner pose is committed by0040CE00 after queries.
template<class Services>void pivot_aggregate_physics(PhysicsObjectMotion& state,PhysicsObjectPivot& pivot,
        float seconds,Services& services){
    using namespace physics_object_pivot_math;
    const auto point=pivot.point.value();const auto offset=difference(services.physics_world_position(),point);
    Vector gravity{{static_cast<float>(static_cast<long double>(seconds)*0.f),static_cast<float>(static_cast<long double>(seconds)*-9.8100004196167f),static_cast<float>(static_cast<long double>(seconds)*0.f),0}};
    auto direction=offset;auto gravity_direction=gravity;
    physics_object_motion_math::normalize(gravity_direction);physics_object_motion_math::normalize(direction);
    auto axis=cross(gravity_direction,direction);physics_object_motion_math::normalize(axis);
    const auto lever=static_cast<float>(magnitude(offset));
    const auto angle=static_cast<float>(static_cast<long double>(lever)*magnitude(gravity));
    services.physics_publish_axis_rotation(state.rotation,axis,angle);
    const auto rotated=services.physics_rotate_offset(offset,axis,angle);auto position=point;
    for(unsigned a=0;a!=3;++a)position[a]=static_cast<float>(static_cast<long double>(point[a])+rotated[a]);
    services.physics_publish_position(position);
}

//0040CE00/0040D6A0 phases1/2. Query the farther old endpoint first (tie1).
//Phase1 tries BOTH primary endpoint trajectories; phase2 only ONE secondary.
template<class Services>bool advance_supported_aggregate_physics(PhysicsObjectMotion& state,PhysicsObjectPivot& pivot,
        float seconds,Services& services){
    using namespace physics_object_pivot_math;
    if(!services.physics_frame_exists())return false;
    const bool secondary=state.phase==2;const unsigned base=secondary?2:0;
    const auto first=services.physics_axis_world(base),last=services.physics_axis_world(base+1);
    if(secondary){
        const Vector gravity{{static_cast<float>(static_cast<long double>(seconds)*0.f),static_cast<float>(static_cast<long double>(seconds)*-9.8100004196167f),static_cast<float>(static_cast<long double>(seconds)*0.f),0}};
        const auto angle=static_cast<float>((magnitude(gravity)*state.impulse_divisor)*.25f);
        services.physics_publish_axis_rotation(state.rotation,pivot.axis.value(),angle);
    }else pivot_aggregate_physics(state,pivot,seconds,services);
    const auto new_first=services.physics_axis_world(base),new_last=services.physics_axis_world(base+1);
    const auto point=pivot.point.value();
    const auto first_distance=static_cast<float>(magnitude(difference(point,first)));
    const bool first_farther=first_distance>magnitude(difference(point,last));
    auto hit=excluded(services,[&]{
        auto value=first_farther?segment(services,first,new_first):segment(services,last,new_last);
        if(!value&&!secondary)value=first_farther?segment(services,last,new_last):segment(services,first,new_first);
        return value;
    });
    if(!hit){state.rotation=services.physics_frame_rotation();state.position=services.physics_world_position();return true;}
    services.physics_restore_pose(state.position,state.rotation); //Quaternion, position08, then ONE frame24.
    if(!secondary){
        state.phase=2;pivot.axis=difference(last,first);physics_object_motion_math::normalize(*pivot.axis);
        pivot.point=state.position;
        const auto low=services.physics_axis_world(2),high=services.physics_axis_world(3);
        if(low[1]>high[1])damp(*pivot.axis,-1.f); //0040D448 strict greater, not decompiler's inverted comparison.
    }
    return false; //Phase1 ALSO returns false immediately after setting phase2.
}
template<class Services>bool advance_aggregate_physics_object(PhysicsObjectMotion& state,PhysicsObjectPivot& pivot,
        float seconds,Services& services){ //0040BEE0 selector, absent frozen.
    if(state.phase==0)return advance_free_aggregate_physics(state,pivot,seconds,services);
    if(state.phase==1||state.phase==2)return advance_supported_aggregate_physics(state,pivot,seconds,services);
    return true; //Native unknown-phase AL1, not a required-service fallback.
}
} //namespace hd2::game
#endif
