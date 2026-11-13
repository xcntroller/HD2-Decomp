#ifndef HD2_GAME_HUMAN_DAMAGE_PRESENTATION_H
#define HD2_GAME_HUMAN_DAMAGE_PRESENTATION_H
#include "game/human_damage_commands.h"
#include "game/human_ai_shot_direction.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {
//005A15D0 is the received damage/presentation path, distinct from005A1C90.
//Its hit point remains packet-relative; only cosmetic effects add model XYZ.
template<class Human,class World,class Frames>void present_human_damage(Human& human,const HumanDamageCommand& command,
        World& world,Frames& frames){
    const auto mode=world.mode_flags();auto& health=human.properties.stats_ec[0];
    if(command.kind==14){
        health=command.health;if((mode&7u)==7u||human_frame_signed_word(health)>0)return;
        const auto y=static_cast<long double>(frames.volume_random_unit())-.5f;
        const auto x=static_cast<long double>(frames.volume_random_unit())-.5f;
        const auto z=static_cast<long double>(frames.volume_random_unit())-.5f;
        HumanVector force{{static_cast<float>(x+x),static_cast<float>(y+y),static_cast<float>(z+z),0}};
        const auto scale=static_cast<long double>(frames.volume_random_unit())*10.f+1.f;
        for(unsigned axis=0;axis!=3;++axis)force[axis]=static_cast<float>(force[axis]*scale);
        world.human_damage_hit(human,8,world.human_damage_head_position(human),force);
        world.human_damage_presentation_cleanup(human);return;
    }
    const float amount=static_cast<float>(human_frame_signed_word(health-command.health));
    if(mode&1u){
        if(human_frame_signed_word(health)<1)world.human_damage_presentation_cleanup(human);
        world.human_damage_message(human,command.kind,command.hit_kind,command.source);return;
    }
    health=command.health;std::uint32_t material=0x42,mask=8;
    const auto matrix=frames.actor_world_matrix(human.base().frame());auto position=command.point;
    for(unsigned axis=0;axis!=3;++axis)position[axis]+=matrix[12+axis];
    switch(command.hit_kind){
    case 0:mask=8;break;case 1:mask=1;break;case 2:case 3:case 4:case 5:mask=2;break;
    case 6:case 7:mask=0x10;break;case 8:case 9:mask=0x20;break;case 11:mask=1;material=12;break;default:break;
    }
    switch(command.kind){case 0:mask|=0x1000u;break;case 9:mask|=0x800u;break;case 10:mask|=0x400u;break;
        case 11:mask|=0x2000u;break;case 13:mask|=0x4000u;break;case 15:mask|=0x8000u;break;default:break;}
    if(command.kind!=5){
        auto force=command.direction;
        const auto square=(static_cast<long double>(force[1])*force[1]+static_cast<long double>(force[2])*force[2])+static_cast<long double>(force[0])*force[0];
        normalize_human_ai_shot(force,square);for(unsigned axis=0;axis!=3;++axis)force[axis]*=amount;
        world.human_damage_hit(human,mask,command.point,force);
    }
    if(human_frame_signed_word(health)<1){
        if(human.properties.field19c==1&&(mode&7u)!=7u)world.human_damage_score(human,command.source);
        world.human_damage_presentation_cleanup(human);
    }
    world.human_damage_presentation_player(human,command.kind);
    if(command.kind==1||command.kind==3){
        world.human_damage_impact_sound(human,material,position,true);
        if(command.kind==1)world.human_damage_blood_surface(position,command.direction);
        auto reverse=command.direction;
        const auto square=(static_cast<long double>(reverse[2])*reverse[2]+static_cast<long double>(reverse[1])*reverse[1])+static_cast<long double>(reverse[0])*reverse[0];
        normalize_human_ai_shot(reverse,square);for(unsigned axis=0;axis!=3;++axis)reverse[axis]=-reverse[axis];
        world.human_damage_material_effect(material,position,reverse);
    }else if(command.kind==2)world.human_damage_impact_sound(human,material,position,true);
    world.human_damage_message(human,command.kind,command.hit_kind,command.source);
}
} //namespace hd2::game
#endif
