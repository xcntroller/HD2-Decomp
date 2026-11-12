#ifndef HD2_GAME_HUMAN_EQUIPMENT_ACTION_H
#define HD2_GAME_HUMAN_EQUIPMENT_ACTION_H

#include "game/human_equipment_state.h"
#include "game/human_ai_shot_direction.h"

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
void attach_human_equipment_action(Frames& frames,State& state,Services& services,unsigned kind){ //0078DF60 cases5..7.
    using Frame=typename Frames::Frame;
    const auto model=state.model(1);if(!model)return;
    auto& words=state.words();words.set(0xb0,kind);
    auto retained=frames.retain_actor_frame(model);retained.release();
    words.set(0x90,frames.actor_world_rotation(model));state.action_rotation_written();
    if(kind==5){
        const auto matrix=frames.actor_world_matrix(model);
        words.set(0xa0,std::array<float,4>{{matrix[12],matrix[13],matrix[14],matrix[15]}});
    }
    frames.human_equipment_link(model,Frame{},0);
    const auto volume=words.template get<std::uint8_t>(0x2c)?state.first_volume():state.second_volume();
    frames.human_equipment_link(model,volume,2);
    frames.object_visible(model,!services.equipment_human_hidden());
    frames.object_position(model,{{0,0,0}});frames.update_scene_object(model);
    if(frames.visual_type(model)==0x5f4a424fu)frames.human_equipment_opacity(model,services.equipment_opacity());
}

//0078DF60 cases4/8. Preserve internal frame10 bounds, actual HW1/HW2,
//Human40 and WORLD pose; case4's invalid X bounds keep only B0/ref mutations.
template<class Frames,class State,class Services>
void attach_human_equipment_held_action(Frames& frames,State& state,Services& services,unsigned kind){
    using Frame=typename Frames::Frame;
    if(!state.model(1))return;
    const auto model=[&]{const auto value=state.model(1);if(!value)throw std::logic_error("Held action lost actual model");return value;};
    auto& words=state.words();words.set(0xb0,kind);
    auto retained=frames.retain_actor_frame(model());retained.release();
    const auto bounds_model=model();
    if(kind==4){const auto box=frames.mission_frame_world_bounds(bounds_model);if(!(box.minimum[0]<box.maximum[0]))return;}
    words.set(0x90,frames.actor_world_rotation(model()));state.action_rotation_written();
    const auto saved=frames.actor_world_matrix(model());
    words.set(0xa0,std::array<float,4>{{saved[12],saved[13],saved[14],saved[15]}});
    if(kind==4){
        const auto box=frames.human_equipment_bounds_data(bounds_model);
        HumanVector centre{};
        const float half_x=static_cast<float>((static_cast<long double>(box.maximum[0])-box.minimum[0])*.5f);
        centre[0]=static_cast<float>(static_cast<long double>(half_x)+box.minimum[0]);
        centre[1]=static_cast<float>((static_cast<long double>(box.maximum[1])-box.minimum[1])*.5f+box.minimum[1]);
        const float extent_z=static_cast<float>(static_cast<long double>(box.maximum[2])-box.minimum[2]);
        centre[2]=static_cast<float>(static_cast<long double>(extent_z)*.5f+box.minimum[2]);
        if(!state.first_volume()||!state.second_volume())throw std::logic_error("Held action requires actual HW1/HW2");
        const auto first=frames.actor_world_matrix(state.first_volume());
        const auto second=frames.actor_world_matrix(state.second_volume());
        HumanVector half{};
        for(unsigned n=0;n!=3;++n){
            if(n==2){const float difference=static_cast<float>(static_cast<long double>(second[14])-first[14]);half[n]=static_cast<float>(static_cast<long double>(difference)*.5f);}
            else half[n]=static_cast<float>((static_cast<long double>(second[12+n])-first[12+n])*.5f);
        }
        const auto fresh=frames.actor_world_matrix(state.first_volume());HumanVector target{};
        for(unsigned n=0;n!=3;++n)target[n]=static_cast<float>(static_cast<long double>(half[n])+fresh[12+n]);
        const auto human_direction=services.equipment_human_direction();
        const auto matrix=frames.actor_world_matrix(model());HumanVector direction{{matrix[8],matrix[9],matrix[10],matrix[11]}};
        normalize_human_ai_shot(direction,(static_cast<long double>(direction[1])*direction[1]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[0])*direction[0]);
        target[0]=static_cast<float>(static_cast<long double>(human_direction[0])*.1f+target[0]);
        HumanVector shift{};
        shift[0]=static_cast<float>(static_cast<long double>(target[0])-centre[0]);
        shift[1]=static_cast<float>(static_cast<long double>(target[1])-centre[1]);
        shift[2]=static_cast<float>((static_cast<long double>(human_direction[2])*.1f+target[2])-centre[2]);
        const auto position=frames.actor_world_matrix(model());
        for(unsigned n=0;n!=3;++n)shift[n]=static_cast<float>(static_cast<long double>(shift[n])+position[12+n]);
        frames.human_mounted_model_position(model(),shift);
        frames.human_equipment_action_link(model(),state.first_volume(),3u);
        frames.human_reaction_model_direction(model(),direction);
    }else{
        frames.human_equipment_action_link(model(),Frame{},0u);
        frames.human_equipment_action_link(model(),state.first_volume(),2u);
    }
    frames.human_equipment_action_visibility(model(),services.equipment_human_identity(),services.equipment_human_hidden());
    if(kind==8)frames.object_position(model(),{{0,0,0}});
    frames.update_scene_object(model());
    if(kind==4){services.equipment_secondary_face(2);services.equipment_primary_face(2);}
    if(frames.visual_type(model())==0x5f4a424fu)frames.human_equipment_opacity(model(),services.equipment_opacity());
}

//00790520 cases4/8. The repeated Y addition and branch-specific B0 tail are
//native behavior (79089D..8B1/790CCD..CE1), not a conventional ground snap.
template<class Frames,class State,class Services>
void retire_human_equipment_held_action(Frames& frames,State& state,Services& services,unsigned kind){
    using Frame=typename Frames::Frame;
    if(!state.model(1))return;
    const auto model=[&]{const auto value=state.model(1);if(!value)throw std::logic_error("Held retirement lost actual model");return value;};
    auto& words=state.words();words.set(0x54,std::int32_t{-10000});
    frames.human_equipment_action_link(model(),Frame{},1u);
    frames.human_equipment_action_link(model(),frames.camera_root_sector(),3u);
    frames.auto_parent_object(model()); //Actual SAME SceneEC BEFORE saved quaternion.
    if(!state.has_action_rotation())throw std::logic_error("Held retirement requires original saved90 pose");
    frames.object_rotation(model(),words.template get<std::array<float,4>>(0x90));
    const auto matrix=frames.actor_world_matrix(services.equipment_human_model());
    HumanVector direction{{matrix[8],matrix[9],matrix[10],matrix[11]}};
    const auto length=(static_cast<long double>(direction[1])*direction[1]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[0])*direction[0];
    auto x=static_cast<long double>(direction[0]);
    constexpr float epsilon=9.99999993922529e-09f;
    if(!(std::abs(length-1)<epsilon)){
        const float stored=static_cast<float>(length);
        if(!(stored<epsilon)){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));x*=inverse;direction[2]=static_cast<float>(direction[2]*inverse);}
        else if(x!=0)x=x<0?-1:1;
        else if(direction[2]!=0)direction[2]=direction[2]<0?-1.f:1.f;
        //Native Y fallback does not affect the X/Z destination.
    }
    const float offset_x=static_cast<float>(x*.6f),offset_z=static_cast<float>(static_cast<long double>(direction[2])*.6f);
    const auto human=frames.actor_world_matrix(services.equipment_human_model());
    HumanVector position{{static_cast<float>(static_cast<long double>(offset_x)+human[12]),human[13],static_cast<float>(static_cast<long double>(offset_z)+human[14]),0}};
    frames.human_mounted_model_position(model(),position);frames.update_scene_object(model());
    const auto box=frames.mission_frame_world_bounds(model());
    position[1]=static_cast<float>((static_cast<long double>(position[1])-box.minimum[1])+position[1]);
    frames.human_mounted_model_position(model(),position);frames.update_scene_object(model());
    frames.release_item_frame(model());state.model(1,Frame{});
    services.equipment_secondary_face(0);services.equipment_primary_face(0);
    if(kind==8)words.set(0xb0,11u);
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
