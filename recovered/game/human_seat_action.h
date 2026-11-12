#ifndef HD2_GAME_HUMAN_SEAT_ACTION_H
#define HD2_GAME_HUMAN_SEAT_ACTION_H

#include "game/human_water_motion.h"
#include "game/human_door_action.h"

namespace hd2::game {
inline bool human_seat_entry_action(std::uint32_t action){return action==2||action==4||action==6||action==11||action==13;}
inline bool human_seat_exit_action(std::uint32_t action){return action==3||action==5||action==7||action==12;}

template<class Control>void reset_human_seat_motion_requests(Control& control){ //00751480/00755500 C00 request triples.
    auto& motion=control.motion_words();
    for(unsigned offset:{0x20u,0x38u,0x4cu}){motion.set(offset,std::int32_t{-10000});motion.set(offset+4,0u);}
}
template<class Words>void clear_human_seat_action(Words& words){
    words.set(0xa44,0u);words.set(0xa38,std::int32_t{-10000});words.set(0xa34,0u);words.set(0x228,0u);
}

template<class Control,class Look,class Human,class Services,class Frames,class Runtime,class Publish>
void advance_human_seat_action(Control& control,Look& look,Human& human,Services& services,Frames& frames,
        Runtime& runtime,Publish& publish,std::uint32_t elapsed){ //00755500 cases2..7/B/C/D.
    auto& physical=control.physical();auto& words=physical.control_frame_words();auto& request=look.frame_words();
    const auto flags=[&]{return words.template get<std::uint32_t>(0xa34);};
    const auto action=flags()&0x3ffu;const auto timer=words.template get<std::int32_t>(0xa38);
    if(!flags()||timer==-10000)return;
    const auto seat=[&]{return words.template get<std::int32_t>(0xa40);};
    const auto target=[&]{return physical.control_interaction_target();};
    const auto lock=[&](std::uint32_t mask,bool set){request.set(0xd4,set?request.template get<std::uint32_t>(0xd4)|mask:request.template get<std::uint32_t>(0xd4)&~mask);};
    if(human_seat_entry_action(action)){
        if(timer>=-999&&timer<=0&&(flags()&0x400u)){
            frames.unregister_human_model(human.base().frame());
            if(services.human_seat_occupant(target(),seat())&&services.human_seat_occupant(target(),seat())!=&human){
                for(unsigned mask:{1u,2u,8u,0x40u,0x4000u})lock(mask,false);
                clear_human_seat_action(words);physical.control_interaction_target(nullptr);return;
            }
            services.human_seat_assign(target(),seat(),&human);
            human.field160=target();human.field164=static_cast<std::uint32_t>(seat());
            auto f=words.template get<std::uint32_t>(0xa44);std::uint32_t posture=0,animation;
            if(f&4u)animation=45;
            else if(f&8u){animation=55;posture=1;}
            else if(f&0x40u){animation=55;posture=2;}
            else if(f&0x80u)animation=action==4?111u:155u;
            else animation=f&1u?158u:159u;
            words.set(0x1fc,animation);normalize_human_primary_animation(words,0x1fc);
            words.set(0x200,posture);normalize_human_primary_animation(words,0x1fc);
            look.force_frame_commands(true);
            look.frame_movement_animation(words.template get<std::int32_t>(0x1fc),publish,false);
            words.set(0x7bc,words.template get<std::uint32_t>(0x200));
            look.frame_posture(words.template get<std::uint32_t>(0x200),publish,false);
            control.geometry().seat_posture(words.template get<std::uint32_t>(0x200));
            look.force_frame_commands(false);
            control.select_geometry_attachment(words.template get<std::uint32_t>(0xa44),target(),static_cast<std::uint32_t>(seat()));
            words.set(0x1cc,0.f);words.set(0x1d0,0.f);
            bool clear_shadow=true;f=words.template get<std::uint32_t>(0xa44);
            auto& equipment=services.human_presentation_equipment(human);
            const auto command=[&](std::uint32_t value){words.set(0x218,value);look.force_frame_commands(true);look.action_command(value,false);look.force_frame_commands(false);};
            if((f&1u)&&!services.human_pose_actor_has_control(target(),static_cast<std::uint32_t>(seat()))){
                words.set(0x21c,0u);command(70);equipment.set_hidden(true);words.set(0xa44,1u);
            }else if(f&0x100u){
                const auto kind=services.human_seat_weapon_kind(target(),seat());
                words.set(0xa44,kind?*kind+6u:9u);
                const auto mode=words.template get<std::int32_t>(0xa44);
                if(mode<9){
                    words.set(0x218,76u);
                    const auto clip=words.template get<std::uint32_t>(0x1fc)==159?typename Frames::Frame{}:
                        services.human_animation_resource(10,static_cast<unsigned>(mode-6)+posture*3);
                    if(clip)clear_shadow=false;
                    control.geometry().capture_action_overlay(clip);
                }else{words.set(0x218,71u);words.set(0x21c,0u);words.set(0xa44,3u);}
                command(words.template get<std::uint32_t>(0x218));equipment.set_hidden(true);
            }else if(!(f&0x1du)&&(f&0x20u)){
                equipment.set_hidden(false);command(76);words.set(0xa44,2u);
            }else if((f&0xcu)&&(f&0x20u)){
                words.set(0x218,76u);equipment.set_hidden(false);words.set(0xa44,4u);
            }else{words.set(0x218,76u);words.set(0xa44,5u);equipment.set_hidden(true);}
            words.set(0xa38,std::int32_t{-2000});words.set(0x1c8,0.f);
            control.arbitrate_seat_animation(look,runtime,publish);
            control.tracks().update(2000,false);
            const auto mode=words.template get<std::uint32_t>(0xa44);
            lock(0x400u,mode!=2&&mode!=4);lock(0x800u,true);lock(8u,true);
            if(equipment.state().selected_model(3))frames.object_visible(equipment.state().selected_model(3),mode==2||mode==4);
            for(unsigned kind:{1u,2u})if(equipment.state().selected_model(kind))frames.object_visible(equipment.state().selected_model(kind),false);
            if(control.shadow().frame()&&clear_shadow)control.shadow().clear();
            if((control.mode_flags()&2u)&&services.current_human_frame_player()==&human)services.refresh_human_seat_visibility();
            words.set(0xa38,std::int32_t{-10000});lock(0x4000u,false);
            if(mode>2&&mode<5)lock(0x800u,false);
            if(services.human_reaction_seat_flags(target(),seat())&0x10u)frames.object_visible(human.base().frame(),false);
        }
    }else if(human_seat_exit_action(action)){
        if(!target())words.set(0xa38,std::int32_t{-10000});
        else if(timer<1)words.set(0xa38,20000u);
        else if(timer>10000){
            if(flags()&0x400u){
                if(!services.human_seat_exit(target(),seat())){
                    lock(0x4000u,false);words.set(0xa38,std::int32_t{-10000});words.set(0xa34,0x402u);words.set(0x228,0u);return;
                }
                frames.register_human_model(human.base().frame());
                control.select_geometry_attachment(words.template get<std::uint32_t>(0xa44),nullptr,static_cast<std::uint32_t>(seat()));
                human.field160=nullptr;human.field164=0xffffffffu;control.movement().frame_words().set(0xd8,0u);
                if(services.human_reaction_seat_flags(target(),seat())&0x10u)frames.object_visible(human.base().frame(),true);
                words.set(0x1fc,45u);normalize_human_primary_animation(words,0x1fc);
                words.set(0x200,request.template get<std::uint32_t>(0x4c));normalize_human_primary_animation(words,0x1fc);
                words.set(0x218,76u);look.force_frame_commands(true);look.frame_movement_animation(45,publish,false);look.action_command(76,false);look.force_frame_commands(false);
                control.arbitrate_seat_animation(look,runtime,publish);
                auto& equipment=services.human_presentation_equipment(human);
                for(unsigned kind:{3u,1u,2u})if(equipment.state().selected_model(kind))frames.object_visible(equipment.state().selected_model(kind),true);
                if((flags()&0x3ffu)!=5)equipment.set_hidden(false);
                for(unsigned mask:{1u,2u,0x40u,0x800u})lock(mask,false);
                control.tracks().discard_pending_tracks(0);reset_human_seat_motion_requests(control);
                words.set(0x1c8,.8f);words.set(0x1cc,1.f);words.set(0x1d0,1.f);
                const auto mode=words.template get<std::uint32_t>(0xa44);if(mode!=2&&mode!=4)lock(0x400u,false);
                look.hit_command(0,HumanVector{},HumanVector{},frames,services);
                control.tracks().update(2000,false);
                const auto world=frames.actor_world_matrix(human.base().frame());human.base().world_position({{world[12],world[13],world[14],world[15]}});
                control.publish_seat_ground_position();words.set(0xa44,0u);
                control.finish_step_fall(look,runtime,publish);
                auto& motion=control.movement().frame_words();motion.set(0x110,0u);motion.set(0x94,1000u);motion.set(0x98,std::uint8_t{1});
                control.refresh_equipment_visibility();physical.refresh_ground_geometry();motion.set(0xfc,100000.f);
                if(control.shadow().frame())frames.update_scene_object(control.shadow().frame());
            }
            if((control.mode_flags()&2u)&&services.current_human_frame_player()==&human)services.refresh_human_seat_visibility();
            words.set(0xa38,std::int32_t{-10000});words.set(0xa34,0u);physical.control_interaction_target(nullptr);words.set(0x228,0u);lock(0x4000u,false);lock(8u,false);
        }
    }
    const auto remaining=words.template get<std::int32_t>(0xa38);
    if(remaining!=-10000)words.set(0xa38,static_cast<std::uint32_t>(remaining)-elapsed);
}

template<class Control,class Look,class Human,class Actor,class Services,class Frames,class Runtime,class Publish>
bool begin_human_seat_action(Control& control,Look& look,Human& human,Actor* target,std::uint32_t flags,std::uint32_t argument,
        Services& services,Frames& frames,Runtime& runtime,Publish& publish){ //00751480 cases2..7/B/C/D.
    auto& physical=control.physical();auto& words=physical.control_frame_words();auto& request=look.frame_words();
    const auto action=flags&0x3ffu;
    if(human_seat_entry_action(action)){
        if(request.template get<std::uint32_t>(0xb8)==5)finish_human_water_motion(control,look,human,services,publish,false);
        services.human_seat_clear_frame_command(human);
        std::int32_t previous=-1; //751495 writes this stack slot BEFORE OutputDebugStringA pops its one argument.
        const auto old=words.template get<std::uint32_t>(0xa34)&0x3ffu;
        if(old==2||old==4||old==11||old==13){
            services.remove_human_reaction_seat(target,words.template get<std::int32_t>(0xa40));frames.register_human_model(human.base().frame());
            previous=words.template get<std::int32_t>(0xa40);words.set(0xa38,std::int32_t{-1});
        }else words.set(0xa38,control.human_is_player()&&(control.mode_flags()&7u)==7u?500u:5u);
        request.set(0xd4,request.template get<std::uint32_t>(0xd4)|0x4000u);
        words.set(0xa40,argument);physical.control_action_frame(typename Frames::Frame{});
        words.set(0xa44,services.human_reaction_seat_flags(target,human_frame_signed_word(argument)));
        words.set(0xa34,flags);physical.control_interaction_target(target);words.set(0xa48,request.template get<std::uint32_t>(0x4c));
        control.tracks().discard_pending_tracks(0);
        for(unsigned mask:{1u,2u,8u,0x40u})request.set(0xd4,request.template get<std::uint32_t>(0xd4)|mask);
        reset_human_seat_motion_requests(control);
        if(auto* occupant=services.human_seat_occupant(target,human_frame_signed_word(argument));occupant&&occupant->base().type()==3){
            if(services.human_seat_health(*occupant)<=0){
                const auto matrix=frames.actor_world_matrix(occupant->base().frame());const HumanVector position{{matrix[12],matrix[13],matrix[14],matrix[15]}};
                services.remove_human_reaction_seat(target,human_frame_signed_word(argument));
                frames.register_human_model(occupant->base().frame());occupant->field160=nullptr;occupant->field164=0xffffffffu;
                services.human_seat_retire_dead(*occupant,position);frames.refresh_dynamic_human_model(human.base().frame());
            }else services.human_seat_displace(*occupant,target,previous<0?(flags&0xfffffc03u)|3u:flags,previous<0?argument:static_cast<std::uint32_t>(previous),runtime);
        }
        if(!control.human_is_player()||(control.mode_flags()&7u)!=7u){words.set(0xa38,0u);advance_human_seat_action(control,look,human,services,frames,runtime,publish,100);}
    }else if(human_seat_exit_action(action)){
        request.set(0xd4,request.template get<std::uint32_t>(0xd4)|0x4000u);
        const auto matrix=frames.actor_world_matrix(human.base().frame());human.base().world_position({{matrix[12],matrix[13],matrix[14],matrix[15]}});
        words.set(0xa40,argument);words.set(0xa34,flags);control.movement().frame_words().set(0xfc,100000.f);
        if(control.human_is_player()&&(control.mode_flags()&7u)==7u)words.set(0xa38,500u);
        else{words.set(0xa38,10001u);advance_human_seat_action(control,look,human,services,frames,runtime,publish,100);}
    }
    return true;
}
} //namespace hd2::game
#endif
