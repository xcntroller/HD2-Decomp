#ifndef HD2_GAME_HUMAN_PLAYER_ITEM_FRAME_H
#define HD2_GAME_HUMAN_PLAYER_ITEM_FRAME_H

#include "game/human_state_frame_timers.h"

namespace hd2::game {

//005433B0's real gates and item-family dispatch. Positive held-item, shot,
//dry-fire, repair and command05 workers remain mandatory runtime receivers.
template<class Player,class Inline,class World,class Runtime>
void advance_player_item_frame(Player& player,Inline& control,World& world,Runtime& runtime,std::uint32_t elapsed){
    auto& words=player.words();words.set(0xb0,words.template get<std::uint32_t>(0xb0)+elapsed);
    auto* first=player.first_frame_inputs();auto* second=player.second_frame_inputs();
    if(!first||!second)return;
    auto* human=player.human();
    if(!human)throw std::logic_error("Player item frame requires its actual Human");
    if((world.mode_flags()&7u)!=7u&&human_frame_signed_word(human->field208)<human_frame_signed_word(world.player_item_spawn_delay_ms()))return;
    auto& state=player.player_words();
    if(state.template get<std::int32_t>(0x52c)==1)return;
    if(human->field160&&(human->field164==0||runtime.player_attachment_blocks_item(*human)))return;
    if(!world.player_item_controls_available()||control.first_person_weapon_suppressed())return;
    player.refresh_item();
    const auto moving_crouched=[&]{const auto flags=words.template get<std::uint32_t>(0x50);return (flags&1u)&&(flags&0xf8u);};
    if(second->template get<float>(0x40)!=0&&!state.template get<std::uint8_t>(0x538)){
        if(moving_crouched()||state.template get<std::int32_t>(0x52c)!=0)return;
        if(const auto entry=player.item()){
            auto& current=world.human_frame_output_item(entry);
            if(world.game_word28()<current.field5c||world.game_word28()<current.field14||current.definition().kind()==9)return;
        }
        if(state.template get<std::uint32_t>(0x540))return;
        const auto grenade=world.human_feedback_kind9(*human);if(!grenade)return;
        state.set(0x538,std::uint8_t{1});state.set(0x53c,0u);state.set(0x544,13u);
        runtime.publish_player_held_item_title(player,world.human_frame_output_item(grenade)); //Actual Human17C/180/184 publication from native definition14.
        return;
    }
    if(state.template get<std::uint8_t>(0x538)&&runtime.advance_player_held_item(player,elapsed))return; //00542E20 genuinely returns0 when538 is clear.
    const auto entry=player.item();
    if(!entry||state.template get<std::uint8_t>(0x538))return;
    auto& item=world.human_frame_output_item(entry);
    const auto kind=item.definition().kind();
    const auto trigger=[&]{return second->template get<float>(0x18)!=0;};
    switch(kind){
    case 0:
        if(trigger())runtime.player_unarmed_item(player); //00542B80.
        return;
    case 6:
        if(words.template get<std::uint32_t>(0x50)&0xf8u){second->set(0x34,0u);second->set(0x18,0u);}
        [[fallthrough]];
    case 2:case 4:first->set(0x18,0u);[[fallthrough]];
    case 3:case 5:case 7:case 12:
        if(!trigger()&&first->template get<float>(0x18)==0){
            if(second->template get<float>(0x34)==0)return;
            if(kind==5&&(words.template get<std::uint32_t>(0x50)&0xfeu))return;
            auto* animation=control.control();
            if(!animation)throw std::logic_error("Player reload requires actual DD0");
            const auto clip=animation->animation_state_word(0x1d8);
            if(clip==8||clip==14||(clip>=106&&clip<=108)||control.frame_navigation_floor()||human->field174||moving_crouched())return;
            const auto amount=item.amount();const auto capacity=item.capacity(); //Native separate virtual0C then10 evaluations.
            if(amount==capacity)return;
            std::int32_t ammunition=-1;
            if(!world.reload_human_ai_weapon(item,*human,ammunition,true))return; //SAME actual item virtual18 with retain-last1.
            control.set_frame_item_action(3);
            runtime.player_item_reload_notification(player,ammunition); //00542920 command05, including real FPV duration/FOV.
            return;
        }
        if(!words.template get<std::uint8_t>(0xac))return;
        if(item.amount()==0||(item.field60&2u)){runtime.advance_player_empty_weapon(player,item,elapsed);return;}
        if(human_frame_signed_word(world.game_word28()-item.field58)<0)return;
        runtime.fire_player_weapon(player,item,elapsed); //Actual camera/shdum query,00545140, item24,005454D0 and integrity-observed branch.
        return;
    case 9:{
        if(moving_crouched()||state.template get<std::uint32_t>(0x540))return;
        const auto action=trigger()?6u:second->template get<float>(0x1c)!=0?7u:0u;
        if(!action)return;
        state.set(0x538,std::uint8_t{1});state.set(0x544,action);state.set(0x53c,0u);
        runtime.publish_player_held_item_title(player,item);return;
    }
    case 10:
        if(trigger()&&words.template get<std::int32_t>(0x60)==-1)runtime.player_item_actor_action(player,17u,human->base().id10());
        return;
    case 11:
        if(trigger()&&words.template get<std::int32_t>(0x60)==-1)runtime.player_repair_item(player);
        return;
    default:return;
    }
}

} //namespace hd2::game
#endif
