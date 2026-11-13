#ifndef HD2_GAME_HUMAN_REACTION_RETIREMENT_H
#define HD2_GAME_HUMAN_REACTION_RETIREMENT_H
#include "game/human_reaction_selection.h"
namespace hd2::game {
//0074D510's selected-reaction tail, sharing the containing DD0's EXACT owners.
//Keep pool dereference at motion-audio retirement, after earlier mutations.
//No owner is copied/moved or replaced with a successful missing receiver.
template<class Control,class Look,class Human,class Physical,class Face,class Action,
    class Tracks,class Movement,class Pool,class Services,class Frames,class Random,class Publish>
void finish_human_reaction_selection(Control& control,Look& look,Human& human,Physical& physical,
        Face& face,Action& action,Tracks& tracks,Movement& movement,Pool& pool,Services& services,
        Frames& frames,Random& random,HumanPhysicsGlobals& globals,std::uint16_t reason,Publish& publish){
    using Frame=typename Control::Frame;
    auto& state=physical.control_frame_words();
    const auto word=[&](unsigned offset){return state.template get<std::int32_t>(offset);};
    const auto queue=[&](std::uint32_t id){state.set(0x1fc,id);normalize_human_primary_animation(state,0x1fc);};
    physical.retire_step_secondary();state.set(0x984,0u);state.set(0x980,0u);
    const auto attachment=word(0xa44),active=word(0x1d8),seat=word(0xa40);
    auto* target=physical.control_interaction_target();
    if(attachment>=6&&attachment<=8&&active!=0x9f){
        services.remove_human_reaction_seat(target,seat);
        human.field160=nullptr;human.field164=-1;state.set(0x1cc,1.f);state.set(0x1d0,1.f);
    }else if(attachment==4&&active>0x36&&active<0x3f&&target&&(services.human_reaction_seat_flags(target,seat)&0x200u)){
        const auto world=frames.actor_world_matrix(human.base().frame());
        const auto forward=human_retained_clip::normalize(HumanVector{{world[8],world[9],world[10],world[11]}});
        frames.human_reaction_model_direction(human.base().frame(),HumanVector{{-forward[0],forward[1],-forward[2],forward[3]}});
        if(random()%3)queue(globals.reaction_sequence%3+0x83);
        if(word(0x1fc)==globals.last_mounted_reaction&&random()%3)queue(globals.reaction_sequence%3+0x83);
        if((seat==3&&word(0x1fc)==0x84)||(seat==2&&word(0x1fc)==0x83))queue(0x8a);
        globals.last_mounted_reaction=word(0x1fc);
    }
    physical.stop_frame_voice();state.set(0x1b0,word(0x1ac));
    if(word(0x1ac)!=1)state.set(0x18c,std::uint8_t{1});
    if(!word(0x1ac))face.activate_air_pose();
    for(unsigned kind=0;kind!=5;++kind)look.control_movement_mode(kind,false,publish);
    auto& commands=look.frame_words();commands.set(0xd4,0u);commands.set(0xd0,0x8e57u);commands.set(0xc0,0u);
    auto& equipment_owner=services.human_presentation_equipment(human);
    equipment_owner.state().words().set(0xcc,2u); //Human+A8 is inline98's230 equipment borrower.
    action.retire_reaction_commands();tracks.fade_pair(1,350);tracks.fade_pair(2,350);
    const HumanVector zero{};
    if(word(0x1ac)==4||word(0x1ac)==6){
        tracks.motion_impulse(zero,0);tracks.secondary_motion_impulse(zero,0);tracks.motion_deceleration(zero,0);
    }
    state.set(0x1e8,0x5cu);
    equipment_owner.transfers().event(2);equipment_owner.toggle_frame_action(false);
    face.frame_running(false,true,services.human_control_animation_scale());
    movement.retire_motion_audio(frames,pool->pool());
    if(physical.control_action_actor())services.human_ladder(physical.control_action_actor()).remove_occupant(human);
    physical.publish_reaction_descriptors();
    auto velocity=state.template get<HumanVector>(0x9d0);
    if(human_retained_clip::dot(velocity,velocity)>=1.0000000168623835e-16f){
        if(word(0x1ac)==3)velocity=human_retained_clip::scale(velocity,4.f);
        tracks.motion_deceleration(velocity,1000);
    }
    tracks.fade_fourth(1);tracks.update(100,false,nullptr); //Empty0077CE10 branch, then007724E0.
    if(!word(0xa30)||word(0xa30)==-10000){equipment_owner.toggle_frame_action(false);equipment_owner.transfers().event(10);}
    else control.retire_look_mount(look,equipment_owner.state().selected_model(9));
    state.set(0x9d0,-10000.f); //Native writes only X after consuming the vector.
    Frame clip{};const auto property=look.state().template get<std::int32_t>(0xa0);
    if(property==1||property==2)clip=services.human_animation_resource(static_cast<unsigned>(property-1),
        static_cast<unsigned>(word(0x1fc)*3+look.state().template get<std::int32_t>(0x4c)));
    const auto model=human.base().frame();
    if(word(0x1ac)<7){
        if(!attachment||(attachment>=6&&attachment<=8&&active!=0x9f))frames.human_reaction_link(model,Frame{});
        frames.place_human_reaction_model(model);frames.refresh_dynamic_human_model(model);
    }else{state.set(0x1ac,0u);state.set(0x18c,std::uint8_t{1});face.activate_air_pose();reason=2;}
    control.begin_geometry_effect(look.state().template get<std::uint32_t>(0x60),look.state().template get<HumanVector>(0x70),clip,reason,nullptr);
}
} //namespace hd2::game
#endif
