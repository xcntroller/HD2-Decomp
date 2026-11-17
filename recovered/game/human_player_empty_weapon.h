#ifndef HD2_GAME_HUMAN_PLAYER_EMPTY_WEAPON_H
#define HD2_GAME_HUMAN_PLAYER_EMPTY_WEAPON_H

#include "game/human_player_item_notification.h"
#include "game/human_packet_commands.h"

namespace hd2::game {
//007D4E90. Reuse process0098AFE0, initialized by007D45B0→004374D0.
//Only the original fields change; previous ranges/scales/volume are retained.
template<class Item,class Human,class World>
void emit_player_empty_weapon_sound(Item& item,Human& human,World& world){
    if(!(world.mode_flags()&2u))return;
    auto& request=world.player_item_sound_request();
    request.position={{0,0,1.1f}};request.group=3;request.entry=4;request.source=&human;
    request.relative=item.model();
    request.flags=world.human_is_player(human)&&world.human_view_offset_enabled()?0x40u:0u;
    (void)world.emit_human_equipment_sound(request);
}

//005433B0's empty/forced-reload branch. A trigger retries actual reload first;
//without a trigger, signed age→FLOAT precedes the single random threshold draw.
template<class Player,class Inline,class Item,class World>
void advance_player_empty_weapon(Player& player,Inline& control,Item& item,World& world){
    auto* human=player.human();auto* input=player.second_frame_inputs();
    if(!human||!input)throw std::logic_error("Empty Player weapon requires its actual Human/input");
    if(input->template get<float>(0x18)!=0){
        std::int32_t ammunition=-1;
        if(world.reload_human_ai_weapon(item,*human,ammunition,true)){
            control.set_frame_item_action(3);item.field60&=~2u;
            PlayerItemNotification notification;notification.command=5;notification.argument=static_cast<std::uint32_t>(ammunition);
            notification.value=human_packet_ftol(item.loaded_ammunition());notify_player_item(player,notification,world);return;
        }
    }else{
        const float age=static_cast<float>(human_frame_signed_word(world.game_word28()-item.field58));
        const auto threshold=static_cast<long double>(world.human_sound_random())*100.f+200.f;
        if(!(static_cast<long double>(age)>=threshold))return; //Equality emits; unordered returns.
    }
    emit_player_empty_weapon_sound(item,*human,world);
    //Native rereads A8 after SOUND; retain a possible callback's item change.
    const auto entry=player.item();if(!entry)throw std::runtime_error("Empty Player weapon lost its cached item after SOUND");
    world.human_frame_output_item(entry).field58=world.game_word28();
    world.player_item_fpv_request(7);
    PlayerItemNotification notification;notification.command=13;notify_player_item(player,notification,world);
}
} //namespace hd2::game
#endif
