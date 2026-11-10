#ifndef HD2_GAME_HUMAN_AREA_AI_STATE_LOADING_H
#define HD2_GAME_HUMAN_AREA_AI_STATE_LOADING_H

#include "game/human_ai_state_loading.h"
#include "game/human_ai_frame_timers.h"

namespace hd2::game {

//005A3F20 case8,00559A50/00559C20. Actual command-area position/radius lives
//at HumanB4+0..10 and requires its real producer before a previous-state bind.
//It is distinct from guard4 and its radius timers/door borrowers.
template<class Frames,class Human,class Inputs,class Contexts,class Services>
class HumanAreaAIStateLoading : public HumanAIStateLoading<Frames,Human,Inputs> {
public:
    using AI=HumanAIStateLoading<Frames,Human,Inputs>;using Base=typename AI::Base;
    using Actor=SceneActor<Frames>;
    HumanAreaAIStateLoading(Frames& frames,Inputs& inputs,MissionTableSet& second,
            HumanInputCallbacks& callbacks,HumanAIAction3Pool& actions,Contexts& contexts,Services& services)
        :AI(frames,inputs,second,callbacks,8),actions_(actions),contexts_(contexts),services_(services) {
        state_.set(0x154,std::uint32_t{0});
    }
    template<class Inline>void bind(Human& human,const Base* previous,MissionNavigation* navigation,
            std::uint32_t mode,Inline& control) {
        AI::bind(human,previous,navigation,mode,control);
        timer_target_=nullptr; //Actual native15C borrower.
        state_.set(0x154,std::uint32_t{0});
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
    Actor* timer_target()const{return timer_target_;}void timer_target(Actor* target){timer_target_=target;}
    template<class Queries>void advance_frame_prefix(std::uint32_t elapsed,Queries& queries){advance_human_area_frame_prefix(*this,elapsed,queries);} //00559C60.
    void select_area_action(){ //0055A130. Area virtual10C is00575820.
        this->resources().retire_current_action();
        //The original stack vector initializes XYZ only.
        this->resources().current_action(actions_.acquire(3,0,std::array<float,3>{}));
        state_.set(0x158,0u);
    }
    std::uint32_t advance_frame_decision(std::uint32_t elapsed){ //00559EB0.
        auto* human=this->human();
        if(!human)throw std::logic_error("Area decision requires the bound Human");
        state_.set(0x158,state_.template get<std::uint32_t>(0x158)+elapsed);
        auto kind=this->resources().current_action_kind();
        if(!kind){
            select_area_action();
            return this->resources().current_action_kind();
        }
        drain_frame_events(); //005A8480 uses the current Area virtualF4.
        kind=this->resources().current_action_kind();
        const auto skill=human_frame_signed_word(human->properties.stats_ec[3]);
        if(kind!=5&&!(state_.template get<std::uint32_t>(0x154)&1u)&&skill<20)
            return kind; //0055B6A0 is a genuine RET.
        if(kind==5&&skill>40)select_area_action();
        else if((state_.template get<std::uint32_t>(0x154)&12u)==4u)
            this->resources().retire_current_action(); //0055A0E0→00575820.
        return this->resources().current_action_kind();
    }
    bool advance_frame_wait_action(){ //0055AD50/0055A6E0, kind 3 wait and no-target modes.
        const auto kind=this->resources().current_action_kind();
        if(kind==5||kind==0)return true; //0055A100 has no worker for these kinds.
        if(kind!=3)return false;
        auto* action=this->resources().template current_action_as<HumanAIAction3Pool>();
        auto* human=this->human();
        auto* command=this->command();
        if(!action||!human||!command)throw std::logic_error("Area worker requires the bound Human, command and kind 3 action");
        auto& words=action->state;
        const auto mode=words.template get<std::uint32_t>(8);
        const bool short_range=(this->words().template get<std::uint32_t>(0x50)&0x400u)!=0;
        const bool expired=words.template get<std::uint32_t>(0x10)==0;
        if(mode==2&&(!expired||short_range))return true;
        if(mode!=0&&mode!=2&&mode!=3)return false;
        if(!human->perception.lists()[0].empty()||command->second.events.size())return false; //0055B860.
        if(mode==0){
            if(expired)state_.set(0x154,state_.template get<std::uint32_t>(0x154)|4u);
            return true;
        }
        //Normal range mode 2 first changes to mode 3, then runs the same wait reset.
        words.set(8,0u);
        std::uint32_t total=0;
        for(unsigned n=0;n<4;++n){
            const auto draw=contexts_.human_script_random();
            if(draw>0x7fffu)throw std::runtime_error("Area wait requires the shared CRT random source");
            total+=draw>>2;
        }
        const auto centered=static_cast<long double>(total)*0x1.0002p-15f-.5;
        const auto lifetime=short_range?centered*6000.+8000.:centered*8000.+16000.;
        words.set(0x10,static_cast<std::uint32_t>(lifetime));
        return true;
    }
private:
    void drain_frame_events(){
        auto* command=this->command();
        if(!command)throw std::logic_error("Area event dispatch requires the actual command owner");
        auto& source=command->first.events;
        auto& processed=command->second.events;
        while(source.size()){
            auto event=source.pop_front();
            switch(event->words[0]){ //00559F60.
            case 0x10000002u:
                processed.merge(std::move(event),30000,false);
                break;
            case 0x3000000bu:case 0x3000000cu:
                if(aim_at_event_target(*event))processed.merge(std::move(event),20000,false);
                break;
            case 0x40000015u:{
                const auto draw=contexts_.human_script_random();
                if(draw>0x7fffu)throw std::runtime_error("Area reaction requires the shared CRT random source");
                this->extend_frame_reaction(static_cast<std::uint32_t>(static_cast<long double>(draw)*0x1.0002p-15f*300.f+100.f));
                (void)aim_at_event_target(*event);
                (void)aim_at_event_target(*event); //Native repeats the common 0x40000010/11 path.
                break;
            }
            case 0x40000010u:case 0x40000011u:
                (void)aim_at_event_target(*event);
                break;
            default:
                break;
            }
        }
    }
    bool aim_at_event_target(const HumanAIEvent& event){
        auto* actor=services_.human_ai_resolve_actor(event.words[4]);
        if(!actor)return false;
        const auto& hostiles=this->human()->perception.lists()[0];
        if(std::find(hostiles.rbegin(),hostiles.rend(),actor)!=hostiles.rend())return false; //005A5E50.
        auto* target=dynamic_cast<Human*>(actor);
        if(!target)throw std::runtime_error("Area event aim requires a Human head marker owner");
        this->resources().direction().target_frame(target->head_marker); //0055D1E0.
        return true;
    }
    HumanAIAction3Pool& actions_;Contexts& contexts_;Services& services_;HumanJointWords<0x180> state_;
    std::vector<std::uint16_t> route_,kind10_;
    Actor* timer_target_=nullptr;
};
} //namespace hd2::game
#endif
