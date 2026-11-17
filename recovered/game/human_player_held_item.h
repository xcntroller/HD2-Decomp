#ifndef HD2_GAME_HUMAN_PLAYER_HELD_ITEM_H
#define HD2_GAME_HUMAN_PLAYER_HELD_ITEM_H

#include "game/human_state_frame_timers.h"

namespace hd2::game {
//00542920's actual value payload. Native zero-vector temporaries do not
//produce W; defined source padding receives no original lane/ABI credit.
struct PlayerItemNotification {
    std::uint32_t command=0;
    HumanVector first{},second{};
    std::uint32_t argument=0,value=0xffffffffu;
    std::uint8_t flag=0xff;
};

template<class Player,class Item>void publish_player_held_item_title(Player& player,const Item& item){
    auto* human=player.human();if(!human)throw std::logic_error("Held item title requires its actual Human");
    human->activation.field17c=human->activation.field180=1000;
    human->activation.field184=item.definition().caption; //005433B0, native definition14.
}

//00542E20. Suppression cancels without arming540. Release publishes the item
//strength/model/action BEFORE command08, FPV and the real hand-holder deadline.
//A required failure in those receivers retains that exact earlier prefix.
template<class Player,class Inline,class World,class Notify>
bool advance_player_held_item(Player& player,Inline& control,World& world,Notify& notify,std::uint32_t elapsed){
    auto& state=player.player_words();if(!state.template get<std::uint8_t>(0x538))return false;
    auto* human=player.human();if(!human)throw std::logic_error("Held item requires its actual Human");
    const auto clear_title=[&]{human->activation.field17c=human->activation.field180=human->activation.field184=0;};
    const auto next=state.template get<std::uint32_t>(0x53c)+elapsed;state.set(0x53c,next);
    if(human_frame_signed_word(next)>1000){state.set(0x53c,1000u);clear_title();}
    if(control.first_person_weapon_suppressed()){
        state.set(0x538,std::uint8_t{0});state.set(0x53c,0u);clear_title();return true;
    }
    const auto action=state.template get<std::int32_t>(0x544);
    const auto* input=player.first_frame_inputs();
    if(!input&&(action==13||action==6||action==7))throw std::runtime_error("Held item reached native missing first input");
    const auto strength=[&]{
        float value=static_cast<float>(static_cast<long double>(human_frame_signed_word(state.template get<std::uint32_t>(0x53c)))*.001f);
        const auto flags=player.words().template get<std::uint32_t>(0x50);
        if(flags&2u){if(value>.85f)value=.85f;}
        else if((flags&1u)&&value>.7f)value=.7f;
        return value;
    };
    const auto deadline=[&](std::uint32_t delay){
        const auto until=world.game_word28()+delay; //Native clock read BEFORE storage/holder query.
        auto* storage=human->base().storage();
        if(!storage||storage->single()||!storage->holder(3))
            throw std::runtime_error("Held item release reached native missing hand holder");
        storage->holder(3)->field18(until); //007D68B0, holder18, NOT item14 or a DD0 timer.
    };
    const auto release=[&](auto& item,std::uint32_t command){
        const auto value=strength();item.field98=value;
        const auto model=item.model(); //007D3AA0(0) BEFORE Look/model publication.
        control.set_retained_frame_item_action(action,model);
        PlayerItemNotification notification;notification.command=command;
        std::memcpy(&notification.argument,&value,4);
        notify(notification);
    };
    if(action==13&&input->template get<float>(0x40)==0){
        if(const auto entry=world.human_feedback_kind9(*human)){
            release(world.human_frame_output_item(entry),16);
            world.player_item_fpv_request(21);deadline(1000);
        }
    }else if((action==6&&input->template get<float>(0x18)==0)||
             (action==7&&input->template get<float>(0x1c)==0)){
        if(const auto entry=player.item()){
            auto& item=world.human_frame_output_item(entry);
            if(item.definition().kind()==9){release(item,action==6?14u:15u);deadline(1500);}
        }
    }else return true;
    state.set(0x53c,0u);state.set(0x538,std::uint8_t{0});clear_title();state.set(0x540,2500u);return true;
}
} //namespace hd2::game
#endif
