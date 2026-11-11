#ifndef HD2_GAME_HUMAN_EQUIPMENT_ACTION_H
#define HD2_GAME_HUMAN_EQUIPMENT_ACTION_H

#include "game/human_equipment_state.h"

namespace hd2::game {

//007933D0: native lens parameters and countdown; model and child are borrowed.
template<class Frames>void human_equipment_action_lens(Frames& frames,
        HumanEquipmentState<typename Frames::Frame>& state,bool enabled) {
    const auto model=state.model(1);if(!model)return;
    const auto lens=frames.actor_find(model,"lensf",1);if(!lens)return;
    state.words().set(0x70,std::int32_t{enabled?1071:-10000}); //trunc(1500.f/1.4f),00810D54/0084E80C.
    frames.human_equipment_lens_parameters(lens,enabled);
    if(enabled)frames.object_visible(lens,true);
    frames.update_scene_object(lens);
}

//007931B0, including the required offline NULL-model no-op and populated
//fire/lens branches. Neither action-model creation nor a simulation tick is
//provided here; actual producers must supply native1C and its original refs.
template<class Frames>void toggle_human_equipment_action(Frames& frames,
        HumanEquipmentState<typename Frames::Frame>& state,typename Frames::Frame light,
        std::uint32_t mode,bool enabled) {
    if((mode&7)!=7||!state.model(1))return;
    const auto fire=frames.actor_find(state.model(1),"fire",0xffff);
    const auto lens=frames.actor_find(state.model(1),"lensf",1);
    if(fire) {
        if(!light)throw std::runtime_error("Human action light is unavailable");
        if(!enabled) {
            if(const auto smoke=frames.actor_find(state.model(1),"light_smoke",0xffff)) {
                frames.link_scene_object(smoke,typename Frames::Frame{});frames.release_item_frame(smoke);
            }
            state.words().set(0x6c,std::int32_t{-10000});
            frames.human_equipment_remove_light_sectors(light);
            human_equipment_action_lens(frames,state,false);
            frames.link_scene_object(state.model(1),typename Frames::Frame{});
            frames.release_item_frame(state.model(1));frames.release_item_frame(state.model(1));
            state.model(1,typename Frames::Frame{});return;
        }
        frames.human_equipment_action_light(light,fire);
        state.words().set(0x6c,std::int32_t{1071});
        frames.human_equipment_place_light(light);
    }
    if(lens){frames.object_visible(lens,enabled);frames.update_scene_object(lens);}
}

template<class Frames,class State,class Services>
void attach_human_equipment_action(Frames& frames,State& state,Services& services,unsigned kind){ //0078DF60 cases5..8.
    using Frame=typename Frames::Frame;
    const auto model=state.model(1);if(!model)return;
    auto& words=state.words();words.set(0xb0,kind);
    auto retained=frames.retain_actor_frame(model);retained.release();
    words.set(0x90,frames.actor_world_rotation(model));
    if(kind==5||kind==8){
        const auto matrix=frames.actor_world_matrix(model);
        words.set(0xa0,std::array<float,4>{{matrix[12],matrix[13],matrix[14],matrix[15]}});
    }
    frames.human_equipment_link(model,Frame{},0);
    const auto volume=kind==8||words.template get<std::uint8_t>(0x2c)?state.first_volume():state.second_volume();
    frames.human_equipment_link(model,volume,2);
    frames.object_visible(model,!services.equipment_human_hidden());
    frames.object_position(model,{{0,0,0}});frames.update_scene_object(model);
    if(frames.visual_type(model)==0x5f4a424fu)frames.human_equipment_opacity(model,services.equipment_opacity());
}

//00790520 cases5..7, after the caller has selected the native B0 retirement mode.
template<class Frames,class State>
void retire_human_equipment_hand_action(Frames& frames,State& state,unsigned kind){
    using Frame=typename Frames::Frame;
    const auto model=state.model(1);if(!model)return;
    auto& words=state.words();
    if(kind==7&&frames.object_parent(model))frames.release_item_frame(model);
    words.set(0x54,std::int32_t{-10000});frames.human_equipment_link(model,Frame{},1);
    if(kind==5)frames.human_equipment_link(model,frames.camera_root_sector(),0);
    frames.object_rotation(model,words.template get<std::array<float,4>>(0x90));
    if(kind==5){
        const auto position=words.template get<std::array<float,4>>(0xa0);
        frames.object_position(model,{{position[0],position[1],position[2]}});
    }
    frames.update_scene_object(model);
    if(kind==5){frames.auto_parent_object(model);frames.update_scene_object(model);}
    frames.release_item_frame(model);state.model(1,Frame{});words.set(0xb0,11u);
}

//0078DE30 tail, AFTER light release/weapon clear/four attachment roots.
//Saved90 quaternion/A0 position belong to the actual action producer.
template<class Frames>void retire_human_equipment_action_model(Frames& frames,
        HumanEquipmentState<typename Frames::Frame>& state,std::uint32_t mode) {
    const auto model=state.model(1);
    if((mode&7)==7&&model) {
        frames.link_scene_object(model,typename Frames::Frame{});
        frames.human_equipment_restore_action(model,
            state.words().template get<std::array<float,4>>(0xa0),
            state.words().template get<std::array<float,4>>(0x90));
        frames.auto_parent_object(model);frames.update_scene_object(model);frames.release_item_frame(model);
    }
    state.model(1,typename Frames::Frame{});
}
} //namespace hd2::game
#endif
