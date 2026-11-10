#ifndef HD2_GAME_PHYSICS_OBJECT_MOTION_H
#define HD2_GAME_PHYSICS_OBJECT_MOTION_H

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <optional>
#include <stdexcept>

namespace hd2::game {

//0040B5E0 known common motion fields. Fourth vector lanes are not initialized
//by the original; define them safely without crediting a native value contract.
struct PhysicsObjectMotion {
    using Vector=std::array<float,4>;
    Vector velocity{{0,-.10000000149011612f,0,0}},angular{},position{},rotation{};
    float impulse_divisor=1; //0040B5E0 native130, fallback1 only when exactly zero.
    std::uint32_t phase=0; //Native164; common impulse restores phase0.
};
struct PhysicsObjectMotionHit {PhysicsObjectMotion::Vector position{},normal{};std::uint32_t flags=0;};
struct PhysicsObjectGroundStop {bool enabled=false,cached=false;std::optional<float> height;}; //Type4 native170/171/174;174 is genuinely unwritten until a floor ray succeeds.

namespace physics_object_motion_math {
using Vector=PhysicsObjectMotion::Vector;
inline float square(const Vector& v){
    const float value=static_cast<float>((static_cast<long double>(v[2])*v[2]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[0])*v[0]);
    if(!std::isfinite(value))throw std::runtime_error("Physics motion requires finite vector lengths");
    return value;
}
inline long double length(float value){
    std::int32_t bits;std::memcpy(&bits,&value,4);
    if(bits<0x322bcc77)return 0;
    if(std::abs(static_cast<long double>(value)-1)<9.99999993922529e-09f)return 1;
    return std::sqrt(static_cast<long double>(value));
}
//Native small-vector fallback changes only the chosen component: X, then Z,
//then Y. It does NOT clear the remaining small components or normalize W.
inline void normalize(Vector& v){
    const auto value=square(v);constexpr float epsilon=9.99999993922529e-09f;
    if(std::abs(static_cast<long double>(value)-1)<epsilon)return;
    if(value>=epsilon){const auto inverse=1/std::sqrt(static_cast<long double>(value));
        for(unsigned a=0;a!=3;++a)v[a]=static_cast<float>(static_cast<long double>(v[a])*inverse);
    }else if(v[0]!=0)v[0]=v[0]<0?-1.f:1.f;
    else if(v[2]!=0)v[2]=v[2]<0?-1.f:1.f;
    else v[1]=v[1]<0?-1.f:1.f;
}
//0040B360: preserve speed, normalize, double cross product, restore speed,
//then actual optional object14C event1 (EDX150 handle/stack154 context).
//No generic event success.
template<class Feedback>void reflect(Vector& velocity,const Vector& normal,Feedback&& feedback){
    const auto speed=length(square(velocity));normalize(velocity);
    const float x=static_cast<float>((static_cast<long double>(velocity[2])*normal[1]-static_cast<long double>(velocity[1])*normal[2])*2);
    const float y=static_cast<float>((static_cast<long double>(velocity[0])*normal[2]-static_cast<long double>(normal[0])*velocity[2])*2);
    const float z=static_cast<float>((static_cast<long double>(normal[0])*velocity[1]-static_cast<long double>(velocity[0])*normal[1])*2);
    velocity[0]=static_cast<float>((static_cast<long double>(y)*normal[2]-static_cast<long double>(z)*normal[1])-velocity[0]);
    velocity[1]=static_cast<float>((static_cast<long double>(z)*normal[0]-static_cast<long double>(x)*normal[2])-velocity[1]);
    velocity[2]=static_cast<float>((static_cast<long double>(x)*normal[1]-static_cast<long double>(y)*normal[0])-velocity[2]);
    for(unsigned a=0;a!=3;++a)velocity[a]=static_cast<float>(speed*velocity[a]);
    feedback();
}
} //namespace physics_object_motion_math

//0040ADE0's common angular update. Required Math is the actual LS3DF
//RotateX/Y/Z -> (X*Y)*Z -> S_quat::RotateByMatrix receiver, not a no-op.
template<class Math>void advance_physics_object_rotation(PhysicsObjectMotion& state,float seconds,Math& math){
    using namespace physics_object_motion_math;
    const auto speed=length(square(state.angular));normalize(state.angular);
    const auto remaining=(1.L-seconds)*speed;PhysicsObjectMotion::Vector angles{};
    for(unsigned a=0;a!=3;++a){
        state.angular[a]=static_cast<float>(remaining*state.angular[a]);
        angles[a]=static_cast<float>(static_cast<long double>(state.angular[a])*seconds);
    }
    state.rotation=math.physics_rotate_quaternion(state.rotation,angles);
}

//0040E160 point/type0 and0040E3B0 sphere/type2. World, collision identities,
//normal and optional feedback are REQUIRED services; callers retain their own
//actual descriptor and perform0040AB80 pose publication even after false.
template<class Services>bool advance_basic_physics_object(PhysicsObjectMotion& state,float seconds,
        bool sphere,float radius,bool collide_dynamic,Services& services){
    using namespace physics_object_motion_math;
    if(!services.physics_frame_exists())return false;
    advance_physics_object_rotation(state,seconds,services); //Native virtual10 BEFORE movement; gravity0C is later.
    const auto start=services.physics_world_position();PhysicsObjectMotion::Vector displacement{};
    for(unsigned a=0;a!=3;++a)displacement[a]=static_cast<float>(static_cast<long double>(seconds)*state.velocity[a]);
    std::optional<PhysicsObjectMotionHit> hit;
    if(sphere){
        auto direction=displacement;normalize(direction);auto adjusted=start;
        for(unsigned a=0;a!=3;++a)adjusted[a]=static_cast<float>(static_cast<long double>(start[a])-static_cast<long double>(direction[a])*.0010000000474974513f);
        hit=services.physics_sphere_query(adjusted,displacement,radius);
    }else hit=services.physics_point_query(start,displacement);
    if(!hit||(!collide_dynamic&&(hit->flags&0x40u))){
        for(unsigned a=0;a!=3;++a)state.position[a]=static_cast<float>(static_cast<long double>(state.position[a])+displacement[a]);
        state.velocity[1]=static_cast<float>(static_cast<long double>(state.velocity[1])-static_cast<long double>(seconds)*9.8100004196167f); //0040AD40.
        return true;
    }
    if(length(square(state.velocity))>=1){
        reflect(state.velocity,hit->normal,[&]{services.physics_collision_feedback();});
        long double factor=.4000000059604645f;
        if(sphere){
            PhysicsObjectMotion::Vector travelled{};
            for(unsigned a=0;a!=3;++a)travelled[a]=static_cast<float>(static_cast<long double>(hit->position[a])-start[a]);
            factor=(length(square(travelled))/length(square(displacement)))*.4000000059604645f;
            for(unsigned a=0;a!=3;++a)state.position[a]=hit->position[a];
        }
        for(unsigned a=0;a!=3;++a)state.velocity[a]=static_cast<float>(factor*state.velocity[a]);
        return true;
    }
    if(sphere)for(unsigned a=0;a!=3;++a)state.velocity[a]=0;
    for(unsigned a=0;a!=3;++a)state.position[a]=hit->position[a];
    return false; //Point leaves its low velocity intact; sphere zeros XYZ.
}

//0040B910/type4, same angular and gravity receivers as point0 but a distinct
//cached4m floor branch and delete158 contract. Native zeroes ONLY linear XYZ
//on either stop path; angular velocity and fourth lanes are left untouched.
template<class Services>bool advance_fading_physics_object(PhysicsObjectMotion& state,PhysicsObjectGroundStop& ground,
        bool& delete_when_stopped,float seconds,bool collide_dynamic,Services& services){
    using namespace physics_object_motion_math;
    if(!services.physics_frame_exists())return false;
    advance_physics_object_rotation(state,seconds,services);
    const auto start=services.physics_world_position();PhysicsObjectMotion::Vector delta{};
    for(unsigned a=0;a!=3;++a)delta[a]=static_cast<float>(static_cast<long double>(seconds)*state.velocity[a]);
    std::optional<PhysicsObjectMotionHit> hit;
    if(!ground.enabled)hit=services.physics_point_query(start,delta);
    else{
        if(delta[1]<0&&!ground.cached){
            ground.cached=true;const auto floor=services.physics_point_query(start,{{0,-4,0,0}});
            if(!floor){
                for(unsigned a=0;a!=3;++a){state.velocity[a]=0;state.position[a]=static_cast<float>(static_cast<long double>(state.position[a])+delta[a]);}
                delete_when_stopped=true;return false;
            }
            ground.height=floor->position[1];
        }
        if(delta[1]>0)ground.cached=false;
        if(!ground.height)throw std::runtime_error("Physics protected floor height is unwritten before its actual descending ray");
        if(static_cast<long double>(state.position[1])+delta[1]<*ground.height){
            ground.cached=false;PhysicsObjectMotionHit floor;
            for(unsigned a=0;a!=3;++a)floor.position[a]=static_cast<float>(static_cast<long double>(state.position[a])+delta[a]);
            floor.position[1]=static_cast<float>(static_cast<long double>(*ground.height)+.00009999999747378752f);
            floor.normal={{0,1,0,0}};floor.flags=0xb0; //Native synthetic pointer is this: vtable0080F5B0 low BYTE has bit40 CLEAR.
            hit=floor;
        }
    }
    if(!hit||(!collide_dynamic&&(hit->flags&0x40u))){
        for(unsigned a=0;a!=3;++a)state.position[a]=static_cast<float>(static_cast<long double>(state.position[a])+delta[a]);
        state.velocity[1]=static_cast<float>(static_cast<long double>(state.velocity[1])-static_cast<long double>(seconds)*9.8100004196167f);
        return true;
    }
    if(length(square(state.velocity))<1){
        for(unsigned a=0;a!=3;++a){state.velocity[a]=0;state.position[a]=hit->position[a];}
        delete_when_stopped=true;return false;
    }
    reflect(state.velocity,hit->normal,[&]{services.physics_collision_feedback();});
    auto travel_length=length(square(delta));if(travel_length==0)travel_length=1; //Native fallback denominator for zero displacement.
    PhysicsObjectMotion::Vector travelled{};
    for(unsigned a=0;a!=3;++a)travelled[a]=static_cast<float>(static_cast<long double>(hit->position[a])-start[a]);
    const auto factor=(length(square(travelled))/travel_length)*.4000000059604645f;
    for(unsigned a=0;a!=3;++a){state.position[a]=hit->position[a];state.velocity[a]=static_cast<float>(factor*state.velocity[a]);}
    return true;
}

//0040EDD0/type1 extends a POINT ray by170, then subtracts that extension
//from both hit and displacement. Unlike0/2/4 it does NOT call angular10 or
//gravity0C on the unobstructed path and ALWAYS returns true with a live frame.
template<class Services>bool advance_extended_ray_physics_object(PhysicsObjectMotion& state,float seconds,
        float extension,bool collide_dynamic,Services& services){
    using namespace physics_object_motion_math;
    if(!services.physics_frame_exists())return false;
    const auto start=services.physics_world_position();auto position=start;
    PhysicsObjectMotion::Vector delta{};
    for(unsigned a=0;a!=3;++a)delta[a]=static_cast<float>(static_cast<long double>(seconds)*state.velocity[a]);
    const auto distance=static_cast<float>(length(square(delta)));normalize(delta);
    const auto extended=static_cast<float>(static_cast<long double>(distance)+extension);
    for(unsigned a=0;a!=3;++a)delta[a]=static_cast<float>(static_cast<long double>(delta[a])*extended);
    auto direction=delta;normalize(direction);auto query_start=start;
    for(unsigned a=0;a!=3;++a)query_start[a]=static_cast<float>(static_cast<long double>(start[a])-static_cast<long double>(direction[a])*.0010000000474974513f);
    auto hit=services.physics_point_query(query_start,delta);
    direction=delta;normalize(direction);
    for(unsigned a=0;a!=3;++a){
        const auto offset=static_cast<long double>(direction[a])*extension;
        if(hit)hit->position[a]=static_cast<float>(static_cast<long double>(hit->position[a])-offset);
        delta[a]=static_cast<float>(static_cast<long double>(delta[a])-offset);
    }
    if(!hit||(!collide_dynamic&&(hit->flags&0x40u))){
        for(unsigned a=0;a!=3;++a)position[a]=static_cast<float>(static_cast<long double>(start[a])+delta[a]);
    }else if(length(square(state.velocity))<1){
        for(unsigned a=0;a!=3;++a)position[a]=hit->position[a]; //Low hit does NOT stop, zero velocity or retire.
    }else{
        reflect(state.velocity,hit->normal,[&]{services.physics_collision_feedback();});
        PhysicsObjectMotion::Vector travelled{};
        for(unsigned a=0;a!=3;++a)travelled[a]=static_cast<float>(static_cast<long double>(hit->position[a])-start[a]);
        const auto travelled_length=static_cast<float>(length(square(travelled)));
        const auto spent=static_cast<long double>(travelled_length)/length(square(state.velocity));
        state.velocity[1]=static_cast<float>(spent*9.8100004196167f+(static_cast<long double>(state.velocity[1])-static_cast<long double>(seconds)*9.8100004196167f));
        seconds=static_cast<float>(static_cast<long double>(seconds)-spent);
        for(unsigned a=0;a!=3;++a)state.velocity[a]=static_cast<float>(static_cast<long double>(state.velocity[a])*.4000000059604645f);
        state.velocity[1]=static_cast<float>(static_cast<long double>(state.velocity[1])-static_cast<long double>(seconds)*9.8100004196167f);
        //0040F510..55A computes remaining-time*velocity into a scratch vector,
        //then0040F5A2 publishes the UNCHANGED start. There is no second ray or
        //remaining displacement addition in the original high-speed branch.
    }
    state.position=position;
    services.physics_publish_extended_position(position); //Native08/24 and FIRST bucket move before common0040AB80.
    return true;
}

//0040B030 common virtual08 and0040EB10 sphere2 override. Sphere's linear
//magnitude is always15 and BOTH its linear/angular paths ignore caller scale.
//Angular input uses inverse(worldPosition+ORIGINAL force), not the normalized
//linear impulse and not a synthetic transform of a direction with W=0.
template<class Services>void impulse_physics_object(PhysicsObjectMotion& state,
        const PhysicsObjectMotion::Vector& force,const PhysicsObjectMotion::Vector& point,
        float scale,bool sphere,Services& services){
    using namespace physics_object_motion_math;
    if(!services.physics_frame_exists())return;
    auto direction=force;const auto speed=length(square(force));normalize(direction);
    const auto magnitude=sphere?15.L:std::sqrt(speed*speed*scale/(state.impulse_divisor==0?1:state.impulse_divisor));
    for(unsigned a=0;a!=3;++a)state.velocity[a]=static_cast<float>(static_cast<long double>(state.velocity[a])+static_cast<long double>(direction[a])*magnitude);
    services.physics_begin_inverse(); //Fresh actual world/inverse AFTER publishing the linear impulse.
    const auto local_point=services.physics_inverse_point(point);auto endpoint=services.physics_world_position();
    for(unsigned a=0;a!=3;++a)endpoint[a]=static_cast<float>(static_cast<long double>(endpoint[a])+force[a]);
    const auto local_end=services.physics_inverse_point(endpoint);
    const long double multiplier=sphere?1.L:scale;
    const float x=static_cast<float>((static_cast<long double>(local_point[2])*local_end[1]-static_cast<long double>(local_point[1])*local_end[2])*multiplier);
    const float y=static_cast<float>((static_cast<long double>(local_point[0])*local_end[2]-static_cast<long double>(local_point[2])*local_end[0])*multiplier);
    const auto z=(static_cast<long double>(local_point[1])*local_end[0]-static_cast<long double>(local_point[0])*local_end[1])*multiplier;
    state.angular[0]=static_cast<float>(static_cast<long double>(state.angular[0])+x);
    state.angular[1]=static_cast<float>(static_cast<long double>(state.angular[1])+y);
    state.angular[2]=static_cast<float>(static_cast<long double>(state.angular[2])+z);
    state.phase=0;
}
} //namespace hd2::game
#endif
