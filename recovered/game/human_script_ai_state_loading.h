#ifndef HD2_GAME_HUMAN_SCRIPT_AI_STATE_LOADING_H
#define HD2_GAME_HUMAN_SCRIPT_AI_STATE_LOADING_H

#include "game/human_ai_state_loading.h"

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
        for(auto offset:{0x19cu,0x1a0u,0x1a4u,0x1a8u})state_.set(offset,std::uint32_t{0});
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
        for(auto offset:{0x1c8u,0x1c4u,0x1e0u,0x1ccu})state_.set(offset,std::uint32_t{0});
        state_.set(0x1d0,std::uint8_t{0});state_.set(0x150,std::uint8_t{0});
        return context_!=nullptr; //Native factory ignores this return; absence is not a successful script bind.
    }
    void close_loading(){AI::close_loading();this->clear_human_reference();} //0058D8E0; no invented base close.
    void sight_enemy_acquired(Human& actor)override{this->emit_sight(actor,0);interrupt(actor,0x20);} //00590060.
    void sight_dead_acquired(Human& actor)override{this->emit_sight(actor,2);interrupt(actor,0x200);} //0058FFE0.
    using AI::sight_enemy_lost;
    Context* context()const{return context_;}
    void script_context(Context* context){context_=context;} //005907C0, actual B0+1AC borrower update.
    std::uint32_t derived_loading_word(unsigned offset)const override {
        if(offset!=0x1b0)throw std::runtime_error("Unrecovered script AI handover word");
        return state_.template get<std::uint32_t>(offset);
    }
    auto& state(){return state_;}const auto& state()const{return state_;}
private:
    void interrupt(Human& actor,std::uint32_t event) {
        auto flags=this->words().template get<std::uint32_t>(0x50);
        if((flags&0x80000000u)||(state_.template get<std::uint32_t>(0x1b8)&event))return;
        state_.set(0x1b4,event);this->words().set(0x50,flags|0x80000000u);
        const auto matrix=this->state_frames().actor_world_matrix(actor.base().frame());
        state_.set(0x140,HumanVector{{matrix[12],matrix[13],matrix[14],matrix[15]}});
        if(context_)contexts_.interrupt_human_script_context(*context_);
    }
    Contexts& contexts_;Context* context_=nullptr;
    HumanJointWords<0x1f0> state_; //Only recovered derived scalars at140+; base/AI live in their actual owners.
};
} //namespace hd2::game
#endif
