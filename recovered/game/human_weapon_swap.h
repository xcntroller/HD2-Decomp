#ifndef HD2_GAME_HUMAN_WEAPON_SWAP_H
#define HD2_GAME_HUMAN_WEAPON_SWAP_H

#include "game/human_weapon_transfer.h"

namespace hd2::game {

//007DA5E0's SAME-Human weapon exchange preflight. It examines the DISPLACED
//definition's allowed slots, not the incoming weapon's. Destination arrays1/6
//return false immediately. Whole actor capacity comparisons have NO epsilon.
//Other runtime families/cross-Human ownership remain separate source paths.
template<class Human,class Items,class Limits,class Entry>
bool human_weapon_swap_blocked(Human& human,Items& items,Limits& limits,Entry incoming,
        std::int32_t source_slot,std::int32_t destination_slot,std::uint32_t mode,std::uint32_t game28) {
    if(destination_slot==1||destination_slot==6)return false;
    auto* storage=human.base().storage();if(!storage||storage->single())throw std::runtime_error("Weapon swap requires actual Human storage");
    auto* destination=storage->holder(static_cast<std::size_t>(destination_slot));
    if(!destination)throw std::runtime_error("Weapon swap requires its actual destination holder");
    const auto displaced=destination->entry();if(!displaced)return false;
    auto& item=items.runtime_item(displaced);
    const auto allowed=item.definition().permitted_slots();
    if((mode&1u)&&(destination->cooling_down(game28)||game28<item.field14))return true;
    if(static_cast<std::uint32_t>(item.family())!=0)throw std::runtime_error("Weapon swap displaced a different runtime family");
    const auto kind=item.definition().kind();
    const auto weight=frontend::pre_mission_detail::Bytes::real(human.base().field7c());
    const auto capacity=static_cast<float>(limits.actor_inventory_capacity(human));
    //Original first stores amount/volume and source-slot subtraction to FLOAT.
    //Same Human delta is zero, but NaN actor weights still fail ordered <=.
    const auto fits_array=[&](unsigned slot) {
        auto* array=storage->slots(slot);if(!array)return false;
        float amount=item.scalar(),volume=static_cast<float>(item.volume());
        if(incoming&&source_slot==static_cast<std::int32_t>(slot)) {
            amount=static_cast<float>(static_cast<long double>(amount)-items.runtime_item(incoming).scalar());
            volume=static_cast<float>(static_cast<long double>(volume)-items.runtime_item(incoming).volume());
        }
        if(!(static_cast<long double>(array->total_scalar())+amount<=array->first_scalar()))return false;
        if(!(static_cast<long double>(array->total_volume())+volume<=array->second_scalar()))return false;
        if(kind!=13&&!(slot==6&&(kind==10||kind==11||item.definition().item_class==0))&&
                item.storage_size()>frontend::pre_mission_detail::Bytes::integer(array->flags()))return false;
        if(!(static_cast<long double>(weight)<=capacity))return false;
        //A source in the SAME array passes even if no whole stack is freed.
        //Keep this native quirk: the later actual insertion may still fail.
        return array->occupied_stacks()<array->slots().size()||(incoming&&source_slot==static_cast<std::int32_t>(slot));
    };
    if((allowed&0x20u)&&fits_array(1))return false;
    if((allowed&0x40u)&&fits_array(6))return false;
    for(const auto slot:{2u,3u}) {
        const auto mask=slot==2?0x10u:4u;if(!(allowed&mask))continue;
        auto* holder=storage->holder(slot);if(!holder)throw std::runtime_error("Weapon swap lost a required holder");
        float amount=item.scalar(),volume=static_cast<float>(item.volume());
        if(incoming&&source_slot==static_cast<std::int32_t>(slot)) {
            amount=static_cast<float>(static_cast<long double>(amount)-items.runtime_item(incoming).scalar());
            volume=static_cast<float>(static_cast<long double>(volume)-items.runtime_item(incoming).volume());
        }
        if(amount<=holder->first_scalar()&&volume<=holder->second_scalar()&&
                item.storage_size()<=frontend::pre_mission_detail::Bytes::integer(holder->flags())&&
                static_cast<long double>(weight)<=capacity&&
                (holder->empty()||(incoming&&source_slot==static_cast<std::int32_t>(slot))))return false;
    }
    if(allowed&0xbu)throw std::runtime_error("Weapon swap requires the actual head/armour/pack preflight path");
    return true;
}

//007D9B40's actual weapon fallback: hand3, eligible shoulder2, holsters1,
//then optional pack contents6. Calls are NO-source, notify0, no overrides.
template<class Frames,class Human,class Items,class Equipment,class Receivers,class Entry>
std::int32_t place_displaced_human_weapon(Frames& frames,Human& human,Items& items,Equipment& equipment,Receivers& receivers,
        Entry entry,std::int32_t& slot,std::uint32_t mode,std::uint32_t game28) {
    slot=7;if(!entry)return -0x29a;
    auto* storage=human.base().storage();if(!storage||storage->single())return -0x65;
    auto& item=items.runtime_item(entry);const auto payload=item.primary_payload_kind(),size=item.storage_size();
    const auto attempt=[&](std::int32_t candidate) {
        Entry displaced{};
        const auto result=transfer_human_weapon(frames,human,items,equipment,receivers,entry,displaced,false,0,candidate,-1,false,false,false,false,mode,game28);
        if(result>0)slot=candidate;
        return result;
    };
    if(payload==7||payload==8)throw std::runtime_error("Displaced weapon requires actual pack/head fallback");
    if(payload==3||payload==2||payload==0||payload==1) {
        if(storage->holder(3)->empty())if(const auto result=attempt(3);result>0)return result;
        //Native kind3 enters this attempt even when shoulder is occupied.
        if(payload==3||(payload==2&&(size==2||size==3)&&storage->holder(2)->empty()))
            if(const auto result=attempt(2);result>0)return result;
    }
    if(const auto result=attempt(1);result>0)return result;
    if(const auto result=attempt(6);result>0)return result;
    return -0x66;
}

//007D9DE0 same-Human weapon source1/2/6→hand3, including the displaced
//entry's failed return/fallback and native no-source/no-destination drop call.
//Every entry remains owned by actual Items even if no holder accepts it.
template<class Frames,class Human,class Items,class Equipment,class Receivers,class Entry>
std::int32_t swap_human_loading_weapon(Frames& frames,Human& human,Items& items,Equipment& equipment,Receivers& receivers,
        Entry incoming,std::int32_t source_slot,Entry& displaced,std::int32_t& displaced_slot,std::uint32_t mode,std::uint32_t game28) {
    displaced=Entry{};displaced_slot=source_slot;
    if(source_slot!=1&&source_slot!=2&&source_slot!=6)throw std::runtime_error("Loading weapon swap requires source1/2/6");
    if(!incoming||((mode&1u)&&game28<items.runtime_item(incoming).field14))return -20;
    if(human_weapon_swap_blocked(human,items,receivers,incoming,source_slot,3,mode,game28))return -21;
    Entry previous{};
    auto result=transfer_human_weapon(frames,human,items,equipment,receivers,incoming,previous,true,source_slot,3,-1,false,false,false,false,mode,game28);
    if(result<1)return result;
    if(!previous) {
        equipment.removed(incoming,source_slot,&equipment,3);
        equipment.moved(incoming,3,&equipment,source_slot,false);
        if((mode&2u)&&receivers.is_current_player(human))receivers.first_person_request(mode,8,0);
        return 1;
    }
    displaced=previous;Entry ignored{};
    result=transfer_human_weapon(frames,human,items,equipment,receivers,previous,ignored,true,3,source_slot,-1,false,true,true,false,mode,game28);
    if(result<1) {
        result=place_displaced_human_weapon(frames,human,items,equipment,receivers,previous,displaced_slot,mode,game28);
        if(result<1) {
            //007D8B40(previous, ..., source=NULL/slot6, dest=NULL/slot0,
            //context=Human, notify0). No source/destination weight or holder
            //writes occur. Native20/24 borrowers are NOT cleared by that call.
            auto& item=items.runtime_item(previous);const auto model=item.model();
            if(item.flag1c&&model){item.flag1c=false;frames.object_visible(model,false);}
            item.field14=game28+0x398u;displaced_slot=7;
            equipment.removed(incoming,source_slot,nullptr,3);
            equipment.removed(previous,3,nullptr,source_slot);
            equipment.moved(incoming,3,nullptr,source_slot,false);
            if((mode&2u)&&receivers.is_current_player(human))receivers.first_person_request(mode,8,0);
            return 1;
        }
    }
    equipment.hide_inventory_holster(incoming,source_slot);
    equipment.hide_inventory_holster(previous,3);
    if(mode&2u)if(receivers.is_current_player(human)) {
        //Source!=3 here; the destination Human is SAME. Native rereads hand.
        receivers.first_person_request(mode,9,8);
    }
    if(source_slot!=displaced_slot&&displaced_slot!=0&&displaced_slot!=2&&displaced_slot!=5&&source_slot!=2)
        equipment.removed(previous,3,nullptr,displaced_slot);
    equipment.moved(previous,displaced_slot,&equipment,3,false);
    if(source_slot==1)equipment.refresh_inventory_holster_animation(); //00791EB0.
    auto actual_source=source_slot;
    if(source_slot!=displaced_slot) {
        if(displaced_slot==0||displaced_slot==2||displaced_slot==5)actual_source=displaced_slot;
        else if(displaced_slot==1||displaced_slot==6){equipment.event(1);equipment.event(2);}
    }
    equipment.moved(incoming,3,&equipment,actual_source,true);
    return 1;
}
} //namespace hd2::game
#endif
