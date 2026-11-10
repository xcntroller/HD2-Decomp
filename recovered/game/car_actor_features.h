#ifndef HD2_GAME_CAR_ACTOR_FEATURES_H
#define HD2_GAME_CAR_ACTOR_FEATURES_H

#include "game/car_sound_updates.h"

namespace hd2::game {

//004D3970 reads the actual active scene camera and the epoch comparison
//produced by004D3110. FFC is actor-relative (native Car coreF6C), distinct
//from actor640/core5B0's light command bits. No camera/visibility is invented.
template<class Frames>void update_car_actor_features(Frames& frames,typename Frames::Frame model,
        typename Frames::Frame camera,bool same_epoch,bool destroyed,std::uint32_t mode,
        std::uint32_t& features,CarDrivetrainState& drive,CarSoundUpdates<Frames>& audio){
    features&=0x7ffc2026u;
    if(destroyed){features|=0x800313d1u;return;}
    if((mode&7u)!=7u){
        if(!(mode&2u)){features|=0x8000cc10u;return;}
        if(mode&1u)features|=0x8000cc10u;
    }
    if(!camera||!model)throw std::runtime_error("Car view features require the actual active camera/model");
    const auto view=frames.actor_world_matrix(camera),world=frames.actor_world_matrix(model);
    const auto y=static_cast<long double>(world[13])-view[13],x=static_cast<long double>(world[12])-view[12],z=static_cast<long double>(world[14])-view[14];
    const auto distance=(z*z+x*x)+y*y; //004D3A07..3A2F keeps all differences/sum extended.
    if(distance<900.f)features|=0x41u;
    if(same_epoch||distance<900.f)features|=0x200u;
    if(same_epoch||distance<600.f)features|=0x1100u;
    if(distance<22500.f)features|=0x80u;
    if(same_epoch||distance<400.f)features|=0xc000u;
    if(same_epoch&&distance<600.f)features|=0x800u;else features|=0x20000u;
    features|=0x80000010u;if(distance<900.f)features|=0x400u;
    audio.audible(drive,distance<10000.f&&frames.car_frame_visible(model));
}

} //namespace hd2::game
#endif
