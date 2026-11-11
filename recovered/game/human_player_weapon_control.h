#ifndef HD2_GAME_HUMAN_PLAYER_WEAPON_CONTROL_H
#define HD2_GAME_HUMAN_PLAYER_WEAPON_CONTROL_H

#include "game/human_ai_shot_direction.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {

//005457E0, actual Player zoom/secondary-view requests and three engine draws.
template<class Player,class Inline,class World,class Runtime>
void advance_player_weapon_mode(Player& player,Inline& control,World& world,Runtime& runtime){
    auto* human=player.human();
    if(!human)throw std::logic_error("Player weapon mode requires its actual Human");
    if(control.frame_navigation_floor()||human->field174)return;
    auto* animation=control.control();
    if(!animation)throw std::logic_error("Player weapon mode requires actual DD0");
    const auto clip=animation->animation_state_word(0x1d8);
    if(clip==8||clip==14||(clip>=106&&clip<=108))return; //005AC2B0.
    if(const auto* attachment=human->field160){
        const auto kind=attachment->base().type();
        if(kind==4){if(!(runtime.player_attachment_control_flags(*human)&0x20u))return;}
        else if(kind==0x14||kind==0x15||kind==0x22)return;
    }
    auto& words=player.words();
    if(!world.player_item_controls_available()||
            (words.template get<std::int32_t>(0x60)!=-1&&words.template get<std::int32_t>(0x64)>0))return;
    player.refresh_item();
    const auto entry=player.item();const auto* input=player.second_frame_inputs();
    auto& state=player.player_words();
    const auto mode=[&]{return state.template get<std::int32_t>(0x52c);};
    if(!entry||!input||input->template get<float>(0x1c)==0){
        if(!input)throw std::runtime_error("Player weapon toggle reached the native missing-input dereference");
        if(input->template get<float>(0x3c)==0)return;
        if(mode()==0){world.set_player_frame_mode(player,1);return;}
        auto direction=words.template get<HumanVector>(0x10);
        for(unsigned axis=0;axis!=3;++axis){
            const auto draw=world.human_sound_random(); //Actual LS3DF RandFloat, no CRT draw.
            direction[axis]=static_cast<float>((static_cast<long double>(draw)-.5f)*.009999999776482582f+direction[axis]);
            words.set(0x10+axis*4,direction[axis]); //Native publishes each draw before requesting the next.
        }
        const float square=static_cast<float>((static_cast<long double>(direction[2])*direction[2]+
            static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[0])*direction[0]);
        normalize_human_ai_shot(direction,square);words.set(0x10,direction);
    }else{
        auto& item=world.human_frame_output_item(entry);
        if(item.primary_payload_kind()!=4||(item.field60&2u)||world.game_word28()<item.field14||
                human_frame_signed_word(world.game_word28()-item.field58)<0)return;
        const auto flags=words.template get<std::uint32_t>(0x50);
        if((flags&0xf8u)&&(flags&0x14000u))return;
        if(item.definition().kind()==5&&(flags&0xfeu))return;
        if(mode()==0){world.set_player_frame_mode(player,item.primary_payload_command()==5?2:3);return;}
    }
    world.set_player_frame_mode(player,0);
}

//00545B90. Readiness changes use the same inline E0 and original Player08
//receiver. Mode changes retain the actual FPV/FOV/integrity owner.
template<class Player,class Inline,class World,class Runtime>
void advance_player_weapon_readiness(Player& player,Inline& control,World& world,Runtime& runtime,std::uint32_t elapsed){
    auto& state=player.player_words();
    if(!world.player_item_controls_available()){
        if(!state.template get<std::uint8_t>(0x538))advance_player_weapon_mode(player,control,world,runtime);
        return;
    }
    if(state.template get<std::uint8_t>(0x538))return;
    auto* human=player.human();
    if(!human)throw std::logic_error("Player readiness requires its actual Human");
    player.refresh_item();
    if(human->field160&&runtime.player_attachment_blocks_item(*human))return;
    advance_player_weapon_mode(player,control,world,runtime);
    auto& words=player.words();
    const auto ready=[&](bool enabled){
        auto notify=[&](std::uint32_t value){world.human_frame_first_person_state(*human,value);};
        control.set_frame_item_ready(enabled,notify); //0058C330/0058C400→005A45F0 subtype14/15.
        world.notify_player_item_readiness(player,enabled?1u:0u); //ALWAYS after inline command, even when E0 is unchanged.
    };
    const auto mode=[&]{return state.template get<std::int32_t>(0x52c);};
    const auto entry=player.item();
    if(!entry){if(words.template get<std::uint8_t>(0xad))ready(false);return;}
    const auto* input=player.second_frame_inputs();
    if(!input)throw std::runtime_error("Player readiness reached the native missing-input dereference");
    const auto kind=world.human_frame_output_item(entry).definition().kind();
    const auto flags=words.template get<std::uint32_t>(0x50);
    if(!words.template get<std::uint8_t>(0xad)){
        if(input->template get<float>(0x18)==0){
            if(mode()==1){if(flags&0xf8u)world.set_player_frame_mode(player,0);return;}
            if(mode()!=2&&mode()!=3&&!(kind==5&&(flags&1u)&&!(flags&0xf8u))){
                if(input->template get<float>(0x34)==0)return;
                if(kind==5&&(flags&0xfeu))return;
            }
            ready(true);return;
        }
        if(kind==5){if((flags&0xf8u)||(flags&6u))return;}
        else if(kind==6&&(flags&0xf9u))return;
        if(!(flags&0xf8u)||!(flags&0x10001u))ready(true);
        return;
    }
    if(((flags&1u)&&(flags&0xf8u))||(kind==6&&(flags&0xf9u))||(kind==5&&(flags&0xfeu))){
        world.set_player_frame_mode(player,0);ready(false);state.set(0x534,0u);return;
    }
    if(mode()==1){
        if(flags&0xf8u){world.set_player_frame_mode(player,0);return;}
        ready(false);return;
    }
    if(mode()==2||mode()==3){
        if((flags&0xf8u)&&(flags&0x14000u)){world.set_player_frame_mode(player,0);state.set(0x534,0u);}
        return;
    }
    if((flags&0xf8u)&&(flags&0x10000u)){ready(false);state.set(0x534,0u);return;}
    const auto remaining=state.template get<std::uint32_t>(0x534);
    if(remaining&&kind!=9){
        const auto next=remaining-elapsed;state.set(0x534,next);
        if(human_frame_signed_word(next)<=0){state.set(0x534,0u);ready(false);}
    }
}

} //namespace hd2::game
#endif
