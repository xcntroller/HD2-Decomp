#ifndef HD2_GAME_CAR_GLASS_DAMAGE_H
#define HD2_GAME_CAR_GLASS_DAMAGE_H
#include "game/car_contact_callbacks.h"
namespace hd2::game {
//00500DA0. Material exchange precedes the zero-health callback.69C's actual
//004D4A10 ignores direction/name, but the selected inline name is still read
//after publishing detached bit1. Positive fragments remain a required receiver.
template<class Frames,class Part,class Name,class Hide>void damage_car_glass(Frames& frames,Part& part,
        float threshold,const CarContactVector& direction,Name&& name,Hide&& hide){
    if(part.health<threshold&&!(part.flags&8u)){
        if(frames.car_exchange_glass_texture(part.mesh,part.glass_texture))part.flags|=8u;
    }
    if(part.health<=0){part.flags|=1u;const auto selected=name();hide(direction,selected.empty()?nullptr:selected.c_str());}
}
}
#endif
