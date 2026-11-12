#ifndef HD2_GAME_HUMAN_TRACK_EQUIPMENT_EVENTS_H
#define HD2_GAME_HUMAN_TRACK_EQUIPMENT_EVENTS_H
#include <cstdint>
#include "game/human_animation_requests.h"

namespace hd2::game {
//00758170(false), reached by00758D20 track0. The true caller has a separate
//timed animation transition and is not substituted by this track receiver.
template<class Control,class Equipment,class Frames>
void apply_human_track_inventory_transfer(Control& control,Equipment& equipment,Frames& frames){
    auto& state=control.physical().control_frame_words();
    const auto secondary=state.template get<std::uint32_t>(0x1e8);
    if(secondary!=0x22u&&secondary!=0x23u&&secondary!=0x24u)return;
    if(state.template get<std::uint32_t>(0xa34)!=0u&&state.template get<std::uint32_t>(0xa34)!=1u&&
       state.template get<std::uint32_t>(0xa44)!=2u&&state.template get<std::uint32_t>(0xa44)!=4u)return;
    const auto shadow=control.shadow().frame();
    const bool visible=shadow&&frames.human_control_visible(shadow);
    equipment.transfers().event(secondary==0x22u?2u:3u);
    const auto current=control.shadow().frame(); //Fresh actual shadow AFTER callback-visible transfer.
    if(current&&!visible)frames.object_visible(current,false);
}
//00758D20 event9. Native signed E4 gate and extra5A release are deliberate.
//Selected model7 remains borrowed until the original NULL-selection call.
template<class Control,class Look,class Equipment,class Frames,class Services,class Publish>
void apply_human_track_action_retirement(Control& control,Look& look,Equipment& equipment,
        Frames& frames,Services& services,Publish& publish){
    const auto secondary=control.animation_state_word(0x1e8);
    if(secondary==0x46u)look.mark_frame_feedback(0x20000u);
    else if(secondary==0x4cu)look.mark_frame_feedback(0x1000u);
    else if(secondary==0x4du)look.mark_frame_feedback(0x2000u);
    else if(secondary==0x4fu||secondary==0x5au){
        control.animation().set_equipment_frame_timer(human_animation_time_integer(1400.L/services.human_control_animation_scale()));
        const auto model=equipment.state().selected_model(7);
        if(model){
            const auto references=frames.item_frame_references(model);
            if(references>=2){frames.release_item_frame(model);frames.release_item_frame(model);}
            else frames.release_item_frame(model);
            if(secondary==0x5au)frames.release_item_frame(model);
        }
        equipment.state().select(typename Frames::Frame{},7,0xffffffffu,-10000,-1);
        look.mark_frame_feedback(secondary==0x4fu?0x4000u:0x8000u);
    }
    if(look.state().template get<std::uint32_t>(0xb8)==4u)look.control_movement_mode(3,false,publish);
}

//00758D20 events13/14 query CURRENT animation metadata00775670(2).
//Only event13 has the secondary4F/5A fallback. Recovered transfer receivers
//attach/retire actual hand-action models5/6/7, with their existing lifetimes.
template<class Tracks,class Control,class Equipment>
void apply_human_track_hand_transfer(std::int32_t event,Tracks& tracks,Control& control,Equipment& equipment){
    const auto tag=tracks.active_event_tag(2);
    if(event==13){
        if(tag==0x50)equipment.transfers().event(11);
        else if(tag==0x52)equipment.transfers().event(7);
        else if(tag==0x53)equipment.transfers().event(9);
        else{
            const auto secondary=control.animation_state_word(0x1e8);
            if(secondary==0x4f||secondary==0x5a)equipment.transfers().event(11);
        }
    }else if(event==14){
        if(tag==0x50)equipment.transfers().event(10);
        else if(tag==0x52)equipment.transfers().event(6);
        else if(tag==0x53)equipment.transfers().event(8);
    }
}
}
#endif
