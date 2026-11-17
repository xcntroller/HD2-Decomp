#ifndef HD2_GAME_DOG_GROUND_QUERY_H
#define HD2_GAME_DOG_GROUND_QUERY_H
#include "game/collision_motion.h"
#include "game/human_animation_requests.h"
namespace hd2::game {
//005C62C0 integrates a COPIED velocity/85ms phase and returns ST0. It does not
//advance the actual Body250/254 owner;005C6250 is their separate producer.
inline long double dog_gravity_displacement(std::int32_t elapsed,float velocity,float phase){
    long double result=0,speed=velocity;auto remaining=elapsed;
    if(remaining<=0)return result;
    while(static_cast<long double>(phase)+remaining>85.f){
        const auto part=85.L-phase;result+=.001f*part*speed;
        speed+=static_cast<long double>(9.8100004196167f)*.3333333432674408f;
        remaining=human_animation_time_integer(static_cast<long double>(remaining)-part);phase=0;
        if(remaining<=0)return result;
    }
    result+=static_cast<long double>(remaining)*.001f*speed;return result;
}
inline void advance_dog_gravity(std::int32_t elapsed,float& velocity,float& phase){ //005C6250; native independent FLOAT velocity stores.
    auto remaining=elapsed;if(remaining<=0)return;
    while(static_cast<long double>(remaining)+phase>85.f){
        velocity=static_cast<float>(static_cast<long double>(9.8100004196167f)*.3333333432674408f+velocity);
        remaining=human_animation_time_integer(static_cast<long double>(remaining)-(85.L-phase));phase=0;
        if(remaining<=0)return;
    }
    phase=static_cast<float>(static_cast<long double>(remaining)+phase);
}
//005C6380 consumes the actual CollisionMotion request, NOT Body+90. The
//normal expression retains multiplication by0: nonfinite X/Z can be unordered.
//All contacts are visited even after one failed the support predicate.
template<class Hit,class Queries>bool dog_contacts_supported(const CollisionMotion<Hit>& request,Queries& queries){
    if(request.contact_count>request.contacts.size())throw std::runtime_error("Dog support exceeds actual contact storage");
    bool supported=true;
    for(std::size_t n=0;n!=request.contact_count;++n){
        const auto& slot=request.contacts[n];if(!slot)throw std::runtime_error("Dog support reads unproduced actual contact");
        const auto& normal=slot->normal;
        bool one=(static_cast<long double>(normal[0])+normal[2])*0.f+normal[1]>.7070000171661377f;
        if(!one)one=(queries.human_geometry_flags(slot->identity)&0x40u)&&(queries.human_geometry_mask(slot->identity)&0x100u);
        supported=bool(supported&one);
    }
    return supported;
}
}
#endif
