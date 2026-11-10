#ifndef HD2_GAME_CAR_SUPPORT_CHECKS_H
#define HD2_GAME_CAR_SUPPORT_CHECKS_H

#include "game/car_health_reset.h"

namespace hd2::game {

//004D5430 on the SAME body/drivetrain/fuel/wheel/up owners. Flag2 is reached
//by the offline cold004D2790 path; the other two tests have no new services.
template<class Frames>bool car_support_checks(const CarHealthState& health,float fuel,float speed,
        const std::vector<CarWheelRuntime<Frames>>& wheels,const typename Frames::Vector& up,unsigned flags){
    if(flags&1u){
        if(health.body<=0||health.drivetrain<=0||fuel<=0)return false;
        for(auto n=wheels.size();n;)if(wheels[--n].flags&0x40000000u)return false;
    }
    if((flags&2u)&&speed<1.f){
        if(wheels.empty())return false;
        unsigned contacts=0;for(auto n=wheels.size();n;)if(wheels[--n].flags&8u)++contacts;
        if(contacts<2)return false;
        if((up[2]+up[0])*0.f+up[1]<.6000000238418579f)return false;
    }
    if(!(flags&4u))return true;
    bool first=false,second=false;
    for(auto n=wheels.size();n;){const auto index=--n;const auto value=wheels[index].flags;
        if((value&8u)&&!(value&0x40000000u)){if(index&1u)second=true;else first=true;}}
    return first&&second;
}

} //namespace hd2::game
#endif
