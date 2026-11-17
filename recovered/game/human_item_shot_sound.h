#ifndef HD2_GAME_HUMAN_ITEM_SHOT_SOUND_H
#define HD2_GAME_HUMAN_ITEM_SHOT_SOUND_H

#include "frontend/spatial_sound.h"
#include "game/human_joint_owners.h"

namespace hd2::game {
//007D5A80, direct local SOUND-frame playback plus a distinct throttled manager
//request for authority/hearing. The SOUND frames are owned by the actual item.
template<class Item,class Human,class Frame,class World>
void play_human_item_shot(Item& item,Human& human,Frame parent,bool silent,World& world){
    if(!item.field88)return;
    if(world.mode_flags()&2u){
        const auto index=world.human_frame_random_word()%item.field88;
        if(const auto sound=world.item_shot_sound_frame(item,index)){
            auto& frames=world.human_command_frames();
            frames.human_sound_link(sound,parent);
            frames.human_sound_parameter94(sound,0);
            const bool first=world.human_is_player(human)&&world.human_view_offset_enabled();
            frames.human_sound_parameter7c(sound,first?3u:1u);
            frames.car_sound_head(sound,HumanVector{{0,0,first?0.f:.7f,0}});
            frames.update_scene_object(sound);frames.human_sound_enabled(sound,true);
        }
    }
    if((world.mode_flags()&1u)&&!silent&&world.game_word28()-item.field8c>1000u){
        item.field8c=world.game_word28();
        auto& request=world.player_item_sound_request();request.group=2;
        request.entry=item.primary_action_sound();request.source=&human;request.position={{0,0,0}};
        request.relative=parent;request.flags=0x22;
        const auto id=item.definition().identifier;request.far_scale=(id==0x16u||id==7u||id==0x104u)?.3f:1.f;
        (void)world.emit_human_equipment_sound(request);
    }
}

//007D6330. These separate SOUND-manager requests precede ammunition removal.
//Only XYZ/group/entry/source/parent/flags are written; other request fields keep
//their process lifetime, including007D5A80's authority far-scale.
template<class Item,class Human,class World>
void play_human_following_shot_sounds(Item& item,Human& human,World& world){
    if(!(world.mode_flags()&2u))return;
    const auto emit=[&](unsigned group,bool last){
        auto& request=world.player_item_sound_request();
        const bool first=world.human_is_player(human)&&world.human_view_offset_enabled();
        request.position={{0,0,first?0.f:.7f}};
        request.relative=first?item.shot_frame():item.model();request.flags=first?0x41u:1u;
        request.group=group;request.entry=last?item.last_round_sound():item.following_shot_sound();request.source=nullptr;
        (void)world.emit_human_equipment_sound(request);
    };
    if(item.amount()==1.f&&item.last_round_sound()>=0)emit(2,true);
    if(item.following_shot_sound()>=0)emit(3,false);
}
} //namespace hd2::game
#endif
