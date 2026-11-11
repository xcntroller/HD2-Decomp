#ifndef HD2_GAME_HUMAN_FOOTSTEP_H
#define HD2_GAME_HUMAN_FOOTSTEP_H
#include "game/human_motion_velocity.h"
#include "game/human_state_frame_timers.h"
#include "game/human_presentation_textures.h"
#include "frontend/spatial_sound.h"

namespace hd2::game {
//0075D210: the actual foot/ground, SOUND, particle and projection owners.
//The native output-only SIMD fourth lanes are excluded from our geometry.
template<class Control,class Look,class Human,class Services,class Frames>
void present_human_footstep(Control& control,const Look& look,Human& human,Services& services,Frames& frames,bool left){
    frontend::SpatialSoundRequest request;const auto mode=services.mode_flags();const bool authority_actor=control.animation_authority_actor();
    if(!(mode&2u)&&!authority_actor)return;
    auto& physical=control.physical();auto& state=physical.control_frame_words();
    const auto word=[&](unsigned offset){return state.template get<std::int32_t>(offset);};
    auto steps=word(0x230);if(steps>0)--steps;else if(steps<0)++steps;state.set(0x230,steps);
    const auto foot=physical.bones().at(left?0u:1u);if(!foot)throw std::logic_error("Footstep requires its actual retained foot frame");
    const auto direction_matrix=frames.actor_world_matrix(foot);
    HumanVector direction{{direction_matrix[8],direction_matrix[9],direction_matrix[10],direction_matrix[11]}};
    normalize_human_ai_shot(direction,(static_cast<long double>(direction[2])*direction[2]+static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[0])*direction[0]);
    const auto position_matrix=frames.actor_world_matrix(foot);HumanVector position{{position_matrix[12],position_matrix[13],position_matrix[14],position_matrix[15]}};
    auto material=word(0x750);
    if(services.human_projection_enabled()==1&&material==-1&&((mode&7u)!=7u||services.human_is_player(human)||authority_actor)){
        HumanVector start=position;
        start[0]=static_cast<float>(static_cast<long double>(direction[0])*.07999999821186066f+position[0]);
        start[1]=static_cast<float>((static_cast<long double>(position[1])+.15000000596046448f)+static_cast<long double>(direction[1])*.07999999821186066f);
        start[2]+=direction[2]*.07999999821186066f;
        const auto hit=[&]{try{physical.exclude_query(false);auto hit=services.human_geometry_ray(start,{{0,-.5f,0,0}},0xffffffffu,0x230000u,true);physical.restore_query(false);return hit;}
            catch(...){const auto failure=std::current_exception();try{physical.restore_query(false);}catch(...){frames.record_actor_cleanup_failure(std::current_exception());}std::rethrow_exception(failure);}}();
        if(!hit)return;
        material=human_frame_signed_word(services.human_geometry_material(*hit));physical.ground_contact(*hit);
    }
    bool scuff=false;
    if(word(0x1e0)==0&&word(0x1dc)!=2){
        const auto count=state.template get<std::uint32_t>(0xa98)+1u;state.set(0xa98,count);
        const auto skill=human_frame_signed_word(human.properties.stats_ec[6]);
        if(static_cast<long double>(skill)*.5f<human_frame_signed_word(count))
            scuff=frames.volume_random_unit()<static_cast<long double>(human_frame_signed_word(100u-static_cast<std::uint32_t>(skill)))*.0010000000474974513f;
    }else state.set(0xa98,std::int32_t{-1});
    if(material>=0&&(word(0x1e0)!=0||scuff)){
        const auto sound=services.human_footstep_sound(static_cast<std::uint32_t>(material));
        if(sound>=0){
            request.position={{0,.2f,0}};request.relative=foot;request.source=&human;request.group=0;request.entry=sound;
            const auto sector=control.animation().sector();const bool root=sector&&sector==frames.camera_root_sector();
            const auto posture=word(0x1dc),movement=look.state().template get<std::int32_t>(0x50);
            if(movement==0){request.far_scale=posture==0?(root?.3f:.25f):posture==1?(root?.2f:.15f):.1f;request.volume=posture==0?.8f:posture==1?.7f:.6f;}
            else if(movement==1||movement==3||movement==4){request.far_scale=posture==0?(root?.7f:.35f):posture==1?(root?.5f:.25f):.4f;request.volume=posture==0?.8f:.7f;}
            else if(movement==2){request.far_scale=posture==0?(root?.9f:.5f):posture==1?(root?.7f:.4f):.5f;request.volume=.8f;}
            else if(movement==5){request.far_scale=root?1.f:.5f;request.volume=1.f;}
            if(services.human_is_player(human)&&services.human_view_offset_enabled()){
                const auto local=frames.human_footstep_local_direction(foot);
                for(unsigned axis=0;axis!=3;++axis)request.position[axis]=static_cast<float>(request.position[axis]+static_cast<long double>(local[axis])*.6666666865348816f);
            }
            if(word(0xa94)>999||scuff){state.set(0xa94,0u);request.flags=(mode&2u)?0x20u:0x22u;}
            else{if(!(mode&2u))return;request.flags=1;}
            if(word(0x1dc)==2)request.flags|=2u;
            request.voice_group=3;services.emit_human_footstep_sound(request);
        }
    }
    if(!(mode&2u))return;
    if((state.template get<std::uint32_t>(0x22c)&0x20u)&&!services.human_is_player(human)&&(material==41||material==42)){
        const auto movement=look.state().template get<std::int32_t>(0x50);auto draw=static_cast<long double>(frames.volume_random_unit());
        if(movement==0)draw-=.5f;else if(movement==1)draw+=.05000000074505806f;else if(movement==2)draw+=.25f;
        if(draw>.8500000238418579f){
            HumanVector point{},back=look.state().template get<HumanVector>(0x10);
            for(unsigned axis=0;axis!=3;++axis){back[axis]=-back[axis];auto amount=static_cast<long double>(direction[axis])*.30000001192092896f;if(axis==2)amount=static_cast<float>(amount);point[axis]=static_cast<float>(position[axis]-amount);}
            (void)services.emit_human_particle(42,point,back,typename Frames::Frame{},true,false);
        }
    }
    if(look.state().template get<std::int32_t>(0x4c)==2||(state.template get<std::uint32_t>(0x22c)&0x80u)||!services.human_projection_enabled())return;
    if(services.near_human_surface_position(position)){steps=10;state.set(0x230,steps);}
    if(steps>0)material=(steps>5)-3;else if(steps<0)material=(steps< -8)-5;else if(material==-1)return;
    unsigned index=0;for(;index<13;++index)if(HumanPresentationTextures<Frames>::definitions[index].key==material)break;
    if(index==13||control.movement().state().template get<float>(0x10c)!=-10000.099609375f||services.human_hidden(human))return;
    auto colour=control.opacity().field0();if(colour>1.f)colour=1.f;else if(colour<.15000000596046448f)colour=.15000000596046448f;
    const float alpha=material==-2||material==-3?static_cast<float>(static_cast<long double>(steps)*.20000000298023224f+.5f):
        material==-4||material==-5?static_cast<float>(.5f-static_cast<long double>(steps)*.0625f):1.f;
    const float angle=static_cast<float>(human_motion_planar_angle(direction,{{0,0,1,0}}));
    position[0]+=direction[0]*.07999999821186066f;position[2]=static_cast<float>(position[2]+static_cast<long double>(direction[2])*.07999999821186066f);
    services.emit_human_footprint(human,index,left,position,angle,control.animation().sector(),alpha,colour);
}
} //namespace hd2::game
#endif
