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
    using Actor=typename Control::SceneInstance;
    HumanLookControl(){reset(1);}
    void bind(Control& control) { //0077DDA0.
        control_=&control;second_control_=&control;human_slot_=control.human_slot_address();
    }
    void reset(std::uint32_t property) { //0077DDC0 calls0077FD40 before its own writes.
        reset_human_look_words(state_);
        action_model_=Frame{}; //Native04 is a typed borrowed frame, never a host pointer DWORD.
        interaction_actor_=nullptr; //Native08.
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
    void interaction_command(Actor* actor,std::uint32_t subtype,std::uint32_t argument){ //0077FBB0, changed=true.
        if(!control_)throw std::logic_error("Human interaction command requires actual DD0");
        const auto animation=static_cast<std::int32_t>(control_->animation_state_word(0x1d8));
        constexpr std::uint32_t mask=0x4000u;
        if(locked(mask)||(!state_.template get<std::uint8_t>(0xb0)&&animation>=8&&animation<=14)){
            defer(mask);
            return;
        }
        if(state_.template get<std::uint32_t>(0x90)==subtype)return;
        state_.set(0x90,subtype);interaction_actor_=actor;
        state_.set(0x94,argument);
        state_.set(0x40,state_.template get<std::uint32_t>(0x40)|mask);
    }
    Actor* interaction_actor()const{return interaction_actor_;}
    void frame_item_action(std::int32_t command){ //0077F380.
        if(locked(0x200u)){defer(0x200u);return;}
        if(state_.template get<std::int32_t>(0x58)==command)return;
        state_.set(0x58,command);
        state_.set(0x40,state_.template get<std::uint32_t>(0x40)|0x200u);
    }
    template<class Notify>void frame_item_ready(bool ready,Notify&& notify){ //0077F3D0.
        constexpr std::uint32_t mask=0x400u;
        if((state_.template get<std::uint32_t>(0xd0)|state_.template get<std::uint32_t>(0xd4))&mask){defer(mask);return;}
        if(state_.template get<std::uint32_t>(0x5c)==std::uint32_t(ready))return;
        state_.set(0x5c,std::uint32_t(ready));
        notify(ready?2u:1u);
        state_.set(0x40,state_.template get<std::uint32_t>(0x40)|mask);
    }
    template<class Publish>void frame_movement_animation(std::int32_t animation,Publish&& publish,bool changed=true){ //0077EC00.
        if(!control_)throw std::logic_error("Human movement animation requires actual DD0");
        const auto mask=state_.template get<std::uint32_t>(0xc0)|state_.template get<std::uint32_t>(0xd0)|
            state_.template get<std::uint32_t>(0xd4)|state_.template get<std::uint32_t>(0xd8);
        if(((mask&1u)||(control_->animation_state_word(0xbd0)&0x80000000u))&&
           !state_.template get<std::uint8_t>(0xb0)){
            defer(1);
            return;
        }
        const auto old=state_.template get<std::int32_t>(0x44);
        if(old>0x70&&old<0x9b)return;
        const auto mode=state_.template get<std::uint32_t>(0xb8);
        const auto current=static_cast<std::int32_t>(control_->animation_state_word(0x1d8));
        const auto in_sequence=[](std::int32_t value){return value>0x70&&value<0x9b;};
        const bool forward=animation==0xbe||animation==0xc6;
        const bool backward=animation==0xbf||animation==0xc7;
        if(mode==1){
            if(forward||backward){
                const auto down=state_.template get<std::uint32_t>(0x50)==2?0x6c:0x6b;
                if(!control_->human_is_player())animation=forward?0x6a:down;
                else if(backward)animation=0xce;
                else{
                    const auto pitch=control_->physical().state().template get<float>(0x944); //0077EBD0.
                    animation=pitch>.17f?0x6a:pitch<-.2f?down:0xce;
                }
            }else{
                if(!(animation>=0x6a&&animation<=0x6c)&&animation!=8&&animation!=0xe&&
                   !in_sequence(animation)&&!(animation==0x12&&current>=0x6a&&current<=0x6c))
                    animation=0xce;
                publish(0u); //005A5C90(0).
            }
        }else if(mode>=2&&mode<=4){
            if(mode==3&&(animation==0x1d||animation==0x25||animation==0x26))animation=0x25;
            else if(mode==2&&(animation==0x1d||animation==0x25||animation==0x26))
                throw std::runtime_error("Human movement requires the actual queued door-look receiver");
            else if(!in_sequence(animation))animation=0xce;
        }else if(mode==5&&!in_sequence(animation)){
            if(forward||backward){
                control_->frame_climb_direction(backward);
                control_->prepare_motion_velocity(*this,true); //0076B670 before the native slot-five lookup and task publication.
                animation=control_->frame_special_movement_item()?0x5c:0x5a;
                publish(backward?0x10u:8u);
            }else if(animation>=0x2d&&animation<=0x41){
                control_->frame_climb_direction(false);
                if(animation==0x2d){
                    animation=control_->frame_special_movement_item()?0x5d:0x5b;
                    publish(0u);
                }
            }else if(animation!=0x59&&animation!=0x68&&animation!=0x69&&animation!=0x5b&&animation!=0x6a){
                control_->frame_climb_direction(false);
                return;
            }
        }
        if(old==animation||(animation>=0xbe&&current==0x11)||(animation==0x1d&&old!=0x22))return;
        state_.set(0xb4,state_.template get<std::uint32_t>(0xb4)+1);
        state_.set(0x44,animation);
        if(changed)state_.set(0x40,state_.template get<std::uint32_t>(0x40)|1u);
    }
    template<class Publish>void frame_movement_mode(std::uint32_t mode,Publish&& publish){ //0077F190.
        if(locked(0x10u)){defer(0x10u);return;}
        if(state_.template get<std::uint32_t>(0x50)==mode)return;
        state_.set(0x50,mode);
        state_.set(0x40,state_.template get<std::uint32_t>(0x40)|0x10u);
        publish(mode==0?0x1000u:mode==5?0x10000u:mode==1?0x2000u:mode==2?0x4000u:0x8000u);
    }
    template<class Publish>void frame_posture(std::uint32_t posture,Publish&& publish){ //0077F0A0.
        if(!control_)throw std::logic_error("Human posture command requires actual DD0");
        const auto mask=state_.template get<std::uint32_t>(0xc0)|state_.template get<std::uint32_t>(0xd0)|
            state_.template get<std::uint32_t>(0xd4);
        const auto current=static_cast<std::int32_t>(control_->animation_state_word(0x1d8));
        if(!state_.template get<std::uint8_t>(0xb0)&&((mask&8u)||state_.template get<std::uint32_t>(0xb8)==5||
           control_->animation_state_word(0xa2c)!=0xffffd8f0u||control_->animation_state_word(0xa30)!=0xffffd8f0u||
           (control_->animation_state_word(0xbd0)&0x80000000u)||(current>=0xa2&&current<=0xa5))){
            defer(8);
            return;
        }
        if(state_.template get<std::uint32_t>(0x4c)!=posture){
            state_.set(0x4c,posture);
            state_.set(0x40,state_.template get<std::uint32_t>(0x40)|8u);
        }
        if(posture<=2)publish(4u>>posture);
    }
    Frame action_model()const{return action_model_;}
    void force_equipment_command(std::int32_t command,bool changed=false) { //00792590/00792720/00792AB0 and00791C60.
        state_.set(0xb0,std::uint8_t{1});
        try{equipment_command(command,changed);}catch(...){state_.set(0xb0,std::uint8_t{0});throw;}
        state_.set(0xb0,std::uint8_t{0});
    }
    const auto& state() const {return state_;}
    void position_command(const HumanVector& value,bool changed=true){ //0077F260.
        if(locked(0x40)){defer(0x40);return;}
        const auto old=state_.template get<HumanVector>(0x10);
        if(old[0]!=value[0]||old[1]!=value[1]||old[2]!=value[2]){
            state_.set(0x10,value);if(changed)state_.set(0x40,state_.template get<std::uint32_t>(0x40)|0x40u);
        } //A fourth-lane-only change leaves all four old lanes unchanged.
    }
    void current_direction(const HumanVector& input){ //0077F460.
        if(locked(0x800)){defer(0x800);return;}
        if(!control_)throw std::logic_error("Human direction requires bound DD0");
        const auto mode=control_->mode_flags();
        if((mode&7u)!=7u&&!state_.template get<std::uint8_t>(0xb0)&&!(mode&1u)&&!control_->human_is_player())return;
        HumanVector value{{input[0],0,input[2],input[3]}};
        const float square=static_cast<float>(static_cast<long double>(value[0])*value[0]+static_cast<long double>(value[2])*value[2]);
        constexpr float epsilon=9.99999993922529e-09f;
        if(!(std::abs(static_cast<long double>(square)-1)<epsilon)){
            if(!(square<epsilon)){
                const auto scale=1/std::sqrt(static_cast<long double>(square));
                for(unsigned axis=0;axis!=3;++axis)value[axis]=static_cast<float>(value[axis]*scale);
            }else{const auto axis=value[0]!=0?0u:value[2]!=0?2u:1u;value[axis]=value[axis]<0?-1.f:1.f;}
        }
        const auto physical=control_->physical().current_direction_override(),old=state_.template get<HumanVector>(0x20);
        const auto differs=[&](const HumanVector& a){return std::abs(static_cast<long double>(a[0])-value[0])+std::abs(static_cast<long double>(a[2])-value[2])>.009999999776482582f;};
        if(differs(physical)&&differs(old)){
            control_->physical().invalidate_current_direction();state_.set(0x20,value);
            state_.set(0x40,state_.template get<std::uint32_t>(0x40)|0x800u);
        }
    }
    void target_direction(const HumanVector& value){ //0077F690 calls geometry007B8E80, not vector normalization.
        if(locked(0x1000)){defer(0x1000);return;}
        if(!control_)throw std::logic_error("Human target direction requires bound DD0");
        control_->geometry().command_target_direction(value,control_->human_is_player());
        control_->physical().target_direction(value);
    }
    std::uint32_t consume_frame_feedback(){ //005AC0D0, EAX returns OLD CC before clearing one-shot bits.
        const auto flags=state_.template get<std::uint32_t>(0xcc);state_.set(0xcc,flags&0xfffd0fefu);return flags;
    }
    void begin_control_frame(std::uint32_t animation){ //00787270, after the actual model-geometry prepare.
        state_.set(0xc8,0u);
        state_.set(0xcc,(state_.template get<std::uint32_t>(0x5c)==1?0x40u:0u)|(animation==0x4a?0x10000u:0u));
        state_.set(0xd8,0u);
    }
    template<class Publish>void advance_timed_posture(std::uint32_t elapsed,Publish& publish){ //0077FCE0/0077FC70.
        const auto timer=state_.template get<std::int32_t>(0xbc);
        if(timer==-10000)return;
        const auto remaining=static_cast<std::uint32_t>(timer)-elapsed;
        state_.set(0xbc,remaining);
        if(remaining&0x80000000u)schedule_frame_posture(-10000,state_.template get<std::uint32_t>(0xc4),publish);
    }
    template<class Publish>void schedule_frame_posture(std::int32_t delay,std::uint32_t posture,Publish& publish){ //0077FC70.
        if(delay!=-10000){
            state_.set(0xbc,delay);
            state_.set(0xc4,posture);
            return;
        }
        state_.set(0xb0,std::uint8_t{1});
        try{frame_posture(state_.template get<std::uint32_t>(0xc4),publish);}
        catch(...){state_.set(0xb0,std::uint8_t{0});throw;}
        defer(8);
        state_.set(0xb0,std::uint8_t{0});state_.set(0xbc,std::int32_t{-10000});state_.set(0xc0,0u);
    }
    void clear_control_frame_commands(){state_.set(0x40,0u);}
    void force_frame_commands(bool enabled){state_.set(0xb0,static_cast<std::uint8_t>(enabled));}
    template<class Publish>void control_movement_mode(std::uint32_t kind,bool enabled,Publish& publish){ //0075CD50.
        if(!control_)throw std::logic_error("Human movement-mode command requires actual DD0");
        auto flags=state_.template get<std::uint32_t>(0xd0);
        switch(kind){
        case 0:
            state_.set(0xb8,enabled?1u:0u);
            state_.set(0xd0,enabled?flags|0x808u:flags&~0x808u);
            if(!enabled)control_->physical().control_frame_words().set(0x988,std::int32_t{-10000});
            return;
        case 1:case 2:
            if(!enabled){
                state_.set(0xb8,0u);
                frame_movement_animation(45,publish);
                flags=state_.template get<std::uint32_t>(0xd0);
                state_.set(0xd0,flags&~0x818u);
            }else{
                frame_movement_animation(kind==1?34:35,publish);
                state_.set(0xb8,kind+1);
                flags=state_.template get<std::uint32_t>(0xd0);
                state_.set(0xd0,flags|0x818u);
            }
            return;
        case 3:
            state_.set(0xb8,enabled?4u:0u);state_.set(0xd0,enabled?flags|0x1au:flags&~0x1au);
            return;
        case 4:
            state_.set(0xb8,enabled?5u:0u);state_.set(0xd0,enabled?flags|8u:flags&~8u);
            return;
        default:return;
        }
    }
    void control_frame_animation_lock(bool enabled){ //0075F230 commands13..15.
        const auto flags=state_.template get<std::uint32_t>(0xd0);
        state_.set(0xd0,enabled?flags|1u:flags&~1u);
    }
private:
    bool locked(std::uint32_t mask)const{return !state_.template get<std::uint8_t>(0xb0)&&((state_.template get<std::uint32_t>(0xd0)|state_.template get<std::uint32_t>(0xd4))&mask);}
    void defer(std::uint32_t mask){state_.set(0xc8,state_.template get<std::uint32_t>(0xc8)|mask);}
    HumanJointWords<0xe0> state_; //Only the stores above are behaviorally reconstructed.
    Control* control_=nullptr;Control* second_control_=nullptr;Frame action_model_{};
    Actor* interaction_actor_=nullptr;
    decltype(std::declval<Control&>().human_slot_address()) human_slot_=nullptr;
};
} //namespace hd2::game
#endif
