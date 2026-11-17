#ifndef HD2_GAME_CAR_OFFLINE_FRAME_H
#define HD2_GAME_CAR_OFFLINE_FRAME_H

#include "frontend/spatial_sound.h"
#include <cstdint>
#include <stdexcept>

namespace hd2::game {

//004D2790/004D3110 visit the SAME ten allocated gun controls, from9 to0.
//Read each slot when reached: an earlier control may retire a later borrower.
template<class Car>void advance_car_mounted_controls(Car& car,bool authority,std::uint32_t elapsed){
    for(auto n=car.fields24b4.size();n;){
        auto& control=car.fields24b4[--n];if(!control)continue;
        if(!control->loading)throw std::logic_error("Car frame requires its actual mounted control lifetime");
        control->loading->advance(authority,elapsed);
    }
}

//004D2790's offline, unsupported/upright test. Support is observed BEFORE
//the extended up-vector comparison. Each selected occupant is reread after
//the preceding command, since that command can change the live seat list.
template<class Car,class Vector,class Support,class Exit>void request_car_unstable_occupant_exit(
        Car& car,std::uint32_t mode,float health,float speed,const Vector& up,Support&& support,Exit&& exit){
    if((mode&7u)!=7u||health==0||!(speed<.10000000149011612f)||support())return;
    if(!((static_cast<long double>(up[2])+up[0])*0.f+up[1]<.7070000171661377f))return;
    for(auto n=car.occupants.size();n;){const auto index=--n;if(car.occupants[index])exit(car.occupants[index]);}
}

//004D2DC0, RET8. Native actorF7C is coreEEC (the exterior sound range),
//not body health.234F is core22BF, the completed engine-start state. This
//reads sound E0&1 and16C directly, without virtual78 playback queries.
template<class Frames,class Sounds,class Fields>void observe_car_engine_sound(Frames& frames,
        const Sounds& sounds,const Fields& fields,bool started,float& volume,float& range){
    range=fields.get(0xeec);volume=0;
    if(!started)return;
    const auto observe=[&](unsigned offset){
        const auto sound=sounds.sound(offset);if(!sound||!frames.car_sound_enabled(sound))return;
        const float value=frames.car_sound_volume(sound);
        if(value>volume)volume=value; //Ordered comparison; NaN never replaces the maximum.
    };
    for(auto first:{0xc58u,0xc6cu,0xc80u})for(unsigned n=0;n!=5;++n)observe(first+n*4);
    observe(0xc54); //The standalone exterior voice is visited LAST.
}

//004D2790's final offline driver-hearing producer. The driver is reread
//after observation; SOUND54 returns an unused handle. No new voice pool.
template<class Car,class Observe,class Emit>void emit_car_engine_hearing(Car& car,Observe&& observe,Emit&& emit){
    if(!car.driver24ac)return;
    float volume=0,range=0;observe(volume,range);
    if(volume==0)return;
    frontend::SpatialSoundRequest request;
    request.flags=2;request.relative=car.base().frame();request.source=car.driver24ac;
    request.group=0;request.entry=43;request.far_scale=range;request.volume=volume;
    (void)emit(request);
}

} //namespace hd2::game
#endif
