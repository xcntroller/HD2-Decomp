#ifndef HD2_GAME_HUMAN_CLOTHING_CONTROL_H
#define HD2_GAME_HUMAN_CLOTHING_CONTROL_H
#include "game/human_joint_owners.h"
namespace hd2::game {
//00786C20. Capture old borrowed bone pose BEFORE releasing each retained bone.
//Rebind model-owned borrowers without restarting DD0, then restore each NEW
//bone and retain it immediately. Same model identity/animation state survives.
template<class Control,class Human,class Frames>void rebind_human_clothing_control(Control& control,Human& human,Frames& frames){
    const auto model=human.base().frame();frames.unregister_human_model(model);
    std::array<decltype(frames.car_rotation(model)),25> rotations;std::array<HumanVector,25> positions;
    for(unsigned n=0;n!=25;++n){
        const auto bone=control.physical().bones()[n];if(!bone)throw std::logic_error("Clothing refresh lost actual Human bone");
        rotations[n]=frames.car_rotation(bone);positions[n]=frames.car_position(bone);control.retire_clothing_bone(n);
    }
    control.refresh_clothing_bindings();
    for(unsigned n=0;n!=25;++n){
        const auto bone=control.physical().bones()[n];if(!bone)throw std::logic_error("Clothing model lacks required Human bone");
        frames.object_rotation(bone,rotations[n]);const auto& p=positions[n];frames.object_position(bone,{{p[0],p[1],p[2]}});
        control.retain_clothing_bone(n);
    }
    frames.register_replaced_human_model(model);frames.flag_human_model(model);frames.update_scene_object(model);
}
} //namespace hd2::game
#endif
