#ifndef HD2_GAME_HUMAN_ACTION_COMPLETION_EFFECTS_H
#define HD2_GAME_HUMAN_ACTION_COMPLETION_EFFECTS_H
#include "game/human_inventory_lookup.h"
#include "game/human_inventory_retirement.h"
#include "game/human_packet_commands.h"
#include "game/human_state_frame_timers.h"
#include "game/human_ai_shot_direction.h"

namespace hd2::game {
//00584E30 action11. The containing query is006F6870, whose guaranteed
//vertical displacement delegates006F5890 with every dynamic layer excluded.
template<class State,class Human,class Frames,class World>
void complete_human_ground_drop(State& state,Human& human,Frames& frames,World& world){
    if(!human.head_marker)throw std::logic_error("Human ground drop requires actual260 marker");
    auto matrix=frames.actor_world_matrix(human.head_marker);
    HumanVector position{{matrix[12],matrix[13],matrix[14],matrix[15]}};
    if(!human.head_marker)throw std::logic_error("Human ground drop lost actual260 marker");
    matrix=frames.actor_world_matrix(human.head_marker);
    HumanVector direction{{matrix[8],matrix[9],matrix[10],matrix[11]}};
    normalize_human_ai_shot(direction,(static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[2])*direction[2]);
    for(unsigned n:{0u,2u})position[n]=static_cast<float>(static_cast<long double>(direction[n])*.5f+position[n]);
    const auto hit=world.human_completion_ground_ray(position,HumanVector{{0,-2,0,0}});
    if(!hit)return;
    if(!state.item())throw std::logic_error("Human ground drop lost its actual item after query");
    const auto definition=world.human_frame_output_item(state.item()).definition().identifier;
    (void)world.human_completion_place_mine(human,definition,hit->position,hit->normal);
}
//00588AA0 shared virtual14 (Player00546DC0 has a separate policy).
//FLOAT medicine amounts convert through native __ftol; initial skill*maximum
//stays extended until that conversion. All life additions wrap native DWORDs.
template<class State,class Items,class World>bool complete_human_base_treatment(
        State& state,std::int32_t target_identity,std::int32_t item_identity,Items& items,World& world){
    auto* target=world.human_treatment_target(target_identity);
    if(!target||target->properties.stats_ec[0]==target->properties.stats_11c[0])return false;
    auto* human=state.human();if(!human)throw std::logic_error("Treatment requires its actual bound B0 Human");
    const auto entry=item_identity==-1?find_human_inventory_kind(*human,items,13):
        find_human_inventory_item_entry(*human,items,item_identity);
    if(!entry)return false;
    if(item_identity==-1)item_identity=items.runtime_item(entry).identity;
    if(items.runtime_item(entry).definition().kind()!=13)return false;
    const auto signed_word=[](std::uint32_t value){return human_frame_signed_word(value);};
    const auto integer=[](long double value){return human_frame_signed_word(human_packet_ftol(value));};
    auto amount=integer((static_cast<long double>(signed_word(human->properties.stats_ec[7]))*
        signed_word(target->properties.stats_11c[0]))*.009999999776482582f);
    if(amount>integer(items.runtime_item(entry).amount()))amount=integer(items.runtime_item(entry).amount());
    const auto maximum=target->properties.stats_11c[0],life=target->properties.stats_ec[0];
    if(signed_word(maximum)<signed_word(life+static_cast<std::uint32_t>(amount)))amount=signed_word(maximum-life);
    const auto cost=static_cast<float>(((100.f-static_cast<long double>(signed_word(human->properties.stats_ec[7])))*
        .009999999776482582f+1.f)*amount);
    items.runtime_item(entry).subtract_amount(cost);
    target->properties.stats_ec[0]+=static_cast<std::uint32_t>(amount);
    if((world.mode_flags()&7u)==7u){
        world.advance_human_medical_skill(*human,.10000000149011612f);
        world.human_treatment_notification(*human,*target,target_identity,item_identity,amount);
    }
    if(integer(items.runtime_item(entry).amount())==0)
        remove_human_inventory_item(*human,items,item_identity,true,world);
    return true;
}
//0060D6F0: same ExplosivePlace/Human/Items/broadcast owners. Native C4 is
//published BEFORE the definition lookup; missing equipment is a real false
//return retaining that write. Item identity is sampled before retirement.
template<class Target,class Human,class Items,class Activation,class Collisions,class Inventory>
bool complete_human_explosive_placement(Target& target,Human& human,Items& items,
        Activation& activation,Collisions& collisions,Inventory& inventory){
    if(target.field94!=0)return false;
    target.placer_identity(static_cast<std::uint32_t>(human.base().id10()));
    const auto entry=find_human_inventory_definition(human,items,
        static_cast<std::int32_t>(target.properties.fielda0));
    if(!entry)return false;
    const auto identity=items.runtime_item(entry).identity;
    remove_human_inventory_item(human,items,identity,true,inventory);
    target.set_state(1,activation,collisions);
    auto& packet=activation.session_broadcast_packets().begin(13,0);
    packet.write(static_cast<std::uint32_t>(target.base().id10()),10);
    packet.write(2,2);
    packet.write(target.placer_identity(),10); //Fresh C4 AFTER state/visibility callbacks.
    packet.write(static_cast<std::uint32_t>(identity),16);
    packet.finish_broadcast();
    return true;
}
}
#endif
