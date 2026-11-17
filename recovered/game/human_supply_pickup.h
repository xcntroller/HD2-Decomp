#ifndef HD2_GAME_HUMAN_SUPPLY_PICKUP_H
#define HD2_GAME_HUMAN_SUPPLY_PICKUP_H
#include "game/human_joint_owners.h"
namespace hd2::game {
//005A37C0. Availability gates precede weapon/side/definition/creation. Output
//item identity is written only after actual placement, count/depletion/visibility.
template<class Human,class World>bool validate_human_supply_pickup(Human& human,std::int32_t identity,std::int32_t index,
        std::int32_t& slot,std::int32_t& item_identity,World& world){
    auto* box=world.human_supply_box(identity);if(!box)return false;
    if(!box->field130||box->field250||!(world.human_supply_flags(*box)&1u))return false;
    const auto definition=world.human_supply_definition(human,*box,index);if(!definition)return false;
    auto entry=world.create_unheld_supply_item(*definition);
    if(world.place_unheld_human_item(human,entry,slot,true)!=1){world.release_failed_supply_item(entry);return false;}
    world.consume_human_supply_item(*box);item_identity=world.human_supply_item_identity(entry);human.activation.flag178=true;return true;
}
//005A38F0. Presentation only gates mode1/box/weapon selection, not count or
//availability. It creates directly into the transmitted slot/identity, ignores
//factory's ordinary result, then freshly tests/decrements supply and refreshes it.
template<class Human,class World>void apply_human_supply_pickup(Human& human,std::int32_t identity,std::int32_t index,
        std::int32_t slot,std::int32_t item_identity,World& world){
    if(world.mode_flags()&1u)return;
    auto* box=world.human_supply_box(identity);if(!box)return;
    const auto definition=world.human_supply_definition(human,*box,index);if(!definition)return;
    world.create_received_supply_item(human,*definition,slot,item_identity);world.consume_human_supply_item(*box);human.activation.flag178=true;
}
} //namespace hd2::game
#endif
