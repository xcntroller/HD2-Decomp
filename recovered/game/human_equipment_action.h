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
