#ifndef HD2_GAME_SHOT_HUMAN_HIT_ZONES_H
#define HD2_GAME_SHOT_HUMAN_HIT_ZONES_H
#include "game/human_joint_owners.h"

namespace hd2::game {
//00455670, SAME process0089A940 model/frames/bone map. Update and pick one
//zone at a time; an early hit must leave subsequent borrowed zone frames alone.
template<class State,class Human,class Items,class Frames>
unsigned shot_human_hit_zone(State& state,Human& human,const HumanVector& start,
        const HumanVector& displacement,Items& items,Frames& frames){
    if(!state.hit_zones||!state.hit_zones->get())return 0xff;
    const auto visual=frames.actor_find(human.base().frame(),"a",1);
    const auto* bones=frames.shot_hit_bone_table(visual);if(!bones)return 0xff;
    for(unsigned zone=0;zone!=10;++zone){
        const auto frame=state.hit_frames[zone];
        frames.shot_hit_zone_pose(frame,frames.actor_world_matrix(bones[state.hit_bones[zone]]));
        if(zone==1){
            auto* storage=human.base().storage();auto* head=storage?storage->holder(0):nullptr;
            if(!head)throw std::logic_error("Shot hit zones require actual Human head holder");
            if(const auto entry=head->entry();entry&&items.runtime_item(entry).primary_payload_command()==0)
                if(frames.car_damage_pick(start,displacement,items.runtime_item(entry).model(),0x04040000u))return 11;
        }
        if(frames.car_damage_pick(start,displacement,frame,0x08040000u))return zone;
    }
    return 0xff;
}
}
#endif
