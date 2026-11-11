#ifndef HD2_GAME_HUMAN_GROUND_GEOMETRY_H
#define HD2_GAME_HUMAN_GROUND_GEOMETRY_H

#include "game/human_ai_shot_direction.h"
#include "game/human_state_frame_timers.h"
#include "frontend/cleanup.h"
#include <limits>

namespace hd2::game {

//00763EC0 refreshes real posture/shot geometry, optionally aligning the model
//temporarily to the actual ActorBase position. All descriptor writes reach the
//same physical owner and collision buckets used during loading and queries.
template<class Control,class Look,class Human,class Services,class Frames>
void refresh_human_ground_geometry(Control& control,const Look& look,Human& human,
        Services& services,Frames& frames,bool presentation){
    auto& physical=control.physical();auto& state=physical.control_frame_words();
    auto& movement=control.movement().frame_words();
    const auto word=[&](unsigned offset){return state.template get<std::int32_t>(offset);};
    const auto move_word=[&](unsigned offset){return movement.template get<std::int32_t>(offset);};
    const auto look_word=[&](unsigned offset){return look.state().template get<std::uint32_t>(offset);};
    bool align_model=look_word(0x4c)==2;
    const float threshold=look_word(0xb8)==5||align_model?.9975f:.9553365111351013f;
    const auto& direction=human.base().world_direction();
    const auto previous=movement.template get<HumanVector>(0xa0);
    const auto dot=(static_cast<long double>(direction[2])*previous[2]+
        static_cast<long double>(direction[1])*previous[1])+static_cast<long double>(direction[0])*previous[0];
    bool refresh_markers=dot<threshold||(word(0x1d8)>=162&&word(0x1d8)<=165);
    if(!state.template get<std::uint8_t>(0x18c))return;
    if(word(0x228)>=2000&&!state.template get<std::uint8_t>(0x761)&&word(0x9ac)==-10000&&!refresh_markers)return;
    const auto model=human.base().frame();
    if(!model)throw std::logic_error("Human ground geometry requires its actual model");
    const auto matrix=frames.actor_world_matrix(model);
    const HumanVector original{{matrix[12],matrix[13],matrix[14],matrix[15]}};
    const auto position=human.base().world_position();
    auto posture=look_word(0x4c);
    if(move_word(0xd0)!=-10000)posture=movement.template get<std::uint32_t>(0xd4);
    else if(look_word(0xb8)==5)posture=2;
    bool translate_only=false;
    const bool forced=posture==2||align_model||word(0x1ac)!=1||
        ((control.mode_flags()&7u)==7u&&human.properties.field19c==1)||
        control.animation_authority_actor()||control.human_is_player()||(!presentation&&word(0xa44)>0);
    if(forced){align_model=true;refresh_markers=true;}
    else if(presentation){align_model=false;refresh_markers=true;}
    else{
        align_model=(!control.human_is_player()&&std::abs(static_cast<long double>(original[0])-position[0])>.001f)||
            std::abs(static_cast<long double>(original[1])-position[1])>.001f||
            std::abs(static_cast<long double>(original[2])-position[2])>.001f;
        const auto vertical=human.activation.vector1b0[1];
        const auto animation=word(0x1d8),attachment=word(0xa34);
        const auto& sphere=physical.posture_spheres()[0];
        refresh_markers=vertical< -1||vertical>1||word(0x9ac)!=-10000||
            (animation<190&&(animation<41||animation>65))||refresh_markers||
            (attachment!=0&&attachment!=1)||std::abs(static_cast<long double>(original[0])-sphere[0])>.5||
            std::abs(static_cast<long double>(original[2])-sphere[2])>.5;
        if(!refresh_markers&&move_word(0xd8)>0){
            if(!align_model)return;
            translate_only=true;
        }
    }
    using Frame=typename Frames::Frame;
    struct Restore {
        Frames& frames;Frame model;HumanVector original;Frame parent{};bool armed=false;
        void finish(){
            armed=false;
            frontend::CleanupErrors errors;
            errors.attempt([&]{frames.object_position(model,{original[0],original[1],original[2]});});
            errors.attempt([&]{frames.actor_link_preserving_world(model,parent);});
            errors.attempt([&]{frames.update_scene_object(model);});
            errors.rethrow();
        }
        ~Restore(){if(armed)try{finish();}catch(...){frames.record_actor_cleanup_failure(std::current_exception());}}
    } restore{frames,model,original};
    bool model_published=false;
    if(align_model&&!translate_only){
        restore.parent=frames.actor_parent(model);restore.armed=true;
        frames.object_position(model,{position[0],position[1],position[2]});
        frames.update_scene_object(model);model_published=true;
        if(!refresh_markers&&move_word(0xd8)>0)translate_only=true;
    }
    if(translate_only)physical.translate_ground_geometry(position);
    else{
        physical.refresh_ground_geometry(); //0075C070 then0075B870, including native shot-relative capture order.
        const auto current=frames.actor_world_matrix(model);
        HumanVector forward{{current[8],current[9],current[10],current[11]}};
        const auto square=(static_cast<long double>(forward[1])*forward[1]+
            static_cast<long double>(forward[0])*forward[0])+static_cast<long double>(forward[2])*forward[2];
        normalize_human_ai_shot(forward,square);movement.set(0xa0,forward);
        if(word(0x228)>-1000){
            if(state.template get<std::uint32_t>(0xa7c)<5000)movement.set(0xd8,0u);
            else{
                const auto truncate=[](long double value){
                    if(!std::isfinite(value)||value< -0x1p63L||value>=0x1p63L)return std::int32_t{0};
                    return human_frame_signed_word(static_cast<std::uint32_t>(static_cast<std::int64_t>(value)));
                };
                auto delay=human.properties.field19c==1?200:truncate(1400.L/services.human_control_animation_scale());
                constexpr std::array<float,6> multipliers{{1.2f,1.f,.9f,1.f,1.5f,.6f}};
                const auto stance=look_word(0x50);
                const auto multiplier=stance<multipliers.size()?multipliers[stance]:1.f;
                delay=truncate(static_cast<long double>(delay)*multiplier);
                if(!align_model&&!refresh_markers)delay=human_frame_signed_word(static_cast<std::uint32_t>(delay)*4u);
                movement.set(0xd8,delay);
            }
        }
    }
    physical.publish_ground_geometry(posture);
    if(align_model&&(refresh_markers||model_published))restore.finish();
}

} //namespace hd2::game
#endif
