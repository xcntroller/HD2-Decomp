#ifndef HD2_GAME_HUMAN_MODEL_ACTION_H
#define HD2_GAME_HUMAN_MODEL_ACTION_H

#include "game/human_door_action.h"
#include "game/human_ai_shot_direction.h"
#include "game/human_animation_requests.h"

namespace hd2::game {

template<class Control,class Look,class Services,class Frames,class Actor,class Publish>
bool begin_human_model_action(Control& control,Look& look,Services& services,Frames& frames,
        Actor* target,std::uint32_t flags,std::uint32_t argument,typename Frames::Frame model,Publish& publish){ //00751480 case8.
    auto& physical=control.physical();auto& words=physical.control_frame_words();auto& request=look.frame_words();
    words.set(0xa44,5u); //Native publishes this before its rejection gates.
    const auto posture=request.template get<std::uint32_t>(0xb8),animation=words.template get<std::uint32_t>(0x1d8);
    if(posture==2||posture==3||animation==159||animation==158||animation==155||animation==111||animation==34||animation==29||animation==38)return false;
    request.set(0xd4,request.template get<std::uint32_t>(0xd4)|0x4000u);
    if(argument)throw std::logic_error("Human model action requires its actual typed frame argument");
    words.set(0xa40,0u);physical.control_action_frame(model);
    if(flags&0x400u){
        words.set(0x1fc,28u);normalize_human_primary_animation(words,0x1fc);
        look.frame_movement_animation(28,publish,false);
        words.set(0x200,0u);normalize_human_primary_animation(words,0x1fc);
        look.frame_posture(0,publish,false);
    }
    control.tracks().discard_pending_tracks(0);
    words.set(0xa34,flags);
    words.set(0xa38,human_primary_animation_duration(look,services,frames,28,0,1));
    physical.control_interaction_target(target);
    return true;
}

template<class Control,class Look,class Human,class Services,class Frames>
void advance_human_model_action(Control& control,Look& look,Human& human,Services& services,
        Frames& frames,std::uint32_t elapsed){ //00755500 case8.
    auto& physical=control.physical();auto& words=physical.control_frame_words();
    const auto timer=words.template get<std::int32_t>(0xa38);
    if(timer>0){
        const auto duration=human_primary_animation_duration(look,services,frames,28,0,1);
        const auto frame=physical.control_action_frame();
        if(timer==duration&&frame&&(words.template get<std::uint32_t>(0xa34)&0x400u)){
            const auto model=human.base().frame();
            frames.unregister_human_model(model);
            auto matrix=frames.actor_world_matrix(frame);
            HumanVector direction{{matrix[8],matrix[9],matrix[10],matrix[11]}};
            const auto square=(static_cast<long double>(direction[0])*direction[0]+
                static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[2])*direction[2];
            normalize_human_ai_shot(direction,square);
            matrix=frames.actor_world_matrix(frame);
            const HumanVector position{{matrix[12],matrix[13]-.5f,matrix[14],matrix[15]}};
            auto& request=look.frame_words();request.set(0xd4,request.template get<std::uint32_t>(0xd4)|0x800u);
            human.base().world_position(position);human.base().world_direction(direction);
            frames.human_action_model_pose(model,position,direction,frame);
            services.human_presentation_equipment(human).set_hidden(true);
        }
    }else{
        words.set(0xa38,std::int32_t{-10000});
        auto& request=look.frame_words();request.set(0xd4,request.template get<std::uint32_t>(0xd4)&~0x4000u);
    }
    const auto remaining=words.template get<std::int32_t>(0xa38);
    if(remaining!=-10000)words.set(0xa38,static_cast<std::uint32_t>(remaining)-elapsed);
}

template<class Control,class Look,class Services,class Publish>
bool begin_human_model_exit(Control& control,Look& look,Services& services,std::uint32_t flags,Publish& publish){ //00751480 cases9/10.
    auto& words=control.physical().control_frame_words();auto& request=look.frame_words();
    const auto posture=request.template get<std::uint32_t>(0xb8),animation=words.template get<std::uint32_t>(0x1d8);
    if((posture!=2&&posture!=3&&animation!=159&&animation!=158&&animation!=155&&animation!=111&&animation!=34)||animation==29||animation==38)return true;
    request.set(0xd4,request.template get<std::uint32_t>(0xd4)|0x4000u);
    if(flags&0x400u){
        const auto desired=(flags&0x3ffu)==10u?38u:29u;
        words.set(0x1fc,desired);normalize_human_primary_animation(words,0x1fc);
        look.frame_movement_animation(desired,publish,false);
    }
    control.tracks().discard_pending_tracks(0);
    words.set(0xa34,flags);
    words.set(0xa38,human_animation_time_integer(900.L/services.human_control_animation_scale()));
    control.movement().frame_words().set(0x130,human_animation_time_integer(500.L/services.human_control_animation_scale()));
    return true; //A3C/A40 remain the actual previously established actor/model borrowers.
}

template<class Control,class Look,class Human,class Services,class Frames>
void advance_human_model_exit(Control& control,Look& look,Human& human,Services& services,
        Frames& frames,std::uint32_t elapsed){ //00755500 cases9/10.
    auto& physical=control.physical();auto& words=physical.control_frame_words();
    const auto timer=words.template get<std::int32_t>(0xa38);
    if(timer>0){words.set(0xa38,static_cast<std::uint32_t>(timer)-elapsed);return;}
    auto& request=look.frame_words();request.set(0xd4,request.template get<std::uint32_t>(0xd4)&~0x4000u);
    const auto frame=physical.control_action_frame(),model=human.base().frame();
    frames.register_human_model(model); //Native z/q lookup and scene138 happen even without the positive frame/400 gate.
    if(frame&&(words.template get<std::uint32_t>(0xa34)&0x400u)){
        const auto marker=frames.actor_find(frame,"__*",0x10020);
        HumanVector displacement,position;
        if(marker){
            auto matrix=frames.actor_world_matrix(marker);
            const HumanVector target{{matrix[12],matrix[13],matrix[14],matrix[15]}};
            (void)frames.actor_world_matrix(marker); //Native reads the marker direction again before the Human pose queries.
            (void)frames.actor_world_matrix(model); //Its normalized forward is unused by the native result.
            matrix=frames.actor_world_matrix(model);
            position={{matrix[12],matrix[13],matrix[14],matrix[15]}};
            displacement=position; //Native temporary W is unwritten. Retain known pose padding.
            for(unsigned axis=0;axis!=3;++axis)displacement[axis]=target[axis]-position[axis];
        }else{
            const auto matrix=frames.actor_world_matrix(model);
            HumanVector direction{{matrix[8],matrix[9],matrix[10],matrix[11]}};
            const auto square=(static_cast<long double>(direction[1])*direction[1]+
                static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[0])*direction[0];
            normalize_human_ai_shot(direction,square);
            direction[1]=0;
            const auto planar=static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[2])*direction[2];
            normalize_human_ai_shot(direction,planar);
            const auto current=frames.actor_world_matrix(model);
            position={{current[12],current[13],current[14],current[15]}};
            displacement=position;
            for(unsigned axis=0;axis!=3;++axis)displacement[axis]=static_cast<float>(static_cast<long double>(direction[axis])*.4000000059604645f);
        }
        const auto duration=human_animation_time_integer(700.L/services.human_control_animation_scale());
        control.tracks().secondary_motion_impulse(displacement,duration);
        frames.refresh_dynamic_human_model(model);
        human.base().world_position(position);
        if(marker)words.set(0x228,std::int32_t{-2000});
        services.human_presentation_equipment(human).set_hidden(false);
        words.set(0xa44,0u);control.refresh_equipment_visibility();
        control.movement().frame_words().set(0xfc,100000.f);
        physical.refresh_ground_geometry();
        if(const auto shadow=control.shadow().frame())frames.update_scene_object(shadow);
    }
    request.set(0xd4,request.template get<std::uint32_t>(0xd4)&~0x800u);
    words.set(0xa38,std::int32_t{-10000});words.set(0xa34,0u);
    physical.control_interaction_target(nullptr);words.set(0x228,0u);
}

} //namespace hd2::game
#endif
