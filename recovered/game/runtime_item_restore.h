#ifndef HD2_GAME_RUNTIME_ITEM_RESTORE_H
#define HD2_GAME_RUNTIME_ITEM_RESTORE_H
#include "game/runtime_actor_packets.h"
namespace hd2::game {
//005B7EC0: SAME retained placement list, first matching Item90, fresh accesses
//after packet/engine/body callbacks. No copied placement or second item owner.
template<class Item,class Placements,class Packets,class Frames>
void restore_retained_runtime_item(Item& actor,Placements& placements,Packets& packets,Frames& frames,std::uint32_t mode){
    if((mode&7u)==7u)return;
    std::size_t index=0;
    while(index<placements.size()&&placements[index].entry!=actor.entry())++index;
    if(index==placements.size())return;
    auto& packet=begin_runtime_actor_message(packets,actor.base().id10(),true);
    packet.write(1,3); //Native log2(DOUBLE4)/log2(DOUBLE2)+DOUBLE1 -> __ftol=3.
    frames.restore_retained_item_pose(actor.base().frame(),placements.at(index).position,placements.at(index).rotation);
    actor.restore_retained_body(); //005B7DA0; prior pose/update is already published on failure.
    const auto axis_angle=frames.retained_item_axis_angle(placements.at(index).rotation);
    (void)packet.write_vector(placements.at(index).position,packet.primary_quantizer(-1),7);
    packet.write_axis_angle(axis_angle.first,axis_angle.second,-1);
    packet.finish_broadcast(); //00478880 AFTER model/body and all payload writes.
}
}
#endif
