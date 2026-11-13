#ifndef HD2_GAME_HUMAN_MOUNT_ASSIGNMENT_H
#define HD2_GAME_HUMAN_MOUNT_ASSIGNMENT_H
#include "game/human_camera_amount.h"
#include <stdexcept>
namespace hd2::game {
//0075DF70. Actual source/target DD0,230,DC0,Scene/SOUND borrowers. No target
//is a legitimate branch, but source equipment attachment15 still runs.
template<class Control,class Look,class Human,class Services,class Frames>
void assign_human_control_mount(Control& control,Look& look,Human& human,Human* mounted,
        Services& services,Frames& frames){
    auto& equipment=services.human_presentation_equipment(human);
    equipment.set_hidden(true);
    for(unsigned kind:{3u,2u})if(equipment.state().selected_model(kind))
        frames.object_visible(equipment.state().selected_model(kind),false);
    auto& state=control.physical().control_frame_words();state.set(0xa30,1u);
    equipment.state().words().set(0xd0,1.f);
    look.force_frame_commands(true);state.set(0x20c,88u);look.force_frame_commands(false);
    if(mounted){
        const auto target=[&]{auto* value=services.human_inline(*mounted).control();if(!value)throw std::logic_error("Mount assignment requires actual target DD0");return value;};
        target()->physical().set_frame_motion_mode(0u,true);
        auto& physical=target()->physical();auto& words=physical.control_frame_words();
        words.set(0x1cc,0u);words.set(0x1d0,0u);
        physical.rebind_frame_posture(services.human_look_control(*mounted).state().template get<std::uint32_t>(0x4c));
        if(mounted->field160){
            const auto type=mounted->field160->base().type();
            if(type==4u||type==20u||type==21u||type==34u){
                const auto parent=frames.object_parent(mounted->base().frame());
                if(parent&&frames.frame_kind(parent)!=5u){
                    if(auto* actor=target()->physical().control_interaction_target()){
                        //Actual actor virtual80/native A40; unavailable families fail
                        //at this precise receiver, before Scene registration/clears.
                        services.detach_human_attachment(*actor,target()->physical().control_frame_words().template get<std::int32_t>(0xa40));
                        frames.register_human_model(mounted->base().frame()); //lookup z,then q; Scene138(model,q,z).
                        mounted->field160=nullptr;target()->physical().control_interaction_target(nullptr);mounted->field164=-1;
                    }
                }
            }
        }
        target()->geometry().retire_mount_geometry_effect(services);
        for(unsigned slot=0;slot!=8;++slot){
            const auto model=target()->tracks().model(); //Actual target DD0+260 first word, fresh each slot.
            if(!model)throw std::logic_error("Mount assignment lost actual track model");
            frames.human_model_animation(model,typename Frames::Frame{},slot,1.f,0u);
        }
        target()->physical().control_frame_words().set(0x1b4,std::uint8_t{1});
        frames.suppress_human_mount_visuals(mounted->base().frame());
        services.apply_human_mount_camera_amount(mounted->base().frame(),human_camera_amount(human));
    }
    equipment.transfers().event(15);
    control.refresh_equipment_visibility();
    auto& words=look.frame_words();words.set(0xd0,words.template get<std::uint32_t>(0xd0)|8u);
}

//00758D20 event8. The retirement case resets A2C BEFORE reading its bit8.
//Mount assignment/retirement resolve actual model9 EC Human at this call site.
template<class Control,class Look,class Human,class Equipment,class Services,class Frames>
void apply_human_track_action_transition(Control& control,Look& look,Human& human,
        Equipment& equipment,Services& services,Frames& frames){
    auto& state=control.physical().control_frame_words();
    const auto mode=state.template get<std::uint32_t>(0xa2c)&~8u;
    if(!mode){
        state.set(0xa2c,state.template get<std::uint32_t>(0xa2c)|1u);
        look.force_frame_commands(true);look.equipment_command(1,false);look.force_frame_commands(false);
        equipment.transfers().event((state.template get<std::uint32_t>(0xa2c)&8u)?13u:5u);
        equipment.refresh(human.base().frame());return;
    }
    if(mode==2u){
        state.set(0xa2c,std::int32_t{-10000});
        equipment.transfers().event((state.template get<std::uint32_t>(0xa2c)&8u)?12u:4u);
        equipment.refresh(human.base().frame());return;
    }
    const auto attachment=state.template get<std::int32_t>(0xa30);
    if(attachment==0){
        const auto model=equipment.state().selected_model(9);
        auto* actor=model?services.template human_mounted_model_actor<Human>(model):nullptr;
        assign_human_control_mount(control,look,human,actor,services,frames);
    }else if(attachment==2)control.retire_look_mount(look,equipment.state().selected_model(9));
}
}
#endif
