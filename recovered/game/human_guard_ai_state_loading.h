#ifndef HD2_GAME_HUMAN_GUARD_AI_STATE_LOADING_H
#define HD2_GAME_HUMAN_GUARD_AI_STATE_LOADING_H

#include "game/human_ai_state_loading.h"
#include <optional>

namespace hd2::game {

//005A3F20 case4,005478C0/00547CF0. Runtime current-action/event producers
//remain separate; three route/door buffers have actual typed ownership.
//Contexts resolves modelEC data1/statusEC; handover replays actual005518F0.
template<class Frames,class Human,class Inputs,class Random,class Contexts>
class HumanGuardAIStateLoading : public HumanAIStateLoading<Frames,Human,Inputs> {
public:
    using AI=HumanAIStateLoading<Frames,Human,Inputs>;using Base=typename AI::Base;
    HumanGuardAIStateLoading(Frames& frames,Inputs& inputs,MissionTableSet& second,
            HumanInputCallbacks& callbacks,Random& random,Contexts& contexts)
        :AI(frames,inputs,second,callbacks,4),random_(random),contexts_(contexts){}
    template<class Inline>void bind(Human& human,const Base* previous,MissionNavigation* navigation,
            std::uint32_t mode,Inline& control) {
        AI::bind(human,previous,navigation,mode,control);
        this->resources().state().set(0xf0,std::uint32_t{0});
        for(auto offset:{0x1b0u,0x148u,0x150u,0x144u,0x14cu,0x19cu,0x1a8u,0x1c4u,0x1d0u,0x1dcu})state_.set(offset,std::uint32_t{0});
        state_.set(0x190,std::int32_t{-1});state_.set(0x154,std::uint8_t{0});
        state_.set(0x1ac,static_cast<std::int32_t>(static_cast<long double>(random_word())*random_scale*8000.f));
        state_.set(0x1c8,std::uint32_t{5000});state_.set(0x1d8,std::int32_t{-1});state_.set(0x140,std::uint8_t{0});
        const auto matrix=this->state_frames().actor_world_matrix(human.base().frame());
        state_.set(0x180,HumanVector{{matrix[12],matrix[13],matrix[14],matrix[15]}});
        //005A6600(0), used only as an ignored pure stat getter when A4 isNULL.
        if(!this->point())(void)human.properties.stats_ec[0];
        if(previous) {
            auto* command=this->command();if(!command)throw std::logic_error("Guard AI handover has no command owner");
            if(command->first.events.size()) {
                do {command->second.events.merge(command->first.events.pop_front(),10000,true);}while(command->first.events.size());
            }else contexts_.replay_guard_ai_handover(*this,*previous); //Actual005518F0, including Human2AC records.
        }
        auto flags=this->words().template get<std::uint32_t>(0x50);
        if(flags&4u){state_.set(0x1a0,.45f);speed_=.45f;}
        else if(flags&2u){state_.set(0x1a0,.59f);speed_=.59f;}
        else if(flags&1u){state_.set(0x1a0,0.f);speed_=0.f;}
        else speed_.reset(); //Original leaves the scalar unwritten in this case.
        if(!(flags&0x700u)){flags|=0x200u;this->words().set(0x50,flags);}
        if(flags&0x100u)constant_timers();
        else if(flags&0x200u)radius_timers(false);
        else if(flags&0x400u)radius_timers(true);
        const auto* context=contexts_.human_script_context(human);
        if(!context||contexts_.human_script_status(*context)==-1)
            state_.set(0x144,state_.template get<std::uint32_t>(0x144)|8u);
        const auto skill=signed_stat(human,3);
        if(skill>=90)state_.set(0x144,state_.template get<std::uint32_t>(0x144)|1u);
        else if(skill>=75)state_.set(0x144,state_.template get<std::uint32_t>(0x144)|2u);
        control.look_command(0,nullptr); //00570A10(0)→005A45F0 command5.
        this->words().set(0x50,this->words().template get<std::uint32_t>(0x50)&~0x400000u);
    }
    void close_loading(){AI::close_loading();primary_.clear();secondary_.clear();doors_.clear();} //00547CF0 retains Human44.
    void constant_timers(){state_.set(0x1a4,std::int32_t{100000});state_.set(0x1a8,std::int32_t{10000});} //00547AD0.
    void radius_timers(bool short_range) { //00547AF0/00547BF0.
        auto* human=this->human();if(!human)throw std::logic_error("Guard radius timers have no bound Human");
        state_.set(0x194,static_cast<float>(static_cast<long double>(random_word())*random_scale*(short_range?5.f:10.f)+(short_range?7.f:20.f)));
        const auto scaled=static_cast<long double>(signed_stat(*human,9))*(short_range?.3333333432674407958984375f:.5f);
        const auto minimum=short_range?25.L:50.L;
        state_.set(0x1a4,static_cast<std::int32_t>(scaled>minimum?scaled:minimum));state_.set(0x1a8,std::uint32_t{0});
        auto* navigation=this->navigation();if(!navigation)throw std::runtime_error("Guard radius timers require the actual navigation owner");
        if(state_.template get<std::int32_t>(0x190)<0) {
            const auto matrix=this->state_frames().actor_world_matrix(human->base().frame());
            state_.set(0x190,static_cast<std::int32_t>(navigation->nearest_volume({{matrix[12],matrix[13],matrix[14]}},5,-1)));
        }
        const auto point=static_cast<std::uint16_t>(state_.template get<std::uint32_t>(0x190));
        const auto radius=state_.template get<float>(0x194);
        primary_.clear();(void)navigation->append_reachable(point,radius,primary_); //00551B00.
        secondary_.clear();(void)navigation->append_marked_reachable(point,radius,secondary_); //00551B50.
        doors_.clear(); //00551BA0, reverse PRIMARY points, not the filtered secondary list.
        for(auto n=primary_.size();n;) {
            const auto index=primary_[--n];const auto* record=navigation->record(index);
            if(!record)throw std::runtime_error("Guard AI radius includes an absent navigation point");
            if(record->byte16==11)if(auto* object=navigation->object_for_reference(index))doors_.push_back(object);
        }
    }
    const auto& primary_route()const{return primary_;}const auto& secondary_route()const{return secondary_;}
    const auto& doors()const{return doors_;}std::optional<float> speed()const{return speed_;}
    auto& state(){return state_;}const auto& state()const{return state_;}
private:
    static constexpr float random_scale=0x1.0002p-15f; //0080FF64, actual CRT rand divisor.
    std::uint32_t random_word(){const auto value=random_();if(value>0x7fff)throw std::runtime_error("Guard AI requires shared CRT random");return value;}
    static std::int32_t signed_stat(const Human& human,unsigned index){const auto bits=human.properties.stats_ec.at(index);std::int32_t value;std::memcpy(&value,&bits,4);return value;}
    Random& random_;Contexts& contexts_;HumanJointWords<0x1e0> state_;
    std::vector<std::uint16_t> primary_,secondary_;std::vector<void*> doors_;
    std::optional<float> speed_;
};
} //namespace hd2::game
#endif
