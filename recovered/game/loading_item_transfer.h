#ifndef HD2_GAME_LOADING_ITEM_TRANSFER_H
#define HD2_GAME_LOADING_ITEM_TRANSFER_H

#include "game/actor_base.h"
#include "frontend/pre_mission_data.h"
#include <cmath>

namespace hd2::game {

//007D8B40's EXACT007D81A0 loading call: no source actor, source slot0,
//destination actor/slot, index−1, no dry-run/override, module7E4 temporarily
//false. Runtime exchanges, drops, notifications and HUD branches are separate.
//This consumes actual holder/stack owners; unsupported destination storage
//rejects, never an opaque saved inventory. Caller owns displaced, if supplied.
template<class Backend,class Items,class Limits>
std::int32_t transfer_loaded_item(Backend& frames,Items& items,Limits& limits,
                                 typename Backend::Entry entry,typename Backend::Entry& displaced,
                                 ActorBase<Backend>& actor,std::int32_t slot,std::uint32_t game_word28) {
    using Entry=typename Backend::Entry;
    displaced=Entry{};
    if(!entry)return -1;
    if(slot<0||slot>6)return -2;
    auto& item=items.runtime_item(entry);
    const auto model=item.model(); //Original obtains this before destination checks, including null.
    auto* storage=actor.storage();if(!storage)throw std::runtime_error("Loaded item target has no actual storage");
    if(storage->single()&&slot!=0)return -2;
    auto* holder=storage->single()?nullptr:storage->holder(static_cast<std::size_t>(slot));
    auto* stacks=storage->single()?storage->slots(0):storage->slots(static_cast<std::size_t>(slot));
    if(!holder&&!stacks)return -2;
    const auto previous=holder?holder->entry():Entry{};
    auto weight=frontend::pre_mission_detail::Bytes::real(actor.field7c());
    if(!std::isfinite(weight)||!std::isfinite(item.scalar()))throw std::runtime_error("Non-finite actor inventory weight");
    constexpr float epsilon=.0000999999974737875163555145263671875f; //0080F53C.
    const auto capacity=static_cast<float>(static_cast<long double>(limits.actor_inventory_capacity(actor))+epsilon);
    const auto old_weight=previous?items.runtime_item(previous).scalar():0.f;
    const bool pack=slot==5&&item.primary_payload_kind()==7;
    const bool expose=pack&&item.pack_exposes_contents();
    float transfer_weight=item.scalar(),replaced_weight=old_weight;
    if(pack) {
        if(expose) {
            if(!item.slots())throw std::runtime_error("Equipped pack has no actual slot owner");
            transfer_weight=static_cast<float>(static_cast<long double>(item.slots()->total_scalar())+transfer_weight);
        }
        if(previous&&items.runtime_item(previous).pack_exposes_contents()) {
            auto* old=items.runtime_item(previous).slots();
            if(!old)throw std::runtime_error("Existing pack has no actual slot owner");
            replaced_weight=static_cast<float>(static_cast<long double>(old->total_scalar())+replaced_weight);
        }
    }
    const auto total=pack?(static_cast<long double>(static_cast<float>(weight+transfer_weight))-replaced_weight):
        ((static_cast<long double>(item.scalar())+weight)-((slot==1||slot==6)?0.f:old_weight));
    if(capacity<total)return -4;
    const auto first=holder?holder->first_scalar():stacks->first_scalar();
    const auto second=holder?holder->second_scalar():stacks->second_scalar();
    const auto flags=holder?holder->flags():stacks->flags();
    const auto incoming_weight=static_cast<float>(item.scalar());
    const auto current_weight=stacks?stacks->total_scalar():0.f;
    if((slot==5?static_cast<float>(static_cast<long double>(first)+epsilon):first)<
       static_cast<long double>(incoming_weight)+((slot==1||slot==6||storage->single())?current_weight:0.f))return -5;
    const auto incoming_volume=static_cast<float>(item.volume());
    const auto current_volume=stacks?stacks->total_volume():0.f;
    if(second<static_cast<long double>(incoming_volume)+((slot==1||slot==6||storage->single())?current_volume:0.f))return -6;
    if(slot!=5&&!(slot==1&&item.definition().kind()==13)&&
       !(slot==6&&(item.definition().item_class==0||item.definition().kind()==11||item.definition().kind()==10))&&
       frontend::pre_mission_detail::Bytes::integer(flags)<item.storage_size())return -7;
    const auto payload=item.primary_payload_kind();
    if(!storage->single())switch(slot) {
    case 0:if(payload!=8)return -8;break;
    case 1:break;
    case 2:if((payload!=2&&payload!=3)||(item.storage_size()!=2&&item.storage_size()!=3))return -8;break;
    case 3:if(static_cast<std::uint32_t>(item.family())!=0)return -8;break;
    case 4:if(payload!=5)return -8;break;
    case 5:if(payload!=7||frontend::pre_mission_detail::Bytes::integer(flags)!=item.storage_size())return -8;break;
    case 6:break;
    }
    if(holder&&!holder->empty())return -6; //007D81A0 has no source; holder must be empty.
    std::int32_t result;
    if(holder)result=holder->replace(entry,displaced);
    else result=stacks->insert(entry);
    if(result<1)return result;
    weight=static_cast<float>(static_cast<long double>(item.scalar())+weight);
    if(displaced)weight=static_cast<float>(static_cast<long double>(weight)-items.runtime_item(displaced).scalar());
    if(expose) {
        storage->pack_slots(item.slots());
        weight=static_cast<float>(static_cast<long double>(item.slots()->total_scalar())+weight);
    }
    std::uint32_t bits;std::memcpy(&bits,&weight,4);actor.field7c(bits);
    if(!storage->single()){item.holder_actor=&actor;item.holder_slot=slot;}
    if(item.flag1c&&model) {item.flag1c=false;frames.object_visible(model,false);}
    item.field14=slot==3?game_word28+0x398u:0u; //007D81A0 resets all other slots immediately after transfer.
    return result;
}

} //namespace hd2::game
#endif
