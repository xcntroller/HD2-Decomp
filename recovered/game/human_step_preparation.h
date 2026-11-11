#ifndef HD2_GAME_HUMAN_STEP_PREPARATION_H
#define HD2_GAME_HUMAN_STEP_PREPARATION_H

#include "game/human_ladder_control.h"

namespace hd2::game {

//0074A690. The shared step/climb clearance worker is also called at a Ladder's
//top exit. It uses the SAME queued primary descriptor, physical spheres and E0.
template<class Control,class Look,class Human,class Services,class Frames,class Runtime,class Publish>
bool prepare_human_step_motion(Control& control,Look& look,Human& human,Services& services,
        Frames& frames,Runtime& runtime,Publish& publish){
    const bool offline=(control.mode_flags()&7u)==7u;
    if(!offline&&!control.human_is_player()&&!control.animation_authority_actor())return true;
    if(human.activation.vector1b0[1]<=-.08f)return false;
    auto& physical=control.physical();auto& state=physical.control_frame_words();
    const auto word=[&](unsigned offset){return state.template get<std::int32_t>(offset);};
    if(word(0xa2c)!=-10000||word(0xa30)!=-10000||(word(0x1d8)>7&&word(0x1d8)<15))return false;
    const auto initial=physical.posture_spheres()[0]; //Native copies BEFORE rebind0075BC80.
    const auto direction=human.base().world_direction();
    const auto posture=look.state().template get<std::uint32_t>(0x4c);
    const bool defer=posture!=0||word(0x9ac)!=-10000;
    const unsigned probes=word(0x1d8)>189&&look.state().template get<std::uint32_t>(0x50)==5?2u:3u;
    state.set(0x96c,std::uint8_t{0});physical.rebind_frame_posture(posture);
    physical.prepare_ladder_secondary();
    physical.exclude_query(false);
    struct Restore {decltype(physical)& owner;~Restore(){owner.restore_query(false);}} restore{physical};
    const float radius=initial[3],base_y=initial[1]-radius;
    float surface=-10000.099609375f,step=-10000.099609375f;
    HumanVector origin{{static_cast<float>(static_cast<long double>(direction[0])*.4f+initial[0]),
        static_cast<float>((static_cast<long double>(base_y)+radius)+2.3f),
        static_cast<float>(initial[2]+static_cast<long double>(direction[2])*.4f),0}};
    auto& sweep=services.human_geometry_sweep();
    for(unsigned probe=0;probe!=probes;++probe){
        const auto hit=sweep.sweep(origin,{{0,-2.3f,0,0}},radius,0xcc202u,0x230000u);
        bool selected=false;
        if(hit){
            const auto* descriptor=hit->dynamic_identity;
            if(descriptor&&(descriptor->flags&0x40u)&&descriptor->frame){
                const auto* data=services.human_geometry_contact_data(*descriptor);
                const auto* actor=data?reinterpret_cast<const typename Control::SceneInstance*>(data->words[0]):nullptr;
                if(actor&&actor->base().type()==21)break;
            }
            if(frames.car_vector_angle(hit->normal,{{0,1,0,0}})<.5235987901687622f)surface=hit->position[1];
            if(static_cast<long double>(base_y)+2.5f<surface)break;
            if(static_cast<long double>(base_y)+.25f<surface){
                step=surface;selected=true;
                HumanVector start{{static_cast<float>(hit->position[0]+static_cast<long double>(direction[0])*.275f),
                    static_cast<float>(static_cast<long double>(hit->position[1])+.1f),
                    static_cast<float>(hit->position[2]+static_cast<long double>(direction[2])*.275f),0}};
                if(!services.human_geometry_ray(start,{{0,static_cast<float>(-.35f-static_cast<long double>(radius)),0,0}},0xcc202u,0x230000u))
                    state.set(0x96c,std::uint8_t{1});
            }
        }
        if(selected)break;
        origin[0]=static_cast<float>(static_cast<long double>(direction[0])*.15f+origin[0]);
        origin[2]=static_cast<float>(static_cast<long double>(direction[2])*.15f+origin[2]);
    }
    if(step==-10000.099609375f)return false;
    const auto surface_offset=static_cast<long double>(step)-radius;
    const float stored_offset=static_cast<float>(surface_offset);
    if(surface_offset-base_y<=.45f&&!control.movement().frame_words().template get<std::uint32_t>(0xf0))return false;
    const float rise=stored_offset-base_y;
    HumanVector forward{{direction[0]*.45f,0,direction[2]*.45f,direction[3]}};
    HumanVector start{{initial[0],base_y,initial[2],0}};
    if(word(0x988)!=-10000){
        const auto& ladder=services.human_ladder_geometry(physical.control_action_actor());
        start[0]=static_cast<float>(static_cast<long double>(ladder.direction()[0])*-.3f+initial[0]);
        start[2]=static_cast<float>(static_cast<long double>(ladder.direction()[2])*-.3f+initial[2]);
    }
    const auto raised_extended=static_cast<long double>(rise)+radius;
    const float raised=static_cast<float>(raised_extended);
    const float height=static_cast<float>(raised_extended-.06f);
    const float query_radius=static_cast<float>((static_cast<long double>(radius)-.06f)-.02f);
    const auto mask=[&]{return word(0x988)==-10000?0x88002u:0x8c002u;};
    if(const auto hit=sweep.sweep(start,{{0,height,0,0}},query_radius,mask(),0)){
        CollisionMotion<std::decay_t<decltype(*hit)>> contact;
        contact.position=hit->position;contact.append(*hit,0);
        (void)resolve_human_motion_contacts(control,look,human,services,contact,true,runtime);
        return false;
    }
    start={{initial[0],raised+base_y,initial[2],0}};
    if(sweep.sweep(start,forward,query_radius,mask(),0x230000u))return false;
    start={{static_cast<float>(initial[0]+static_cast<long double>(forward[0])*.8f),
        static_cast<float>((static_cast<long double>(height)+base_y)+static_cast<long double>(forward[1])*.8f),
        static_cast<float>(initial[2]+static_cast<long double>(forward[2])*.8f),0}};
    const auto clearance=sweep.sweep(start,{{0,state.template get<float>(0x744)-.12f,0,0}},query_radius,mask(),0x230000u);
    bool shortened=false;
    if(clearance){
        const auto available=(static_cast<long double>(clearance->position[1])-rise)-base_y;
        const auto full=static_cast<long double>(state.template get<float>(0x744))*.95f+radius;
        if(!(full<=available)){
            if(available<static_cast<long double>(state.template get<float>(0x744))*.45f+radius)return false;
            shortened=true;
        }
    }
    if(!defer){
        physical.publish_step_secondary(forward,shortened?raised:rise,shortened);
        state.set(0x958,rise);
        if(word(0x988)==-10000){
            if(rise<=.45f)return false;
            std::uint32_t animation=10;
            if(rise>=1.5499999523162842f)animation=13;
            else if(rise>=1.0779999494552612f)animation=12;
            else if(rise>=.6f)animation=11;
            state.set(0x1fc,animation);
        }else state.set(0x1fc,14u);
        normalize_human_primary_animation(state,0x1fc);
    }
    if((word(0x1fc)==10||word(0x1fc)==9)&&shortened)return false;
    if(defer){
        auto& queue=physical.control_action_queue();if(!queue.empty())return false;
        queue.push_back({{1500,4,0}});queue.push_back({{500,posture==1?1u:2u,0}});
        look.frame_posture(0,publish);
        return false;
    }
    state.set(0x200,0u);normalize_human_primary_animation(state,0x1fc);
    if(!offline){
        switch(word(0x1fc)){
        case 10:runtime.notify_human_ladder_probe(human,6u);break;
        case 11:runtime.notify_human_ladder_probe(human,1u);break;
        case 12:runtime.notify_human_ladder_probe(human,2u);break;
        case 13:runtime.notify_human_ladder_probe(human,3u);break;
        case 14:runtime.notify_human_ladder_probe(human,0u);break;
        default:break;
        }
    }
    state.set(0x954,std::uint32_t(shortened));
    look.force_frame_commands(true);
    try{look.position_command({{0,0,0,0}});}catch(...){look.force_frame_commands(false);throw;}
    look.force_frame_commands(false);
    HumanVector output{};control.tracks().motion_velocity({{0,0,0,0}},0,0,output);
    control.movement().control_frame_track_velocity(output);
    for(unsigned axis=0;axis!=3;++axis)state.set(0x4d0+axis*4,0.f);
    return true;
}

} //namespace hd2::game
#endif
