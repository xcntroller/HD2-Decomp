#ifndef HD2_GAME_DYNAMIC_ITEM_CONTACT_SOUND_H
#define HD2_GAME_DYNAMIC_ITEM_CONTACT_SOUND_H

#include "frontend/pre_mission_data.h"
#include "frontend/spatial_sound.h"

namespace hd2::game {

//005B7570, borrowing actual actor6A4, native Items entry90/definition38 and
//world frame20. Failed SOUND emission must not publish the cooldown write.
template<class Frames,class Items,class Sound>
void emit_dynamic_item_contact(Frames& frames,Items& items,typename Frames::Entry entry,
                              typename Frames::Frame frame,std::int32_t& cooldown,Sound& sound) {
    if(!entry||cooldown>=50)return;
    const auto identity=frontend::pre_mission_detail::Bytes::integer(items.runtime_item(entry).definition().fields38[0]);
    if(identity<=0)return;
    frontend::SpatialSoundRequest request;request.flags=1;request.group=5;request.entry=identity;
    const auto world=frames.actor_world_matrix(frame);
    request.position={{world[12],world[13],world[14],world[15]}};
    (void)sound.emit(request);cooldown=2000;
}
} //namespace hd2::game
#endif
