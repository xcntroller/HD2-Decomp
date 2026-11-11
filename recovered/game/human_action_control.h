#ifndef HD2_GAME_HUMAN_ACTION_CONTROL_H
#define HD2_GAME_HUMAN_ACTION_CONTROL_H

#include "game/human_joint_owners.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {

//00749E30 is AA0's five borrowed links, not C00. The actual call at00785BA5
//uses ECX=DD0+AA0. C00 is00777A80's four links/reset/HD lookup. Keep typed
//links outside native32 scalar slots; original scratch/padding stays unclaimed.
template<class Control,class Movement,class Action,class Motion,class Human>
struct HumanControlLinks {
    void bind(Control& owner,Movement& movement,Action& action,Motion& motion,Human& human) {
        owner_=&owner;movement_=&movement;action_=&action;motion_=&motion;human_=&human;
    }
    Control* owner() const{return owner_;}Movement* movement() const{return movement_;}
    Action* action() const{return action_;}Motion* motion() const{return motion_;}Human* human() const{return human_;}
private:
    Control* owner_=nullptr;Movement* movement_=nullptr;Action* action_=nullptr;Motion* motion_=nullptr;Human* human_=nullptr;
};

template<class Frames,class Control,class Links,class Movement,class Human>
class HumanActionControl {
public:
    using Frame=typename Frames::Frame;
    HumanActionControl(){state_.set(0x1c,std::uint8_t{0});} //007855C0's C1C store.
    void bind(Frames& frames,Control& owner,Links& links,Movement& movement,Human& human) { //00777A80.
        owner_=&owner;links_=&links;movement_=&movement;human_=&human;reset();
        const auto model=human.base().frame();
        if(!model)throw std::runtime_error("Human C00 requires its actual model");
        marker_=frames.actor_find(model,"HD",0x20);
    }
    void reset() { //00777A40. Pointer80 and cold flag1C are not reset here.
        for(auto offset:{0x24u,0x3cu,0x50u})state_.set(offset,std::uint32_t{0});
        for(auto offset:{0x20u,0x38u,0x4cu,0x58u})state_.set(offset,std::int32_t{-10000});
        for(auto offset:{0x28u,0x40u,0x54u})state_.set(offset,std::uint32_t{0x20000});
        state_.set(0xa0,std::uint32_t{0xc61c4066}); //00816820, -10000.099609375f.
    }
    Control* owner() const{return owner_;}Links* links() const{return links_;}
    Movement* movement() const{return movement_;}Human* human() const{return human_;}
    Frame marker() const{return marker_;}
    const auto& state() const{return state_;}
    void advance_presentation_timer(std::uint32_t elapsed){ //00787D50, DD0+C58.
        const auto timer=state_.template get<std::uint32_t>(0x58);
        if(human_frame_signed_word(timer)>0)state_.set(0x58,timer-elapsed);
    }
    void reset_equipment_command(){state_.set(0x38,std::int32_t{-10000});state_.set(0x3c,std::uint32_t{0});}
    void finish_special_motion_command(){state_.set(0x20,0u);} //00768170, SAME C00 linked at AC0+4C.
    void lateral_stride_limit(float amount){state_.set(0xa0,amount);} //0074A290, SAME C00+A0.
private:
    Control* owner_=nullptr;Links* links_=nullptr;Movement* movement_=nullptr;Human* human_=nullptr;
    Frame marker_{};HumanJointWords<0xb0> state_;
};
} //namespace hd2::game
#endif
