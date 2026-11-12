#ifndef HD2_GAME_HUMAN_ITEM_FIRE_H
#define HD2_GAME_HUMAN_ITEM_FIRE_H
#include "game/human_item_shot_sound.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {
//Actual weapon virtual24/28:007D4FD0/007D5210. Selected projectile/melee
//receivers are required World operations, never successful admission doubles.
template<class Item,class Human,class World>
std::uint32_t fire_human_weapon_item(Item& item,Human& human,const HumanVector& position,
        const HumanVector& direction,std::uint32_t argument,bool primary,World& world){
    if(item.family()!=Item::Family::weapon)throw std::runtime_error("Item fire requires actual selected instance-family dispatch");
    if(!primary){
        if(item.definition().kind()==9)world.emit_item_grenade(item,human,position,direction,argument);
        return 0;
    }
    ++item.field6c; //Before payload classification and the kind9 branch.
    const auto payload_kind=item.definition().payloads[0].type?item.primary_payload_kind():10;
    if(item.definition().kind()==9){world.emit_item_grenade(item,human,position,direction,argument);return 0;}
    const bool silent=(argument&0xffu)!=0;
    if(payload_kind==3){
        const auto command=item.definition().equipment_command();
        const bool trace=(command==1||command==4)&&human_frame_signed_word(item.field6c)%3==0;
        world.emit_item_bullet(item,human,position,direction,trace,silent);
        auto parent=item.shot_frame();play_human_item_shot(item,human,parent,silent,world);
        play_human_following_shot_sounds(item,human,world);item.subtract_amount(1.f);
        item.field68=world.game_word28()-item.field64;item.field64=world.game_word28();return 0;
    }
    if(payload_kind==2){
        auto parent=item.projectile_effect_frame();
        if(parent)world.emit_item_muzzle_particle(item,parent);
        auto projectile=item.projectile_frame();world.emit_item_capsule(item,human,position,direction,projectile,argument);
        if(auto model=item.projectile_frame())world.human_command_frames().object_visible(model,false);
        if(!parent)parent=item.model();
        play_human_item_shot(item,human,parent,silent,world);
        item.field68=world.game_word28()-item.field64;item.field64=world.game_word28();item.subtract_amount(1.f);return 0;
    }
    if(payload_kind==0&&(world.mode_flags()&1u))world.apply_item_melee(item,human,position,direction);
    return 0; //Native unhandled primary payloads and non-grenade secondary are genuine returns.
}
} //namespace hd2::game
#endif
