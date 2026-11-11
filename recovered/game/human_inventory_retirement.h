#ifndef HD2_GAME_HUMAN_INVENTORY_RETIREMENT_H
#define HD2_GAME_HUMAN_INVENTORY_RETIREMENT_H

#include "game/human_inventory_lookup.h"
#include "game/human_joint_owners.h"
#include "frontend/pre_mission_data.h"
#include "game/human_equipment_offsets.h"

namespace hd2::game {
//00791EF0. The equipment offsets are the SAME process initializers used by
//attachment placement. Raw00791F46/1FA9/200C passes marker+40 itself,
//so transform local B0 plus offset by the MARKER world matrix (including its
//translation). Do not replace this native arithmetic with a parent transform.
template<class State,class Frames>HumanVector human_retired_item_position(State& state,unsigned kind,Frames& frames){
    if(kind>2){
        const auto marker=state.first_volume();if(!marker)return {};
        const auto world=frames.actor_world_matrix(marker);return {{world[12],world[13],world[14],world[15]}};
    }
    const unsigned index=kind==0?1u:kind==1?2u:0u;
    const auto marker=state.markers()[index];
    if(!marker)throw std::logic_error("Retired Human item requires its actual equipment marker");
    const auto matrix=frames.actor_world_matrix(marker);
    auto point=frames.car_position(marker);
    const auto& offset=kind==0?human_equipment_offset24:kind==1?human_equipment_offset28:human_equipment_offset20;
    for(unsigned axis=0;axis!=3;++axis)point[axis]+=offset[axis];
    return frames.car_transform(point,matrix);
}

//007DC2A0, actual Items ownership, weight and seven-pointer Human storage.
//The entry stays module-owned: detach never releases or deletes it.
template<class Human,class Entry,class Items,class World>
void detach_human_inventory_entry(Human& human,Entry entry,unsigned slot,Items& items,World& world){
    auto& item=items.runtime_item(entry);item.holder_actor=nullptr;
    if((world.mode_flags()&2u)&&items.equipment_notifications_enabled())world.notify_human_inventory_changed(human);
    human.activation.flag178=true;
    const auto subtract=[&](float value){
        const float weight=static_cast<float>(static_cast<long double>(frontend::pre_mission_detail::Bytes::real(human.base().field7c()))-value);
        std::uint32_t bits;std::memcpy(&bits,&weight,4);human.base().field7c(bits);
    };
    subtract(item.scalar());
    if(item.pack_exposes_contents()){
        if(!item.slots())throw std::logic_error("Human pack retirement requires its actual contents owner");
        subtract(item.slots()->total_scalar());
    }
    auto* storage=human.base().storage();if(!storage)throw std::logic_error("Human item retirement requires its actual storage");
    if(slot<7){
        std::int32_t result;
        if(auto* holder=storage->holder(slot))result=holder->detach(entry);
        else if(auto* slots=storage->slots(slot))result=slots->detach(entry);
        else throw std::logic_error("Human item retirement lost its actual holder");
        if(result<1)throw std::logic_error("Human item retirement entry disagrees with its holder");
        if(slot==5&&item.definition().kind()==17)storage->pack_slots(nullptr);
    }
}

//005A7B70. Preserve holder priority and the original stack scan: the last
//matching stack wins, with entries traversed backwards within each stack.
template<class Human,class Items,class World>
void remove_human_inventory_item(Human& human,Items& items,std::int32_t identity,bool destroy,World& world){
    using Entry=typename Items::Entry;Entry entry{};unsigned selected=7;
    auto* storage=human.base().storage();
    if(!storage||storage->single())throw std::logic_error("Human item removal requires actual Human storage");
    for(unsigned slot:{3u,0u,2u,4u,5u}){
        auto* holder=storage->holder(slot);if(!holder)throw std::logic_error("Human item removal lost its holder");
        if(const auto candidate=holder->entry();candidate&&items.runtime_item(candidate).identity==identity){
            entry=candidate;selected=slot;
            if(slot==3){world.stop_human_item_voice(entry);world.reset_human_item_player_mode(human);}
            break;
        }
    }
    if(selected==7)for(unsigned slot:{1u,6u}){
        if(auto* stacks=storage->slots(slot))for(std::size_t n=0;n<stacks->occupied_stacks();++n){
            const auto& entries=stacks->slots().at(n).entries;
            for(auto at=entries.rbegin();at!=entries.rend();++at)if(items.runtime_item(*at).identity==identity){entry=*at;selected=slot;break;}
        }
        if(selected!=7)break;
    }
    if(!entry)return;
    world.retire_human_inventory_model(human,entry,selected); //007DBD80, before optional Items deletion.
    human.activation.flag178=true;
    if(destroy)items.remove_runtime_item(entry);
}
} //namespace hd2::game
#endif
