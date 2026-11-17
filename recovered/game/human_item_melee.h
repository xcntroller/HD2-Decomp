#ifndef HD2_GAME_HUMAN_ITEM_MELEE_H
#define HD2_GAME_HUMAN_ITEM_MELEE_H
#include "game/human_joint_owners.h"
#include "frontend/pre_mission_data.h"

namespace hd2::game {
//00451230's ordinary (undercut=false) entry selected by007D4FD0. Query
//exclusions end BEFORE descriptor/actor inspection; D4 receives impact then
//the scaled displacement. Other actor kinds are material picks, not damage.
template<class Item,class Human,class World>void strike_human_item(
        Item& item,Human& source,const HumanVector& start,const HumanVector& direction,World& world){
    const auto& payload=item.definition().payloads[0];
    if(payload.bytes.size()<0x10)throw std::runtime_error("Melee requires actual primary payload10");
    const float damage=frontend::pre_mission_detail::Bytes::real(frontend::pre_mission_detail::Bytes::word(payload.bytes.data()+0xc));
    HumanVector displacement{};
    for(unsigned axis=0;axis!=3;++axis)displacement[axis]=static_cast<float>(static_cast<long double>(direction[axis])*1.2000000476837158f);
    const auto hit=[&]{
        world.item_melee_exclude(source);
        struct Restore{World& world;Human& human;~Restore(){world.item_melee_restore(human);}}restore{world,source};
        return world.human_geometry_ray(start,displacement,0,0,true);
    }();
    if(!hit)return;
    std::uint32_t material=world.human_geometry_flags(*hit)>>24;
    if((world.human_geometry_flags(*hit)&0x40u)&&hit->dynamic_identity&&hit->dynamic_identity->frame){
        auto* actor=world.item_melee_actor(*hit);if(!actor)return;
        const auto kind=actor->base().type();
        if(kind==3||kind==21){
            auto* target=world.item_melee_human(*actor);if(!target)return;
            const auto zone=world.item_melee_hit_zone(*target,start,displacement);
            //004558A0's debug strings end in actual0069BFE0 RET.
            if(zone==0xff){world.item_melee_miss(*target,source,displacement);return;}
            material=0x42; //0059C2A0 Human D8, genuine constant; not a new body credit.
            world.apply_human_damage(*target,3,damage,hit->position,displacement,&source,zone);
        }else if(kind==30){
            material=world.item_melee_material(*actor,decltype(hit->dynamic_identity->frame){});
            world.item_melee_dog_damage(*actor,damage,hit->position,displacement,source);
        }else{
            const auto frame=world.human_command_frames().car_damage_pick(start,displacement,actor->base().frame(),0);
            if(!frame)return;
            material=world.item_melee_material(*actor,frame);
        }
    }
    world.item_melee_material_effect(material,hit->position,hit->normal);
}
}
#endif
