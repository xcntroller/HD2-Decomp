#ifndef HD2_GAME_HUMAN_SCENE_FRAME_H
#define HD2_GAME_HUMAN_SCENE_FRAME_H

#include "game/human_frame_samples.h"
#include <cstring>

namespace hd2::game {
//0059FE30/0059FFD0, using the SAME containing state/inline/model owners.
//Runtime supplies REQUIRED real derived B0, DD0, fire, contact, stat and
//formation receivers. No cold successful substitutes or actor-list copies.
template<class Human,class World,class Runtime,class Frames>
void advance_human_scene_frame(Human& human,bool authority,std::uint32_t elapsed,
        World& world,Runtime& runtime,Frames& frames){
    const auto signed_word=[](std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;};
    const auto state=[&]() -> auto& {
        auto* current=world.human_state_owner(human).get();
        if(!current)throw std::logic_error("Human frame requires actual current B0");
        return *current;
    };
    const auto refresh=[&]{if(auto* current=world.human_state_owner(human).get())current->refresh_item();}; //005A6A90 genuine NULL RET.
    const auto directions=[&]{
        world.human_inline(human).frame_current_direction(state().current_direction());
        world.human_inline(human).frame_position(state().words().template get<HumanVector>(0x30));
        world.human_inline(human).frame_target_direction(state().target_direction());
    };
    if(!authority){
        advance_human_presentation_timers(human.activation,elapsed);
        if((world.mode_flags()&7u)!=7u){
            human.field208+=elapsed;
            if(signed_word(human.properties.stats_ec[0])<=0&&(world.mode_flags()&2u)){
                human.field204+=elapsed;const auto config=world.human_respawn_configuration();
                if(config.style==0&&signed_word(human.field204)>signed_word((config.delay+3u)*1000u)&&config.enabled&&world.human_current_player_id()==human.base().id10()){
                    runtime.request_human_respawn();human.field204=0xfffff060u; //-4000 AFTER actual006DC940/0072B1C0 request.
                }
            }else human.field204=0;
        }
        if(human.base().id14()<0)return; //004D0E80 virtual64, separate from runtime byte1C.
        human.activation.flag178=false;refresh();runtime.advance_human_bleeding(human,elapsed); //005A0480.
        runtime.advance_human_state(human,false,elapsed); //Actual B0 virtual24.
        directions();world.human_inline(human).advance_presentation_frame(elapsed,runtime);
        if(!(world.mode_flags()&1u))sample_human_frame_motion(human,frames,elapsed);
        const auto flags=world.human_inline(human).consume_frame_feedback();
        runtime.apply_human_frame_feedback(human,flags);return;
    }
    if(human.base().id14()<0)return;
    refresh();advance_human_float_timer(human.activation,elapsed);
    world.refresh_human_frame_perception(human); //005A50A0 THEN pooled005A5380, even when first is a dead-Human RET.
    if((world.mode_flags()&7u)==7u&&world.sight_controlled_target(human))world.adjust_human_sight_alignment(human,.5f);
    world.advance_human_command_events(human,elapsed);runtime.advance_human_state(human,true,elapsed); //B0+28 AFTER queues.
    if((world.mode_flags()&1u)||(world.mode_flags()&7u)==7u)directions();
    world.human_inline(human).advance_authority_frame(elapsed,runtime);
    if(!(world.mode_flags()&2u)){
        const auto flags=world.human_inline(human).consume_frame_feedback();runtime.apply_human_frame_feedback(human,flags);
    }
    if(signed_word(human.properties.stats_ec[0])<=0)return;
    sample_human_frame_motion(human,frames,elapsed);
    const auto& velocity=human.activation.vector1b0;
    const auto square=(static_cast<long double>(velocity[2])*velocity[2]+static_cast<long double>(velocity[1])*velocity[1])+static_cast<long double>(velocity[0])*velocity[0];
    if(square>=1.0000000168623835e-16f&&(state().words().template get<std::uint32_t>(0x5c)&0x16000u))runtime.human_frame_contact(human); //005A5C80 returns5C;006FCB60 radius.5/maskFFFBFFFF and actual actor005FEFB0.
    if((world.mode_flags()&7u)==7u&&human.properties.field19c==1)world.advance_human_periodic_stats(human,elapsed); //005A88F0.
    if(human.formation)runtime.advance_human_formation(*human.formation,elapsed); //Actual2C8 receiver0059BA70.
}
} //namespace hd2::game
#endif
