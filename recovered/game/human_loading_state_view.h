#ifndef HD2_GAME_HUMAN_LOADING_STATE_VIEW_H
#define HD2_GAME_HUMAN_LOADING_STATE_VIEW_H

#include "game/human_joint_owners.h"

namespace hd2::game {

//Typed borrowed HumanB0 view shared by the actual Human, state owner and Sight.
//This is source dispatch, not an emulation of native object/vtable layout.
template<class Human>class HumanLoadingStateView {
public:
    struct PlayerHandover {std::uint32_t flags;bool posture;std::uint32_t action;};
    virtual ~HumanLoadingStateView()=default;
    virtual HumanJointWords<0xc0>& words()=0;
    virtual const HumanJointWords<0xc0>& words()const=0;
    virtual HumanVector current_direction()const=0;
    virtual HumanVector target_direction()const=0;
    virtual std::uint32_t derived_loading_word(unsigned)const=0;
    virtual PlayerHandover player_handover()const=0;
    virtual void sight_enemy_acquired(Human&)=0;
    virtual void sight_enemy_lost(Human&)=0;
    virtual void sight_dead_acquired(Human&)=0;
};
} //namespace hd2::game
#endif
