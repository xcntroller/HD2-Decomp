#ifndef HD2_GAME_HUMAN_AREA_AI_STATE_LOADING_H
#define HD2_GAME_HUMAN_AREA_AI_STATE_LOADING_H

#include "game/human_ai_state_loading.h"

namespace hd2::game {

//005A3F20 case8,00559A50/00559C20. Actual command-area position/radius lives
//at HumanB4+0..10 and requires its real producer before a previous-state bind.
//It is distinct from guard4 and its radius timers/door borrowers.
template<class Frames,class Human,class Inputs,class Contexts,class Services>
class HumanAreaAIStateLoading : public HumanAIStateLoading<Frames,Human,Inputs> {
public:
    using AI=HumanAIStateLoading<Frames,Human,Inputs>;using Base=typename AI::Base;
    HumanAreaAIStateLoading(Frames& frames,Inputs& inputs,MissionTableSet& second,
            HumanInputCallbacks& callbacks,Contexts& contexts,Services& services)
        :AI(frames,inputs,second,callbacks,8),contexts_(contexts),services_(services) {
        state_.set(0x154,std::uint32_t{0});
    }
    template<class Inline>void bind(Human& human,const Base* previous,MissionNavigation* navigation,
            std::uint32_t mode,Inline& control) {
        AI::bind(human,previous,navigation,mode,control);
        state_.set(0x154,std::uint32_t{0});state_.set(0x15c,std::uint32_t{0});
        this->resources().state().set(0xf0,std::uint32_t{0});
        state_.set(0x150,this->words().template get<std::uint32_t>(0x50)&7u);
        const auto matrix=this->state_frames().actor_world_matrix(human.base().frame());
        state_.set(0x140,HumanVector{{matrix[12],matrix[13],matrix[14],matrix[15]}});
        if(previous) {
            if(!navigation)throw std::runtime_error("Area AI handover requires actual navigation");
            const auto area=services_.human_ai_command_area(human); //Required actual B4 XYZ/radius; no origin fallback.
            const auto point=navigation->nearest_volume({{area.position[0],area.position[1],area.position[2]}},5,-1);
            (void)navigation->append_reachable(point,area.radius,route_);
            for(auto n=route_.size();n;) {
                const auto index=route_[--n];const auto* record=navigation->record(index);
                if(!record)throw std::runtime_error("Area AI route contains an absent navigation point");
                if(record->byte16==10)kind10_.push_back(index); //004DFA30 appends to separate native170 WORD vector.
            }
            auto* storage=human.base().storage();
            if(!storage||storage->single()||!storage->holder(3))throw std::runtime_error("Area AI has no actual hand holder");
            if(!storage->holder(3)->empty()&&!this->item()) {
                if(!human.loading_inventory)throw std::runtime_error("Area AI requires the actual Human inventory command owner");
                if(!human.loading_inventory->equip_shoulder())(void)human.loading_inventory->equip_holster(2); //005A6830(2), exact native conjunction.
            }
            const auto* context=contexts_.human_script_context(human);
            if(!context||contexts_.human_script_status(*context)==-1)state_.set(0x154,std::uint32_t{8});
            const auto bits=human.properties.stats_11c[3];std::int32_t skill;std::memcpy(&skill,&bits,4);
            if(skill>90)state_.set(0x154,state_.template get<std::uint32_t>(0x154)|1u);
            if(skill>75)state_.set(0x154,state_.template get<std::uint32_t>(0x154)|2u); //Independent, strict tests.
        }
        auto flags=this->words().template get<std::uint32_t>(0x50);
        if(!(flags&0x700u))this->words().set(0x50,flags|0x200u);
    }
    void close_loading(){AI::close_loading();this->clear_human_reference();route_.clear();} //00559C20 retains native170 list.
    const auto& route()const{return route_;}const auto& kind10_points()const{return kind10_;}
    auto& state(){return state_;}const auto& state()const{return state_;}
private:
    Contexts& contexts_;Services& services_;HumanJointWords<0x180> state_;
    std::vector<std::uint16_t> route_,kind10_;
};
} //namespace hd2::game
#endif
