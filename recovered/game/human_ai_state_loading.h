#ifndef HD2_GAME_HUMAN_AI_STATE_LOADING_H
#define HD2_GAME_HUMAN_AI_STATE_LOADING_H

#include "game/human_ai_resources.h"
#include "game/human_input_callbacks.h"
#include "game/human_state_base_loading.h"
#include "game/human_ai_event_queue.h"
#include "game/scene_actor.h"
#include "game/human_ai_direction_frame.h"

namespace hd2::game {

//0056F200's loading path, including offline7. Uses the SAME B0/input/navigation/inline98
//owners. The containing derived state supplies its own fields and receivers.
//The non7 authority-mode initial command packet still requires its actual
//command/transport receiver. Offline7 never constructs that packet or input.
template<class Frames,class Human,class Inputs>
class HumanAIStateLoading : public HumanStateBaseLoading<Frames,Human,Inputs> {
public:
    using Base=HumanStateBaseLoading<Frames,Human,Inputs>;
    HumanAIStateLoading(Frames& frames,Inputs& inputs,MissionTableSet& second,
                        HumanInputCallbacks& callbacks,std::uint32_t kind)
        :Base(frames,inputs,kind),inputs_(inputs),callbacks_(callbacks),resources_(kind,second) {}
    ~HumanAIStateLoading(){detach_callbacks();}
    template<class Inline>
    void bind(Human& human,const Base* previous,MissionNavigation* navigation,std::uint32_t mode,Inline& control) {
        if((mode&7u)!=7u&&(mode&1u)&&!previous)throw std::runtime_error("Human AI authority binding requires actual initial command20 transport");
        Base::bind(human,previous,navigation,mode);
        resources_.state().set(0x11c,std::uint8_t{0});resources_.state().set(0x120,std::uint32_t{0});
        if(this->words().template get<std::uint32_t>(0x50)&0x100000u) {
            if(!navigation)throw std::runtime_error("Human AI interaction query requires actual navigation");
            auto* actor=control.control()->physical().interaction_actor(); //005AC2E0.
            const auto references=navigation->actor_references(actor); //0051E6D0, including null query.
            if(actor&&references.size()==2) {
                auto& query=resources_.query();query.actor=actor;
                query.state.set(0x48,references[0]);query.state.set(0x4a,references[1]);
                query.state.set(0x50,query.state.template get<std::uint32_t>(0x50)|0x10u);
            }
        }
        const auto kind=this->words().template get<std::uint32_t>(0x48);
        if(kind==4||kind==8){control.clear_control_flags(4);control.add_control_flags(2);}
        else if(kind==3){control.clear_control_flags(6);control.add_control_flags(1);}
        else {control.clear_control_flags(6);control.add_control_flags(4);}
        this->refresh_navigation_point(); //Native intentionally repeats the base query after physical changes.
        auto& direction=resources_.direction();
        direction.set_current(this->words().template get<HumanVector>(0x20));
        direction.set_target(this->words().template get<HumanVector>(0x10));
        if((mode&7u)==7u){bound_=true;return;} //Native7C remains NULL; no callback/history/packet side effects.
        auto* input=this->input();
        if(!input)throw std::runtime_error("Human AI requires the actual input record");
        //All three slots use actual callback bodies, dispatched by their slot.
        input->first_receiver=&callbacks_;input->first_owner=this;
        input->second_receiver=&callbacks_;input->auxiliary_receiver=&callbacks_;input->second_owner=this;
        input->word38=7;input->flag40=false;if(!(mode&1u))input->word3c=0;
        auto* history=input->history(1);
        if(!history) {detach_callbacks();throw std::runtime_error("Human AI input has no required history1");}
        history->position_value.seed(human.base().world_position());
        bound_=true;
    }
    //0056F630's loading branch: F8 jobs/104 current job stay empty; typed
    //F0 actions return to their typed process pools. Base84/94
    //commands also have no producer here. No successful runtime close is claimed.
    void close_loading() {
        if(resources_.state().template get<std::uint32_t>(0xf0))
            throw std::runtime_error("Human AI close requires actual action-pool retirement");
        resources_.retire_current_action();
        bound_=false; //Native does not call00583690 or remove the shared input.
    }
    auto& resources(){return resources_;}const auto& resources()const{return resources_;}
    HumanVector current_direction()const override{return resources_.direction().state().template get<HumanVector>(0);} //0056FBF0.
    HumanVector target_direction()const override{return resources_.direction().state().template get<HumanVector>(0x10);} //0056FC00.
    bool bound()const{return bound_;}
    template<class Queries>void advance_frame_timers(std::uint32_t elapsed,Queries& queries){
        advance_human_ai_frame_timers(resources_.state(),elapsed);Base::advance_frame_timers(elapsed,queries); //00571680→00588010.
    }
    void extend_frame_reaction(std::uint32_t duration){ //00571A10.
        auto& words=resources_.state();
        if(words.template get<std::uint32_t>(0xc8)<duration)words.set(0xc8,duration);
        if(this->words().template get<std::uint32_t>(0x50)&0x200000u)
            words.set(0xc4,words.template get<std::uint32_t>(0xc4)+duration);
    }
    template<class Inline>void look_command(std::uint32_t command,Inline& control,const HumanVector* target=nullptr){ //00570A10/00570980.
        control.look_command(command,target); //005A45F0→005ABD90 kind5, BEFORE flag publication.
        const auto old=this->words().template get<std::uint32_t>(0x50);
        this->words().set(0x50,command==3||command==4?old|0x400000u:old&0xffbfffffu);
    }
    template<class Inline>std::uint32_t advance_frame_direction(std::uint32_t elapsed,Inline& control){
        auto look=[&](std::uint32_t command,const HumanVector* target){look_command(command,control,target);};
        return advance_human_ai_frame_direction(*this,elapsed,this->state_frames(),look); //00575940.
    }
    HumanSightMetricFamily sight_metric_family()const override{return HumanSightMetricFamily::ai;} //B0+34=0056FC80,+38=00570280.
    void sight_enemy_acquired(Human& actor) override {
        const auto kind=this->words().template get<std::uint32_t>(0x48);
        if(kind==6)return; //005A4360 genuinely returns without side effects.
        if(kind!=4&&kind!=8)throw std::runtime_error("Human sight acquisition requires its actual derived B0 receiver");
        emit_sight(actor,0); //00574070.
    }
    void sight_enemy_lost(Human& actor) override {
        const auto kind=this->words().template get<std::uint32_t>(0x48);
        if(kind==6)return; //005A4370.
        if(kind!=4&&kind!=5&&kind!=8)throw std::runtime_error("Human sight loss requires its actual derived B0 receiver");
        emit_sight(actor,1); //00574130, inherited by5 as well as4/8.
    }
    void sight_dead_acquired(Human& actor) override {
        const auto kind=this->words().template get<std::uint32_t>(0x48);
        if(kind!=4&&kind!=6&&kind!=8)throw std::runtime_error("Human dead-sight requires its actual derived B0 receiver");
        emit_sight(actor,2); //005741F0.
    }
protected:
    void emit_sight(Human& actor,unsigned kind) {
        auto* command=this->command();if(!command)throw std::logic_error("Human sight callback has no bound command owner");
        queue_human_ai_sight_event(this->state_frames(),command->first.events,actor,kind);
    }
private:
    void detach_callbacks()noexcept {
        auto* input=this->input();
        if(!input||!inputs_.contains(input))return;
        //A later state binds BEFORE the previous state is destroyed. Never
        //erase that replacement's borrowers. Guard only against a dead owner.
        if(input->first_owner==this){input->first_receiver=nullptr;input->first_owner=nullptr;}
        if(input->second_owner==this){input->second_receiver=nullptr;input->auxiliary_receiver=nullptr;input->second_owner=nullptr;}
    }
    Inputs& inputs_;HumanInputCallbacks& callbacks_;HumanAIResources<SceneActor<Frames>> resources_;bool bound_=false;
};
} //namespace hd2::game
#endif
