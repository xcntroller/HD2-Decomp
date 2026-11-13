#ifndef HD2_GAME_HUMAN_INVENTORY_SWAP_H
#define HD2_GAME_HUMAN_INVENTORY_SWAP_H
#include "game/human_inventory_transfer.h"
#include "game/human_state_frame_timers.h"
namespace hd2::game {
//007DA5E0 checks places for the DISPLACED definition on the source Human.
//No epsilon, ordered comparisons; each native FLOAT spill is retained. Arrays
//may pass while full when the incoming entry is leaving that same array.
template<class Human,class Items,class World,class Entry>
bool human_inventory_swap_blocked(Human* source,std::int32_t source_slot,Human* destination,
        std::int32_t destination_slot,Entry incoming,Items& items,World& world){
    if(destination_slot==1||destination_slot==6)return false;
    if(!destination)throw std::logic_error("Swap holder preflight requires actual destination Human");
    auto* destination_storage=destination->base().storage();
    auto* target=destination_storage?destination_storage->holder(destination_slot):nullptr;
    if(!target)throw std::logic_error("Swap preflight lost destination holder");
    const auto previous=target->entry();if(!previous)return false;
    auto& old=items.runtime_item(previous);const auto allowed=old.definition().permitted_slots();
    if(world.mode_flags()&1u){
        if(target->cooling_down(world.game_word28()))return true;
        if((world.mode_flags()&1u)&&world.game_word28()<old.field14)return true;
    }
    auto* storage=source?source->base().storage():nullptr;
    const auto fits=[&](unsigned slot,bool pack){
        if(!source||!storage||storage->single())throw std::logic_error("Selected displaced-item return requires actual source Human storage");
        float amount=old.scalar(),volume=static_cast<float>(old.volume());
        if(pack&&old.pack_exposes_contents())amount=static_cast<float>(static_cast<long double>(human_inventory_detail::pack_weight(old))+amount);
        float delta=0;
        if(source!=destination){
            delta=static_cast<float>(static_cast<long double>(amount)-(incoming?static_cast<long double>(items.runtime_item(incoming).scalar()):0.L)); //0080F51C=0.
            if(pack&&incoming&&items.runtime_item(incoming).pack_exposes_contents())delta=static_cast<float>(static_cast<long double>(delta)-human_inventory_detail::pack_weight(items.runtime_item(incoming)));
        }
        const bool freeing=incoming&&source_slot==static_cast<std::int32_t>(slot);
        if(freeing){
            auto& item=items.runtime_item(incoming);
            amount=static_cast<float>(static_cast<long double>(amount)-item.scalar());
            volume=static_cast<float>(static_cast<long double>(volume)-item.volume());
            if(pack&&item.pack_exposes_contents())amount=static_cast<float>(static_cast<long double>(amount)-human_inventory_detail::pack_weight(item));
        }
        auto* holder=storage->holder(slot);auto* array=storage->slots(slot);if(!holder&&!array)return false;
        if(array){
            if(!(static_cast<long double>(array->total_scalar())+amount<=array->first_scalar()))return false;
            if(!(static_cast<long double>(array->total_volume())+volume<=array->second_scalar()))return false;
        }else if(!(amount<=holder->first_scalar()&&volume<=holder->second_scalar()))return false;
        const auto kind=old.definition().kind();
        const bool skip=array&&(kind==13||(slot==6&&(kind==10||kind==11||old.definition().item_class==0)));
        if(!skip&&old.storage_size()>frontend::pre_mission_detail::Bytes::integer(holder?holder->flags():array->flags()))return false;
        //007DA79C..A8 keeps the integer capacity and weight+FLOAT delta
        //in x87 registers, without a final FLOAT spill or epsilon.
        const auto total=static_cast<long double>(human_inventory_detail::weight(*source))+delta;
        if(!(total<=static_cast<long double>(world.actor_inventory_capacity(source->base()))))return false;
        return holder?(holder->empty()||freeing):(array->occupied_stacks()<array->slots().size()||freeing);
    };
    if((allowed&0x20u)&&fits(1,false))return false;
    if((allowed&0x40u)&&fits(6,false))return false;
    for(const auto slot:{2u,3u,4u,0u}){
        const auto mask=slot==2?0x10u:slot==3?4u:slot==4?2u:1u;
        if((allowed&mask)&&fits(slot,false))return false;
    }
    return !(allowed&8u)||!fits(5,true);
}

//007D9B40 uses the full no-source transfer kernel for displaced-entry fallback.
//This shares the pickup's candidate order, including payload3's occupied
//shoulder attempt; it is different from007DA3F0's source-move order below.
template<class Frames,class Actor,class Items,class World,class Entry>
std::int32_t place_unheld_inventory_item(Frames& frames,Items& items,World& world,Entry entry,
        Actor* destination,std::int32_t& slot,bool notify){
    slot=7;if(!destination)return -100;
    auto* storage=destination->base().storage();if(!storage||storage->single())return -101;
    if(!entry)return -666;
    auto& item=items.runtime_item(entry);const auto payload=item.primary_payload_kind(),size=item.storage_size();
    const auto empty=[&](unsigned n){auto* holder=storage->holder(n);if(!holder)throw std::logic_error("Automatic placement lost Human holder");return holder->empty();};
    const auto attempt=[&](std::int32_t target){Entry displaced{};const auto result=transfer_human_inventory(frames,items,world,entry,displaced,static_cast<Actor*>(nullptr),0,destination,target,-1,static_cast<Actor*>(nullptr),notify,false,false,false);if(result>0)slot=target;return result;};
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

//007D9DE0. Notifications deliberately follow BOTH storage moves. They borrow
//the same actual230 peers; no private equipment state or synthetic drop actor.
template<class Frames,class Actor,class Items,class World,class Entry>
std::int32_t swap_human_inventory(Frames& frames,Items& items,World& world,Entry incoming,Actor* source,
        std::int32_t source_slot,Actor* destination,std::int32_t destination_slot,Entry& displaced,
        std::int32_t& placed_slot,std::int32_t& returned_slot){
    displaced=Entry{};placed_slot=destination_slot;returned_slot=source_slot;
    if(!source||!destination)throw std::logic_error("Swap requires its actual source and destination actors");
    auto* from=source->base().type()==3?world.inventory_human(source):world.inventory_human(static_cast<Actor*>(nullptr));
    auto* to=destination->base().type()==3?world.inventory_human(destination):world.inventory_human(static_cast<Actor*>(nullptr));
    if(!incoming)return -20;
    const auto now=world.game_word28();if(now<items.runtime_item(incoming).field14&&(world.mode_flags()&1u))return -20;
    if(human_inventory_swap_blocked(from,source_slot,to,destination_slot,incoming,items,world))return -21;
    Entry previous{};
    auto result=transfer_human_inventory(frames,items,world,incoming,previous,source,source_slot,destination,destination_slot,-1,static_cast<Actor*>(nullptr),false,false,false,false);
    if(result<1)return result;
    const auto equipment=[&](auto* human){auto* value=world.inventory_equipment(human);if(human&&!value)throw std::logic_error("Swap notification requires actual230");return value;};
    if(!previous){
        if(from){equipment(from)->removed(incoming,source_slot,equipment(to),destination_slot);if(source_slot==3)world.inventory_fpv(from,9,0);}
        if(to){equipment(to)->moved(incoming,destination_slot,equipment(from),source_slot,false);if(destination_slot==3)world.inventory_fpv(to,8,0);}
        return 1;
    }
    displaced=previous;Entry ignored{};auto return_slot=source_slot;
    result=transfer_human_inventory(frames,items,world,previous,ignored,destination,destination_slot,source,source_slot,-1,static_cast<Actor*>(nullptr),false,true,true,false);
    bool abandoned=false;
    if(result<1){
        result=place_unheld_inventory_item(frames,items,world,previous,source,return_slot,false);returned_slot=return_slot;
        if(result<1){
            (void)transfer_human_inventory(frames,items,world,previous,ignored,static_cast<Actor*>(nullptr),6,static_cast<Actor*>(nullptr),0,-1,source,false,false,false,false);
            returned_slot=7;if(!from)throw std::logic_error("Swap drop notification lost source Human");
            equipment(from)->removed(incoming,source_slot,nullptr,destination_slot);abandoned=true;
        }
    }
    if(abandoned||!from){
        if(to){equipment(to)->removed(previous,destination_slot,nullptr,source_slot);equipment(to)->moved(incoming,destination_slot,nullptr,source_slot,false);if(destination_slot==3)world.inventory_fpv(to,8,0);}
        return 1;
    }
    equipment(from)->hide_inventory_holster(incoming,source_slot);
    if(source_slot==4&&(world.mode_flags()&1u))world.inventory_clothing_command(*from,items.runtime_item(previous).identity);
    //Native requires a destination Human on this occupied-holder path.
    if(!to)throw std::logic_error("Occupied swap requires actual destination Human equipment");
    equipment(to)->hide_inventory_holster(previous,destination_slot);
    if(destination_slot==4&&(world.mode_flags()&1u))world.inventory_clothing_command(*to,items.runtime_item(incoming).identity);
    if(source_slot==3&&destination_slot==3){
        equipment(from)->event(1);equipment(to)->event(1);
        if(world.mode_flags()&2u){if(world.inventory_is_current(from)||world.inventory_is_current(to))world.inventory_fpv_unchecked(9,8);}
        equipment(from)->moved(previous,return_slot,equipment(to),destination_slot,human_frame_signed_word(to->properties.stats_ec[0])<1);
        equipment(to)->moved(incoming,destination_slot,equipment(from),source_slot,human_frame_signed_word(from->properties.stats_ec[0])<1);return 1;
    }
    if(source_slot!=3&&destination_slot!=3){
        equipment(from)->moved(previous,return_slot,equipment(to),destination_slot,human_frame_signed_word(to->properties.stats_ec[0])<1);
        equipment(to)->moved(incoming,destination_slot,equipment(from),source_slot,human_frame_signed_word(from->properties.stats_ec[0])<1);return 1;
    }
    if(world.mode_flags()&2u){
        if(source_slot==3)world.inventory_fpv(from,9,return_slot==3?8u:0u);
        else if(return_slot==3)world.inventory_fpv(from,8,0);
        if(destination_slot==3){
            if(world.inventory_is_current(to)){
                auto* hand=to->base().storage()->holder(3);if(!hand)throw std::logic_error("Swap FPV lost destination hand");
                if(hand->empty())world.inventory_fpv_unchecked(8,0);else world.inventory_fpv_unchecked(9,8);
            }
        }
    }
    if(source_slot==3)equipment(from)->moved(previous,return_slot,equipment(to),destination_slot,true);
    else{
        if(source_slot!=return_slot&&return_slot!=0&&return_slot!=2&&return_slot!=5&&source_slot!=0&&source_slot!=2&&source_slot!=5)
            equipment(to)->removed(previous,destination_slot,nullptr,return_slot);
        equipment(from)->moved(previous,return_slot,equipment(to),destination_slot,false);
        if(source_slot==1)equipment(to)->refresh_inventory_holster_animation();
    }
    if(destination_slot!=3)equipment(to)->moved(incoming,destination_slot,equipment(from),source_slot,false);
    else{
        auto actual_source=source_slot;
        if(source_slot!=return_slot){
            if(return_slot==0||return_slot==2||return_slot==5)actual_source=return_slot;
            else if(return_slot==1||return_slot==6){equipment(to)->event(1);equipment(to)->event(2);}
        }
        equipment(to)->moved(incoming,3,equipment(from),actual_source,true);
    }
    return 1;
}

//007DA3F0, automatic SOURCE move. Pack is terminal; payload2/3 only may
//choose hand/shoulder, and shoulder must be empty even for payload3.
template<class Frames,class Actor,class Items,class World,class Entry>
std::int32_t move_human_inventory_automatically(Frames& frames,Items& items,World& world,Entry entry,
        Actor* source,std::int32_t source_slot,Actor* destination,std::int32_t& slot){
    slot=7;if(!source||!destination)return -100;
    if(!entry)throw std::logic_error("Automatic source move requires actual incoming entry");
    auto* storage=destination->base().storage();if(!storage||storage->single())throw std::logic_error("Automatic source move requires actual Human target");
    auto& item=items.runtime_item(entry);const auto size=item.storage_size(),payload=item.primary_payload_kind();
    const auto empty=[&](unsigned n){auto* holder=storage->holder(n);if(!holder)throw std::logic_error("Automatic source move lost Human holder");return holder->empty();};
    const auto attempt=[&](std::int32_t target){Entry displaced{};std::int32_t return_slot=7;return swap_human_inventory(frames,items,world,entry,source,source_slot,destination,target,displaced,slot,return_slot);};
    if(payload==7){if(!empty(5))return -102;return attempt(5);}
    if(payload==8&&empty(0))if(const auto result=attempt(0);result>0)return result;
    if(payload==2||payload==3){
        if(empty(3))if(const auto result=attempt(3);result>0)return result;
        if((size==2||size==3)&&empty(2))if(const auto result=attempt(2);result>0)return result;
    }
    if(const auto result=attempt(1);result>0)return result;
    if(const auto result=attempt(6);result>0)return result;
    return -102;
}
} //namespace hd2::game
#endif
