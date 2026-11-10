#ifndef HD2_GAME_HUMAN_LOOK_CONTROL_H
#define HD2_GAME_HUMAN_LOOK_CONTROL_H

#include "game/human_joint_owners.h"
#include "game/human_equipment_command.h"

namespace hd2::game {

//0077FD40 is also embedded at DD0+770, independently of the E0 owner.
template<std::size_t Size>void reset_human_look_words(HumanJointWords<Size>& state,std::size_t base=0) {
    for(auto offset:{0x10u,0x4cu,0x54u,0x14u,0x58u,0x5cu,0x60u,0x18u,
        0u,8u,0x90u,0x20u,0x94u,4u,0x24u,0x28u,0x70u,0x74u,0x78u,0x80u,0x84u,0x88u})
        state.set(base+offset,std::uint32_t{0});
    state.set(base+0x44,std::uint32_t{45});state.set(base+0x48,std::uint32_t{75});state.set(base+0x50,std::uint32_t{1});
}

//E0 owner0077DD80,0077DDA0,0077DDC0/0077FD40,0077FC30. DD0+258 is
//the ADDRESS of the Human-pointer slot, not the Human pointer's current value.
//The containing inline98 owner binds DD0 before supplying any command.
template<class Control>
class HumanLookControl {
public:
    using Frame=typename Control::Frame;
    HumanLookControl(){reset(1);}
    void bind(Control& control) { //0077DDA0.
        control_=&control;second_control_=&control;human_slot_=control.human_slot_address();
    }
    void reset(std::uint32_t property) { //0077DDC0 calls0077FD40 before its own writes.
        reset_human_look_words(state_);
        action_model_=Frame{}; //Native04 is a typed borrowed frame, never a host pointer DWORD.
        state_.set(0xbc,std::int32_t{-10000});state_.set(0xa0,property);state_.set(0xb0,std::uint8_t{0});
        for(auto offset:{0xb8u,0xc0u,0xd4u,0xd0u,0x40u,0xc8u,0xccu,0xb4u,0xd8u})state_.set(offset,std::uint32_t{0});
    }
    void command(std::uint32_t flags,const HumanVector* target) { //0077FC30.
        if(!control_)throw std::runtime_error("Human look control is not bound to its DD0 owner");
        if((!(state_.template get<std::uint32_t>(0xd0)&0x8000)&&!(state_.template get<std::uint32_t>(0xd4)&0x8000))||state_.template get<std::uint8_t>(0xb0))
            control_->geometry().command_look(flags,target);
        else state_.set(0xc8,state_.template get<std::uint32_t>(0xc8)|0x8000u);
    }
    void equipment_command(std::int32_t command,bool changed) { //0077F2F0.
        if(!control_)throw std::runtime_error("Human equipment command has no bound DD0 owner");
        human_equipment_command(state_,command,changed,*control_);
    }
    void action_model(Frame model,bool changed) { //0077F710, native04 frame borrower.
        if((!(state_.template get<std::uint32_t>(0xd0)&0x10000u)&&!(state_.template get<std::uint32_t>(0xd4)&0x10000u))||state_.template get<std::uint8_t>(0xb0)){
            if(action_model_!=model){action_model_=model;if(changed)state_.set(0x40,state_.template get<std::uint32_t>(0x40)|0x10000u);}
        }else state_.set(0xc8,state_.template get<std::uint32_t>(0xc8)|0x10000u);
    }
    void action_command(std::int32_t command,bool changed) { //0077F050, independent of equipment54.
        if((!(state_.template get<std::uint32_t>(0xd0)&2u)&&!(state_.template get<std::uint32_t>(0xd4)&2u))||state_.template get<std::uint8_t>(0xb0)){
            if(state_.template get<std::int32_t>(0x48)!=command){state_.set(0x48,command);if(changed)state_.set(0x40,state_.template get<std::uint32_t>(0x40)|2u);}
        }else state_.set(0xc8,state_.template get<std::uint32_t>(0xc8)|2u);
    }
    Frame action_model()const{return action_model_;}
    void force_equipment_command(std::int32_t command) { //00792590/00792720/00792AB0 set B0=1 then0.
        state_.set(0xb0,std::uint8_t{1});
        try{equipment_command(command,false);}catch(...){state_.set(0xb0,std::uint8_t{0});throw;}
        state_.set(0xb0,std::uint8_t{0});
    }
    const auto& state() const {return state_;}
private:
    HumanJointWords<0xe0> state_; //Only the stores above are behaviorally reconstructed.
    Control* control_=nullptr;Control* second_control_=nullptr;Frame action_model_{};
    decltype(std::declval<Control&>().human_slot_address()) human_slot_=nullptr;
};
} //namespace hd2::game
#endif
