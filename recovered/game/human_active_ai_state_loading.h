#ifndef HD2_GAME_HUMAN_ACTIVE_AI_STATE_LOADING_H
#define HD2_GAME_HUMAN_ACTIVE_AI_STATE_LOADING_H

#include "game/human_ai_state_loading.h"
#include <optional>
#include "game/human_active_ai_grenade.h"

namespace hd2::game {

//00524C60/00524E70/00525110, actual initial offline B0kind3. Equipment selection
//00530D40/00530A40 use the SAME typed inventory owner. Players/Speech and
//0089AC30 consumers are mandatory Services methods.
//Runtime task/formation producers are not exposed by this loading owner.
template<class Frames,class Human,class Inputs,class Services>
class HumanActiveAIStateLoading : public HumanAIStateLoading<Frames,Human,Inputs> {
public:
    void sight_enemy_killed(Human&)override{} //005A4350, genuine RET4 Active AI, absent frozen.
    using AI=HumanAIStateLoading<Frames,Human,Inputs>;using Base=typename AI::Base;
    HumanActiveAIStateLoading(Frames& frames,Inputs& inputs,MissionTableSet& second,
            HumanInputCallbacks& callbacks,HumanAIAction1Pool& actions,Services& services)
        :AI(frames,inputs,second,callbacks,3),actions_(actions),services_(services) {
        for(auto offset:{0x204u,0x209u})state_.set(offset,std::uint8_t{0});
        for(auto offset:{0x202u,0x206u})state_.set(offset,std::uint8_t{1});
        for(auto offset:{0x214u,0x218u})state_.set(offset,std::uint32_t{0});
        //Native224 task/250 list pointers are typed guards, with no producers here.
    }
    template<class Inline>void bind(Human& human,const Base* previous,MissionNavigation* navigation,
            std::uint32_t mode,Inline& control) {
        AI::bind(human,previous,navigation,mode,control);
        for(auto offset:{0x1bcu,0x178u,0x180u,0x17cu,0x174u,0x1c4u,0x1a0u,0x1a4u})state_.set(offset,std::uint32_t{0});
        state_.set(0x170,std::uint32_t{0x2000});state_.set(0x19c,std::uint32_t{200});
        for(auto offset:{0x18cu,0x188u,0x18au})state_.set(offset,std::uint8_t{0});
        state_.set(0x189,std::uint8_t{1});
        const bool alive=signed_health(human)>0;
        if(alive)set_action_mode(12);
        for(auto offset:{0x190u,0x1b0u,0x1a8u,0x1acu,0x23cu,0x240u})state_.set(offset,std::uint32_t{0});
        state_.set(0x18b,std::uint8_t{0});state_.set(0x18e,std::uint8_t{0});
        state_.set(0x160,this->current_direction()); //005A80D0 observes the already published NEW B0.
        set_follow(false,true);
        state_.set(0x1cc,std::int32_t{-1});state_.set(0x1d4,std::int32_t{-1});
        state_.set(0x1b4,std::uint32_t{0});state_.set(0x1b8,std::uint32_t{0});
        state_.set(0x18d,std::uint8_t{0});state_.set(0x244,std::uint8_t{0});state_.set(0x248,std::uint32_t{100});
        if(previous) {
            const auto kind=previous->words().template get<std::uint32_t>(0x48);bool replay=true;
            if(kind==1) {
                if(alive)set_action_mode(previous->derived_loading_word(0x548));
                state_.set(0x174,previous->derived_loading_word(0x528));set_follow(false,false);
                if(state_.template get<std::uint32_t>(0x174)&0x20u)
                    state_.set(0x17c,previous->words().template get<std::uint32_t>(0x50)&7u);
                state_.set(0x160,previous->target_direction());replay=false;
            }else if(kind==3)replay=false;
            else if(kind==5) {
                const auto flags=previous->derived_loading_word(0x1b0);
                if(flags&0x1000u)set_follow(false,true);
                if(flags&0x8000u){state_.set(0x174,state_.template get<std::uint32_t>(0x174)|1u);set_follow(false,true);}
                if((flags&0x100u)&&alive)set_action_mode(16);
            }
            if(alive&&replay) {
                const auto& enemies=human.perception.lists()[0];
                for(auto n=enemies.size();n;)services_.player_add_enemy(enemies[--n]); //00461410→0045E410.
            }
        }else state_.set(0x174,std::uint32_t{0});
        state_.set(0x234,std::uint32_t{0});
        if(!this->item()&&alive) {
            if(!human.loading_inventory)throw std::runtime_error("Active AI requires the actual Human inventory command owner");
            (void)human.loading_inventory->equip_shoulder(); //005A67C0.
        }
        reset_action();state_.set(0x184,std::int32_t{-1}); //0052A3B0(NULL).
        const auto matrix=this->state_frames().actor_world_matrix(human.base().frame());
        const HumanVector position{{matrix[12],matrix[13],matrix[14],matrix[15]}};
        this->resources().current_action(actions_.acquire(position,0xffff,15.f,this->target_direction()));
        state_.set(0x19c,std::uint32_t{0});state_.set(0x198,std::uint32_t{0});refresh_action_mode();
        control.look_command(0,nullptr);
    }
    void close_loading() { //00525110; AI close before virtual128(0), then clear Human44.
        AI::close_loading();reset_orders_loading();this->clear_human_reference();
    }
    void reset_orders_loading() { //0052EF20(0); actual250 list has no loading producer.
        state_.set(0x1bc,static_cast<std::uint32_t>(services_.game_frame_serial()+1u));
        auto& query=this->resources().query().state;query.set(0x50,query.template get<std::uint32_t>(0x50)|0x2000u);
        this->resources().path().stop_loading();
        state_.set(0x202,std::uint8_t{1});state_.set(0x206,std::uint8_t{1});state_.set(0x18d,std::uint8_t{0});
    }
    void sight_enemy_acquired(Human& actor) override { //0052E5B0: shared event FIRST, before Players/Speech.
        this->emit_sight(actor,0);
        auto* human=this->human();if(!human)throw std::logic_error("Active AI sight has no Human");
        const auto a=actor.base().world_position(),b=human->base().world_position();
        const float x=a[0]-b[0],y=a[1]-b[1],z=a[2]-b[2];
        const auto square=(static_cast<long double>(x)*x+static_cast<long double>(y)*y)+static_cast<long double>(z)*z;
        if(!services_.player_knows_enemy(actor))services_.queue_active_ai_speech(*this,square>=400.f?0x5cu:0x5eu,1,2000,0); //008101E0.
        services_.player_add_enemy(&actor);
        if(human->perception.lists()[0].size()==1)state_.set(0x1a0,std::uint32_t{0});
    }
    void sight_enemy_lost(Human& actor) override { //0052E6F0.
        this->emit_sight(actor,1);services_.player_remove_enemy(&actor);
        if(!services_.player_knows_enemy(actor))services_.queue_active_ai_speech(*this,0x60u,1,2000,0);
    }
    void sight_dead_acquired(Human& actor)override{(void)services_.player_knows_enemy(actor);} //0052E6D0; NO common dead event.
    bool scene_movement_enabled()const override{ //00532880.
        return !(state_.template get<std::uint32_t>(0x174)&1u)&&AI::scene_movement_enabled();
    }
    std::uint32_t derived_loading_word(unsigned offset)const override {
        if(offset!=0x174&&offset!=0x1d8)throw std::runtime_error("Unrecovered active AI handover word");
        if(offset==0x1d8&&!action_mode_)throw std::runtime_error("Active AI action mode has no native initializer");
        return state_.template get<std::uint32_t>(offset);
    }
    bool follow_enabled()const{return state_.template get<std::uint8_t>(0x189)!=0;}
    bool follow_immediate()const{return state_.template get<std::uint8_t>(0x188)!=0;}
    void disable_follow_loading(bool immediate){set_follow(false,immediate);} //005377F0, same actual state.
    bool process_available()const{return state_.template get<std::uint8_t>(0x202)!=0;}
    bool has_pending_orders_loading()const {
        //Native250/254/258 have no populated loading producer yet. Reject raw
        //writes instead of treating an unknown pointer as an empty order list.
        for(auto offset:{0x250u,0x254u,0x258u})if(state_.template get<std::uint32_t>(offset))
            throw std::runtime_error("Active AI populated orders require their actual typed producer");
        return false;
    }
    typename Base::PlayerHandover player_handover()const override {
        if(!action_mode_)throw std::runtime_error("Active AI handover has no native initialized action mode");
        const bool alternate=state_.template get<std::uint8_t>(0x18a)!=0;
        const auto action=*action_mode_==0?12u+unsigned(alternate):*action_mode_==1?16u+unsigned(alternate):
            *action_mode_==2?14u+unsigned(alternate):12u; //005385F0, virtual148.
        return {state_.template get<std::uint32_t>(0x174),alternate||state_.template get<std::uint8_t>(0x188)!=0,action}; //00524E00/005385D0.
    }
    void first_player_shot(){ //00459360: direct1D8 store, NOT005380C0's broader side effects.
        if(!action_mode_)throw std::runtime_error("First Player shot requires the actual initialized active-AI mode");
        if(*action_mode_!=0)return;
        action_mode_=1;state_.set(0x1d8,1u);refresh_action_mode();
    }
    void notify_nearby_player_damage(){ //0045F110, only prior14/15 modes change.
        const auto action=player_handover().action;
        if(action==14)set_action_mode(16);else if(action==15)set_action_mode(17);
    }
    void retire_formation_leader(){ //00595710 uses the actual005385F0 action selector, then005380C0.
        const auto action=player_handover().action;
        if(action==12||action==14)set_action_mode(16);
        else if(action==13||action==15)set_action_mode(17);
    }
    void react_to_grenade(const HumanVector& position,const HumanVector& direction,Human* source){react_human_active_ai_grenade(*this,position,direction,source,services_);} //00530FF0.
    auto& state(){return state_;}const auto& state()const{return state_;}
    std::int32_t hud_first_mode()const override { //00538590, virtual13C: pending1CC unless-1, else current1C8.
        const auto pending=state_.template get<std::int32_t>(0x1cc);
        return pending==-1?state_.template get<std::int32_t>(0x1c8):pending;
    }
    std::int32_t hud_second_mode()const override { //005385B0, virtual140.
        const auto pending=state_.template get<std::int32_t>(0x1d4);
        return pending==-1?state_.template get<std::int32_t>(0x1d0):pending;
    }
    SceneActor<Frames>* task_target=nullptr; //Native224 borrower; runtime task writer remains separate.
    bool reselect_weapon() { //00530D40, including repeated00530A40 command gate.
        if(weapon_command_blocked())return false;
        auto* target=this->resources().target().actor;
        if(target&&target->base().type()==3) {
            auto* human=dynamic_cast<Human*>(target);
            if(!human)throw std::runtime_error("Weapon target3 has no actual Human owner");
            if(human->field160)target=human->field160;
        }
        if(!state_.template get<std::uint8_t>(0x202)&&!target) {
            const auto action=state_.template get<std::int32_t>(0x220);
            if(action==25||action==26||action==22)target=task_target;
        }
        reselect_weapon_for_target(target);return true;
    }
    void reselect_weapon_for_target(SceneActor<Frames>* target) { //00530A40.
        if(weapon_command_blocked())return;
        auto* human=this->human();if(!human||!human->loading_inventory)
            throw std::runtime_error("Weapon reselection requires actual Human inventory command owner");
        auto& inventory=*human->loading_inventory;
        this->refresh_item();const bool alternate=state_.template get<std::uint8_t>(0x18a)!=0;
        if(inventory.weapon_usable(this->item(),target,alternate))return;
        std::int32_t slot=7;const auto selected=inventory.preferred_weapon(target,alternate,slot);
        if(selected&&selected!=this->item()) {
            //0069BFE0 debug message receiver is genuine RET. Timer precedes command.
            state_.set(0x1a0,std::uint32_t{4000});inventory.select_weapon(selected,slot);
        }
    }
private:
    bool weapon_command_blocked()const {
        return this->words().template get<std::int32_t>(0x64)>0&&this->words().template get<std::int32_t>(0x60)!=-1;
    }
    static std::int32_t signed_health(const Human& human){const auto bits=human.properties.stats_ec[0];std::int32_t result;std::memcpy(&result,&bits,4);return result;}
    void reset_action(){this->resources().retire_current_action();} //00575820.
    void set_follow(bool enabled,bool immediate) { //005377F0(false,...) loading branch.
        if(enabled)throw std::runtime_error("Active AI follow requires actual formation/runtime orders");
        state_.set(0x188,std::uint8_t(immediate));state_.set(0x189,std::uint8_t{0});
        state_.set(0x1b0,std::uint32_t{0});state_.set(0x160,this->current_direction());set_first_mode(immediate?3:2);
    }
    void set_first_mode(unsigned value) { //00538510.
        if(value<16)state_.set(0x1c8,static_cast<std::uint32_t>(value));
        else if(value<20){state_.set(0x1cc,static_cast<std::uint32_t>(value));state_.set(0x1b4,std::uint32_t{2000});}
    }
    void set_second_mode(unsigned value) { //00538550.
        if(value<12)state_.set(0x1d0,static_cast<std::uint32_t>(value));
        else if(value<40){state_.set(0x1d4,static_cast<std::uint32_t>(value));state_.set(0x1b8,std::uint32_t{2000});}
    }
    void refresh_action_mode() { //00538640.
        if(!action_mode_)throw std::runtime_error("Active AI dead loading requires an actual saved action mode");
        const bool alternate=state_.template get<std::uint8_t>(0x18a)!=0;
        set_second_mode(*action_mode_==0?unsigned(alternate):*action_mode_==1?4u+unsigned(alternate):*action_mode_==2?2u:4u);
    }
    void set_action_mode(unsigned value) { //005380C0 cases reached by loading/player handover.
        bool adjust=false,prefer_ready=false;
        if(value==12||value==13||value==14||value==15) {
            if(value<=13)state_.set(0x18a,std::uint8_t(value==13)); //14/15 preserve the prior alternate byte.
            if(value>=14)state_.set(0x174,state_.template get<std::uint32_t>(0x174)|4u);
            action_mode_=value>=14?2u:0u;state_.set(0x1d8,*action_mode_);
            state_.set(0x174,state_.template get<std::uint32_t>(0x174)&~0x10u);
            this->resources().target().bind_actor(nullptr); //005607B0(0), actual typed target30.
            refresh_action_mode();prefer_ready=value==12||(value==13&&special_hand_item());adjust=true;
        }else if(value==16||value==17) {
            state_.set(0x18a,std::uint8_t(value==17));state_.set(0x174,state_.template get<std::uint32_t>(0x174)&~4u);
            action_mode_=1;state_.set(0x1d8,std::uint32_t{1});set_second_mode(value==16?4:5);
            if(value==16||this->item()){prefer_ready=value==16||special_hand_item();adjust=true;}
        }
        if(adjust) {
            //00537FB0 has no effect until both native74/78 command producers exist.
            if(this->first_frame_inputs()&&this->second_frame_inputs())
                services_.adjust_active_ai_stance(*this,prefer_ready);
        }
        if(!state_.template get<std::uint8_t>(0x18a)||!this->item()||!special_hand_item())
            (void)reselect_weapon(); //00530D40→00530A40→00538770 on SAME actual inventory.
    }
    bool special_hand_item() { //Native item+4 descriptor8:16/7/104, SAME actual Items owner.
        auto* human=this->human();if(!human||!human->loading_inventory)
            throw std::runtime_error("Special-hand check requires actual Human inventory owner");
        return human->loading_inventory->special_hand_item(this->item());
    }
    HumanAIAction1Pool& actions_;Services& services_;HumanJointWords<0x260> state_;std::optional<std::uint32_t> action_mode_;
};
} //namespace hd2::game
#endif
