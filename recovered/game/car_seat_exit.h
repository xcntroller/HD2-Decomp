#ifndef HD2_GAME_CAR_SEAT_EXIT_H
#define HD2_GAME_CAR_SEAT_EXIT_H
#include "game/human_ai_shot_direction.h"
namespace hd2::game {
//004DA420. The caller owns the original seat/body/bounds and SAME query exclusions.
//An obstructed lateral sweep or missing floor writes neither output vector.
template<class Frames,class Queries,class Exclusions,class Descriptors,class Bounds>
bool car_seat_exit_pose(Frames& frames,Queries& queries,Exclusions& exclusions,const Descriptors& descriptors,
        typename Frames::Frame marker,typename Frames::Frame body,const Bounds& bounds,std::uint32_t flags,
        typename Frames::Vector& position,typename Frames::Vector& direction){
    using Vector=typename Frames::Vector;
    const auto seat=frames.actor_world_matrix(marker);const Vector seat_position{{seat[12],seat[13],seat[14],seat[15]}};
    const auto world=frames.actor_world_matrix(body);const auto inverse=frames.car_inverse_matrix(world);
    auto local=frames.car_transform(seat_position,inverse);const bool left=local[0]<0;
    const auto next=frames.actor_world_matrix(body);Vector right{{next[0],0,next[2],next[3]}};
    const float planar=static_cast<float>(static_cast<long double>(right[0])*right[0]+static_cast<long double>(right[2])*right[2]);
    normalize_human_ai_shot(right,planar);
    const float distance=static_cast<float>((static_cast<long double>(bounds.maximum[0])-bounds.minimum[0])*.75f+((flags&0x200u)?0.f:.5f));
    const float square=static_cast<float>((static_cast<long double>(right[2])*right[2]+static_cast<long double>(right[0])*right[0])+static_cast<long double>(right[1])*right[1]);
    normalize_human_ai_shot(right,square);
    const float signed_distance=left?-distance:distance;
    Vector lateral=right;for(unsigned axis=0;axis!=3;++axis)lateral[axis]=static_cast<float>(static_cast<long double>(right[axis])*signed_distance);
    local[0]=0;if(flags&0x200u)local[2]=bounds.minimum[2]-1.f;
    const auto centre=frames.car_transform(local,frames.actor_world_matrix(body));
    exclusions.many(descriptors.data(),descriptors.size(),nullptr);
    struct Clear {Exclusions& owner;bool pending=true;~Clear(){if(pending)owner.clear();}}clear{exclusions};
    Vector start=centre;start[1]+=1.f;
    if(queries.sweep(start,lateral,.25f,0,0x102u))return false;
    for(unsigned axis=0;axis!=3;++axis)start[axis]=static_cast<float>(static_cast<long double>(centre[axis])+lateral[axis]);
    start[1]+=1.5f;
    const auto floor=queries.sweep(start,Vector{{0,-3.5f,0,0}},.25f,0,0x102u);
    if(!floor)return false;
    auto placed=floor->position;placed[1]-=.20000000298023224f;
    exclusions.clear();clear.pending=false;
    const auto current=frames.actor_world_matrix(marker);auto facing=placed;
    for(unsigned axis=0;axis!=3;++axis)facing[axis]=current[12+axis]-placed[axis];
    facing[1]=0;
    const float length=static_cast<float>(static_cast<long double>(facing[0])*facing[0]+static_cast<long double>(facing[2])*facing[2]);
    normalize_human_ai_shot(facing,length);position=placed;direction=facing;return true;
}
} //namespace hd2::game
#endif
