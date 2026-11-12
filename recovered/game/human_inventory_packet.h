#ifndef HD2_GAME_HUMAN_INVENTORY_PACKET_H
#define HD2_GAME_HUMAN_INVENTORY_PACKET_H
#include "game/human_inventory_lookup.h"
#include "game/human_inventory_swap.h"
namespace hd2::game {
//005A2D50/005A2E10: authority is slot-scoped; presentation searches the
//source's whole inventory and ignores the ordinary transfer result. Received
//commands never reenter the queued B0+74 request producer.
template<class Human,class Items,class World>
bool receive_human_inventory_move(Human& human,Items& items,World& world,bool authority,
        std::int32_t identity,std::int32_t source_identity,std::int32_t source_slot,
        std::int32_t& destination_slot,std::int32_t index){
    if(!authority&&(world.mode_flags()&1u))return false;
    auto* source=world.human_inventory_source(human,source_identity);
    if(!source)throw std::logic_error("Human inventory command lost its actual source actor");
    const auto entry=authority?find_human_inventory_entry(*source,items,identity,source_slot):find_human_inventory_item_entry(*source,items,identity);
    if(authority&&!entry)return false;
    const auto result=world.move_received_human_inventory(human,*source,entry,source_slot,destination_slot,index,authority);
    if(authority&&result!=1)return false;
    human.activation.flag178=true;
    if(authority&&(source_slot==3||destination_slot==3))world.reset_human_item_player_mode(human);
    return true;
}
//005A2E90/005A2F50. Presentation still performs both whole-inventory lookups,
//but unlike authority it does not require a current-entry match before swapping.
template<class Human,class Items,class World>
bool receive_human_inventory_swap(Human& human,Items& items,World& world,bool authority,
        std::int32_t current_identity,std::int32_t current_slot,std::int32_t source_identity,
        std::int32_t selected_identity,std::int32_t selected_slot){
    if(!authority&&(world.mode_flags()&1u))return false;
    auto* source=world.human_inventory_source(human,source_identity);
    if(!source)throw std::logic_error("Human inventory swap lost its actual source actor");
    const auto current=authority?find_human_inventory_entry(human,items,current_identity,current_slot):find_human_inventory_item_entry(human,items,current_identity);
    const auto selected=authority?find_human_inventory_entry(*source,items,selected_identity,selected_slot):find_human_inventory_item_entry(*source,items,selected_identity);
    if(authority&&(!current||!selected))return false;
    const auto result=world.swap_received_human_inventory(human,*source,selected,selected_slot,current_slot);
    if(authority&&result!=1)return false;
    human.activation.flag178=true;source->activation.flag178=true;
    if(authority&&(current_slot==3||selected_slot==3))world.reset_human_item_player_mode(human);
    return true;
}
} //namespace hd2::game
#endif
