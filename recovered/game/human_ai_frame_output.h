#ifndef HD2_GAME_HUMAN_AI_FRAME_OUTPUT_H
#define HD2_GAME_HUMAN_AI_FRAME_OUTPUT_H

#include "game/human_ai_navigation_path.h"
#include "game/human_sight_relations.h"

namespace hd2::game {

template<class AI,class Inline>void apply_human_ai_frame_movement(AI& ai,Inline& control){ //00583C30.
    auto* human=ai.human();
    if(!human)throw std::logic_error("AI frame output requires the bound Human");
    if(const auto* attachment=human->field160){
        const auto type=attachment->base().type();
        if(type==4||type==0x14||type==0x15||type==0x22)return;
    }
    const auto* first=ai.first_frame_inputs();
    const auto* second=ai.second_frame_inputs();
    if(!first||!second)throw std::logic_error("AI frame output requires the actual B0 input buffers");
    auto& words=ai.words();
    const auto publish=[&](std::uint32_t flags){ai.publish_frame_command_flags(flags);};
    const auto animation=[&](std::int32_t command){control.frame_movement_animation(command,publish);};
    const auto live_flags=[&]{return words.template get<std::uint32_t>(0x50);};
    const auto requested_stance=[&]{return words.template get<std::uint32_t>(0x5c);};
    const auto pressed=[](const auto& input,unsigned offset){return input.template get<float>(offset)!=0;};

    if(control.frame_special_movement_item()&&!control.frame_navigation_floor()&&(requested_stance()&0x14000u))
        words.set(0x5c,0x2000u);
    using namespace human_ai_navigation;
    Position movement{};
    words.set(0x30,movement); //Native reset initializes XYZ; its temporary W has no writer.
    const auto previous=live_flags();
    std::uint32_t request=pressed(*first,0)?8u:pressed(*first,4)?0x10u:0u;
    request|=pressed(*first,8)?0x20u:pressed(*first,0xc)?0x40u:0u;
    if(!request){
        if(pressed(*first,0x28))request=0x1000000u;
        else if(pressed(*first,0x2c))request=0x2000000u;
    }
    if(pressed(*second,0x20))request|=2;
    else if(pressed(*second,0x24))request|=1;
    const auto direction=words.template get<Position>(0x20);
    if(request&0x3000000u){
        if(!(previous&request&0x3000000u))animation(request&0x1000000u?0xa2:0xa3);
    }else if(request&0x18u){
        const bool forward=request&8u;
        if(!(previous&(forward?8u:0x10u))||((previous^request)&0x60u))animation(forward?0xbe:0xbf);
        movement[0]=forward?direction[0]:-direction[0];
        movement[2]=forward?direction[2]:-direction[2];
        words.set(0x30,movement);
    }
    if(request&0x60u){
        const bool left=request&0x20u;
        const auto active=live_flags();
        if(!(active&(left?0x20u:0x40u))||((active^request)&0x18u)){
            const std::int32_t lateral=request&8u?0xc2:request&0x10u?0xc4:0xc0;
            animation(lateral+(left?0:1));
        }
        movement[0]+=left?-direction[2]:direction[2];
        movement[2]+=left?direction[0]:-direction[0];
    }
    if((product(movement[2],movement[2])+product(movement[1],movement[1]))+
       product(movement[0],movement[0])>=1.e-16f){
        movement=unit(movement,square_length(movement));
        const auto requested_speed=words.template get<float>(0x58);
        const auto speed=requested_speed==0?10.f:requested_speed; //0080FA14.
        for(auto& component:movement)component=static_cast<float>(product(speed,component));
    }
    words.set(0x30,movement);
    const auto stance=requested_stance();
    if(stance!=(live_flags()&0x1f000u)){
        if(stance&0x10000u)control.frame_movement_mode(5,publish);
        else if(stance&0x4000u)control.frame_movement_mode(2,publish);
        else if(stance&0x2000u)control.frame_movement_mode(1,publish);
        else if(stance&0x8000u)control.frame_movement_mode(4,publish);
        else if(stance&0x1000u)control.frame_movement_mode(0,publish);
    }
    if(request&3u){
        const bool changed=!(live_flags()&(request&3u));
        if(changed&&requested_stance()==0x10000u)words.set(0x5c,0x4000u);
        control.frame_posture(changed?(request&2u?1u:2u):0u,publish);
    }
    if(pressed(*second,0x44)){
        //00571C10 dispatches subtype1F immediately and appends a separate
        //kind-three copy to the B0+94 queue for its original deferred consumer.
        throw std::runtime_error("AI climb output requires its actual B0 deferred command receiver");
    }
    if(pressed(*second,0x30)&&(request&8u)&&human->activation.field14c>10.f){
        animation(0x12);
        words.set(0x50,(live_flags()&0xffffff87u)|0x80u);
        const auto remaining=static_cast<float>(static_cast<long double>(human->activation.field14c)-10.f);
        human->activation.field14c=remaining<0?0:remaining;
    }else if((previous&0x30000f8u)&&!(request&0x30000f8u))animation(0x2d);
}

template<class AI,class Inline,class World,class Runtime>
void apply_human_ai_frame_items(AI& ai,Inline& control,World& world,Runtime& runtime,
        std::uint32_t elapsed,std::uint32_t fire_argument){ //0058BAF0.
    auto& words=ai.words();
    words.set(0xb0,words.template get<std::uint32_t>(0xb0)+elapsed);
    const auto* second=ai.second_frame_inputs();
    auto* first=ai.first_frame_inputs();
    if(!second||!first)return;
    auto* human=ai.human();
    if(!human)throw std::logic_error("AI item output requires the bound Human");
    if(human->field160)throw std::runtime_error("AI item output requires the actual attachment input receiver");
    if(second->template get<float>(0x40)!=0){
        if(!world.human_feedback_kind9(*human))return;
        throw std::runtime_error("AI item output requires the actual kind-nine release receiver");
    }
    const auto entry=ai.item();
    if(!entry)return;
    auto& item=world.human_frame_output_item(entry);
    const auto kind=item.definition().kind();
    const bool trigger=second->template get<float>(0x18)!=0;
    switch(kind){
    case 2:case 4:case 6:first->set(0x18,0.f);break;
    case 3:case 5:case 7:case 0xc:break;
    case 9:
        if(trigger){
            control.set_frame_item_action(6);
            world.human_frame_item_action_fpv(*human,2);
        }else if(second->template get<float>(0x1c)!=0){
            control.set_frame_item_action(7);
            world.human_frame_item_action_fpv(*human,4);
        }
        return;
    default:return;
    }
    const auto reload=[&](bool forced){
        std::int32_t ammunition=-1;
        if(!world.reload_human_ai_weapon(item,*human,ammunition,human->properties.field19c==2))return;
        world.human_frame_item_action_fpv(*human,6);
        control.set_frame_item_action(3);
        if(forced)item.field60&=~2u;
        if((world.mode_flags()&7u)!=7u)throw std::runtime_error("AI reload requires actual B0 command05 transport");
    };
    if(!trigger&&first->template get<float>(0x18)==0){
        if(second->template get<float>(0x34)!=0)reload(false);
        return;
    }
    if(!words.template get<std::uint8_t>(0xad)){
        if((words.template get<std::uint32_t>(0x50)&0x10008u)==0x10008u)return;
        control.set_frame_item_ready(true,[&](std::uint32_t value){world.human_frame_first_person_state(*human,value);});
        if((world.mode_flags()&7u)!=7u)throw std::runtime_error("AI item readiness requires the actual nonoffline command receiver");
        return;
    }
    if(item.amount()==0||(item.field60&2u)){
        if(trigger)reload(true);
        return;
    }
    if(human_frame_signed_word(world.game_word28()-item.field58)<0)return;
    auto direction=words.template get<HumanVector>(0x10);
    auto frame=item.shot_frame();
    if(frame)frame=item.shot_frame();else frame=item.model();
    const auto position=world.sight_position(frame);
    world.prepare_human_ai_shot_direction(ai,control,item,direction); //B0 virtual04, after sampling shdum.
    runtime.fire_human_item(item,*human,position,direction,fire_argument,true);
    const auto interval=item.ai_shot_interval();
    item.field58=world.game_word28()+interval;
    control.set_frame_item_action(1);
    words.set(0xb0,0u);
    set_human_sight_alignment(*human,-30.f,world.mode_flags());
    world.human_frame_item_action_fpv(*human,2);
    if((world.mode_flags()&7u)!=7u)throw std::runtime_error("AI fire requires actual B0 command02 transport");
}
} //namespace hd2::game
#endif
