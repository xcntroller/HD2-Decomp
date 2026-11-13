#ifndef HD2_GAME_HUMAN_PLAYER_TREATMENT_H
#define HD2_GAME_HUMAN_PLAYER_TREATMENT_H

#include "game/human_action_completion_effects.h"

namespace hd2::game {
//00546DC0, Player virtual14. Keep the protection prefix, nonzero-amount gate,
//FLOAT amount spill, pre-notification empty snapshot and FIRST90 publication
//distinct from00588AA0. Notification may retire/change the selected item.
template<class Player,class Items,class World>bool complete_human_player_treatment(
        Player& player,std::int32_t target_identity,std::int32_t item_identity,Items& items,World& world){
    auto* human=player.human();if(!human)throw std::logic_error("Player treatment requires actual B0 Human");
    if((world.mode_flags()&7u)==7u&&!world.human_integrity_matches(19)&&
            static_cast<float>(human_frame_signed_word(world.human_frame_random_word()))<327.66998291015625f){
        if(auto* target=world.human_treatment_target(target_identity)){
            auto reverse=world.human_damage_target_direction(*target);
            for(unsigned axis=0;axis!=3;++axis)reverse[axis]=-reverse[axis];
            const auto point=world.human_damage_random_position(*target);
            world.apply_human_damage(*target,6,-1.f,point,reverse,human,0);
        }
    }
    auto* target=world.human_treatment_target(target_identity); //Fresh lookup AFTER D4.
    if(!target||target->properties.stats_ec[0]==target->properties.stats_11c[0])return false;
    const auto entry=item_identity==-1?find_human_inventory_kind(*human,items,13):
        find_human_inventory_item_entry(*human,items,item_identity);
    if(!entry)return false;
    if(item_identity==-1)item_identity=items.runtime_item(entry).identity;
    if(items.runtime_item(entry).definition().kind()!=13)return false;
    const auto signed_word=[](std::uint32_t value){return human_frame_signed_word(value);};
    const auto integer=[&](long double value){return signed_word(human_packet_ftol(value));};
    auto amount=integer((static_cast<long double>(signed_word(human->properties.stats_ec[7]))*
        signed_word(target->properties.stats_11c[0]))*.009999999776482582f);
    if(!amount)return false;
    if(amount>integer(items.runtime_item(entry).amount()))amount=integer(items.runtime_item(entry).amount());
    const auto maximum=target->properties.stats_11c[0],life=target->properties.stats_ec[0];
    if(signed_word(maximum)<signed_word(life+static_cast<std::uint32_t>(amount)))amount=signed_word(maximum-life);
    const float healed=static_cast<float>(amount);
    const float cost=static_cast<float>(((100.f-static_cast<long double>(signed_word(human->properties.stats_ec[7])))*
        .009999999776482582f+1.f)*healed);
    items.runtime_item(entry).subtract_amount(cost);
    target->properties.stats_ec[0]+=static_cast<std::uint32_t>(amount);
    const bool empty=integer(items.runtime_item(entry).amount())==0;
    if((world.mode_flags()&7u)==7u){
        world.advance_human_medical_skill(*human,.10000000149011612f);
        world.human_treatment_notification(*human,*target,target_identity,item_identity,amount);
    }else{
        HumanPacketCommand command;command.kind=14;command.fields.set(8,target_identity);
        command.fields.set(0xc,items.runtime_item(entry).identity);command.fields.set(0x10,amount);
        command.fields.set(0x14,static_cast<std::uint32_t>(empty));
        player.queue_frame_command(std::move(command),false);
    }
    if(empty)remove_human_inventory_item(*human,items,item_identity,true,world);
    if((world.mode_flags()&7u)!=7u&&target_identity!=human->base().id10())
        world.award_human_treatment_score(human->base().id10(),human_packet_ftol(static_cast<long double>(healed)*.004000000189989805f));
    return true;
}
}
#endif
