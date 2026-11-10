#ifndef HD2_GAME_PHYSICS_OBJECT_TIPPING_H
#define HD2_GAME_PHYSICS_OBJECT_TIPPING_H

#include "game/physics_object_pivot.h"

namespace hd2::game {
struct PhysicsObjectTipping {
    using Vector=PhysicsObjectMotion::Vector;
    bool active=false; //200.
    std::optional<bool> oscillating; //202 is NOT initialized by00407390.
    float progress=0,damping=.4000000059604645f,speed=1,angle=1;
    std::optional<float> amplitude; //210 first written by actual contact.
    std::uint32_t phase=0;
    Vector saved{}; //1E0 supplied from the real frame during construction/contact.
    std::optional<Vector> positive,negative; //1C0/1D0 first written by contact.
};

//00406D10. Both static endpoint rays execute, even if the first hits. A blocked
//step rolls back the OWNER quaternion by .01 progress; frame remains tentative
//until common0040AB80.004D0440 debug calls are genuine RET16, not a renderer.
template<class Services>bool interpolate_tipping_physics_object(PhysicsObjectMotion& motion,PhysicsObjectTipping& state,Services& services){
    using namespace physics_object_pivot_math;
    if(!services.physics_frame_exists())return false;
    const auto& target=state.positive.value();
    if(state.progress==1)motion.rotation=target;
    else motion.rotation=services.physics_slerp(state.saved,target,static_cast<float>(static_cast<long double>(state.amplitude.value())*state.progress),true);
    const auto first=services.physics_tipping_endpoint(0),last=services.physics_tipping_endpoint(1);
    services.physics_publish_rotation(motion.rotation);
    const auto new_first=services.physics_tipping_endpoint(0),new_last=services.physics_tipping_endpoint(1);
    const bool blocked=excluded(services,[&]{
        const auto a=services.physics_tipping_query(first,difference(new_first,first));
        const auto b=services.physics_tipping_query(last,difference(new_last,last));
        return bool(a)||bool(b);
    });
    if(!blocked)return true;
    const auto factor=static_cast<float>((static_cast<long double>(state.progress)-.009999999776482582f)*state.amplitude.value());
    motion.rotation=services.physics_slerp(state.saved,target,factor,true);return false;
}

//004070C0. Endpoints use direct assignment at EXACT progress1, independently
//of amplitude; phase transitions require strict overshoot and do not consume
//a second phase in the same frame. Negative elapsed is not clamped.
template<class Services>bool advance_tipping_physics_object(PhysicsObjectMotion& motion,PhysicsObjectTipping& state,float seconds,Services& services){
    const auto elapsed=static_cast<long double>(seconds)*state.speed;
    if(!state.oscillating.value()){
        const auto progress=elapsed+state.progress;state.progress=static_cast<float>(progress);
        const bool overshot=progress>1;if(overshot)state.progress=1;
        const bool clear=interpolate_tipping_physics_object(motion,state,services);return clear&&!overshot;
    }
    seconds=static_cast<float>(elapsed);
    if(state.phase==0)return false;
    if(state.phase>4)return true; //Native default AL1, no frame access.
    const auto phase=state.phase;
    if(phase==1||phase==3){
        const auto progress=static_cast<long double>(seconds)+state.progress;state.progress=static_cast<float>(progress);
        if(progress>1){state.progress=1;state.phase=phase+1;}
        if(state.progress==1){motion.rotation=(phase==1?state.positive:state.negative).value();return true;}
    }else{
        const auto progress=static_cast<long double>(state.progress)-seconds;state.progress=static_cast<float>(progress);
        if(progress<0){
            const auto amplitude=static_cast<long double>(state.damping)*state.amplitude.value();
            state.progress=0;state.phase=phase==2?3:1;state.amplitude=static_cast<float>(amplitude);
            if(amplitude<.10000000149011612f){state.active=false;return false;}
        }
    }
    const auto factor=static_cast<float>(static_cast<long double>(state.amplitude.value())*state.progress);
    motion.rotation=services.physics_slerp(state.saved,(phase<=2?state.positive:state.negative).value(),factor,(phase&1u)!=0); //Rising phases true, returning phases false.
    return true;
}
} //namespace hd2::game
#endif
