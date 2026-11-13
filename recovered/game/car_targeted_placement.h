#ifndef HD2_GAME_CAR_TARGETED_PLACEMENT_H
#define HD2_GAME_CAR_TARGETED_PLACEMENT_H

#include "game/car_contact_callbacks.h"
#include <optional>

namespace hd2::game {
struct CarTargetedPick {float distance;CarContactVector normal;};
struct CarTargetedPlacement {CarContactVector position{},direction{};};

//004DEC60 RET16. Pick against the actual target model, not the World physics
//tree. A miss leaves both native outputs unwritten; represent that absence until
//the containing command actually needs them. Fourth lanes are scratch padding.
template<class Frames,class Frame>std::optional<CarTargetedPlacement> car_targeted_placement(
        Frames& frames,Frame target,const CarContactVector& head,const CarContactVector& direction){
    auto displacement=direction;
    for(unsigned axis=0;axis!=3;++axis)displacement[axis]=direction[axis]*10.f; //FLOAT0080FA14.
    const auto hit=frames.car_targeted_pick(head,displacement,target);
    if(!hit)return std::nullopt;
    CarTargetedPlacement result;
    for(unsigned axis=0;axis!=2;++axis)result.position[axis]=static_cast<float>(
        static_cast<long double>(hit->distance)*direction[axis]+head[axis]);
    const float z=hit->distance*direction[2];result.position[2]=z+head[2];
    for(unsigned axis=0;axis!=3;++axis)result.direction[axis]=-hit->normal[axis];
    return result;
}
} //namespace hd2::game
#endif
