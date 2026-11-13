#ifndef HD2_GAME_HUMAN_PLAYER_ACTOR_ACTIONS_H
#define HD2_GAME_HUMAN_PLAYER_ACTOR_ACTIONS_H

#include "game/human_packet_commands.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {
//0053D340 Player D8. Copy only subtype/identity/argument into a NEW kind3
//command on the SAME first queue. This producer does not execute DC early.
template<class Player>void queue_player_item_action(Player& player,std::uint32_t subtype,std::int32_t identity,std::uint32_t argument=0){
    HumanPacketCommand command;command.kind=3;command.fields.set(8,subtype);
    command.fields.set(0xc,identity);command.fields.set(0x10,argument);
    player.queue_frame_command(std::move(command),false);
}

//005433B0 kind11, after trigger/action-id gates. Query/Car health precede D8;
//the subtype14 consumer remains the existing shared targeted-command path.
template<class Player,class World>void repair_player_item_target(Player& player,World& world){
    if(auto* target=world.player_item_repair_target(player))queue_player_item_action(player,0x14u,target->base().id10());
}

//00542B80. The stealth query is offline-only. Ordinary melee changes the SAME
//Look action; both paths then play the real item sound and set its cooldown.
template<class Player,class Inline,class World>
void advance_player_melee(Player& player,Inline& control,World& world){
    auto& words=player.words();
    if(!words.template get<std::uint8_t>(0xac)||(words.template get<std::uint32_t>(0x50)&1u))return;
    auto* human=player.human();if(!human)throw std::logic_error("Player melee requires its actual Human");
    const auto current=[&]() -> decltype(auto){
        const auto entry=player.item();if(!entry)throw std::runtime_error("Player melee reached native missing cached item");
        return world.human_frame_output_item(entry);
    };
    if(world.game_word28()<current().field14||human_frame_signed_word(world.game_word28()-current().field58)<0)return;
    bool stealth=false;
    if((world.mode_flags()&7u)==7u)if(auto* target=world.player_item_melee_target(player)){
        const auto identity=target->base().id10();world.player_item_alignment(player,-10.f);
        queue_player_item_action(player,0x24u,identity);stealth=true;
    }
    if(!stealth)control.set_frame_item_action(8);
    const bool silent=(world.mode_flags()&7u)!=7u;
    const auto model=current().model();world.play_player_item_sound(current(),*human,model,silent);
    auto& item=current();const auto interval=item.ai_shot_interval();item.field58=world.game_word28()+interval;
}
} //namespace hd2::game
#endif
