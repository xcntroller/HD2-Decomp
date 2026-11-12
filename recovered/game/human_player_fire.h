#ifndef HD2_GAME_HUMAN_PLAYER_FIRE_H
#define HD2_GAME_HUMAN_PLAYER_FIRE_H

#include "game/human_player_shot_direction.h"
#include "game/human_player_item_notification.h"

namespace hd2::game {
//005433B0's positive shot. Fire MUST execute the actual item virtual24; it is
//not an admission predicate or an animation-success callback. Reread cachedA8
//after fire/Look/FPV/notification: those receivers can change the item lifetime.
template<class Player,class Inline,class World,class Fire>
void fire_player_weapon(Player& player,Inline& control,World& world,Fire& fire){
    auto* human=player.human();if(!human)throw std::logic_error("Player fire requires its actual Human");
    const auto current=[&]() -> decltype(auto){
        const auto entry=player.item();if(!entry)throw std::runtime_error("Player fire reached native missing cached item");
        return world.human_frame_output_item(entry);
    };
    HumanVector position{},direction{};
    world.prepare_player_item_shot(player,current(),position,direction);
    spread_player_shot(current(),direction,world);
    (void)fire(current(),*human,position,direction,0u,true); //Actual item24, AFTER spread.
    player.refresh_item();if(!player.item())return;
    auto facing=player.words().template get<HumanVector>(0x10);
    auto publish=[&](unsigned axis,float value){player.words().set(0x10+axis*4,value);};
    recoil_player_shot(player,current(),facing,world,publish);
    control.set_frame_item_action(1);
    world.player_item_fpv_request(current().amount()==0?3:2);
    auto& item=current();const auto interval=item.ai_shot_interval();
    item.field58=world.game_word28()+interval;
    PlayerItemNotification notification;notification.command=2;notification.first=position;notification.second=direction;
    notify_player_item(player,notification,world);
    player.words().set(0xb0,0u);
    if((world.mode_flags()&7u)==7u&&!world.human_integrity_matches(77)&&
            static_cast<long double>(human_frame_signed_word(world.human_frame_random_word()))<163.83499145507812f){
        auto velocity=player.target_direction();
        for(unsigned axis=0;axis!=3;++axis)velocity[axis]=static_cast<float>(static_cast<long double>(velocity[axis])*4.f);
        world.drop_human_damage_item(*human,player.item(),velocity);
    }
}
} //namespace hd2::game
#endif
