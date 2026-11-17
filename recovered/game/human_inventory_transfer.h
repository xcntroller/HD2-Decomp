#ifndef HD2_GAME_HUMAN_INVENTORY_TRANSFER_H
#define HD2_GAME_HUMAN_INVENTORY_TRANSFER_H
#include "frontend/pre_mission_data.h"
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <utility>
namespace hd2::game {
namespace human_inventory_detail {
template<class Actor>float weight(Actor& actor){return frontend::pre_mission_detail::Bytes::real(actor.base().field7c());}
template<class Actor>void weight(Actor& actor,float value){std::uint32_t bits;std::memcpy(&bits,&value,4);actor.base().field7c(bits);}
template<class Item>float pack_weight(Item& item){
    if(!item.slots())throw std::logic_error("Exposed pack lost its actual contents owner");
    return item.slots()->total_scalar();
}
}
//007D8B40. Source/destination are actual Scene2 actors, including single-array
//Car/storage actors. Only Human storage publishes item20/24 and230 notifications.
//Insertion precedes weight/publication/detachment; retain does NOT skip detach.
//Fresh clocks/mode/Players observations stay at their native callback boundaries.
template<class Frames,class Actor,class Items,class World,class Entry>
std::int32_t transfer_human_inventory(Frames& frames,Items& items,World& world,Entry entry,Entry& displaced,
        Actor* source,std::int32_t source_slot,Actor* destination,std::int32_t destination_slot,std::int32_t index,
        Actor* context,bool notify,bool override_cooldown,bool retain,bool dry_run){
    using Storage=std::remove_pointer_t<decltype(std::declval<Actor&>().base().storage())>;
    using Holder=typename Storage::Holder;using Slots=typename Storage::Slots;
    displaced=Entry{};if(!entry)return -1;
    if(source_slot>6||source_slot<0)return -3;
    if(destination_slot>6||destination_slot<0)return -2;
    if(source_slot==4&&destination_slot!=4)return -2;
    if(!context&&source)context=source;
    auto& item=items.runtime_item(entry);const auto model=item.model(); //007D3AA0(0), before target validation.
    Holder* from_holder=nullptr;Slots* from_array=nullptr;
    auto* source_human=world.inventory_human(static_cast<Actor*>(nullptr));
    if(source){
        auto* storage=source->base().storage();if(!storage)throw std::logic_error("Inventory source lost actual storage");
        if(storage->single())from_array=storage->slots(0);
        else {source_human=world.inventory_human(source);from_holder=storage->holder(source_slot);from_array=storage->slots(source_slot);}
        if(!from_holder&&!from_array)throw std::logic_error("Inventory source lost selected storage receiver");
    }
    Holder* holder=nullptr;Slots* array=nullptr;Storage* target_storage=nullptr;
    auto* destination_human=world.inventory_human(static_cast<Actor*>(nullptr));
    bool drop=false,expose=false;
    constexpr float epsilon=.0000999999974737875163555145263671875f; //0080F53C.
    if(!destination){
        if(destination_slot!=0)return -2;
        drop=true;expose=item.pack_exposes_contents();
    }else{
        target_storage=destination->base().storage();if(!target_storage)throw std::logic_error("Inventory destination lost actual storage");
        if(target_storage->single()){
            if(destination_slot!=0)return -2;
            array=target_storage->slots(0);
        }else{
            holder=target_storage->holder(destination_slot);array=target_storage->slots(destination_slot);
        }
        if(destination_slot==5&&item.primary_payload_kind()!=7)return -8; //Before reading the previous pack.
        if(!holder&&!array&&destination_slot!=6)return -2;
        const auto previous=holder?holder->entry():Entry{};
        float incoming_weight=item.scalar(),previous_weight=previous?items.runtime_item(previous).scalar():0.f;
        if(destination_slot==5){
            expose=item.pack_exposes_contents();
            if(previous&&items.runtime_item(previous).pack_exposes_contents())previous_weight=static_cast<float>(static_cast<long double>(human_inventory_detail::pack_weight(items.runtime_item(previous)))+previous_weight);
            if(expose)incoming_weight=static_cast<float>(static_cast<long double>(human_inventory_detail::pack_weight(item))+incoming_weight);
        }
        if(destination!=source){
            const auto limit=static_cast<long double>(world.actor_inventory_capacity(destination->base()))+epsilon;
            //Pack5 keeps capacity and the final sum/subtraction in x87
            //registers (007D94D9..EF); other slots spill capacity to FLOAT.
            const auto capacity=destination_slot==5?limit:static_cast<long double>(static_cast<float>(limit));
            const auto old=human_inventory_detail::weight(*destination);
            const auto total=destination_slot==5?(static_cast<long double>(old)+incoming_weight)-previous_weight:
                (static_cast<long double>(item.scalar())+old)-previous_weight;
            if(capacity<total)return -4;
        }
        //Native slot6 checks whole-actor capacity BEFORE its optional
        //pack receiver, so an overweight/no-pack target returns -4, not -2.
        if(!holder&&!array)return -2;
        //Hand family is checked BEFORE its scalar/volume/size limits.
        if(destination_slot==3&&static_cast<std::uint32_t>(item.family())!=0)return -8;
        const auto first=holder?holder->first_scalar():array->first_scalar();
        const auto second=holder?holder->second_scalar():array->second_scalar();
        const auto flags=holder?holder->flags():array->flags();
        const auto amount=array?static_cast<long double>(array->total_scalar())+static_cast<float>(item.scalar()):static_cast<long double>(item.scalar());
        if((destination_slot==5||destination_slot==6?static_cast<float>(static_cast<long double>(first)+epsilon):first)<amount)return -5;
        const auto volume=array?static_cast<long double>(array->total_volume())+static_cast<float>(item.volume()):item.volume();
        if(second<volume)return -6;
        const auto kind=item.definition().kind();
        const bool skip_size=(destination_slot==1&&kind==13)||(destination_slot==6&&(kind==13||kind==10||kind==11||item.definition().item_class==0));
        if(destination_slot!=5&&!skip_size&&frontend::pre_mission_detail::Bytes::integer(flags)<item.storage_size())return -7;
        if(!target_storage->single()){
            const auto payload=item.primary_payload_kind();
            if(destination_slot==0&&payload!=8)return -8;
            if(destination_slot==2&&((payload!=2&&payload!=3)||(item.storage_size()!=2&&item.storage_size()!=3)))return -8;
            if(destination_slot==4&&payload!=5)return -8;
            if(destination_slot==5&&frontend::pre_mission_detail::Bytes::integer(flags)!=item.storage_size())return -8;
            destination_human=world.inventory_human(destination);
            if(!source&&holder&&!holder->empty())return -6;
            if(destination_slot==3&&(world.mode_flags()&1u)&&previous&&world.game_word28()<items.runtime_item(previous).field14)return -11;
        }
    }
    if(items.equipment_notifications_enabled()&&(world.mode_flags()&1u)){
        if(holder&&holder->cooling_down(world.game_word28())&&!override_cooldown)return -10;
        if((world.mode_flags()&1u)&&from_holder&&from_holder->cooling_down(world.game_word28())&&!override_cooldown)return -9;
    }
    if(((holder&&holder==from_holder)||(array&&array==from_array))&&index<0)return -12;
    if(dry_run)return 1;
    const auto result=holder?holder->replace(entry,displaced):array?array->insert(entry):1;if(result<1)return result;
    using human_inventory_detail::weight;
    if(destination)weight(*destination,static_cast<float>(static_cast<long double>(item.scalar())+weight(*destination)));
    if(source&&!retain)weight(*source,static_cast<float>(static_cast<long double>(weight(*source))-item.scalar()));
    if(displaced&&!retain){
        auto& previous=items.runtime_item(displaced);
        weight(*destination,static_cast<float>(static_cast<long double>(weight(*destination))-previous.scalar()));
        if(previous.pack_exposes_contents())weight(*destination,static_cast<float>(static_cast<long double>(weight(*destination))-human_inventory_detail::pack_weight(previous)));
    }
    if(expose){
        if(destination){target_storage->pack_slots(item.slots());weight(*destination,static_cast<float>(static_cast<long double>(human_inventory_detail::pack_weight(item))+weight(*destination)));}
        if(source&&!retain)weight(*source,static_cast<float>(static_cast<long double>(weight(*source))-human_inventory_detail::pack_weight(item)));
    }
    if(destination_human){item.holder_actor=&destination->base();item.holder_slot=destination_slot;}
    if(from_holder)(void)from_holder->detach(entry);else if(from_array)(void)from_array->detach(entry);
    if(item.flag1c&&model){item.flag1c=false;frames.object_visible(model,false);}
    if(expose&&(from_holder||from_array)&&source&&source_slot==5&&!override_cooldown)source->base().storage()->pack_slots(nullptr);
    if(items.equipment_notifications_enabled()){
        if(source_human&&notify)if(auto* equipment=world.inventory_equipment(source_human))equipment->removed(entry,source_slot,world.inventory_equipment(destination_human),destination_slot);
        if(destination_human&&notify)if(auto* equipment=world.inventory_equipment(destination_human)){
            if(!source)equipment->added(entry,destination_slot);
            else equipment->moved(entry,destination_slot,world.inventory_equipment(source_human),source_slot,false);
        }
        if(world.mode_flags()&2u)world.inventory_hud_changed(destination_human);
        if(destination_human)destination_human->activation.flag178=true;
        if(source_human){
            source_human->activation.flag178=true;
            if(source_slot==3){
                if(from_holder)from_holder->field18(world.game_word28()+0x398u);
                if(destination_slot==2)holder->field18(world.game_word28()+0x398u);
                world.created_human_refresh_item(*source_human);
                if(notify)world.inventory_fpv(source_human,20,0);
            }
        }
        if(destination_human&&destination_slot==3){
            holder->field18(world.game_word28()+0x398u);world.created_human_refresh_item(*destination_human);
            if(notify)world.created_human_hand_fpv(*destination_human);
        }
    }
    if(drop&&source_slot==3)world.inventory_drop_fpv(context);
    item.field14=world.game_word28()+0x398u;return result;
}
} //namespace hd2::game
#endif
