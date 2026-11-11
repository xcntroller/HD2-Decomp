#ifndef HD2_GAME_HUMAN_FRAME_LIGHT_H
#define HD2_GAME_HUMAN_FRAME_LIGHT_H

#include "game/human_joint_owners.h"
#include <exception>

namespace hd2::game {

template<class Control,class Look,class Human,class Services,class Frames>
void refresh_human_frame_light(Control& control,const Look& look,Human& human,Services& services,Frames& frames){ //00787270 light/opacity stage.
    auto& physical=control.physical();
    const auto& state=physical.control_frame_words();
    const auto word=[&](unsigned offset){return state.template get<std::int32_t>(offset);};
    auto& equipment=services.human_presentation_equipment(human).state();
    const bool attached=word(0xa30)>=1&&equipment.model(9);
    physical.exclude_query(false);
    struct Restore {
        decltype(physical) owner;Frames& frames;bool active=true;
        ~Restore(){if(active)try{owner.restore_query(false);}catch(...){frames.record_actor_cleanup_failure(std::current_exception());}}
    } restore{physical,frames};
    const auto observe=[&](float height){
        const auto bone=physical.bones().at(16);
        if(!bone)throw std::logic_error("Human light observation requires its actual bone16");
        const auto matrix=frames.actor_world_matrix(bone);
        const HumanVector position{{matrix[12],matrix[13]+height,matrix[14],matrix[15]}};
        control.opacity().observe(frames,services,control.animation().sector(),human.base().frame(),position,false,
            [&](auto model,float value){control.human_opacity_changed(model,value);});
    };
    if(attached){
        const auto model=equipment.model(9); //Native repeats the equipment query after exclusions.
        if(!model)throw std::logic_error("Human light observation lost its actual equipment model9");
        frames.object_visible(model,false);
        try{observe(0);}catch(...){
            try{frames.object_visible(model,frames.car_frame_visible(human.base().frame()));}
            catch(...){frames.record_actor_cleanup_failure(std::current_exception());}
            throw;
        }
        const auto current=equipment.model(9);
        if(!current)throw std::logic_error("Human light observation lost its equipment model9 after observation");
        frames.object_visible(current,frames.car_frame_visible(human.base().frame()));
    }else{
        auto* actor=physical.control_interaction_target();
        if(word(0xa44)<1||word(0xa44)>7||!actor||actor->base().type()!=4||!services.human_action_car_disabled(*actor)){
            const auto posture=look.state().template get<std::uint32_t>(0xb8);
            const auto animation=word(0x1d8);
            const bool raised=posture==2||posture==3||animation==159||animation==158||animation==155||
                animation==111||animation==34||word(0xa44)>0;
            observe(raised?.8f:0.f);
        }
    }
    restore.active=false;
    physical.restore_query(false);
}

} //namespace hd2::game
#endif
