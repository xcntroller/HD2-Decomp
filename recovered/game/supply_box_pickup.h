#ifndef HD2_GAME_SUPPLY_BOX_PICKUP_H
#define HD2_GAME_SUPPLY_BOX_PICKUP_H
#include "frontend/spatial_sound.h"
#include "game/human_joint_owners.h"
namespace hd2::game {
//005FC860(2)→005FCD80(1), native opposite track/350 crossfade or initial weight1.
//Sound precedes duration and state1 publication; the closed clip is a borrowed
//actual cache3 resource, not a successful missing-animation fallback.
template<class Actor,class Frames,class Sound>void deplete_supply_box(Actor& actor,Frames& frames,typename Frames::Frame closed,Sound&& sound){
    const auto model=actor.base().frame();
    if(actor.field24c!=5u&&actor.field248!=0xffffffffu){
        actor.field248=actor.field248==0u?1u:0u;frames.human_model_animation(model,closed,actor.field248,0.f,1);actor.field24c=2;actor.field244=350;
    }else{
        if(actor.field248==0xffffffffu)actor.field248=0;
        frames.human_model_animation(model,closed,actor.field248,1.f,1);actor.field244=0;actor.field24c=2;
    }
    frontend::SpatialSoundRequest request;const auto world=frames.actor_world_matrix(model);request.position={{world[12],world[13],world[14],world[15]}};
    request.flags=1;request.group=1;request.entry=11;sound(request);
    actor.field254=closed?static_cast<std::uint32_t>(frames.human_clip_duration(closed)):1000u;actor.field250=1;
}
//005A37C0/005A38F0's freshly observed decrement. Native DWORD wraps, then
//SIGNED<1 triggers depletion. Refresh runs even when state/flag suppresses count.
template<class Actor,class Deplete,class Refresh>void consume_supply_box_item(Actor& actor,std::uint32_t flags,Deplete&& deplete,Refresh&& refresh){
    if(!actor.field250&&(flags&1u)){
        --actor.field130;std::int32_t count;std::memcpy(&count,&actor.field130,4);if(count<1)deplete();
    }
    refresh();
}
} //namespace hd2::game
#endif
