#ifndef HD2_GAME_HUMAN_PASSIVE_STATES_LOADING_H
#define HD2_GAME_HUMAN_PASSIVE_STATES_LOADING_H

#include "game/human_ai_state_loading.h"

namespace hd2::game {

//005A3F20 case6,005A42F0/005A4310: no additional derived allocations or
//loading fields beyond the actual AI owner. This is a loading receiver only;
//simulation/update and the containing actor publication remain required.
template<class Frames,class Human,class Inputs>
class HumanPassiveAIStateLoading : public HumanAIStateLoading<Frames,Human,Inputs> {
public:
    using AI=HumanAIStateLoading<Frames,Human,Inputs>;
    HumanPassiveAIStateLoading(Frames& frames,Inputs& inputs,MissionTableSet& second,HumanInputCallbacks& callbacks)
        :AI(frames,inputs,second,callbacks,6) {}
    void close_loading(){AI::close_loading();this->clear_human_reference();}
};

//005A3F20 case7,00592140/00592200/005A4450. Native kind7 is the ordinary
//remote/passive state; it is distinct from global offline MODE7. It owns only
//the B0 base and two256-byte scalar arrays. No AI direction/path resources.
template<class Frames,class Human,class Inputs>
class HumanRemoteStateLoading : public HumanStateBaseLoading<Frames,Human,Inputs> {
public:
    using Base=HumanStateBaseLoading<Frames,Human,Inputs>;
    HumanRemoteStateLoading(Frames& frames,Inputs& inputs,HumanInputCallbacks& callbacks)
        :Base(frames,inputs,7),inputs_(inputs),callbacks_(callbacks) {}
    ~HumanRemoteStateLoading(){detach_callbacks();}
    template<class Inline>
    void bind(Human& human,const Base* previous,MissionNavigation* navigation,std::uint32_t mode,Inline& control) {
        Base::bind(human,previous,navigation,mode);
        first_.fill(0);second_.fill(0);
        control.control()->physical().enable(false); //HumanA0+18C.
        auto* input=this->input();
        if(!input)throw std::runtime_error("Remote Human state requires actual input");
        auto* history=input->history(1);
        if(!history)throw std::runtime_error("Remote Human input has no required history1");
        input->first_receiver=&callbacks_;input->first_owner=this;
        input->second_receiver=&callbacks_;input->auxiliary_receiver=&callbacks_;input->second_owner=this;
        //00592140 preserves38/3C/40. AI/player bind changes these fields.
        history->position_value.seed(human.base().world_position());
        this->words().set(0x70,std::uint8_t{0});flag2c0_=flag2c1_=false;bound_=true;
    }
    void close(std::uint32_t mode){Base::close(mode);bound_=false;} //Removes input, unlike state6.
    const auto& first()const{return first_;}const auto& second()const{return second_;}
    void receive_packet_inputs(const HumanPacketCommand& command){ //00592FE0 case0, SAME C0/1C0 arrays.
        for(unsigned n=0;n<64;++n){first_[n]=command.inputs[0].template get<std::uint32_t>(n*4);second_[n]=command.inputs[1].template get<std::uint32_t>(n*4);}
    }
    void clear_packet_action_suppression()noexcept{flag2c1_=false;} //00592FE0 command23.
    bool bound()const{return bound_;}
    bool suppress_hostile_selection()const override{return flag2c1_;} //005A4410.
private:
    void detach_callbacks()noexcept {
        auto* input=this->input();
        if(!input||!inputs_.contains(input))return;
        if(input->first_owner==this){input->first_receiver=nullptr;input->first_owner=nullptr;}
        if(input->second_owner==this){input->second_receiver=nullptr;input->auxiliary_receiver=nullptr;input->second_owner=nullptr;}
    }
    Inputs& inputs_;HumanInputCallbacks& callbacks_;
    std::array<std::uint32_t,64> first_{},second_{};
    bool flag2c0_=false,flag2c1_=false,bound_=false;
};
} //namespace hd2::game
#endif
