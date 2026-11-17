#ifndef HD2_GAME_HUMAN_PLAYER_ITEM_NOTIFICATION_H
#define HD2_GAME_HUMAN_PLAYER_ITEM_NOTIFICATION_H

#include "game/human_player_held_item.h"
#include "game/human_input_records.h"
#include "game/human_frame_stats.h"

namespace hd2::game {
//00542920, including its late non7 publication. Local effects happen before
//allocating/adopting the SAME Record's kind0 pending payload. A failed local
//effect does not queue a notification; failed queueing retains local mutations.
template<class Player,class World>
void notify_player_item(Player& player,const PlayerItemNotification& notification,World& world){
    auto& state=player.player_words();
    switch(notification.command){
    case 2:
        if((world.mode_flags()&7u)==7u&&player.words().template get<std::int32_t>(0xb0)>500)
            world.player_item_first_shot(player); //004563E0 singleton virtual24=00459360.
        state.set(0x534,10000u);world.player_item_alignment(player,-30.f);break;
    case 5:
        if(state.template get<std::int32_t>(0x52c)!=0){
            world.player_item_reset_zoom(); //00492130(0, SAME gameBC).
            const auto duration=world.player_item_reload_duration(player); //Actual cached A8/payload virtual20, after zoom.
            state.set(0x530,duration-200u);
        }
        [[fallthrough]];
    case 1:state.set(0x534,10000u);break;
    case 8:case 9:case 14:case 15:case 16:world.player_item_alignment(player,-10.f);break;
    default:break;
    }
    if((world.mode_flags()&7u)!=7u)world.publish_player_item_input(player,notification);
}

//00542920's kind0/0x70 allocation and0073E5A0. Input manager and Record are
//borrowed actual process owners. Vector ordering is native second→30, first→40;
//68 is a signed elapsed-shot integer divided by100, not loaded ammunition.
template<class Player,class Inputs,class Item>
void publish_player_item_input(Player& player,Inputs& inputs,const PlayerItemNotification& notification,
        std::uint32_t mode,Item* item){
    auto message=std::make_unique<HumanInputMessage>(0);
    auto& words=message->words();words.set(0x60,notification.command);
    words.set(0x2c,static_cast<std::uint8_t>((~mode)&1u));
    words.set(0x30,notification.second);words.set(0x40,notification.first);
    words.set(0x50,item?human_frame_signed_word(item->field68)/100:0);
    words.set(0x58,notification.value);words.set(0x54,notification.argument);words.set(0x5c,notification.flag);
    const auto stamp=inputs.state().template get<std::uint32_t>(0xdc);
    auto* record=player.input();
    if(!record||!inputs.contains(record))throw std::runtime_error("Player item notification requires its SAME live process input Record");
    auto* payload=record->adopt_pending(std::move(message));record->append_pending(*payload,stamp,true);
}
} //namespace hd2::game
#endif
