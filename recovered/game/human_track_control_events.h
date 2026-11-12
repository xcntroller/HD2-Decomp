#ifndef HD2_GAME_HUMAN_TRACK_CONTROL_EVENTS_H
#define HD2_GAME_HUMAN_TRACK_CONTROL_EVENTS_H
#include "game/human_joint_owners.h"
#include "game/human_animation_requests.h"
namespace hd2::game {
//00758D20 control-owned events. Original event7 publishes A44 BEFORE testing
//authority, and only then reads A40/actor/side. Equipment20 always reaches lens
//after the optional action toggle; failures retain the reached prefix.
template<class Control,class Human,class Services>
void apply_human_track_door(Control& control,Human& human,Services& services){
    auto& state=control.physical().control_frame_words();
    if((state.template get<std::uint32_t>(0xa34)&0x3ffu)!=1u)return;
    state.set(0xa44,std::int32_t{-1});
    if(!(services.mode_flags()&1u))return;
    services.human_track_request_door(control.physical().control_interaction_target(),
        state.template get<std::uint32_t>(0xa40)==0u?3u:2u,human.properties.field19c!=1u);
}
template<class Control,class Equipment>
void apply_human_track_action_light(Control& control,Equipment& equipment){
    if(control.physical().control_frame_words().template get<std::uint32_t>(0x1f4)==0x11u)
        equipment.toggle_frame_action(true); //007931B0(1).
    equipment.set_frame_action_lens(true); //007933D0(1), even without toggle.
}
template<class Control,class Human,class Services>
void apply_human_track_deceleration(Control& control,const Human& human,Services& services){
    control.physical().control_frame_words().set(0x9c9,std::uint8_t{1});
    const auto direction=human.base().world_direction(); //Actual Human40, not B0 direction.
    HumanVector velocity{}; //Native fourth scratch lane has no writer or semantic consumer.
    velocity[0]=direction[0]*.05f;velocity[1]=.1f;velocity[2]=direction[2]*.05f;
    const auto duration=human_animation_time_integer(700.L/services.human_control_animation_scale()); //SAME live0084E80C.
    control.tracks().motion_deceleration(velocity,duration);
}
template<class Control>void apply_human_track_motion_reset(Control& control){
    control.physical().control_frame_words().set(0x228,std::int32_t{-2000});
    control.movement().frame_words().set(0xd8,0u); //SAME DD0 movement owner, AFTER physical228.
}
}
#endif
