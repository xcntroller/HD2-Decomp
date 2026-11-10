#ifndef HD2_GAME_HUMAN_GUARD_AI_STATE_LOADING_H
#define HD2_GAME_HUMAN_GUARD_AI_STATE_LOADING_H

#include "game/human_ai_state_loading.h"
#include "game/human_ai_frame_timers.h"
#include <optional>

namespace hd2::game {

//005A3F20 case4,005478C0/00547CF0. Runtime current-action/event producers
//remain separate; three route/door buffers have actual typed ownership.
//Contexts resolves modelEC data1/statusEC; handover replays actual005518F0.
template<class Frames,class Human,class Inputs,class Random,class Contexts>
class HumanGuardAIStateLoading : public HumanAIStateLoading<Frames,Human,Inputs> {
public:
    using AI=HumanAIStateLoading<Frames,Human,Inputs>;using Base=typename AI::Base;
    using Actor=SceneActor<Frames>;
    HumanGuardAIStateLoading(Frames& frames,Inputs& inputs,MissionTableSet& second,
            HumanInputCallbacks& callbacks,HumanAIActionPools& actions,Random& random,Contexts& contexts)
        :AI(frames,inputs,second,callbacks,4),actions_(actions),random_(random),contexts_(contexts){}
    template<class Inline>void bind(Human& human,const Base* previous,MissionNavigation* navigation,
            std::uint32_t mode,Inline& control) {
        AI::bind(human,previous,navigation,mode,control);
        timer_target_=nullptr; //Native19C borrower, independent from AI128's target actor.
        this->resources().state().set(0xf0,std::uint32_t{0});
        for(auto offset:{0x1b0u,0x148u,0x150u,0x144u,0x14cu,0x1a8u,0x1c4u,0x1d0u,0x1dcu})state_.set(offset,std::uint32_t{0});
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
    Actor* timer_target()const{return timer_target_;}void timer_target(Actor* target){timer_target_=target;}
    void refresh_reaction_timer(){refresh_human_guard_reaction_timer(*this,this->state_frames(),random_);} //00547DA0.
    template<class Queries>void advance_frame_prefix(std::uint32_t elapsed,Queries& queries){advance_human_guard_frame_prefix(*this,elapsed,queries);} //00547E70.
    void reset_frame_action()override{ //00548040→005ADD20, then00575820.
        const auto kind=this->resources().current_action_kind();
        if(kind==2)selected_event_.reset();
        else if(selected_event_)throw std::logic_error("Guard task event has no matching kind 2 action");
        this->resources().retire_current_action();
    }
    const HumanAIEvent* selected_frame_event()const{return selected_event_.get();}
    template<class World,class Inline>void select_frame_task(World& world,Inline& control){ //00549360.
        auto* human=this->human();
        auto* command=this->command();
        if(!human||!command)throw std::logic_error("Guard task selection requires the bound Human and command owner");
        state_.set(0x198,0u);
        this->resources().target().bind_actor(nullptr);
        state_.set(0x1b0,0u);
        if(this->words().template get<std::uint32_t>(0x50)&1u)request_posture(2,control,world.mode_flags());
        const bool replacing=this->resources().current_action_kind()!=0;
        if(replacing)reset_frame_action();

        auto& queue=command->second.events;
        const auto* event=queue.strongest_at(frame_position(*human));
        if(event){
            if(auto* target=world.human_ai_resolve_actor(event->words[4])){
                const auto mode=replacing?6u:(event->words[0]&0xf0000000u)==0x40000000u?0u:4u;
                select_target_task(queue,*event,*target,mode,false);
                return;
            }
            const auto kind=event->words[0];
            if(kind==0x20000012u||kind==0x20000004u){
                auto* source=world.human_ai_resolve_actor(event->words[3]);
                if(kind==0x20000012u&&!source)
                    throw std::runtime_error("Guard task 0x20000012 requires its actual source actor");
                if(source&&(kind==0x20000012u||world.human_sound_friendly(*human,source))){
                    auto* source_human=dynamic_cast<Human*>(source);
                    if(!source_human||!source_human->command)
                        throw std::runtime_error("Guard follow task requires the source Human command owner");
                    const auto* linked=source_human->command->second.events.last_of_kind(0x10000016u); //005AE270, reverse.
                    if(linked)if(auto* target=world.human_ai_resolve_actor(linked->words[4])){
                        select_target_task(queue,*event,*target,4,true);
                        return;
                    }
                }
            }
        }
        //00549690 initializes only XYZ in each stack vector. W is unspecified.
        selected_event_.reset();
        const std::array<float,3> origin{};
        auto action=actions_.kind2.acquire(origin,origin,2);
        auto& words=action->state;
        this->resources().current_action(std::move(action));
        words.set(0x10,5000u);
    }
    template<class World,class Inline>bool advance_frame_wait_action(World& world,Inline& control){ //00549AE0 modes 2, 0D, 0C.
        if(this->resources().current_action_kind()!=2)return false;
        auto* action=this->resources().template current_action_as<HumanAIAction2Pool>();
        if(!action)throw std::logic_error("Guard kind 2 action requires its typed pool record");
        auto& words=action->state;
        const auto action_mode=words.template get<std::uint32_t>(8);
        if(action_mode!=2&&action_mode!=0xd&&action_mode!=0xc)return false;
        auto* human=this->human();
        if(!human)throw std::logic_error("Guard action worker requires the bound Human");
        if(!human->perception.lists()[0].empty()&&can_interrupt_task(control)){
            select_hostile_action();
            return true;
        }
        auto& query=this->resources().query();
        if(action_mode==0xd){
            if(!(query.state.template get<std::uint32_t>(0x50)&0x80000000u))return true;
            this->resources().flag().request(0);
            words.set(8,0xcu);
            words.set(0x10,static_cast<std::uint32_t>(static_cast<long double>(random_word())*random_scale*3000.f+2000.f));
            return true;
        }
        if(action_mode==0xc){
            const auto flags=state_.template get<std::uint32_t>(0x144);
            if(!words.template get<std::uint32_t>(0x10)&&!(flags&0xcu))state_.set(0x144,flags|4u);
            return true;
        }
        selected_event_.reset();
        if(!(this->words().template get<std::uint32_t>(0x50)&0x400000u))this->look_command(3,control);
        if(action->state.template get<std::uint32_t>(0x10))return true;
        control.clear_frame_item_ready([&](std::uint32_t value){world.human_frame_first_person_state(*human,value);}); //0058C400.
        if((world.mode_flags()&7u)!=7u)throw std::runtime_error("Guard readiness command requires its actual nonoffline packet receiver");
        request_posture(4,control,world.mode_flags());
        this->resources().flag().request(1);
        query.state.set(0x54,0x2008u);
        query.request_position(state_.template get<HumanVector>(0x180));
        action->state.set(8,0xdu);
        return true;
    }
private:
    template<class Inline>static bool can_interrupt_task(Inline& control){ //00553CC0→005AC2B0.
        const auto* animation=control.control();
        if(!animation)throw std::logic_error("Guard worker requires the actual DD0 owner");
        const auto state=animation->animation_state_word(0x1d8);
        return state!=8&&state!=14&&(state<106||state>108);
    }
    void select_hostile_action(){ //00548890/0054AB00.
        reset_frame_action();
        auto& query=this->resources().query().state;
        query.set(0x50,query.template get<std::uint32_t>(0x50)|0x2000u);
        if(this->words().template get<std::uint32_t>(0x50)&0x100u){
            this->resources().current_action(actions_.kind3.acquire(0,0,std::array<float,3>{}));
            state_.set(0x1cc,0u);
            state_.set(0x198,0u);
        }else if(const auto point=state_.template get<std::int32_t>(0x190);point>=0){
            auto action=actions_.kind1.acquire(state_.template get<HumanVector>(0x180),
                static_cast<std::uint16_t>(point),state_.template get<float>(0x194),this->target_direction());
            auto& words=action->state;
            this->resources().current_action(std::move(action));
            state_.set(0x198,0u);
            words.set(8,1u);
        }
    }
    HumanVector frame_position(Actor& actor){
        const auto matrix=this->state_frames().actor_world_matrix(actor.base().frame());
        return {{matrix[12],matrix[13],matrix[14],matrix[15]}};
    }
    void select_target_task(HumanAIEventQueue& queue,const HumanAIEvent& event,Actor& target,
            std::uint32_t mode,bool retarget){
        HumanVector position;
        std::memcpy(position.data(),event.words.data()+8,sizeof position);
        this->resources().current_action(actions_.kind2.acquire(position,frame_position(target),mode));
        auto copy=queue.pool().acquire();
        copy->words[0]=event.words[0];
        copy->words[1]=10000;
        copy->words[2]=event.words[2];
        copy->words[3]=event.words[3];
        copy->words[4]=retarget?static_cast<std::uint32_t>(target.base().id10()):event.words[4];
        std::copy_n(event.words.begin()+8,5,copy->words.begin()+8);
        selected_event_=std::move(copy);
    }
    template<class Inline>void request_posture(std::uint32_t posture,Inline& control,std::uint32_t mode){ //00559360→005767F0.
        const auto flags=this->words().template get<std::uint32_t>(0x50);
        if(flags&posture)return;
        const auto speed=posture==2?.59f:.45f;
        state_.set(0x1a0,speed);
        speed_=speed;
        this->resources().state().set(0x10c,0u);
        this->request_navigation_posture(posture,control,mode);
    }
    static constexpr float random_scale=0x1.0002p-15f; //0080FF64, actual CRT rand divisor.
    std::uint32_t random_word(){const auto value=random_();if(value>0x7fff)throw std::runtime_error("Guard AI requires shared CRT random");return value;}
    static std::int32_t signed_stat(const Human& human,unsigned index){const auto bits=human.properties.stats_ec.at(index);std::int32_t value;std::memcpy(&value,&bits,4);return value;}
    HumanAIActionPools& actions_;Random& random_;Contexts& contexts_;HumanJointWords<0x1e0> state_;
    std::vector<std::uint16_t> primary_,secondary_;std::vector<void*> doors_;
    std::optional<float> speed_;
    HumanAIEventPool::Lease selected_event_{nullptr,HumanAIEventPool::Return{}}; //Native148, a separate clone from the processed queue.
    Actor* timer_target_=nullptr; //Actual19C; scalar guard words never publish a host pointer.
};
} //namespace hd2::game
#endif
