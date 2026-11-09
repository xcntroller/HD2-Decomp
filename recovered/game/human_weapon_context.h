#ifndef HD2_GAME_HUMAN_WEAPON_CONTEXT_H
#define HD2_GAME_HUMAN_WEAPON_CONTEXT_H

#include "game/human_weapon_selection.h"
#include "game/model_scene_actors.h"

namespace hd2::game {

//Actual0052F750/00530B30 context reads. DD0's existing sector and scene1F8
//share identity. Car5E0 remains a mandatory actual runtime read, not a default.
template<class Frames,class Human,class Inline,class World>
HumanWeaponSelectionContext human_weapon_context(Frames& frames,Human& human,Inline& control,World& world,
        SceneActor<Frames>* target,bool alternate,bool priorities) {
    HumanWeaponSelectionContext context;context.alternate=alternate;context.has_target=target!=nullptr;
    if(target) {
        const auto type=target->base().type();context.human_target=type==3;
        if(type==4)context.special_target=world.car_field5e0(target)!=0;
        else if(type==0x19) {
            const auto* animation=dynamic_cast<AnimationSceneActor<Frames>*>(target);
            if(!animation)throw std::runtime_error("Weapon target19 has no actual Animation actor owner");
            context.special_target=animation->fieldd8==2;
        }
    }
    if(priorities)context.root_sector=control.scene_sector()==frames.camera_root_sector();
    if(target&&(priorities?context.root_sector:!context.special_target)) {
        //Original updates target matrix before Human matrix. Subtractions stay
        //x87; ONLY usability stores the final result to FLOAT.
        const auto target_matrix=frames.actor_world_matrix(target->base().frame());
        const auto human_matrix=frames.actor_world_matrix(human.base().frame());
        const long double x=static_cast<long double>(human_matrix[12])-target_matrix[12];
        const long double y=static_cast<long double>(human_matrix[13])-target_matrix[13];
        const long double z=static_cast<long double>(human_matrix[14])-target_matrix[14];
        context.squared_distance=priorities?(z*z+x*x)+y*y:static_cast<long double>(static_cast<float>((z*z+y*y)+x*x));
    }
    return context;
}
} //namespace hd2::game
#endif
