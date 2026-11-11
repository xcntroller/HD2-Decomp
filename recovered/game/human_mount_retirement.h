#ifndef HD2_GAME_HUMAN_MOUNT_RETIREMENT_H
#define HD2_GAME_HUMAN_MOUNT_RETIREMENT_H

#include "game/human_joint_owners.h"

namespace hd2::game {

//0075E290. Input is the actual equipment model's EC-bound Human borrower.
//The target DD0 supplies the actual007B0910 reason4/non-null-source branch.
template<class Control,class Look,class Human,class Services,class Frames>
void retire_human_control_mount(Control& control,Look& look,Human& human,Human* mounted,
        Services& services,Frames& frames){
    auto& equipment=services.human_presentation_equipment(human);
    equipment.set_hidden(false);
    for(unsigned kind:{2u,3u})if(equipment.state().selected_model(kind))
        frames.object_visible(equipment.state().selected_model(kind),true);
    const auto command=look.state().template get<std::uint32_t>(0x54);
    auto& physical=control.physical();auto& state=physical.control_frame_words();
    if(!command||command==6)state.set(0x20c,92u);
    state.set(0xa30,std::int32_t{-10000});equipment.state().words().set(0xd0,0u);
    equipment.transfers().event(14);
    if(mounted){
        auto& inline_owner=services.human_inline(*mounted);auto* target=inline_owner.control();
        if(!target)throw std::runtime_error("Mounted Human retirement requires its actual DD0");
        target->refresh_equipment_visibility(); //007866D0→007920B0.
        inline_owner.shadow_visible(frames,true);
        auto& target_physical=target->physical();auto& target_state=target_physical.control_frame_words();
        target_state.set(0x1b4,std::uint8_t{0});target_state.set(0x1cc,1.f);target_state.set(0x1d0,1.f);
        target_physical.rebind_frame_posture(services.human_look_control(*mounted).state().template get<std::uint32_t>(0x4c));
        target_physical.refresh_shot_geometry();
        const auto model=mounted->base().frame();
        if(!model)throw std::runtime_error("Mounted Human retirement lost its actual model");
        frames.restore_human_mount_visuals(model);
        physical.refresh_shot_geometry();physical.publish_retired_mount_shots();
        target->begin_mounted_geometry_effect(human); //007B0910, SAME target geometry DC0.
        frames.object_visible(model,true);
        services.restore_human_mount_camera_amount(model);
    }
    control.refresh_equipment_visibility(); //007866D0, current Human AFTER target publication.
    auto& words=look.frame_words();words.set(0xd0,words.template get<std::uint32_t>(0xd0)&~8u);
}

} //namespace hd2::game
#endif
