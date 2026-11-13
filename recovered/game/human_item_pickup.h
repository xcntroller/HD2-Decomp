#ifndef HD2_GAME_HUMAN_ITEM_PICKUP_H
#define HD2_GAME_HUMAN_ITEM_PICKUP_H
#include "game/prepared_item_transfer.h"
namespace hd2::game {
//007D9B40, no source, automatic Human placement. Pack7 is terminal; head8
//can fall through. Payload3 tries shoulder even occupied; payload2 requires
//size2/3 and empty shoulder. Notify0 still runs native HUD/state/timer effects.
template<class Frames,class Human,class Items,class World>std::int32_t place_unheld_human_item(
        Frames& frames,Human* human,Items& items,World& world,typename Frames::Entry entry,std::int32_t& slot,bool notify){
    slot=7;if(!human)return -100;
    auto* storage=human->base().storage();if(!storage||storage->single())return -101;
    if(!entry)return -666;
    auto& item=items.runtime_item(entry);const auto size=item.storage_size();const auto payload=item.primary_payload_kind();
    const auto empty=[&](unsigned n){auto* holder=storage->holder(n);if(!holder)throw std::logic_error("Human automatic placement lost native holder");return holder->empty();};
    const auto attempt=[&](std::int32_t target){const auto result=transfer_created_human_item(frames,*human,items,world,entry,target,notify);if(result>0)slot=target;return result;};
    if(payload==7){if(!empty(5))return -102;return attempt(5);}
    if(payload==8&&empty(0))if(const auto result=attempt(0);result>0)return result;
    if(payload==3||payload==2||payload==0||payload==1){
        if(empty(3))if(const auto result=attempt(3);result>0)return result;
        if(payload==3||(payload==2&&(size==2||size==3)&&empty(2)))if(const auto result=attempt(2);result>0)return result;
    }
    if(const auto result=attempt(1);result>0)return result;
    if(const auto result=attempt(6);result>0)return result;
    return -102;
}
//005A39D0: actual runtime loose item; successful placement BEFORE EC/item/frame
//handoff005B6980 and ordered runtime retirement00477190, then Human178.
template<class Human,class World>bool validate_human_item_pickup(Human& human,std::int32_t identity,std::int32_t& slot,World& world){
    auto* actor=world.human_loose_item(identity);if(!actor)return false;
    if(world.place_unheld_human_item(human,actor->entry(),slot,true)!=1)return false;
    world.retire_picked_up_item(*actor);human.activation.flag178=true;return true;
}
//005A3A40: mode1 BEFORE lookup. Native ignores transfer's ordinary result and
//retires the loose actor even on rejected placement. Required exceptions remain
//exceptions; do not turn a failed admission into a retained duplicate owner.
template<class Human,class World>void apply_human_item_pickup(Human& human,std::int32_t identity,std::int32_t slot,World& world){
    if(world.mode_flags()&1u)return;
    auto* actor=world.human_loose_item(identity);if(!actor)return;
    (void)world.apply_unheld_human_item(human,actor->entry(),slot);
    world.retire_picked_up_item(*actor);human.activation.flag178=true;
}
} //namespace hd2::game
#endif
