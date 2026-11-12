#ifndef HD2_GAME_HUMAN_POSITION_REPLAY_H
#define HD2_GAME_HUMAN_POSITION_REPLAY_H
#include "game/human_joint_owners.h"
namespace hd2::game {
//00759E20, DD0 over SAME Human/E0/model/physics/collision owners. Original's
//distance comparison has no consumer; resolving the model world matrix still
//precedes frame08. Descriptor/marker publication and Human position stay ordered.
template<class Control,class Look,class Human,class Services,class Frames>
void replay_human_control_position(Control& control,const Look& look,Human& human,
        Services& services,Frames& frames,const HumanVector& position){
    if(control.collision_disabled())return;
    const auto posture=look.state().template get<std::uint32_t>(0x4c);
    auto& physical=control.physical();
    if((services.mode_flags()&7u)==7u||physical.control_frame_words().template get<std::uint32_t>(0x1ac)!=1u){
        physical.enable(true);return;
    }
    if(const auto* actor=human.field160){
        const auto kind=actor->base().type();
        if(kind==4||kind==0x14||kind==0x15||kind==0x22){physical.enable(true);return;}
    }
    (void)frames.actor_world_matrix(human.base().frame());
    frames.replay_human_model_position(human.base().frame(),position); //08 -> Scene144 ->24.
    physical.publish_replayed_geometry(posture); //snapshot -> posture -> primary -> shots -> shot descriptors.
    human.base().world_position(position); //All four lanes, LAST.
}
}
#endif
