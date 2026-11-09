#ifndef HD2_GAME_HUMAN_WEAPON_TRANSFER_H
#define HD2_GAME_HUMAN_WEAPON_TRANSFER_H

#include "frontend/pre_mission_data.h"
#include <cstring>
#include <stdexcept>

namespace hd2::game {

//007D8B40 weapon paths needed by the SAME-Human00538770/007D9DE0 exchange:
//source1/2/3/6 or no source, destination1/2/3/6. Pack/header/armour/world-drop
//transfers are separate. Retain controls weight accounting, NOT source detach.
//All runtime handles remain owned by actual holder/stack entries.
template<class Frames,class Human,class Items,class Equipment,class Receivers,class Entry>
std::int32_t transfer_human_weapon(Frames& frames,Human& human,Items& items,Equipment& equipment,Receivers& receivers,
        Entry entry,Entry& displaced,bool has_source,std::int32_t source_slot,std::int32_t destination_slot,std::int32_t index,
        bool notify,bool override_cooldown,bool retain,bool dry_run,std::uint32_t mode,std::uint32_t game28) {
    displaced=Entry{};if(!entry)return -1;
    if(source_slot>6)return -3;
    if(destination_slot>6)return -2;
    const auto supported=[](std::int32_t slot){return slot==1||slot==2||slot==3||slot==6;};
    if((has_source&&!supported(source_slot))||!supported(destination_slot))
        throw std::runtime_error("Weapon transfer requires its actual non-weapon/world-drop path");
    auto& item=items.runtime_item(entry);const auto model=item.model();
    if(static_cast<std::uint32_t>(item.family())!=0)throw std::runtime_error("Weapon exchange received a different runtime family");
    auto* storage=human.base().storage();if(!storage||storage->single())throw std::runtime_error("Weapon transfer requires actual Human storage");
    auto* holder=storage->holder(static_cast<std::size_t>(destination_slot));
    auto* stacks=storage->slots(static_cast<std::size_t>(destination_slot));
    if(!holder&&!stacks)return -2;
    auto* source_holder=has_source?storage->holder(static_cast<std::size_t>(source_slot)):nullptr;
    auto* source_stacks=has_source?storage->slots(static_cast<std::size_t>(source_slot)):nullptr;
    if(has_source&&!source_holder&&!source_stacks)throw std::runtime_error("Weapon transfer lost actual source storage");
    constexpr float epsilon=.0000999999974737875163555145263671875f; //0080F53C.
    const auto weight=[&]{return frontend::pre_mission_detail::Bytes::real(human.base().field7c());};
    const auto store_weight=[&](float value){std::uint32_t bits;std::memcpy(&bits,&value,4);human.base().field7c(bits);};
    if(!has_source) {
        const auto previous=holder?holder->entry():Entry{};
        const auto capacity=static_cast<float>(static_cast<long double>(receivers.actor_inventory_capacity(human))+epsilon);
        if(capacity<(static_cast<long double>(item.scalar())+weight())-(previous?items.runtime_item(previous).scalar():0.f))return -4;
    }
    const auto first=holder?holder->first_scalar():stacks->first_scalar();
    const auto second=holder?holder->second_scalar():stacks->second_scalar();
    const auto flags=holder?holder->flags():stacks->flags();
    if((destination_slot==6?static_cast<float>(static_cast<long double>(first)+epsilon):first)<
            static_cast<long double>(item.scalar())+(stacks?stacks->total_scalar():0.f))return -5;
    if(second<item.volume()+(stacks?stacks->total_volume():0.f))return -6;
    const auto kind=item.definition().kind();
    const bool skip_size=stacks&&(kind==13||(destination_slot==6&&(item.definition().item_class==0||kind==10||kind==11)));
    if(!skip_size&&frontend::pre_mission_detail::Bytes::integer(flags)<item.storage_size())return -7;
    if(destination_slot==2) {
        const auto payload=item.primary_payload_kind(),size=item.storage_size();
        if((payload!=2&&payload!=3)||(size!=2&&size!=3))return -8;
    }
    if(!has_source&&holder&&!holder->empty())return -6;
    if(destination_slot==3&&(mode&1u)&&holder->entry()&&game28<items.runtime_item(holder->entry()).field14)return -11;
    const bool enabled=items.equipment_notifications_enabled();
    if(enabled&&(mode&1u)&&!override_cooldown) {
        if(holder&&holder->cooling_down(game28))return -10;
        if(source_holder&&source_holder->cooling_down(game28))return -9;
    }
    if(has_source&&source_slot==destination_slot&&index<0)return -12;
    if(dry_run)return 1;
    const auto result=holder?holder->replace(entry,displaced):stacks->insert(entry);if(result<1)return result;
    store_weight(static_cast<float>(static_cast<long double>(item.scalar())+weight()));
    if(has_source&&!retain)store_weight(static_cast<float>(static_cast<long double>(weight())-item.scalar()));
    if(displaced&&!retain)store_weight(static_cast<float>(static_cast<long double>(weight())-items.runtime_item(displaced).scalar()));
    item.holder_actor=&human.base();item.holder_slot=destination_slot;
    if(source_holder)(void)source_holder->detach(entry);else if(source_stacks)(void)source_stacks->detach(entry);
    if(item.flag1c&&model){item.flag1c=false;frames.object_visible(model,false);}
    if(enabled) {
        if(has_source&&notify)equipment.removed(entry,source_slot,&equipment,destination_slot);
        if(notify){if(has_source)equipment.moved(entry,destination_slot,&equipment,source_slot,false);else equipment.added(entry,destination_slot);}
        if((mode&2u)&&receivers.is_current_player(human))receivers.hud_inventory_changed();
        human.activation.flag178=true;
        if(has_source&&source_slot==3) {
            if(source_holder)source_holder->field18(game28+0x398u);
            if(destination_slot==2)holder->field18(game28+0x398u);
            receivers.refresh_item();
            if((mode&2u)&&receivers.is_current_player(human)&&notify)receivers.first_person_request(mode,0x14,0);
        }
        if(destination_slot==3) {
            holder->field18(game28+0x398u);receivers.refresh_item();
            if((mode&2u)&&receivers.is_current_player(human)&&!(equipment.transfer_state().words().template get<std::uint32_t>(0x50)&1u)&&notify)
                receivers.first_person_request(mode,8,0);
        }
    }
    item.field14=game28+0x398u;return result;
}
} //namespace hd2::game
#endif
