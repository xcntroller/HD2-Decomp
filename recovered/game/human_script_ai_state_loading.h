#ifndef HD2_GAME_HUMAN_SCRIPT_AI_STATE_LOADING_H
#define HD2_GAME_HUMAN_SCRIPT_AI_STATE_LOADING_H

#include "game/human_ai_state_loading.h"
#include "game/human_script_formation_output.h"
#include "game/human_ai_frame_metrics.h"

namespace hd2::game {

//005A3F20 case5,0058D810/0058D8E0 and00590060/0058FFE0. Contexts resolves
//the SAME modelEC data1 and actual0041DA20; no empty script context is supplied.
//Unknown140..1EF bytes are guards; scalar state never stores native1AC pointer.
template<class Frames,class Human,class Inputs,class Contexts>
class HumanScriptAIStateLoading : public HumanAIStateLoading<Frames,Human,Inputs> {
public:
    using AI=HumanAIStateLoading<Frames,Human,Inputs>;
    using Base=typename AI::Base;using Context=typename Contexts::Context;
    HumanScriptAIStateLoading(Frames& frames,Inputs& inputs,MissionTableSet& second,
            HumanInputCallbacks& callbacks,Contexts& contexts)
        :AI(frames,inputs,second,callbacks,5),contexts_(contexts) {
        state_.set(0x198,std::uint8_t{0}); //00591EE0 inline helper at198; no owned allocation.
        for(auto offset:{0x19cu,0x1a0u,0x1a4u})state_.set(offset,std::uint32_t{0});
    }
    template<class Inline>bool bind(Human& human,const Base* previous,MissionNavigation* navigation,
            std::uint32_t mode,Inline& control) {
        AI::bind(human,previous,navigation,mode,control);
        state_.set(0x1d8,std::int32_t{-1});state_.set(0x1dc,std::int32_t{-1});
        if(state_.template get<std::uint8_t>(0x198))state_.set(0x19c,std::uint32_t{0x80}); //00591ED0.
        state_.set(0x1b0,std::uint32_t{0});
        context_=contexts_.human_script_context(human);
        state_.set(0x1b4,std::uint32_t{0});state_.set(0x1bc,std::uint8_t{0});state_.set(0x1d1,std::uint8_t{0});
        state_.set(0x1b8,std::uint32_t{0});
        this->words().set(0x50,this->words().template get<std::uint32_t>(0x50)&0x7fffffffu);
        state_.set(0x1c0,std::uint32_t{0});state_.set(0x1d4,std::int32_t{-1});
        for(auto offset:{0x1d2u,0x18cu,0x1bdu})state_.set(offset,std::uint8_t{0});
        for(auto offset:{0x1c8u,0x1c4u,0x1ccu})state_.set(offset,std::uint32_t{0});
        formation_=nullptr; //Native1E0 typed borrower, never a pointer in DWORD storage.
        state_.set(0x1d0,std::uint8_t{0});state_.set(0x150,std::uint8_t{0});
        return context_!=nullptr; //Native factory ignores this return; absence is not a successful script bind.
    }
    void close_loading(){AI::close_loading();this->clear_human_reference();alarm_target_=nullptr;} //0058D8E0.
    template<class World,class Runtime>void advance_script_presentation(std::uint32_t elapsed,World& world,Runtime& runtime){ //0058E1C0.
        if(!(this->words().template get<std::uint32_t>(0x50)&0x800u)){
            if(state_.template get<std::uint8_t>(0x1d0))update_script_attachment(runtime);
            if((world.mode_flags()&7u)!=7u)runtime.advance_network_ai_presentation(*this,elapsed); //0056F710.
        }
        this->process_second_frame_commands();
    }
    template<class Queries,class Inline,class World,class Runtime>void advance_script_authority(
            std::uint32_t elapsed,Queries& queries,Inline& control,World& world,Runtime& runtime){ //0058E210.
        this->publish_authority_input_gate(control);
        const auto flags=[&]{return this->words().template get<std::uint32_t>(0x50);};
        if(!(flags()&0x800u)){
            this->advance_frame_timers(elapsed,queries);
            auto& inputs=queries.human_ai_frame_inputs();
            inputs.clear();this->bind_frame_inputs(inputs.first,inputs.second);
            if(auto* command=this->command())for(auto n=command->first.events.size();n;){
                if(command->first.events.at(--n)->words[0]==0x10000017u)
                    runtime.dispatch_script_ai_event(*this,command->first.events,n); //0058FD30.
            }
            if(alarm_enabled())advance_script_reaction(elapsed,control,world,runtime);
            update_script_follow_gate(world);
            auto& resources=this->resources();
            resources.reconcile_direction_flags();
            if(!this->human()->field160){
                const bool formation=formation_&&runtime.advance_script_formation_path(*this);
                auto& query=resources.query().state;
                const auto before=query.template get<std::uint32_t>(0x50);
                this->advance_frame_query(control,world,elapsed);
                if((before&0x10000000u)&&!(query.template get<std::uint32_t>(0x50)&0x10000000u)){
                    if(!formation){
                        if(context_)contexts_.finish_human_script_navigation(*context_,state_.template get<std::int32_t>(0x1d8));
                        state_.set(0x1d8,std::int32_t{-1});
                    }
                    if(alarm_enabled())state_.set(0x19c,0x80u); //00591ED0.
                }
            }else if(state_.template get<std::uint8_t>(0x1d0))update_script_attachment(runtime);
            if(alarm_enabled())this->advance_frame_aim(world);
            this->advance_frame_direction(elapsed,control);
            const auto alarm_posture=state_.template get<std::uint32_t>(0x1a4);
            const auto posture=alarm_enabled()&&alarm_posture?alarm_posture:state_.template get<std::uint32_t>(0x1b0);
            if(!(posture&flags()&7u))this->request_navigation_posture(posture&7u,control,world.mode_flags());
            if(this->human()->field160&&resources.path().state().template get<std::uint32_t>(0x74))
                runtime.advance_script_vehicle_path(*this,elapsed); //00578770→00578790, actual path+74 gate.
            if(state_.template get<std::uint32_t>(0x1c0))runtime.advance_script_timed_action(*this,elapsed); //0058E500.
            this->apply_frame_movement(control);
            this->apply_frame_items(control,world,runtime,elapsed,this->words().template get<std::uint8_t>(0xb4));
        }
        this->process_second_frame_commands();
    }
    void sight_enemy_acquired(Human& actor)override{this->emit_sight(actor,0);interrupt(actor,0x20);} //00590060.
    void damage_interrupt(Human& actor){interrupt(actor,0x40);} //00590400, after the shared AI damage receiver.
    void sight_dead_acquired(Human& actor)override{this->emit_sight(actor,2);interrupt(actor,0x200);} //0058FFE0.
    using AI::sight_enemy_lost;
    Context* context()const{return context_;}
    void script_context(Context* context){context_=context;} //005907C0, actual B0+1AC borrower update.
    template<class Inline>void resume_after_ai(Inline& control){ //00590700.
        auto* human=this->human();
        if(!human)throw std::logic_error("Script AI resume requires the actual Human");
        context_=contexts_.human_script_context(*human);
        if(context_)contexts_.finish_human_script_alarm(*context_);
        this->look_command(0,control);
    }
    void set_alarm(bool enabled) { //0058E710; true is genuinely empty.
        if(enabled)return;
        this->words().set(0x50,this->words().template get<std::uint32_t>(0x50)&0x7fffffffu);
        const auto* human=this->human();if(!human)throw std::logic_error("Script alarm requires its actual Human");
        const auto& enemies=human->perception.lists()[0];
        if(enemies.empty())return; //No random draw on the cold empty-list branch.
        auto* actor=dynamic_cast<Human*>(enemies.at(contexts_.human_script_random()%enemies.size()));
        if(!actor)throw std::runtime_error("Script alarm target requires its actual Human sight receiver");
        this->sight_enemy_acquired(*actor);
    }
    void request_guard_mode() { //0058F330; AI update consumes this request later.
        state_.set(0x1b0,(state_.template get<std::uint32_t>(0x1b0)&0xffff8fffu)|0x8000u);
    }
    void ai_mode(std::uint32_t mask) { //0058F610 replaces ONLY bits700.
        const auto old=this->words().template get<std::uint32_t>(0x50);
        this->words().set(0x50,old^((old^mask)&0x700u));
    }
    template<class Receivers>void events(bool enabled,Receivers& receivers) { //0058F5D0/0058F5F0.
        const auto old=this->words().template get<std::uint32_t>(0x50);
        this->words().set(0x50,enabled?old&0xbfffffffu:old|0x40000000u);
        auto* human=this->human();if(!human)throw std::logic_error("Script event control requires its actual Human");
        receivers.human_events(*human,enabled);
    }
    template<class Receivers>void suspend(bool value,Receivers& receivers) { //0058FAA0.
        if(bool(state_.template get<std::uint8_t>(0x1d2))==value)return;
        if((receivers.mode_flags()&7u)!=7u)
            throw std::runtime_error("Script suspension requires its actual queued command16 and B0+FC receivers outside offline7");
        state_.set(0x1d2,std::uint8_t(value));
        auto* human=this->human();if(!human)throw std::logic_error("Script suspension requires its actual Human");
        receivers.suspend_human(*human,value);
        events(!value,receivers);
    }
    template<class Inline>void working_mode(std::int32_t mode,Inline& control) { //0058FCC0.
        if(state_.template get<std::int32_t>(0x1d4)==mode)return;
        control.working_mode(mode);state_.set(0x1d4,mode);
    }
    bool scene_movement_enabled()const override{return false;} //005A42A0.
    std::uint32_t derived_loading_word(unsigned offset)const override {
        if(offset==0x1d2)return state_.template get<std::uint8_t>(offset); //005A5D30, shared actual suspension byte.
        if(offset!=0x1b0)throw std::runtime_error("Unrecovered script AI handover word");
        return state_.template get<std::uint32_t>(offset);
    }
    auto& state(){return state_;}const auto& state()const{return state_;}
    using Formation=std::remove_reference_t<decltype(*std::declval<Human&>().formation)>;
    Formation* script_formation()const{return formation_;}
    void script_formation(Formation* formation){ //0058D900, actual producer004603C0 / retirement00460850.
        bind_human_script_formation(*this,formation_,formation,[&](std::int32_t pc){
            if(context_)contexts_.finish_human_script_navigation(*context_,pc);
        });
    }
private:
    Formation* formation_=nullptr;
    bool alarm_enabled()const{return state_.template get<std::uint8_t>(0x198)!=0;}
    template<class Runtime>void update_script_attachment(Runtime& runtime){ //0058DB20.
        const auto* attachment=this->human()->field160;
        if(attachment){
            const auto kind=attachment->base().type();
            if(kind==4||kind==0x14||kind==0x15||kind==0x22){runtime.update_script_attachment(*this);return;}
        }
        state_.set(0x1d0,std::uint8_t{0});
    }
    template<class World>void update_script_follow_gate(World& world){
        if(!state_.template get<std::uint8_t>(0x18c))return;
        const auto* player=world.current_human_frame_player();
        if(!player)return;
        const auto origin=this->human()->base().world_position(),target=player->base().world_position();
        const HumanVector delta{{target[0]-origin[0],target[1]-origin[1],target[2]-origin[2],target[3]-origin[3]}};
        const auto square=(static_cast<long double>(delta[0])*delta[0]+
            static_cast<long double>(delta[1])*delta[1])+static_cast<long double>(delta[2])*delta[2];
        auto& resources=this->resources();
        auto& query=resources.query().state;
        if(square<=state_.template get<float>(0x190)){
            if(square<state_.template get<float>(0x194))query.set(0x58,std::uint8_t{0});
        }else{
            query.set(0x58,std::uint8_t{1});
            if(state_.template get<std::uint8_t>(0x18d))resources.direction().set_target(delta);
        }
    }
    template<class Inline,class World,class Runtime>void advance_script_reaction(
            std::uint32_t elapsed,Inline& control,World& world,Runtime& runtime){ //00591930.
        const auto action=state_.template get<std::uint32_t>(0x19c);
        auto& resources=this->resources();
        if(action!=0x80){
            const auto remaining=state_.template get<std::uint32_t>(0x1a0)-elapsed;
            state_.set(0x1a0,remaining);
            if(!this->human()->perception.lists()[0].empty()){
                if(human_frame_signed_word(remaining)<=0)runtime.choose_script_alarm_action(*this); //00591550.
                else if(action==0x40&&(!alarm_target_||world.human_ai_resolve_actor(alarm_target_id_)!=alarm_target_||
                        human_frame_signed_word(alarm_target_->properties.stats_ec[0])<=0)){
                    alarm_target_=select_human_hostile<Human>(*this,this->state_frames(),world);
                    if(alarm_target_)alarm_target_id_=static_cast<std::uint32_t>(alarm_target_->base().id10());
                    resources.target().bind_actor(alarm_target_);
                }
                return;
            }
            if(action!=0x40)return;
        }
        control.clear_frame_item_ready([&](std::uint32_t value){world.human_frame_first_person_state(*this->human(),value);});
        if((world.mode_flags()&7u)!=7u)throw std::runtime_error("Script aim stop requires actual B0 command00 transport");
        resources.flag().request(1);
        resources.target().bind_actor(nullptr);
        resources.query().state.set(0x58,std::uint8_t{0});
        const auto timer=static_cast<std::uint32_t>(static_cast<long double>(world.human_frame_random_word())*
            3.0518509447574615e-05f*3000.f+500.f);
        state_.set(0x19c,action==0x80?0u:0x10u);
        if(action==0x80)state_.set(0x198,std::uint8_t{0});
        state_.set(0x1a0,timer);state_.set(0x1a4,0u);
        alarm_target_=nullptr;
    }
    void interrupt(Human& actor,std::uint32_t event) {
        auto flags=this->words().template get<std::uint32_t>(0x50);
        if((flags&0x80000000u)||(state_.template get<std::uint32_t>(0x1b8)&event))return;
        state_.set(0x1b4,event);this->words().set(0x50,flags|0x80000000u);
        const auto matrix=this->state_frames().actor_world_matrix(actor.base().frame());
        state_.set(0x140,HumanVector{{matrix[12],matrix[13],matrix[14],matrix[15]}});
        if(context_)contexts_.interrupt_human_script_context(*context_);
    }
    Contexts& contexts_;Context* context_=nullptr;Human* alarm_target_=nullptr; //Native1A8.
    std::uint32_t alarm_target_id_=0; //Resolve the original borrower before touching its health.
    HumanJointWords<0x1f0> state_; //Only recovered derived scalars at140+; base/AI live in their actual owners.
};
} //namespace hd2::game
#endif
