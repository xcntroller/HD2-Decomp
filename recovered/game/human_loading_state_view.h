#ifndef HD2_GAME_HUMAN_LOADING_STATE_VIEW_H
#define HD2_GAME_HUMAN_LOADING_STATE_VIEW_H

#include "game/human_joint_owners.h"
#include "game/human_ai_navigation_jobs.h"
#include "game/human_sight_metrics.h"

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
    virtual bool project_ground_position(HumanVector&)=0; //Actual B0+58.
    virtual bool observe_navigation_light(float&,float&){return false;} //00583150, genuine AL0/RET8 base/Player/Remote receiver, absent frozen.
    virtual bool frame_actor_contact(std::uint32_t actor_kind,std::int32_t actor_identity,std::uint32_t mode)=0; //Actual B0+C8, three native families.
    virtual void sight_enemy_acquired(Human&)=0;
    virtual void sight_enemy_lost(Human&)=0;
    virtual void sight_dead_acquired(Human&)=0;
    virtual void sight_enemy_killed(Human&)=0;
    virtual bool suppress_hostile_selection()const{return false;} //00524DB0 returns AL=0.
    virtual bool aims_at_human(const Human&)const{return false;} //Typed view of AI D0.
    virtual bool selected_hostile_is(const Human&)const{return false;} //Base virtualA4 005831A0.
    virtual std::vector<HumanAINavigationJobPool::Lease>* scene_navigation_jobs(){return nullptr;} //00583240.
    virtual bool scene_movement_enabled()const{return false;} //Player and Remote virtual104.
    virtual bool scene_movement_interaction()const{return false;} //00583270.
    virtual HumanSightMetricFamily sight_metric_family()const{return HumanSightMetricFamily::base;} //005830B0/C0 genuine RET12.
    virtual std::int32_t hud_first_mode()const{throw std::runtime_error("Human state has no actual Active AI HUD first-mode receiver");}
    virtual std::int32_t hud_second_mode()const{throw std::runtime_error("Human state has no actual Active AI HUD second-mode receiver");}
};
} //namespace hd2::game
#endif
