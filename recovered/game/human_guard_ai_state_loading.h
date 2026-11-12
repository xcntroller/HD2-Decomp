#ifndef HD2_GAME_HUMAN_GUARD_AI_STATE_LOADING_H
#define HD2_GAME_HUMAN_GUARD_AI_STATE_LOADING_H

#include "game/human_ai_state_loading.h"
#include "game/human_ai_frame_timers.h"
#include "game/human_ai_cover.h"
#include <optional>

namespace hd2::game {

//005A3F20 case4,005478C0/00547CF0. Runtime current-action/event producers
//remain separate; three route/emplacement buffers have actual typed ownership.
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
        selected_emplacement_=nullptr;timer_target_=nullptr; //Native19C borrower, independent from AI128's target actor.
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
    void close_loading(){AI::close_loading();primary_.clear();secondary_.clear();emplacements_.clear();} //00547CF0 retains Human44.
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
        emplacements_.clear(); //00551BA0, reverse PRIMARY points, not the filtered secondary list.
        for(auto n=primary_.size();n;) {
            const auto index=primary_[--n];const auto* record=navigation->record(index);
            if(!record)throw std::runtime_error("Guard AI radius includes an absent navigation point");
            if(record->byte16==11)if(auto* object=navigation->object_for_reference(index))emplacements_.push_back(object);
        }
    }
    const auto& primary_route()const{return primary_;}const auto& secondary_route()const{return secondary_;}
    const auto& emplacements()const{return emplacements_;}std::optional<float> speed()const{return speed_;}
    template<class World>void* select_emplacement(World& world){ //0054E600, timer draw precedes even an empty list.
        state_.set(0x1c4,static_cast<std::uint32_t>((static_cast<long double>(random_word())*random_scale+1)*2000.f));
        std::vector<void*> available;
        for(auto i=emplacements_.size();i;){auto* actor=emplacements_[--i];
            auto* occupant=world.human_seat_occupant(actor,0);
            if((!occupant||human_frame_signed_word(world.human_seat_occupant(actor,0)->properties.stats_ec[0])<1)&&!reject_emplacement(actor,world))available.push_back(actor);
        }
        if(available.empty())return nullptr;
        if(available.size()==1)return available.front();
        const auto origin=frame_position(*this->human());void* selected=nullptr;float best=10000;
        for(auto i=available.size();i;){auto* actor=available[--i];const auto p=world.human_emplacement_position(actor);const auto x=static_cast<long double>(p[0])-origin[0],y=static_cast<long double>(p[1])-origin[1],z=static_cast<long double>(p[2])-origin[2];const float square=static_cast<float>((x*x+y*y)+z*z);if(square<best){best=square;selected=actor;}}
        return selected; //0054E520 reverse strict nearest.
    }
    auto& state(){return state_;}const auto& state()const{return state_;}
    Actor* timer_target()const{return timer_target_;}void timer_target(Actor* target){timer_target_=target;}
    void refresh_reaction_timer(){refresh_human_guard_reaction_timer(*this,this->state_frames(),random_);} //00547DA0.
    template<class Queries>void advance_frame_prefix(std::uint32_t elapsed,Queries& queries){advance_human_guard_frame_prefix(*this,elapsed,queries);} //00547E70.
    template<class Queries,class Inline,class World,class Runtime>
    bool advance_authority_frame(std::uint32_t elapsed,Queries& queries,Inline& control,World& world,Runtime& runtime){ //00548170.
        this->publish_authority_input_gate(control);
        if(this->words().template get<std::uint32_t>(0x50)&0x4000800u){
            this->process_second_frame_commands();
            return false;
        }
        advance_frame_prefix(elapsed,queries);
        if(advance_frame_decision(elapsed,control,world)){
            advance_frame_action(elapsed,world,control);
            const auto* attachment=this->human()->field160;
            const auto type=attachment?attachment->base().type():0;
            if(attachment&&(type==4||type==0x14||type==0x15||type==0x22)){
                this->advance_frame_direction(elapsed,control);
                world.human_attachment_frame_inputs(*this);
            }else{
            this->resources().reconcile_direction_flags();
            this->advance_frame_query(control,world,elapsed,runtime);
            const auto was_firing=this->words().template get<std::uint32_t>(0x50)&0x200000u;
            this->advance_frame_aim(world); //00548080 ->00576350.
            if(was_firing&&!(this->words().template get<std::uint32_t>(0x50)&0x200000u))
                finish_frame_aim_stop(world);
            this->advance_frame_direction(elapsed,control);
            this->apply_frame_movement(control);
            }
            this->apply_frame_items(control,world,runtime,elapsed,0);
        }
        this->process_second_frame_commands();
        if(this->resources().current_action_kind()&&!(this->words().template get<std::uint32_t>(0x50)&0x400u))return false;
        adjust_human_frame_stat3(*this->human(),100); //00547D80.
        reset_frame_action();
        return true;
    }
    void reset_frame_action()override{ //00548040→005ADD20, then00575820.
        const auto kind=this->resources().current_action_kind();
        if(kind==2)selected_event_.reset();
        else if(selected_event_)throw std::logic_error("Guard task event has no matching kind 2 action");
        this->resources().retire_current_action();
    }
    const HumanAIEvent* selected_frame_event()const{return selected_event_.get();}
    bool suppress_hostile_selection()const override{ //00558E20.
        const auto* idle=this->resources().template current_action_as<HumanAIAction5Pool>();
        return !(this->words().template get<std::uint32_t>(0x50)&0x4000800u)&&
            idle&&idle->state.template get<std::uint32_t>(8)==0x2000003u;
    }
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
    template<class World,class Inline>bool advance_frame_wait_action(World& world,Inline& control){ //00549AE0, cold arms and complete positive task-mode dispatch.
        if(this->resources().current_action_kind()!=2)return false;
        auto* action=this->resources().template current_action_as<HumanAIAction2Pool>();
        if(!action)throw std::logic_error("Guard kind 2 action requires its typed pool record");
        auto& words=action->state;
        const auto action_mode=words.template get<std::uint32_t>(8);
        if(action_mode!=2&&action_mode!=0xd&&action_mode!=0xc)return advance_positive_task_action(world,control);
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
    std::uint16_t select_cover_point(Actor* target){ //005579E0, kind1 only.
        auto* action=this->resources().template current_action_as<HumanAIAction1Pool>();if(!action)return 0xffff;
        if(!target){action->state.set(8,0u);return 0xffff;}
        if(secondary_.empty()){action->state.set(8,8u);return 0xffff;}
        auto points=secondary_;auto* navigation=this->navigation();if(!navigation)throw std::logic_error("Guard cover requires actual navigation");
        const auto origin=frame_position(*this->human());const auto target_position=frame_position(*target);
        (void)human_ai_cover::filter_points(*navigation,origin,target_position,points,0xc0004102u);
        //The second filter shifts, retaining surviving order before the CRT modulo.
        for(auto n=points.size();n;){const auto i=--n;const auto* point=navigation->record(points[i]);if(!point)continue;
            const auto p=human_ai_cover::position(*point);const auto current=frame_position(*target);
            const auto distance=human_ai_cover::length(static_cast<float>(human_ai_cover::square(human_ai_cover::difference(current,p))));
            if(distance-static_cast<long double>(point->prefix[0x13])*.0625f<6.f)points.erase(points.begin()+static_cast<std::ptrdiff_t>(i));
        }
        return points.empty()?0xffff:points[random_word()%points.size()];
    }
    //00551E80/00551DB0: preferred engagement ranges are independent of
    //the wider minimum/maximum fire predicates used by00553D10.
    template<class World>std::array<float,2> preferred_range(World& world){
        if(!this->item())return {{20,60}};
        switch(weapon_kind(world)){case 2:return {{5,20}};case 3:return {{6,30}};case 4:return {{10,60}};
        case 5:return {{15,70}};case 6:return {{8,35}};case 7:return {{5,15}};case 9:return {{12,45}};case 12:return {{10,70}};default:return {{0,2}};}
    }
    template<class World>int preferred_range_status(World& world){const float distance=static_cast<float>(human_ai_actor_distance(*this->human(),*timer_target_,this->state_frames()));const auto range=preferred_range(world);return distance<range[0]?-1:range[1]<distance?1:0;}
    std::uint16_t approach_point(const HumanVector& target){ //005572F0.
        if(human_ai_cover::square(target)<1.e-16f)return 0xffff;
        auto* navigation=this->navigation();if(!navigation)throw std::logic_error("Guard approach requires actual navigation");
        auto points=primary_;
        if(this->words().template get<std::uint32_t>(0x50)&0x100u){points.clear();(void)navigation->append_reachable((this->point()?navigation->index_of(*this->point()):std::uint16_t{0xffff}),15.f,points);}
        //00556610 only writes two discarded stack distances; no owner mutation.
        return human_ai_cover::filter_points(*navigation,frame_position(*this->human()),target,points,0x40004101u);
    }
    template<class World>std::uint16_t retreat_point(World& world){ //00552B00.
        if(!(human_ai_actor_distance(*this->human(),*timer_target_,this->state_frames())>=12.f)&&!weapon_allows_crouch(world))return 0xffff;
        auto* navigation=this->navigation();if(!navigation||!this->point())return 0xffff;
        std::vector<std::uint16_t> points;(void)navigation->append_reachable((this->point()?navigation->index_of(*this->point()):std::uint16_t{0xffff}),20.f,points);
        const auto origin=frame_position(*this->human());auto toward=human_ai_cover::difference(frame_position(*timer_target_),origin);
        normalize_human_ai_shot(toward,human_ai_cover::square(toward));
        for(auto n=points.size();n;){const auto i=--n;const auto* point=navigation->record(points[i]);if(!point)throw std::logic_error("Guard retreat lost actual point");auto delta=human_ai_cover::difference(human_ai_cover::position(*point),origin);
            const auto dot=[&]{return (static_cast<long double>(delta[2])*toward[2]+static_cast<long double>(delta[1])*toward[1])+static_cast<long double>(delta[0])*toward[0];};
            bool remove=dot()>=0;
            if(!remove){normalize_human_ai_shot(delta,human_ai_cover::square(delta));remove=dot()<-.9599999785423279f;}
            if(remove){points[i]=points.back();points.pop_back();}
        }
        std::vector<std::uint32_t> scores(points.size());
        for(auto n=points.size();n;){const auto i=--n;const auto* point=navigation->record(points[i]);if(point==this->point())continue;
            const float own_square=static_cast<float>(human_ai_cover::square(human_ai_cover::difference(human_ai_cover::position(*point),frame_position(*this->human()))));
            const auto target_square=human_ai_cover::square_xyz(human_ai_cover::difference(human_ai_cover::position(*point),frame_position(*timer_target_)));
            const float reciprocal=own_square>1?static_cast<float>(5.L/own_square):5.f;const float factor=static_cast<float>(static_cast<long double>(reciprocal)+target_square);
            const auto& enemies=this->human()->perception.lists()[0];for(auto h=enemies.size();h;){const auto mask=human_ai_cover::blocked_toward(*point,frame_position(*enemies[--h]));scores[i]+=static_cast<long double>(mask&4u)*factor!=0?0u:mask&2u?3u:(mask&1u)*2u;}
        }
        std::int32_t best=-1;std::uint16_t selected=0xffff;
        for(auto n=points.size();n;){const auto i=--n;std::int32_t score;std::memcpy(&score,&scores[i],4);if(score>best){best=score;selected=points[i];}}
        return selected;
    }
    template<class World>std::uint16_t nearby_attack_point(World& world){ //00553580.
        if(!(human_ai_actor_distance(*this->human(),*timer_target_,this->state_frames())>=12.f)&&!weapon_allows_crouch(world))return 0xffff;
        auto* navigation=this->navigation();if(!navigation||!this->point())return 0xffff;
        std::vector<std::uint16_t> points;(void)navigation->append_reachable((this->point()?navigation->index_of(*this->point()):std::uint16_t{0xffff}),10.f,points);
        const auto origin=frame_position(*this->human()),target=frame_position(*timer_target_);auto toward=human_ai_cover::difference(target,origin);normalize_human_ai_shot(toward,human_ai_cover::square(toward));
        const auto maximum=maximum_weapon_range(world),minimum=minimum_weapon_range(world);
        for(auto n=points.size();n;){const auto i=--n;const auto* point=navigation->record(points[i]);if(!point)throw std::logic_error("Guard attack route lost actual point");const auto p=human_ai_cover::position(*point),delta=human_ai_cover::difference(p,origin);
            const auto dot=(static_cast<long double>(delta[2])*toward[2]+static_cast<long double>(delta[1])*toward[1])+static_cast<long double>(delta[0])*toward[0];
            bool remove=!(dot>=0);if(!remove){const auto distance=human_ai_cover::distance_xyz(target,p);remove=distance<minimum||maximum<distance;}
            if(remove){points[i]=points.back();points.pop_back();}
        }
        //Both model queries are repeated after the candidate filter in native.
        const auto live_target=frame_position(*timer_target_),live_origin=frame_position(*this->human());
        return human_ai_cover::filter_points(*navigation,live_origin,live_target,points,0xc0004101u);
    }
    std::uint16_t distant_attack_point(Actor* target){ //00554E80, original radius and two separate rejection sites.
        if(!target)return 0xffff;
        const float radius=static_cast<float>(static_cast<long double>(random_word())*random_scale*10.f+15.f);
        const auto initial_target=frame_position(*target),initial_origin=frame_position(*this->human());
        const float range=static_cast<float>(human_ai_cover::distance_xyz(initial_origin,initial_target));
        auto* navigation=this->navigation();if(!navigation)throw std::logic_error("Guard distant route requires actual navigation");
        std::vector<std::uint16_t> points;(void)navigation->append_marked_reachable((this->point()?navigation->index_of(*this->point()):std::uint16_t{0xffff}),radius,points);
        const auto live_target=frame_position(*target),live_origin=frame_position(*this->human());
        (void)human_ai_cover::filter_points(*navigation,live_origin,live_target,points,0xc0004102u);
        for(auto n=points.size();n;){const auto i=--n;if(i>=points.size())throw std::logic_error("Guard distant route native repeated removal exhausted candidates");const auto* point=navigation->record(points[i]);if(!point)continue;
            const auto p=human_ai_cover::position(*point);const float self_distance=static_cast<float>(human_ai_cover::distance_xyz(frame_position(*this->human()),p));
            const float clearance=static_cast<float>(human_ai_cover::distance_xyz(frame_position(*target),p)-static_cast<long double>(point->prefix[0x13])*.0625f);
            const auto remove=[&]{if(points.empty()||i>=points.size())throw std::logic_error("Guard distant route repeated native pop is invalid");points[i]=points.back();points.pop_back();};
            if(range<clearance)remove();
            if(!(clearance>=6.f&&self_distance<=range))remove();
        }
        if(points.empty())return 0xffff;
        if(points.size()==1)return points.front();
        return points[random_word()%points.size()];
    }
    template<class World>std::uint16_t engagement_point(World& world){ //00551E10→00553530/00553550.
        const auto range=preferred_range_status(world);if(range<0)return retreat_point(world);if(range>0)return distant_attack_point(timer_target_);return approach_point(frame_position(*timer_target_));
    }
    template<class World>bool hold_current_cover(World& world){ //00553BE0.
        if(!timer_target_||!this->point())return false;
        if(state_.template get<std::int32_t>(0x1dc)<2000){
            auto* command=this->command();if(!command)throw std::logic_error("Guard cover hold requires actual command owner");
            //00553B90 visits B4+7C forward and tests self friendship, not the neighbour.
            for(auto id:command->lists[0])if(auto* actor=world.human_ai_resolve_actor(id))if(actor->base().type()==3&&world.human_sound_friendly(*this->human(),this->human()))return static_cast<float>(static_cast<long double>(random_word())*random_scale*2.f)!=0;
            return true;
        }
        const auto target=frame_position(*timer_target_);const auto* point=this->point();if(!point)throw std::logic_error("Guard cover hold lost A4 during model query");
        if(human_ai_cover::blocked_toward(*point,target,1)){if(state_.template get<std::int32_t>(0x1cc)<7001)return true;state_.set(0x1cc,0u);}return false;
    }
    template<class World>std::uint32_t weapon_kind(World& world){if(!this->item())throw std::logic_error("Guard weapon predicate requires actual selected item");return world.human_frame_output_item(this->item()).definition().kind();}
    template<class World>float minimum_weapon_range(World& world){ //00551FC0.
        if(!this->item())return 20.f;
        switch(weapon_kind(world)){case 6:return 4;case 7:return 3;case 9:return 12;case 12:return 5;default:return 0;}
    }
    template<class World>float maximum_weapon_range(World& world){ //00552040.
        if(!this->item())return 0;
        switch(weapon_kind(world)){case 0:return 2;case 2:return 30;case 3:return 70;case 4:return 250;case 5:case 12:return 300;case 6:return 50;case 7:return 25;case 9:return 45;default:return 0;}
    }
    template<class World>bool weapon_allows_crouch(World& world){switch(weapon_kind(world)){case 0:case 2:case 3:case 7:case 9:return true;default:return false;}} //00552140.
    template<class World>bool target_in_weapon_range(World& world){ //005520E0.
        if(!this->item()||!timer_target_)return false;
        const float distance=static_cast<float>(human_ai_actor_distance(*this->human(),*timer_target_,this->state_frames()));const float maximum=maximum_weapon_range(world);return minimum_weapon_range(world)<distance&&distance<maximum;
    }
    template<class Words,class Inline,class World>bool request_cover_route(std::uint16_t point,Words& action,Inline& control,World& world){ //00553D10.
        if(point==0xffff)return false;
        auto& resources=this->resources();auto& query=resources.query();
        if(human_ai_closest_enemy_distance(*this->human(),this->state_frames())<minimum_weapon_range(world)){
            request_posture(4,control,world.mode_flags());resources.flag().request(2);query.request_point(point);query.state.set(0x54,0x2008u);action.set(8,5u);return true;
        }
        if(human_ai_closest_enemy_distance(*this->human(),this->state_frames())>maximum_weapon_range(world)){
            clear_item_ready(control,world);request_posture(4,control,world.mode_flags());resources.flag().request(1);query.request_point(point);query.state.set(0x54,0x4008u);action.set(8,6u);return true;
        }
        if(target_in_weapon_range(world)&&weapon_allows_crouch(world)){
            request_posture(2,control,world.mode_flags());resources.flag().request(2);query.request_point(point);query.state.set(0x54,0x4008u);action.set(8,5u);return true;
        }
        if(!weapon_allows_crouch(world))clear_item_ready(control,world);
        request_posture(4,control,world.mode_flags());resources.flag().request(1);query.request_point(point);query.state.set(0x54,world.human_guard_root_sector(*this->human())?0x10008u:0x4008u);action.set(8,6u);return true;
    }
    template<class Inline,class World>void clear_item_ready(Inline& control,World& world){control.clear_frame_item_ready([&](std::uint32_t value){world.human_frame_first_person_state(*this->human(),value);});if((world.mode_flags()&7u)!=7u)throw std::runtime_error("Guard readiness requires actual non7 transport");}
    template<class Inline,class World>bool can_use_prone(Inline&,World& world){ //00552200.
        const auto from=frame_position(*this->human()),to=frame_position(*timer_target_);const auto delta=human_ai_cover::difference(to,from);const float square=static_cast<float>((static_cast<long double>(delta[1])*delta[1]+static_cast<long double>(delta[2])*delta[2])+static_cast<long double>(delta[0])*delta[0]);
        if(!(human_ai_cover::length(square)>=20.f)||!(std::abs(static_cast<long double>(delta[1])/human_ai_cover::length(square))<=.17364799976348877f)||!this->item())return false;
        switch(weapon_kind(world)){case 0:case 2:case 6:case 7:case 9:return false;default:break;}
        if(!world.human_guard_clear_neighbours(*this->human())||!world.human_guard_root_sector(*this->human()))return false;
        auto planar=delta;planar[1]=0;normalize_human_ai_shot(planar,static_cast<long double>(delta[2])*delta[2]+static_cast<long double>(delta[0])*delta[0]);
        auto probe=from;probe[0]=static_cast<float>(static_cast<long double>(planar[0])*.5f+from[0]);probe[1]=static_cast<float>(static_cast<long double>(from[1])+.15000000596046448f+static_cast<long double>(planar[1])*.5f);probe[2]=static_cast<float>(static_cast<long double>(planar[2])*.5f+from[2]);
        return world.human_guard_prone_ray(*timer_target_,probe);
    }
    template<class Inline,class World>void request_cover_posture(Inline& control,World& world){ //00552970.
        if(!this->point()||!timer_target_)return;
        const auto blocked=[&](std::uint8_t posture){const auto target=frame_position(*timer_target_);const auto* point=this->point();if(!point)throw std::logic_error("Guard cover posture lost actual A4 during model update");return human_ai_cover::blocked_toward(*point,target,posture);};
        if(!blocked(1)&&can_use_prone(control,world)){request_posture(1,control,world.mode_flags());return;}
        if(!blocked(2)){request_posture(2,control,world.mode_flags());return;}
        if(!blocked(4))request_posture(4,control,world.mode_flags());
    }
    template<class World>bool reject_emplacement(void* actor,World& world){ //0054E390, AL-only result.
        const auto origin=frame_position(*this->human());const auto p=world.human_emplacement_position(actor);
        const auto x=static_cast<long double>(p[0])-origin[0],y=static_cast<long double>(p[1])-origin[1],z=static_cast<long double>(p[2])-origin[2];const float square=static_cast<float>((y*y+x*x)+z*z);
        const auto& hostiles=this->human()->perception.lists()[0];
        if(square>=36.f){
            if(!(square<100.f)||!(human_ai_closest_enemy_distance(*this->human(),this->state_frames())>=30.f))return true;
            for(auto i=hostiles.size();i;)if(world.human_emplacement_can_target(actor,0,world.human_emplacement_target(*hostiles[--i])))return false;
            return true;
        }
        if(!(human_ai_closest_enemy_distance(*this->human(),this->state_frames())>=12.f))return true;
        for(auto i=hostiles.size();i;){auto* target=hostiles[--i];if(human_ai_actor_distance(*this->human(),*target,this->state_frames())<30.f&&!world.human_emplacement_can_target(actor,0,world.human_emplacement_target(*target)))return true;}
        return false;
    }
    template<class Inline,class World>
    bool advance_frame_decision(std::uint32_t elapsed,Inline& control,World& world){ //00548610.
        auto& words=this->words();
        const auto flags=words.template get<std::uint32_t>(0x50);
        auto* human=this->human();
        state_.set(0x198,state_.template get<std::uint32_t>(0x198)+elapsed);
        if(!(flags&0x400u)&&(flags&0xf8u)&&control.frame_movement_mode()==3){ //005484B0.
            words.set(0x50,(flags&0xfffffcffu)|0x400u);
            reset_frame_action();
            radius_timers(true);
        }else if((flags&0x100u)&&guard_health_percent(*human)<50){
            words.set(0x50,(flags&~0x100u)|0x200u);
            reset_frame_action();
            radius_timers(false);
        }
        const bool hostile=!human->perception.lists()[0].empty(); //005582F0.
        const auto mounted=[&]{return world.human_guard_mounted(*human);}; //00548590, SAME Actor98 control presence.
        auto& resources=this->resources();
        auto kind=resources.current_action_kind();
        if(!kind){ //00548520.
            if(state_.template get<std::uint32_t>(0x144)&4u)return false;
            (void)world.prepare_guard_frame_weapon(*this);
            if(mounted()){state_.set(0x144,state_.template get<std::uint32_t>(0x144)|0x10u);select_mounted_action();return true;}
            if(hostile)select_hostile_action();else select_frame_task(world,control);
            return resources.current_action_kind()!=0;
        }
        drain_guard_frame_events(world,control); //005A8480, via current Guard F4.
        if(task_animation(control)!=0)return true; //00548610 requires exact idle0, unlike00553CC0 hostile interruption.
        kind=resources.current_action_kind();
        if(!kind)return false;
        const auto skill=signed_stat(*human,3);
        if(kind==5){
            if(skill>40&&equip_missing_frame_weapon(world)){
                control.action_command(0x4c);if((world.mode_flags()&7u)!=7u)throw std::runtime_error("Guard armed task requires actual005890D0 non7 transport");
                if(hostile)select_hostile_action();else select_frame_task(world,control);
            }
            return resources.current_action_kind()!=0;
        }
        if(kind==6){
            if(mounted())return true;
            state_.set(0x144,state_.template get<std::uint32_t>(0x144)&~0x20u);
            if(hostile)select_hostile_action();else select_frame_task(world,control);
            return resources.current_action_kind()!=0;
        }
        if(mounted()){select_mounted_action();return true;}
        if(skill>19||(state_.template get<std::uint32_t>(0x144)&1u)){
            if(this->item())return true;
            this->refresh_item(); //005593D0.
            if((this->item()||world.human_guard_shoulder_weapon(*human)||world.human_guard_holster_weapon(*human))&&
               human_frame_signed_word(state_.template get<std::uint32_t>(0x1d0))<1000){
                (void)equip_missing_frame_weapon(world);
                return true;
            }
        }
        reset_frame_action(); //0054B730.
        resources.current_action(actions_.kind5.acquire(0x1000001u));
        state_.set(0x198,0u);
        if(words.template get<std::uint32_t>(0x50)&1u)request_posture(4,control,world.mode_flags());
        return true;
    }
    template<class World,class Inline>void drain_guard_frame_events(World& world,Inline& control){
        auto* command=this->command();
        if(!command)throw std::logic_error("Guard event processing requires actual B4 command owner");
        auto& incoming=command->first.events;
        auto& retained=command->second.events;
        auto& resources=this->resources();
        auto& human=*this->human();
        const auto speech=[&](std::int32_t category,std::uint32_t delay){
            if(!suppress_hostile_selection())world.queue_guard_frame_speech(*this,category,delay);
        };
        while(incoming.size()){
            auto event=incoming.pop_front();
            const auto code=event->words[0];
            const auto kind=resources.current_action_kind();
            if(kind==2){ //005482A0→00549940, before the common retained/speech pass.
                advance_task_event(incoming,*event,world,control);
            }else{
                const bool damage=code==0x40000010u||code==0x40000011u||code==0x40000015u;
                const bool report=code==0x3000000bu||code==0x3000000cu;
                const auto& hostiles=human.perception.lists()[0];
                if(kind==6)advance_mounted_event(*event,world,control);
                if((kind==1&&(damage||report||code==0x30000001u||(code==0x20000013u&&hostiles.empty())))||
                   (kind==3&&(damage||report))||(kind==5&&(damage||(code>=0x3000000bu&&code<=0x3000000fu)))){
                    if(auto* actor=world.human_ai_resolve_actor(event->words[4])){
                        auto* source=dynamic_cast<Human*>(actor);
                        if(!source)throw std::logic_error("Guard event aim requires its actual Human");
                        const bool hostile=std::find(hostiles.rbegin(),hostiles.rend(),actor)!=hostiles.rend();
                        const auto look=[&]{resources.direction().target_frame(source->head_marker);};
                        if(kind==3){
                            if(!hostile)look(); //0054AB70.
                        }else if(kind==1){
                            auto& action=resources.template current_action_as<HumanAIAction1Pool>()->state;
                            const auto mode=action.template get<std::uint32_t>(8);const bool vehicle=vehicle_target(source,world);
                            if(damage){
                                if(!vehicle||mode==301){if(!hostile)look();if(vehicle_target(source,world)||mode==0||mode==302)action.set(8,1u);}
                            }else if(code==0x30000001u){if(vehicle||mode==0||mode==302)action.set(8,1u);}
                            else if(!hostile)look();
                        }else{
                            auto& action=resources.template current_action_as<HumanAIAction5Pool>()->state;
                            const auto mode=action.template get<std::uint32_t>(8);
                            if(!damage){
                                if(!hostile&&(!(mode&0x2000000u)||hostiles.empty()))look();
                            }else if(!(mode&0x4000000u)){
                                if(!hostile){if(hostiles.empty())look();}
                                else if(mode&0x2000000u){
                                    if(!this->item())throw std::runtime_error("Guard damage response requires the actual weapon lookup");
                                    action.set(8,0x8000001u);
                                }
                            }
                        }
                    }
                }
            }
            switch(code){ //005482F0 runs after the current-action event worker.
            case 0x10000002u:retained.merge(std::move(event),30000,false);break;
            case 0x10000003u:adjust_human_frame_stat3(human,10);speech(0x76,8000);break;
            case 0x20000004u:case 0x20000012u:adjust_human_frame_stat3(human,0xfffffffbu);break;
            case 0x30000001u:speech(0x79,10000);break;
            case 0x3000000bu:case 0x3000000cu:case 0x3000000du:case 0x3000000fu:
                retained.merge(std::move(event),20000,false);
                break;
            case 0x40000010u:case 0x40000011u:
                adjust_human_frame_stat3(human,0xfffffffeu);
                retained.merge(std::move(event),10000,false);
                speech(0x7a,5000);
                break;
            case 0x40000015u:
                this->extend_frame_reaction(static_cast<std::uint32_t>(static_cast<long double>(random_word())*random_scale*300.f+100.f));
                adjust_human_frame_stat3(human,0xfffffff1u);
                retained.merge(std::move(event),10000,false);
                break;
            default:break;
            }
        }
    }
    bool mounted_special()const{return (state_.template get<std::uint32_t>(0x144)&0x20u)!=0;}
    static long double mounted_dot(const HumanVector& a,const HumanVector& b){return (static_cast<long double>(a[0])*b[0]+static_cast<long double>(a[1])*b[1])+static_cast<long double>(a[2])*b[2];}
    template<class World>bool mounted_reject_target(Actor& target,World& world){ //0054DC50, second target is raised by.5.
        auto& human=*this->human();if(!human.field160)throw std::logic_error("Guard mounted capability requires actual attachment");auto point=world.human_emplacement_target(target);
        if(world.human_emplacement_can_target(human.field160,human.field164,point))return false;
        point[1]=static_cast<float>(static_cast<long double>(point[1])+.5f);return !world.human_emplacement_can_target(human.field160,human.field164,point);
    }
    template<class World>bool mounted_leave_threat(World& world){ //0054D5C0/0054D570;0054D560 is genuinely AL1.
        auto& human=*this->human();if(!human.field160||human.field160->base().type()!=0x15)return false;
        const auto& enemies=human.perception.lists()[0];for(auto n=enemies.size();n;){auto* target=enemies[--n];const auto distance=human_ai_actor_distance(human,*target,this->state_frames());if(distance<8.f||(distance<20.f&&mounted_reject_target(*target,world)))return true;}
        return false;
    }
    std::uint32_t mounted_wait_duration(){ //0054D620, native distance/2 FLOAT spill precedes the CRT draw.
        if(this->human()->perception.lists()[0].empty())return 500;
        const float half=static_cast<float>(human_ai_closest_enemy_distance(*this->human(),this->state_frames())*.5f);
        return static_cast<std::uint32_t>((static_cast<long double>(random_word())*random_scale*130.f+70.f)*half);
    }
    template<class World>HumanVector mounted_direction(bool spread,World& world){ //0054D800 returns its actual output vector pointer.
        auto& human=*this->human();auto* target=dynamic_cast<Human*>(timer_target_);if(!human.field160||!target||!target->loading_state)throw std::logic_error("Guard mounted direction requires attachment and actual target B0");
        const auto origin=world.human_emplacement_position(human.field160,human.field164);const auto flags=target->loading_state->words().template get<std::uint32_t>(0x50);HumanVector point;
        if(flags&6u){const auto m=this->state_frames().actor_world_matrix(target->head_marker);point={{m[12],static_cast<float>(static_cast<long double>(m[13])-.30000001192092896f),m[14],0}};}
        else if(static_cast<long double>(random_word())*random_scale==0)point=frame_position(*target);
        else{const auto m=this->state_frames().actor_world_matrix(target->head_marker);point={{m[12],m[13],m[14],m[15]}};}
        for(unsigned n=0;n<3;++n)point[n]=static_cast<float>(static_cast<long double>(point[n])+static_cast<long double>(target->activation.vector1b0[n])*.125f);
        if(spread){
            const auto x=static_cast<long double>(point[0])-origin[0],y=static_cast<long double>(point[1])-origin[1],z=static_cast<long double>(point[2])-origin[2];const float distance=static_cast<float>(human_ai_cover::length(static_cast<float>((x*x+y*y)+z*z)));
            const float skill=static_cast<float>(1.L-static_cast<long double>(signed_stat(human,4))*.009999999776482582f);
            const auto first=random_word();const float rz=static_cast<float>((static_cast<long double>(first)*random_scale+static_cast<long double>(first)*random_scale)-1.f);
            const auto second=random_word();const float ry=static_cast<float>(mounted_special()?static_cast<long double>(second)*random_scale-.30000001192092896f:static_cast<long double>(second)*random_scale*1.2000000476837158f-1.f);
            const auto last=random_word();const float amplitude=static_cast<float>((static_cast<long double>(skill)*.03999999910593033f+(mounted_special()?.03999999910593033f:.05999999865889549f))*distance);
            const float rx=static_cast<float>(amplitude*((static_cast<long double>(last)*random_scale+static_cast<long double>(last)*random_scale)-1.f));const float dy=ry*amplitude,dz=rz*amplitude;
            point[0]=static_cast<float>(static_cast<long double>(point[0])+rx);point[1]=static_cast<float>(static_cast<long double>(point[1])+dy);point[2]=static_cast<float>(static_cast<long double>(point[2])+dz);
        }
        return human_ai_cover::difference(point,origin);
    }
    template<class World>Actor* mounted_hostile(World& world){ //0054F040, reverse scored hostile head markers.
        auto& human=*this->human();if(!human.field160)return nullptr;const auto origin=frame_position(human);float best=-100000.f;Actor* selected=nullptr;
        const auto& enemies=human.perception.lists()[0];for(auto n=enemies.size();n;){auto* actor=enemies[--n];auto* target=dynamic_cast<Human*>(actor);if(!target)throw std::logic_error("Guard mounted selection requires actual hostile Human");
            if(human_frame_signed_word(target->properties.stats_ec[0])<1||world.human_unavailable(*target)||(target->properties.field19c==2&&target->loading_state&&target->loading_state->suppress_hostile_selection()))continue;
            const auto m=this->state_frames().actor_world_matrix(target->head_marker);const HumanVector point{{m[12],m[13],m[14],m[15]}};
            if(mounted_special()&&!world.human_guard_mounted_target_clear(human,point))continue;
            const auto x=static_cast<long double>(point[0])-origin[0],y=static_cast<long double>(point[1])-origin[1],z=static_cast<long double>(point[2])-origin[2];const auto distance=human_ai_cover::length(static_cast<float>((x*x+y*y)+z*z));const float capped=distance>100?100.f:static_cast<float>(distance);
            auto* state=target->loading_state;if(!state)throw std::logic_error("Guard mounted score requires actual target B0");float score=human_ai_aims_at(*target,*state,human,this->state_frames())?50.f:0.f;
            if(world.human_emplacement_can_target(human.field160,human.field164,point))score+=20.f;
            score=static_cast<float>((100.L-capped)*.5f+score);if(state->words().template get<std::int32_t>(0xb0)<400)score+=20.f;
            const auto posture=state->words().template get<std::uint32_t>(0x50)&3u;if(!posture)score+=10.f;else if(posture&2u)score+=5.f;
            if(score>best){best=score;selected=actor;}
        }
        return selected;
    }
    template<class World>Actor* mounted_select_target(World& world){ //0054D300, forward retained reports can replace the initial hostile on equality.
        auto& human=*this->human();if(!human.field160)return nullptr;auto* selected=mounted_hostile(world);if(mounted_special())return selected;
        const auto origin=frame_position(human);float best=1000000.f;if(selected){const auto point=frame_position(*selected);const auto x=static_cast<long double>(point[0])-origin[0],y=static_cast<long double>(point[1])-origin[1],z=static_cast<long double>(point[2])-origin[2];best=static_cast<float>((x*x+y*y)+z*z);}
        auto* command=this->command();if(!command)throw std::logic_error("Guard mounted report selection requires actual command queue");const auto& enemies=human.perception.lists()[0];
        for(std::size_t n=0;n<command->second.events.size();++n){const auto& event=*command->second.events.at(n);const auto age=human_frame_signed_word(event.words[2]),life=human_frame_signed_word(event.words[1]);const auto twice=human_frame_signed_word(event.words[2]*2u);if(twice>life)continue;
            auto* actor=world.human_ai_resolve_actor(event.words[4]);if(!actor)continue;if(auto* target=dynamic_cast<Human*>(actor);target&&human_frame_signed_word(target->properties.stats_ec[0])<1)continue;
            if(std::find(enemies.begin(),enemies.end(),actor)!=enemies.end())continue;
            const auto point=event_position(event);if(!world.human_emplacement_can_target(human.field160,human.field164,point))continue;
            if(!life)throw std::logic_error("Guard mounted report divides by actual zero lifetime");
            const auto quotient=age/life;const auto x=static_cast<long double>(point[0])-origin[0],y=static_cast<long double>(point[1])-origin[1],z=static_cast<long double>(point[2])-origin[2];const float weighted=static_cast<float>((static_cast<float>(quotient)+1.399999976158142f)*((x*x+y*y)+z*z));
            if(weighted<=best){selected=world.human_ai_resolve_actor(event.words[4]);best=weighted;}
        }
        return selected;
    }
    template<class World>void mounted_scan(World& world){ //0054E7D0, reverse enemies then forward retained reports.
        auto& human=*this->human();if(!human.field160)return;auto aim=world.human_pose_actor_direction(human.field160,human.field164);const auto origin=world.human_emplacement_position(human.field160,human.field164);normalize_human_ai_shot(aim,(static_cast<long double>(aim[0])*aim[0]+static_cast<long double>(aim[2])*aim[2])+static_cast<long double>(aim[1])*aim[1]);
        float best=-100000000.f;HumanVector selected{};const auto score=[&](const HumanVector& point){auto delta=human_ai_cover::difference(point,origin);const float square=static_cast<float>(human_ai_cover::square_xyz(delta));const auto distance=human_ai_cover::length(square);normalize_human_ai_shot(delta,square);const float value=static_cast<float>(mounted_dot(delta,aim)*5.f+(100.L-distance)*.5f);if(value>best){best=value;selected=point;}};
        const auto& enemies=human.perception.lists()[0];for(auto n=enemies.size();n;)score(frame_position(*enemies[--n]));
        auto* command=this->command();if(!command)throw std::logic_error("Guard mounted scan requires actual command owner");
        for(std::size_t n=0;n<command->second.events.size();++n){const auto& event=*command->second.events.at(n);if(human_frame_signed_word(event.words[2]*2u)>human_frame_signed_word(event.words[1]))continue;auto* actor=world.human_ai_resolve_actor(event.words[4]);auto* target=dynamic_cast<Human*>(actor);if(!target||human_frame_signed_word(target->properties.stats_ec[0])<1||std::find(enemies.begin(),enemies.end(),actor)!=enemies.end())continue;score(event_position(event));}
        if(best>-100000.f){auto delta=human_ai_cover::difference(selected,origin);normalize_human_ai_shot(delta,human_ai_cover::square_xyz(delta));this->set_target_direction(delta);}
    }
    template<class World>void mounted_track(World& world){ //0054DD20.
        if(!timer_target_)return;
        auto direction=mounted_direction(false,world);normalize_human_ai_shot(direction,(static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[1])*direction[1]);this->set_target_direction(direction);
    }
    template<class World>void mounted_attack(World& world){ //0054DED0/0054F280, input requests only; H6 executes firing/reload.
        if(!timer_target_)return;
        auto& human=*this->human();if(!human.field160)throw std::logic_error("Guard mounted attack requires actual attachment");
        auto direction=mounted_direction(false,world);auto aim=world.human_pose_actor_direction(human.field160,human.field164);
        normalize_human_ai_shot(aim,(static_cast<long double>(aim[0])*aim[0]+static_cast<long double>(aim[2])*aim[2])+static_cast<long double>(aim[1])*aim[1]);
        const auto x=static_cast<long double>(direction[0])*direction[0],y=static_cast<long double>(direction[1])*direction[1],z=static_cast<long double>(direction[2])*direction[2];normalize_human_ai_shot(direction,mounted_special()?(x+z)+y:human_ai_cover::square_xyz(direction)); //Special X/Z/Y order.
        this->set_target_direction(direction);auto* first=this->first_frame_inputs();auto* second=this->second_frame_inputs();if(!first||!second)throw std::logic_error("Guard mounted attack requires actual74/78 inputs");
        if(mounted_special()){if(!world.human_emplacement_ammunition(human.field160,human.field164)){first->set(0x34,1.f);return;}if(state_.template get<std::int32_t>(0x1a8)<=0)return;}
        else{if(state_.template get<std::int32_t>(0x1a8)<=0)return;if(!world.human_emplacement_ammunition(human.field160,human.field164)){first->set(0x34,1.f);return;}}
        bool aligned;
        if(mounted_special()){auto flat=direction,flat_aim=aim;flat[1]=flat_aim[1]=0;normalize_human_ai_shot(flat,static_cast<long double>(flat[0])*flat[0]+static_cast<long double>(flat[2])*flat[2]);normalize_human_ai_shot(flat_aim,static_cast<long double>(flat_aim[0])*flat_aim[0]+static_cast<long double>(flat_aim[2])*flat_aim[2]);aligned=mounted_dot(flat,flat_aim)>=.9998000264167786f;}
        else aligned=mounted_dot(direction,aim)>.9850000143051147f;
        if(!aligned)return;
        bool visible=false;const auto count=world.human_guard_sight_feedback(human,*timer_target_,visible);const auto flags=this->words().template get<std::uint32_t>(0x50);
        if(!visible&&((flags&0x400u)||((flags&0x200u)&&human_frame_signed_word(count)<=2)||((flags&0x100u)&&human_frame_signed_word(count)<=0)))return;
        auto shot=mounted_direction(true,world);normalize_human_ai_shot(shot,mounted_special()?(static_cast<long double>(shot[0])*shot[0]+static_cast<long double>(shot[2])*shot[2])+static_cast<long double>(shot[1])*shot[1]:human_ai_cover::square_xyz(shot));this->set_target_direction(shot);
        if(!mounted_special()&&state_.template get<std::uint8_t>(0x140)){state_.set(0x140,std::uint8_t{0});return;}
        state_.set(0x140,std::uint8_t{0});first->set(0x18,1.f);second->set(0x18,1.f);
    }
    bool mounted_static()const{auto* human=this->human();return human&&human->field160&&human->field160->base().type()==0x15;} //0054DCF0.
    bool mounted_target_member(Actor* actor)const{const auto& enemies=this->human()->perception.lists()[0];return actor&&std::find(enemies.begin(),enemies.end(),actor)!=enemies.end();} //005582A0 without an added health test.
    static bool mounted_actor_alive(Actor& actor){auto* target=dynamic_cast<Human*>(&actor);if(!target)throw std::logic_error("Guard mounted action health requires actual Human target");return human_frame_signed_word(target->properties.stats_ec[0])>0;}
    template<class World,class Inline>void advance_mounted_action(World& world,Inline& control){ //0054CF70.
        auto& resources=this->resources();auto& action=resources.template current_action_as<HumanAIAction6Pool>()->state;const auto mode=[&]{return action.template get<std::uint32_t>(8);};
        const auto hostile=[&]{return !this->human()->perception.lists()[0].empty();};const auto flag=[&](std::uint32_t value){resources.flag().request(value);resources.reconcile_direction_flags();};
        const auto wait=[&]{action.set(0x10,mounted_wait_duration());action.set(8,4u);};
        const auto reload=[&]{auto* input=this->first_frame_inputs();if(!input)throw std::logic_error("Guard mounted reload requires actual74 input");input->set(0x34,1.f);};
        auto& human=*this->human();
        if(mode()==0){flag(2);action.set(8,2u);if(human.field160)if(const auto kind=world.human_seat_weapon_kind(human.field160,human.field164);kind&&*kind==2)state_.set(0x144,state_.template get<std::uint32_t>(0x144)|0x20u);}
        if(mode()==1){
            if(!(this->words().template get<std::uint32_t>(0x50)&0x400000u))this->look_command(3,control);
            if(!hostile()){if(!action.template get<std::uint32_t>(0x10)){this->look_command(0,control);action.set(8,5u);}else if(mounted_special())mounted_scan(world);}
            else action.set(8,2u);
        }
        if(mode()==2){
            timer_target_=mounted_select_target(world);
            if(!timer_target_){
                if(!hostile()||!mounted_special()){resources.direction().look_interval(true,4000);action.set(0x10,static_cast<std::uint32_t>(static_cast<long double>(random_word())*random_scale*10000.f+15000.f));action.set(8,1u);}
                else wait();
            }else if(!mounted_reject_target(*timer_target_,world)){action.set(0x10,static_cast<std::uint32_t>(static_cast<long double>(random_word())*random_scale*3000.f+500.f));action.set(8,3u);state_.set(0x140,std::uint8_t{1});}
            else{timer_target_=nullptr;wait();}
        }
        if(mode()==3){
            if(mounted_leave_threat(world))action.set(8,7u);
            else if(!timer_target_||(mounted_actor_alive(*timer_target_)&&mounted_reject_target(*timer_target_,world)))action.set(8,2u);
            else if(human.field160&&!world.human_emplacement_ammunition(human.field160,human.field164)){reload();wait();}
            else if(action.template get<std::uint32_t>(0x10))mounted_attack(world);
            else wait();
        }
        if(mode()==4){
            if(mounted_leave_threat(world))action.set(8,7u);
            else if(!action.template get<std::uint32_t>(0x10))action.set(8,2u);
            else if(mounted_special())mounted_scan(world);
            else if(mounted_target_member(timer_target_))mounted_track(world);
        }
        if(mode()==5){this->look_command(0,control);if(state_.template get<std::uint32_t>(0x144)&0x10u){state_.set(0x144,state_.template get<std::uint32_t>(0x144)|4u);return;}world.human_guard_exit_emplacement(*this);return;}
        if(mode()==6){timer_target_=nullptr;this->look_command(0,control);flag(0);return;} //Native mode6 does NOT execute54DC10.
        if(mode()==7){timer_target_=nullptr;this->look_command(0,control);flag(0);world.human_guard_exit_emplacement(*this);}
    }
    template<class World,class Inline>void advance_mounted_event(const HumanAIEvent& event,World& world,Inline& control){ //0054CBB0, then the unchanged common event pass.
        auto& action=this->resources().template current_action_as<HumanAIAction6Pool>()->state;const auto mode=[&]{return action.template get<std::uint32_t>(8);};const auto code=event.words[0];
        const auto resolve=[&](unsigned word){return world.human_ai_resolve_actor(event.words[word]);};
        const auto acquire=[&](Actor& actor,float scale,float base){timer_target_=&actor;action.set(0x10,static_cast<std::uint32_t>(static_cast<long double>(random_word())*random_scale*scale+base));action.set(8,3u);state_.set(0x140,std::uint8_t{1});};
        if(code==0x20000004u||code==0x20000012u){
            if(mounted_special()||mode()!=1)return;
            auto* source=dynamic_cast<Human*>(resolve(3));if(!source||!world.human_sound_friendly(*this->human(),source))return;
            source=dynamic_cast<Human*>(resolve(3));if(!source||!source->command)throw std::logic_error("Guard mounted follow event requires actual source command owner");const auto* link=source->command->second.events.last_of_kind(0x10000016u);auto* target=link?world.human_ai_resolve_actor(link->words[4]):nullptr;
            if(!target||!mounted_actor_alive(*target)||mounted_reject_target(*target,world))return;
            if(target->base().type()==3)acquire(*target,2000,1500);
            return;
        }
        if(code==0x20000013u||(code>=0x3000000bu&&code<=0x3000000fu)){
            if(mounted_special()||mode()!=1||!resolve(4))return;
            auto* target=resolve(4);if(world.human_sound_friendly(*this->human(),target))return;target=resolve(4);if(!target||!mounted_actor_alive(*target))return;
            auto point=event_position(event);point[1]=static_cast<float>(static_cast<long double>(point[1])+1.f);this->look_command(7,control,&point);target=resolve(4);if(!target||target->base().type()!=3)return;
            if(!mounted_reject_target(*resolve(4),world))if(auto* live=resolve(4))acquire(*live,2500,1000);
            return;
        }
        if(code!=0x40000010u&&code!=0x40000011u&&code!=0x40000015u)return;
        if(!this->human()||!this->human()->field160||!resolve(4))return;
        auto* target=resolve(4);if(world.human_sound_friendly(*this->human(),target))return;target=resolve(4);if(!target||target->base().type()!=3)return;target=resolve(4);if(!target||!mounted_actor_alive(*target)||mounted_target_member(resolve(4)))return;
        if(!mounted_static()){if(mode()==1)action.set(8,2u);return;} //0054D560 is AL1.
        target=resolve(4);if(human_ai_actor_distance(*this->human(),*target,this->state_frames())>=8.f&&!mounted_reject_target(*resolve(4),world)){if(mode()==1)action.set(8,2u);return;}
        action.set(8,7u);
    }
    template<class Words,class World,class Inline>bool advance_vehicle_action(Words& action,World& world,Inline& control,std::uint32_t idle){ //00548B00/0054AC40 shared vehicle modes.
        const auto mode=[&]{return action.template get<std::uint32_t>(8);};if(mode()<100)return false;
        auto& resources=this->resources();auto& query=resources.query();
        const auto alive=[&]{if(!timer_target_)return false;auto* target=dynamic_cast<Human*>(timer_target_);if(!target)throw std::logic_error("Guard vehicle action requires actual Human target");return human_frame_signed_word(target->properties.stats_ec[0])>0;};
        const auto finish=[&]{resources.target().bind_actor(nullptr);action.set(8,idle);resources.flag().request(0);};
        const auto query_mask=[&]{return world.human_guard_root_sector(*this->human())?0x10008u:0x4008u;};
        if(mode()==100){
            if(world.human_guard_equip_rocket(*this->human())){
                if(!preferred_range_status(world)){resources.flag().request(2);action.set(8,102u);action.set(0x10,4000u);return true;}
                const auto point=vehicle_route_point(timer_target_,false);
                if(point==0xffff){resources.flag().request(2);action.set(8,102u);action.set(0x10,4000u);return true;}
                request_posture(4,control,world.mode_flags());resources.flag().request(1);query.request_point(point);query.state.set(0x54,0x4008u);action.set(8,101u);return true;
            }
            if(!alive()||!action.template get<std::uint32_t>(0x10)){action.set(8,idle);resources.flag().request(0);}
        }else if(mode()==101){
            if(!timer_target_){action.set(8,idle);return true;}
            if(query.state.template get<std::uint32_t>(0x50)&0x10000000u){
                if(action.template get<std::uint32_t>(0x10))return true;
                if(human_ai_actor_distance(*this->human(),*timer_target_,this->state_frames())<8.f)action.set(8,idle);
                action.set(0x10,1000u);return true;
            }
            if(!preferred_range_status(world)){resources.flag().request(2);action.set(8,102u);action.set(0x10,3000u);}else action.set(8,idle);
            return true;
        }else if(mode()==102){
            if(!target_still_hostile()||!this->item()||!action.template get<std::uint32_t>(0x10)){finish();}
            else if(weapon_kind(world)==6)advance_attack(world,control);else finish(); //00559570.
        }
        if(mode()==200)action.set(8,idle);
        if(mode()==300){
            if(!timer_target_||this->human()->perception.lists()[0].empty()){action.set(8,idle);return true;}
            const auto point=vehicle_route_point(timer_target_,true);
            if(point==0xffff){clear_item_ready(control,world);resources.flag().request(1);query.state.set(0x54,query_mask());query.request_mode(idle?3u:6u,idle?20u:40u);action.set(8,302u);action.set(0x10,1000u);return true;}
            task_route_stance(true,control,world);query.state.set(0x54,query_mask());query.request_point(point);action.set(8,301u);action.set(0x10,1500u);return true;
        }
        if(mode()==301){
            if(!timer_target_){action.set(8,idle);return true;}
            if(!(query.state.template get<std::uint32_t>(0x50)&0x10000000u)){
                if(cover_from_vehicle(timer_target_,control,world)){action.set(0x10,2000u);action.set(8,302u);}else action.set(8,idle);
                return true;
            }
            if(action.template get<std::uint32_t>(0x10))return true;
            if(human_ai_actor_distance(*this->human(),*timer_target_,this->state_frames())<10.f){action.set(8,idle);action.set(0x10,1000u);return true;}
            if(human_ai_closest_enemy_distance(*this->human(),this->state_frames())<20.f){if(world.human_guard_root_sector(*this->human())&&!(this->words().template get<std::uint32_t>(0x50)&0x10000u))query.state.set(0x54,0x10008u);}
            else if(this->words().template get<std::uint32_t>(0x50)&0x10000u)query.state.set(0x54,0x4008u);
            action.set(0x10,1000u);return true;
        }
        if(mode()==302&&!action.template get<std::uint32_t>(0x10))action.set(8,idle);
        return true;
    }
    template<class World>bool vehicle_target(Actor* actor,World& world){ //00559610, AL-only seat flag10.
        auto* target=dynamic_cast<Human*>(actor);if(!target||!target->field160)return false;const auto type=target->field160->base().type();return (type==4||type==0x15)&&(world.human_reaction_seat_flags(target->field160,target->field164)&0x10u);
    }
    template<class World>bool has_rocket(World& world){ //00559660.
        if(this->item()&&weapon_kind(world)==6)return true;
        if(world.human_guard_shoulder_weapon(*this->human()))return world.human_frame_output_item(world.human_guard_shoulder_weapon(*this->human())).definition().kind()==6;
        return false;
    }
    template<class World>Actor* other_hostile(World& world){ //005596D0; no added health/availability filter.
        Actor* selected=nullptr;float best=100000;const auto& enemies=this->human()->perception.lists()[0];
        for(auto n=enemies.size();n;){auto* actor=enemies[--n];if(actor==timer_target_||vehicle_target(actor,world))continue;const auto distance=human_ai_actor_distance(*this->human(),*actor,this->state_frames());if(distance<best){best=static_cast<float>(distance);selected=actor;}}
        return selected;
    }
    template<class Inline,class World>bool cover_from_vehicle(Actor* actor,Inline& control,World& world){ //005525F0 also retains its detached-target two live hostile passes.
        const auto* point=this->point();if(!point)return false;auto* target=dynamic_cast<Human*>(actor);
        if(target&&target->field160){
            if(human_ai_cover::blocked_toward(*point,frame_position(*target),4))return true;
            auto* attachment=target->field160;const auto own=frame_position(*this->human());auto origin=frame_position(*attachment);origin[1]=static_cast<float>(static_cast<long double>(origin[1])+1.f);const auto delta=human_ai_cover::difference(own,origin);
            if((static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[2])*delta[2])+static_cast<long double>(delta[1])*delta[1]>=100.f){
                if(human_ai_cover::blocked_toward(*point,frame_position(*target),2)){request_posture(2,control,world.mode_flags());return true;}
                if(human_ai_cover::blocked_toward(*point,frame_position(*target),1)){request_posture(1,control,world.mode_flags());return true;}
            }
            return false;
        }
        const auto all_blocked=[&]{const auto& enemies=this->human()->perception.lists()[0];for(auto n=enemies.size();n;)if(!human_ai_cover::blocked_toward(*point,frame_position(*enemies[--n]),4))return false;return true;};
        if(all_blocked())return true;
        if(all_blocked()){request_posture(2,control,world.mode_flags());return true;}
        return false;
    }
    std::uint16_t vehicle_route_point(Actor* actor,bool avoid){ //00555400 anti-tank /00555CA0 avoidance.
        auto* target=dynamic_cast<Human*>(actor);if(!target||!target->field160)return 0; //Native AX0, not the FFFF search failure.
        auto* attachment=target->field160;auto* navigation=this->navigation();if(!navigation)throw std::logic_error("Guard vehicle route requires actual navigation");
        auto points=secondary_;if(this->words().template get<std::uint32_t>(0x50)&0x100u){points.clear();(void)navigation->append_reachable(this->point()?navigation->index_of(*this->point()):0xffff,avoid?25.f:15.f,points);}
        if(avoid&&points.empty())return 0xffff;
        const auto own=frame_position(*this->human());auto origin=frame_position(*attachment);origin[1]=static_cast<float>(static_cast<long double>(origin[1])+1.f);
        const auto matrix=this->state_frames().actor_world_matrix(attachment->base().frame());HumanVector forward{{matrix[8],matrix[9],matrix[10],matrix[11]}};
        const auto x=static_cast<long double>(forward[0])*forward[0],y=static_cast<long double>(forward[1])*forward[1],z=static_cast<long double>(forward[2])*forward[2];normalize_human_ai_shot(forward,avoid?(z+x)+y:(z+y)+x);
        for(unsigned n=0;n<3;++n)forward[n]=static_cast<float>(static_cast<long double>(forward[n])*10.f);
        const auto prior=human_ai_cover::filter_points(*navigation,own,origin,points,avoid?0x40000104u:0x40004104u);if(points.empty())return prior;
        forward[1]=0;normalize_human_ai_shot(forward,static_cast<long double>(forward[0])*forward[0]+static_cast<long double>(forward[2])*forward[2]);
        for(auto n=points.size();n;){const auto i=--n;const auto* point=navigation->record(points[i]);if(!point)continue;const auto delta=human_ai_cover::difference(human_ai_cover::position(*point),origin);const float square=static_cast<float>(human_ai_cover::square(delta));auto radial=delta;radial[1]=0;normalize_human_ai_shot(radial,static_cast<long double>(radial[0])*radial[0]+static_cast<long double>(radial[2])*radial[2]);
            const auto dot=(static_cast<long double>(radial[1])*forward[1]+static_cast<long double>(radial[0])*forward[0])+static_cast<long double>(radial[2])*forward[2];bool remove=false;
            if(!avoid){
                if(square<225.f||square>1600.f)remove=true;
                else if(dot<=.7070000171661377f||square>=900.f){auto self=human_ai_cover::difference(own,origin);self[1]=0;normalize_human_ai_shot(self,static_cast<long double>(self[2])*self[2]+static_cast<long double>(self[0])*self[0]);remove=!((static_cast<long double>(self[2])*radial[2]+static_cast<long double>(self[1])*radial[1])+static_cast<long double>(self[0])*radial[0]>=0);}
                else remove=true;
            }else if(dot<=.7070000171661377f||square>=1000.f){
                if((dot>0&&square<dot*100.f+25.f)||square<25.f)remove=true;
                else{auto self=human_ai_cover::difference(own,origin);self[1]=0;normalize_human_ai_shot(self,static_cast<long double>(self[2])*self[2]+static_cast<long double>(self[0])*self[0]);const auto facing=(static_cast<long double>(self[2])*radial[2]+static_cast<long double>(self[1])*radial[1])+static_cast<long double>(self[0])*radial[0];
                    if(!(facing>=-.5f))remove=true;
                    else if(facing>0&&dot>0){const float side=static_cast<float>((static_cast<long double>(radial[1])*0+static_cast<long double>(forward[2])*radial[0])+static_cast<long double>(radial[2])*-forward[0]);const auto own_side=(static_cast<long double>(self[1])*0+static_cast<long double>(forward[2])*self[0])+static_cast<long double>(self[2])*-forward[0];remove=static_cast<long double>(side)*own_side>0;}
                }
            }else remove=true;
            if(remove){points[i]=points.back();points.pop_back();}
        }
        if(points.empty())return 0xffff;
        return points[random_word()%points.size()]; //Both native workers draw even for a single survivor.
    }
    template<class World,class Inline>void panic_face_nearest(World&,Inline& control){ //00558A60→00558980.
        Actor* selected=nullptr;float best=10000000.f;const auto& enemies=this->human()->perception.lists()[0];
        for(auto n=enemies.size();n;){auto* actor=enemies[--n];const auto origin=frame_position(*this->human());const auto other=frame_position(*actor);const auto x=static_cast<long double>(other[0])-origin[0],y=static_cast<long double>(other[1])-origin[1],z=static_cast<long double>(other[2])-origin[2];const float square=static_cast<float>((y*y+x*x)+z*z);if(square<best){best=square;selected=actor;}}
        if(!selected)return;
        auto* human=dynamic_cast<Human*>(selected);if(!human||!human->head_marker)throw std::logic_error("Guard panic facing requires actual hostile head");const auto head=this->state_frames().actor_world_matrix(human->head_marker);(void)face_task_position({{head[12],head[13],head[14],head[15]}},control);
    }
    template<class World>void panic_drop_hand(World& world){ //00558AA0.
        auto* storage=this->human()->base().storage();if(!storage||!storage->holder(3))throw std::logic_error("Guard panic drop requires actual hand3 holder");
        const auto entry=storage->holder(3)->entry();
        if(entry==this->item())this->clear_cached_item();
        if(entry)world.human_guard_drop_hand(*this->human(),entry,this->words().template get<HumanVector>(0x10));
    }
    std::uint32_t stopped_aim_wait(){ //00558110, native FLOAT distance and independent CRT thresholds.
        if(!timer_target_)return 0;
        const float distance=static_cast<float>(human_ai_actor_distance(*this->human(),*timer_target_,this->state_frames()));
        if(static_cast<long double>(random_word())*random_scale*4.f+6.f>distance)return 0;
        if(distance<static_cast<long double>(random_word())*random_scale*5.f+15.f)return static_cast<std::uint32_t>(static_cast<long double>(random_word())*random_scale*500.f+300.f);
        if(this->resources().query().state.template get<std::uint32_t>(0x50)&0x10000000u)return static_cast<std::uint32_t>(static_cast<long double>(random_word())*random_scale*1100.f+700.f);
        const float low=distance*20.f;return static_cast<std::uint32_t>(static_cast<long double>(random_word())*random_scale*(static_cast<long double>(distance)*60.f-low)+low);
    }
    template<class World>void stopped_aim_parameters(World& world){ //00558BD0, original0083C568..5E7 immutable tables.
        std::int32_t first=-1,second=-1,first_step=-1,second_step=-1;
        if(timer_target_&&this->item()){
            const auto kind=world.human_frame_output_item(this->item()).definition().kind();
            const std::array<std::int32_t,8>* row=nullptr;
            static constexpr std::array<std::int32_t,8> pistol{{50,110,25,625,480,1000,25,400}},rifle{{95,380,25,2500,580,1480,25,2500}},automatic{{93,280,25,2500,480,1780,100,10000}},heavy{{380,480,100,10000,1400,2520,100,10000}};
            if(kind==2)row=&pistol;else if(kind==3)row=&rifle;else if(kind==4)row=&automatic;else if(kind==5||kind==12)row=&heavy;
            if(row&&(*row)[0]!=-1){
                const auto target=frame_position(*timer_target_),self=frame_position(*this->human());const auto x=static_cast<long double>(self[0])-target[0],y=static_cast<long double>(self[1])-target[1],z=static_cast<long double>(self[2])-target[2];const float square=static_cast<float>((z*z+x*x)+y*y);
                const auto& r=*row;
                if(square<static_cast<float>(r[2]))first=r[1];
                else if(static_cast<float>(r[3])<square)first=r[0];
                else first=static_cast<std::int32_t>((1.L-(static_cast<long double>(square)-static_cast<float>(r[2]))/(r[3]-r[2]))*(r[1]-r[0])+r[0]);
                first_step=static_cast<std::int32_t>(static_cast<long double>(first)*.10000000149011612f);
                if(square<static_cast<float>(r[6]))second=r[4];
                else if(static_cast<float>(r[7])<square)second=r[5];
                else second=static_cast<std::int32_t>(((static_cast<long double>(square)-static_cast<float>(r[6]))/(r[7]-r[6]))*(r[5]-r[4])+r[4]);
                second_step=static_cast<std::int32_t>(static_cast<long double>(second)*.10000000149011612f);
            }
        }
        auto& words=this->resources().target().state;words.set(0x3c,first);words.set(0x40,first_step);words.set(0x44,second);words.set(0x48,second_step); //00560820/840.
    }
    template<class World>void finish_frame_aim_stop(World& world){ //00548080 AFTER00576350 cleared firing.
        if(this->item()){auto& item=world.human_frame_output_item(this->item());if(item.definition().identifier==0x26&&item.amount()==0){panic_drop_hand(world);return;}}
        state_.set(0x1b0,stopped_aim_wait());bool low=false;if(this->item())low=world.human_frame_output_item(this->item()).amount()<10.f;state_.set(0x1d4,static_cast<std::uint8_t>(low)); //00548010.
        stopped_aim_parameters(world);
        if(this->resources().current_action_kind()){
            //Original tests ACTION+8 (mode), not the action kind.
            const auto mode=this->resources().current_action_mode();
            if(mode&0x8000000u){auto& words=this->resources().target().state;words.set(0x44,200);words.set(0x48,100);}
        }
    }
    template<class World>bool panic_should_reload(World& world){ //00558230.
        const auto entry=this->item();if(!entry)return false;auto& item=world.human_frame_output_item(entry);
        if(item.field14>world.game_word28())return false;
        const float amount=item.amount();const auto capacity=item.capacity();return capacity!=amount||(item.field60&2u);
    }
    void request_panic_reload(){ //00572C00.
        auto* input=this->second_frame_inputs();if(!input)throw std::logic_error("Guard reload request requires actual78 input");input->set(0x34,1.f);auto& words=this->resources().state();words.set(0xd8,words.template get<std::uint32_t>(0xd8)&~1u);
    }
    template<class World,class Inline>void advance_panic_action(World& world,Inline& control){ //0054B8F0, elapsed only forwarded to attack's timer-independent output.
        auto& resources=this->resources();auto& action=resources.template current_action_as<HumanAIAction5Pool>()->state;auto& query=resources.query();
        const auto mode=[&]{return action.template get<std::uint32_t>(8);};const auto hostile=[&]{return !this->human()->perception.lists()[0].empty();};
        const auto distance=[&]{return human_ai_closest_enemy_distance(*this->human(),this->state_frames());};
        const auto clear=[&]{resources.flag().request(0);resources.target().bind_actor(nullptr);};
        const auto action_command=[&](std::int32_t id,bool surrendered){control.action_command(id);if((world.mode_flags()&7u)!=7u)throw std::runtime_error(surrendered?"Guard surrender requires actual005890D0 transport":"Guard armed recovery requires actual005890D0 transport");};
        if(mode()==0x1000000u&&hostile())action.set(8,0x1000001u);
        if(mode()==0x1000001u){
            if(!hostile()){resources.flag().request(0);action.set(8,0x1000000u);}
            else if(!this->item()||!(state_.template get<std::uint32_t>(0x144)&2u)){
                if((this->words().template get<std::uint32_t>(0x50)&0x400u)||distance()<15.f){clear();action.set(8,0x2000001u);}
                else{clear();action.set(8,0x4000002u);}
            }else action.set(8,0x8000001u);
        }
        if(mode()==0x2000001u){
            state_.set(0x1d8,0xffffffffu);if(!hostile()){action.set(8,0x1000001u);return;}
            clear();query.state.set(0x50,query.state.template get<std::uint32_t>(0x50)|0x2000u);request_posture(4,control,world.mode_flags());panic_drop_hand(world);panic_face_nearest(world,control);action.set(8,0x2000002u);action.set(0x10,300u);
        }else if(mode()==0x2000002u){
            if(!action.template get<std::uint32_t>(0x10)){
                if(this->words().template get<std::uint32_t>(0x50)&4u){action_command(0x34,true);action.set(8,0x2000003u);action.set(0x10,30000u);state_.set(0x1d8,world.game_word28());}
                else{request_posture(4,control,world.mode_flags());action.set(0x10,300u);}
            }
        }else if(mode()==0x2000003u){if(hostile()&&distance()>5.f)panic_face_nearest(world,control);return;}
        if(mode()==0x4000002u){
            state_.set(0x1d8,0xffffffffu);if(!hostile()){action.set(8,0x1000001u);return;}
            resources.flag().request(1);clear_item_ready(control,world);query.state.set(0x54,world.human_guard_root_sector(*this->human())?0x10008u:0x4008u);query.request_mode(2,40);action.set(8,0x4000001u);
        }else if(mode()==0x4000001u)action.set(8,query.state.template get<std::uint32_t>(0x50)&0x10000000u?0x4000003u:0x2000001u);
        else if(mode()==0x4000003u){
            if(query.state.template get<std::uint32_t>(0x50)&0x80000000u){resources.flag().request(0);action.set(8,0x4000004u);action.set(0x10,6000u);}
            else if(distance()<20.f){if(world.human_guard_root_sector(*this->human())&&!(this->words().template get<std::uint32_t>(0x50)&0x10000u))query.state.set(0x54,0x10008u);}
            else if(this->words().template get<std::uint32_t>(0x50)&0x10000u)query.state.set(0x54,0x4008u);
        }else if(mode()==0x4000004u){
            if(distance()>=20.f){if(!action.template get<std::uint32_t>(0x10)){resources.flag().request(0);action.set(8,0x1000000u);}}
            else action.set(8,0x4000002u);
        }
        if(mode()==0x8000001u){
            state_.set(0x1d8,0xffffffffu);if(!this->item())(void)equip_missing_frame_weapon(world);
            request_posture(4,control,world.mode_flags());resources.flag().request(2);action_command(0x4c,false);action.set(8,0x8000002u);return;
        }
        if(mode()==0x8000002u){
            if(!this->item()){action.set(8,0x1000001u);return;}
            timer_target_=select_human_hostile<Human>(*this,this->state_frames(),world);
            if(!timer_target_){resources.target().bind_actor(nullptr);action.set(8,0x8000004u);action.set(0x10,6000u);return;}
            if(human_ai_actor_distance(*this->human(),*timer_target_,this->state_frames())>10.f){request_posture(4,control,world.mode_flags());query.request_position(frame_position(*timer_target_));query.state.set(0x54,0x4008u);}
            action.set(0x10,6000u);action.set(8,0x8000003u);return;
        }
        if(mode()==0x8000004u){
            if(!hostile()){if(!action.template get<std::uint32_t>(0x10)){resources.flag().request(0);action.set(8,0x1000000u);}}
            else action.set(8,0x8000002u);
            if(panic_should_reload(world))request_panic_reload();
            return;
        }
        if(mode()!=0x8000003u)return;
        if(!this->item()){action.set(8,0x1000001u);return;}
        if((query.state.template get<std::uint32_t>(0x50)&0x10000000u)&&distance()<10.f)query.state.set(0x50,query.state.template get<std::uint32_t>(0x50)|0x2000u);
        if(!target_still_hostile()||!action.template get<std::uint32_t>(0x10)){action.set(8,0x8000002u);return;}
        advance_attack(world,control);
    }
    template<class World,class Inline>bool advance_positive_task_action(World& world,Inline& control){ //00549AE0 positive modes; elapsed is genuinely unused.
        auto& resources=this->resources();
        if(!this->human()->perception.lists()[0].empty()&&can_interrupt_task(control)){select_hostile_action();return true;}
        auto& action=resources.template current_action_as<HumanAIAction2Pool>()->state;auto& query=resources.query();
        const auto mode=[&]{return action.template get<std::uint32_t>(8);};
        const auto flag=[&](std::uint32_t value){resources.flag().request(value);resources.reconcile_direction_flags();}; //0056F1E0.
        const auto wait=[&]{action.set(8,2u);action.set(0x10,static_cast<std::uint32_t>(static_cast<long double>(random_word())*random_scale*10000.f+15000.f));};
        const auto actor=[&]()->Actor*{return selected_event_?world.human_ai_resolve_actor(selected_event_->words[4]):nullptr;};
        const auto event=[&](){if(!selected_event_)throw std::logic_error("Guard task lost actual selected event");return event_position(*selected_event_);};
        const auto facing=[&](const HumanVector& target,bool head){const auto origin=head?this->state_frames().actor_world_matrix(this->human()->head_marker):this->state_frames().actor_world_matrix(this->human()->base().frame());HumanVector delta=target;for(unsigned n=0;n<3;++n)delta[n]=static_cast<float>(static_cast<long double>(target[n])-origin[12+n]);resources.direction().set_target(delta);resources.direction().set_current(delta);};
        if(mode()==0xf){
            auto* navigation=this->navigation();if(!navigation)throw std::logic_error("Guard Ladder exit requires actual navigation");
            auto first=query.state.template get<std::uint16_t>(0x48);const auto second=query.state.template get<std::uint16_t>(0x4a);const auto* a=navigation->record(first);const auto* b=navigation->record(second);
            if(!a||!b)action.set(8,2u);
            else{if(human_ai_cover::position(*b)[1]<=human_ai_cover::position(*a)[1])first=second;query.request_point(first);query.state.set(0x54,0x4010u);action.set(8,0xeu);}
        }else if(mode()==0xe){
            if(this->words().template get<std::uint32_t>(0x50)&0x100000u)return true;
            if(!actor())wait();else action.set(8,4u);
        }
        if(!actor()){if(!task_animation(control))wait();else action.set(8,0xfu);return true;}
        if(mode()==4){
            flag(0);if(task_animation(control)){action.set(8,0xfu);return true;}
            if(query.state.template get<std::uint32_t>(0x50)&0x10000000u)query.state.set(0x50,query.state.template get<std::uint32_t>(0x50)|0x2000u);
            if(selected_event_->words[0]!=0x3000000eu||!face_task_position(event(),control)){
                auto* target=actor();if(!target)throw std::logic_error("Guard task target disappeared during facing");if(!face_task_position(frame_position(*target),control))return true;
                const auto own=frame_position(*this->human());target=actor();if(!target)throw std::logic_error("Guard task target disappeared before Sight");const auto to=frame_position(*target);
                const auto delta=human_ai_cover::difference(to,own);if((static_cast<long double>(delta[1])*delta[1]+static_cast<long double>(delta[2])*delta[2])+static_cast<long double>(delta[0])*delta[0]<900.f){if(auto* live=actor())world.human_area_force_sight(*this->human(),*live);}
            }
            action.set(8,5u);action.set(0xc,0u);this->look_command(0,control);if(!this->words().template get<std::uint8_t>(0xad))set_task_ready(control,world);
        }
        if(mode()==0){
            const auto target=event();const auto delta=human_ai_cover::difference(target,frame_position(*this->human()));
            if(world.human_guard_root_sector(*this->human())&&(static_cast<long double>(delta[1])*delta[1]+static_cast<long double>(delta[2])*delta[2])+static_cast<long double>(delta[0])*delta[0]>=400.f){const auto point=task_cover_point(target);if(point!=0xffff){task_route_stance(true,control,world);query.request_point(point);action.set(8,3u);return true;}}
            request_posture(2,control,world.mode_flags());action.set(8,4u);return true;
        }
        if(mode()==3&&(query.state.template get<std::uint32_t>(0x50)&0x80000000u)){
            set_task_ready(control,world);flag(2);const auto own=frame_position(*this->human());auto* target=actor();if(!target)throw std::logic_error("Guard task route completion lost target");const auto delta=human_ai_cover::difference(frame_position(*target),own);resources.direction().set_target(delta);resources.direction().set_current(delta);action.set(0xc,0u);action.set(8,5u);this->look_command(0,control);
        }
        if(mode()==5){
            if(!this->words().template get<std::uint8_t>(0xad))set_task_ready(control,world);
            const auto age=static_cast<float>(action.template get<std::int32_t>(0xc));bool search=false;
            if(this->words().template get<std::uint32_t>(0x50)&0x100u)search=static_cast<long double>(random_word())*random_scale*600.f+200.f<age;
            if(search||static_cast<long double>(random_word())*random_scale*2000.f+500.f<age)action.set(8,6u);
        }
        if(mode()==6){
            const auto point=task_search_point();
            if(point==0xffff){request_posture(2,control,world.mode_flags());wait();return true;}
            const auto delta=human_ai_cover::difference(event(),frame_position(*this->human()));const float distance=static_cast<float>(human_ai_cover::length(static_cast<float>((static_cast<long double>(delta[1])*delta[1]+static_cast<long double>(delta[2])*delta[2])+static_cast<long double>(delta[0])*delta[0])));
            if(!(this->words().template get<std::uint32_t>(0x50)&0x100u)){task_route_stance(false,control,world);flag(1);request_posture(2,control,world.mode_flags());}
            else if(world.human_guard_root_sector(*this->human())&&static_cast<long double>(random_word())*random_scale*20.f+60.f<distance)task_route_stance(true,control,world);
            else if(distance<=(static_cast<long double>(random_word())*random_scale+1)*20.f){resources.flag().request(1);query.state.set(0x54,0x2008u);this->look_command(4,control);}
            else{request_posture(4,control,world.mode_flags());task_route_stance(false,control,world);}
            query.request_point(point);action.set(8,7u);return true;
        }
        if(mode()==7){
            if(query.state.template get<std::uint32_t>(0x50)&0x10000000u){
                const auto delta=human_ai_cover::difference(event(),frame_position(*this->human()));const float square=static_cast<float>((static_cast<long double>(delta[1])*delta[1]+static_cast<long double>(delta[2])*delta[2])+static_cast<long double>(delta[0])*delta[0]);
                const auto jitter=[&]{unsigned total=0;for(unsigned n=0;n<4;++n)total+=random_word()>>2;return static_cast<long double>(total)*random_scale-.5L;};
                bool slow=false;const auto mask=query.state.template get<std::uint32_t>(0x54);
                if(mask&0x10000u)slow=jitter()*100.L+150.L>square;
                if(!slow&&(mask&0x4000u))slow=(jitter()+1.L)*100.L>square;
                if(slow){this->look_command(4,control);query.state.set(0x54,0x2008u);if(!this->words().template get<std::uint8_t>(0xad))set_task_ready(control,world);}
                else if(!(mask&0x1000u)&&square<static_cast<long double>(random_word())*random_scale*24.f+25.f){request_posture(2,control,world.mode_flags());this->look_command(4,control);query.state.set(0x54,0x1008u);if(!this->words().template get<std::uint8_t>(0xad))set_task_ready(control,world);}
                if(square<36.f){
                    if(!resources.state().template get<std::uint32_t>(0x108)){const auto target=event();this->look_command(7,control,&target);resources.state().set(0x108,static_cast<std::uint32_t>((static_cast<long double>(random_word())*random_scale+1)*1500.f));}
                    else if(actor()){const auto& lost=this->human()->perception.lists()[2];for(auto n=lost.size();n;)if(actor()==lost[--n]){query.state.set(0x50,query.state.template get<std::uint32_t>(0x50)|0x2000u);break;}}
                }
            }
            const auto flags=query.state.template get<std::uint32_t>(0x50);
            if(!(flags&0x10000000u)||(flags&0x80000000u)){set_task_ready(control,world);flag(2);facing(event(),true);this->look_command(4,control);action.set(8,8u);action.set(0x10,static_cast<std::uint32_t>(static_cast<long double>(random_word())*random_scale*2000.f+1500.f));}
        }
        if(mode()==8){
            auto* target=dynamic_cast<Human*>(actor());if(!target)throw std::logic_error("Guard task observation requires actual Human target");
            if(human_frame_signed_word(target->properties.stats_ec[0])<1||!(this->words().template get<std::uint32_t>(0x50)&0x100u)){
                auto* command=this->command();if(!command)throw std::logic_error("Guard task observation requires actual command queue");const auto* next=command->second.events.strongest_at(frame_position(*this->human()));bool same=false;
                if(next&&selected_event_){same=next->words[3]==selected_event_->words[3]&&next->words[0]==selected_event_->words[0]&&next->words[4]==selected_event_->words[4]&&next->words[12]==selected_event_->words[12];const auto a=event_position(*next),b=event();same=same&&a[0]==b[0]&&a[1]==b[1]&&a[2]==b[2];}
                if(!next||same||!world.human_ai_resolve_actor(next->words[4])){request_posture(2,control,world.mode_flags());wait();}
                else{clone_frame_event(command->second.events,*next,next->words[4]);action.set(8,5u);action.set(0xc,0u);}
            }else if(!action.template get<std::uint32_t>(0x10))action.set(8,9u);
        }
        if(mode()==9){
            const auto own=frame_position(*this->human());auto* target=actor();if(!target)throw std::logic_error("Guard task reposition lost target");const auto delta=human_ai_cover::difference(frame_position(*target),own);
            if(human_ai_cover::square_xyz(delta)>=36.f){const auto point=task_reposition_point();if(point==0xffff){request_posture(2,control,world.mode_flags());wait();}
                else{request_posture(4,control,world.mode_flags());resources.flag().request(1);query.state.set(0x54,0x2008u);query.request_point(point);this->look_command(3,control);action.set(8,10u);}}
            else{request_posture(2,control,world.mode_flags());resources.flag().request(4);query.state.set(0x54,0x1008u);target=actor();if(!target)throw std::logic_error("Guard task close route lost target");query.request_position(frame_position(*target));this->look_command(0,control);action.set(8,0xbu);}
        }
        if(mode()==10&&(query.state.template get<std::uint32_t>(0x50)&0x80000000u)){if(static_cast<long double>(random_word())*random_scale*100.f>=60.f){request_posture(2,control,world.mode_flags());wait();}else action.set(8,9u);}
        if(mode()==0xb){
            if(query.state.template get<std::uint32_t>(0x50)&0x10000000u){const auto own=frame_position(*this->human());auto* target=actor();if(!target)throw std::logic_error("Guard task close tracking lost target");resources.direction().set_target(human_ai_cover::difference(frame_position(*target),own));}
            if(query.state.template get<std::uint32_t>(0x50)&0x80000000u){if(static_cast<long double>(random_word())*random_scale*100.f<80.f){action.set(8,9u);return true;}request_posture(2,control,world.mode_flags());wait();}
        }
        return true;
    }
    std::uint16_t task_search_point(){ //00553EB0→00557D10.
        if(!selected_event_)throw std::logic_error("Guard search requires actual selected event");
        auto* navigation=this->navigation();if(!navigation)throw std::logic_error("Guard search requires actual navigation");
        const auto target=event_position(*selected_event_);const auto flags=this->words().template get<std::uint32_t>(0x50);
        if(!(flags&0x100u)){
            const auto& points=flags&0x200u?primary_:secondary_;std::uint16_t selected=0xffff;float best=100000.f;
            for(auto n=points.size();n;){const auto i=--n;const auto* point=navigation->record(points[i]);if(!point)throw std::logic_error("Guard nearest search lost actual point");const float square=static_cast<float>(human_ai_cover::square_xyz(human_ai_cover::difference(target,human_ai_cover::position(*point))));if(square<best){best=square;selected=points[i];}}
            return selected; //00557D10's last-square output is discarded by this caller.
        }
        const auto nearest=navigation->nearest_volume({{target[0],target[1],target[2]}},5.f,-1);if(nearest==0xffff)return nearest;
        std::vector<std::uint16_t> points;(void)navigation->append_reachable(nearest,5.f,points);
        if(points.empty())return nearest;
        const auto from=human_ai_cover::difference(frame_position(*this->human()),target);
        for(auto n=points.size();n;){const auto i=--n;const auto* point=navigation->record(points[i]);if(!point)continue;const auto delta=human_ai_cover::difference(human_ai_cover::position(*point),target);
            if((static_cast<long double>(from[0])*delta[0]+static_cast<long double>(delta[1])*from[1])+static_cast<long double>(delta[2])*from[2]<0){points[i]=points.back();points.pop_back();}
        }
        if(points.empty())return nearest;
        if(points.size()==1)return points.front();
        return points[random_word()%points.size()];
    }
    std::uint16_t task_reposition_point(){ //005541F0.
        if(!selected_event_)throw std::logic_error("Guard reposition requires actual selected event");
        auto* navigation=this->navigation();if(!navigation)throw std::logic_error("Guard reposition requires actual navigation");
        std::vector<std::uint16_t> points;(void)navigation->append_reachable(this->point()?navigation->index_of(*this->point()):0xffff,20.f,points);
        if(points.empty())return 0xffff;
        const auto target=event_position(*selected_event_);auto from=human_ai_cover::difference(frame_position(*this->human()),target);normalize_human_ai_shot(from,human_ai_cover::square_xyz(from));
        for(auto n=points.size();n;){const auto i=--n;const auto* point=navigation->record(points[i]);if(!point)continue;
            const auto p=human_ai_cover::position(*point);const auto self=human_ai_cover::difference(p,frame_position(*this->human()));bool remove=true;
            if(human_ai_cover::square_xyz(self)>=100.f){auto delta=human_ai_cover::difference(p,target);normalize_human_ai_shot(delta,(static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[2])*delta[2])+static_cast<long double>(delta[1])*delta[1]);
                const auto dot=(static_cast<long double>(delta[0])*from[0]+static_cast<long double>(delta[1])*from[1])+static_cast<long double>(delta[2])*from[2];remove=!(dot<=.5f);}
            if(remove){points[i]=points.back();points.pop_back();}
        }
        if(points.empty())return 0xffff;
        if(points.size()==1)return points.front();
        return points[random_word()%points.size()];
    }
    std::uint16_t task_cover_point(const HumanVector& target){ //00557540.
        if(human_ai_cover::square(target)<1.e-16f)return 0xffff;
        auto* navigation=this->navigation();if(!navigation)throw std::logic_error("Guard task cover requires actual navigation");
        std::vector<std::uint16_t> points;(void)navigation->append_reachable(this->point()?navigation->index_of(*this->point()):0xffff,10.f,points);
        if(points.empty())return 0xffff;
        const auto selected=human_ai_cover::filter_points(*navigation,frame_position(*this->human()),target,points,0xc0004101u);
        //Native performs a second shifting angular filter, then discards that
        //vector and returns the previously selected ID. Keep its live pose reads.
        auto toward=human_ai_cover::difference(target,frame_position(*this->human()));toward[1]=0;normalize_human_ai_shot(toward,static_cast<long double>(toward[0])*toward[0]+static_cast<long double>(toward[2])*toward[2]);
        for(auto n=points.size();n;){const auto i=--n;const auto* point=navigation->record(points[i]);if(!point)continue;auto delta=human_ai_cover::difference(human_ai_cover::position(*point),frame_position(*this->human()));delta[1]=0;normalize_human_ai_shot(delta,static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[2])*delta[2]);
            const auto dot=(static_cast<long double>(delta[1])*toward[1]+static_cast<long double>(delta[0])*toward[0])+static_cast<long double>(toward[2])*delta[2];
            if(dot<-.7070000171661377f||dot>.7070000171661377f)points.erase(points.begin()+static_cast<std::ptrdiff_t>(i));
        }
        return selected;
    }
    template<class Inline>bool face_task_position(const HumanVector& target,Inline& control){ //00554840.
        const auto head=this->state_frames().actor_world_matrix(this->human()->head_marker);HumanVector forward{{head[8],head[9],head[10],head[11]}};
        normalize_human_ai_shot(forward,(static_cast<long double>(forward[0])*forward[0]+static_cast<long double>(forward[2])*forward[2])+static_cast<long double>(forward[1])*forward[1]);forward[1]=0;
        auto delta=human_ai_cover::difference(target,frame_position(*this->human()));delta[1]=0;
        normalize_human_ai_shot(forward,static_cast<long double>(forward[0])*forward[0]+static_cast<long double>(forward[2])*forward[2]);normalize_human_ai_shot(delta,static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[2])*delta[2]);
        const auto dot=(static_cast<long double>(delta[1])*forward[1]+static_cast<long double>(delta[0])*forward[0])+static_cast<long double>(delta[2])*forward[2];
        if(dot>=.7599999904632568f)return true;
        this->look_command(7,control,&target);auto& direction=this->resources().direction();direction.set_current(delta);direction.set_target(delta);return false;
    }
    template<class Inline,class World>void set_task_ready(Inline& control,World& world){control.set_frame_item_ready(true,[&](std::uint32_t value){world.human_frame_first_person_state(*this->human(),value);});if((world.mode_flags()&7u)!=7u)throw std::runtime_error("Guard task readiness requires actual non7 command01 transport");}
    template<class Inline,class World>void task_route_stance(bool run,Inline& control,World& world){ //00554800/005547B0.
        if(run){clear_item_ready(control,world);request_posture(4,control,world.mode_flags());}
        else if(this->words().template get<std::uint8_t>(0xad)&&this->item()){
            const auto kind=weapon_kind(world);if(kind!=0&&kind!=3&&kind!=7&&kind!=9)clear_item_ready(control,world); //00554760.
        }
        this->resources().flag().request(1);this->resources().query().state.set(0x54,run?0x10008u:0x4008u);this->look_command(0,control);
    }
    static HumanVector event_position(const HumanAIEvent& event){HumanVector result;std::memcpy(result.data(),event.words.data()+8,16);return result;}
    template<class Inline>static std::uint32_t task_animation(Inline& control){auto* animation=control.control();if(!animation)throw std::logic_error("Guard task requires actual DD0");return animation->animation_state_word(0x1d8);}
    template<class World,class Inline>void task_side_look(const HumanVector& position,World& world,Inline& control){ //00557F70.
        const auto head=this->state_frames().actor_world_matrix(this->human()->head_marker);HumanVector forward{{head[8],head[9],head[10],head[11]}};
        normalize_human_ai_shot(forward,(static_cast<long double>(forward[0])*forward[0]+static_cast<long double>(forward[2])*forward[2])+static_cast<long double>(forward[1])*forward[1]);
        const auto origin=frame_position(*this->human());
        const auto cross=static_cast<long double>(forward[2])*(static_cast<long double>(position[0])-origin[0])-(static_cast<long double>(position[2])-origin[2])*forward[0];
        this->look_command(cross>=0?5u:6u,control);
        this->resources().state().set(0x108,static_cast<std::uint32_t>(static_cast<long double>(world.human_sound_random())*6000.f+4000.f)); //Actual LS3DF RandFloat, not CRT rand.
    }
    template<class Inline>void replace_task_event(HumanAIEventQueue& queue,const HumanAIEvent& event,Inline& control){ //00554C30.
        clone_frame_event(queue,event,event.words[4]);auto& action=this->resources().template current_action_as<HumanAIAction2Pool>()->state;
        const auto mode=action.template get<std::uint32_t>(8);if(mode!=4&&mode!=0&&mode!=3&&!task_animation(control))action.set(8,4u);
    }
    template<class World,class Inline>void refresh_task_event(HumanAIEventQueue& queue,const HumanAIEvent& event,World&,Inline& control){ //00549760.
        auto& action=this->resources().template current_action_as<HumanAIAction2Pool>()->state;
        if(action.template get<std::uint32_t>(8)!=7){replace_task_event(queue,event,control);return;}
        if(!selected_event_)throw std::logic_error("Guard task refresh lost actual event");
        const auto previous=human_ai_cover::difference(event_position(*selected_event_),frame_position(*this->human()));
        const auto current=human_ai_cover::difference(event_position(event),frame_position(*this->human()));
        const auto delta=human_ai_cover::difference(previous,current);
        clone_frame_event(queue,event,event.words[4]);
        const auto dot=(static_cast<long double>(current[0])*previous[0]+static_cast<long double>(current[1])*previous[1])+static_cast<long double>(current[2])*previous[2];
        if(dot<0)action.set(8,4u);else if(human_ai_cover::square_xyz(delta)>100.f)action.set(8,6u);
    }
    template<class World,class Inline>void advance_task_event(HumanAIEventQueue& queue,const HumanAIEvent& event,World& world,Inline& control){ //00549940.
        if(!world.human_ai_resolve_actor(event.words[4]))return;
        auto& action=this->resources().template current_action_as<HumanAIAction2Pool>()->state;
        const auto priority=event.words[0]&0xf0000000u;
        if(!selected_event_&&priority){if(world.human_ai_resolve_actor(event.words[4])){clone_frame_event(queue,event,event.words[4]);action.set(8,4u);}return;}
        bool same=false;
        if(selected_event_&&world.human_ai_resolve_actor(selected_event_->words[4])){
            auto* target=world.human_ai_resolve_actor(event.words[4]);same=target==world.human_ai_resolve_actor(selected_event_->words[4]);
        }
        if(same&&priority<0x40000000u){refresh_task_event(queue,event,world,control);return;}
        const auto code=event.words[0];
        if(code==0x30000001u){if(can_interrupt_task(control))select_hostile_action();return;}
        if(code==0x20000013u){ //00554E20 does not replace its selected event.
            const auto mode=action.template get<std::uint32_t>(8);if(mode!=4&&mode!=0&&!task_animation(control)){if(mode==7)task_side_look(event_position(event),world,control);else action.set(8,4u);}return;
        }
        if((code>=0x3000000bu&&code<=0x3000000du)||code==0x3000000fu){ //00554D20.
            if(!selected_event_)throw std::logic_error("Guard report requires actual selected task event");
            if((selected_event_->words[0]&0xf0000000u)<priority||action.template get<std::uint32_t>(8)!=7){replace_task_event(queue,event,control);return;}
            const auto delta=human_ai_cover::difference(event_position(event),frame_position(*this->human()));const float square=static_cast<float>(human_ai_cover::square_xyz(delta));
            if(human_ai_cover::length(square)>=20.f){task_side_look(event_position(event),world,control);return;}
            replace_task_event(queue,event,control);return;
        }
        if(code==0x40000010u||code==0x40000011u||code==0x40000015u)replace_task_event(queue,event,control);
    }
    template<class World>bool equip_missing_frame_weapon(World& world){ //00559420.
        this->refresh_item();
        if(this->item())return true;
        auto* human=this->human();
        if(human->base().storage()->holder(3)->cooling_down(world.game_word28()))return false;
        auto* inventory=human->loading_inventory;
        if(!inventory)throw std::logic_error("Guard weapon selection requires the actual inventory owner");
        if(const auto shoulder=world.human_guard_shoulder_weapon(*human)){
            if(world.human_frame_output_item(shoulder).definition().kind()==6&&inventory->equip_holster(2))return true;
            if(inventory->equip_shoulder())return true;
        }
        return inventory->equip_holster(2);
    }
    template<class World,class Inline>
    void advance_frame_action(std::uint32_t elapsed,World& world,Inline& control){ //00548840.
        auto& resources=this->resources();
        const auto kind=resources.current_action_kind();
        if(kind==2&&advance_frame_wait_action(world,control))return;
        if(kind==6){advance_mounted_action(world,control);return;}
        const bool hostile=!this->human()->perception.lists()[0].empty();
        if(kind==3&&!(resources.query().state.template get<std::uint32_t>(0x50)&0x10000000u))
            state_.set(0x1cc,state_.template get<std::uint32_t>(0x1cc)+elapsed);
        if(kind==1||kind==3){
            const auto wait_or_retry=[&](auto& action,std::uint32_t wait,std::uint32_t select){
                const auto mode=action.template get<std::uint32_t>(8);
                if(mode==wait){
                    if(!hostile){
                        if(!action.template get<std::uint32_t>(0x10))select_frame_task(world,control);
                        return true;
                    }
                    action.set(8,select);
                }else if(mode!=select)return false;
                this->look_command(0,control);
                timer_target_=select_human_hostile<Human>(*this,this->state_frames(),world);
                if(!timer_target_){
                    action.set(0x10,200u);
                    action.set(8,wait);
                    return true;
                }
                if(kind==1){ //00548B00 invokes the Door search before classifying the target attachment.
                    if(auto* emplacement=select_emplacement(world)){
                        auto* navigation=this->navigation();if(!navigation)throw std::logic_error("Guard emplacement route requires actual navigation");
                        const auto references=navigation->actor_references(emplacement);const auto point=references.empty()?0xffffu:references.front();
                        if(point!=0xffff){selected_emplacement_=emplacement;action.set(8,2u);resources.query().state.set(0x54,0x4008u);resources.query().request_point(point);}
                        return true;
                    }
                }
                if(vehicle_target(timer_target_,world)){
                    if(has_rocket(world)){action.set(0x10,1000u);action.set(8,100u);return false;}
                    //00559650 is genuinely AL0 and has no state effects.
                    if(auto* other=other_hostile(world))timer_target_=other;
                    else{if(cover_from_vehicle(timer_target_,control,world)){action.set(0x10,1000u);action.set(8,302u);}else action.set(8,300u);return false;}
                }
                action.set(8,kind==1?3u:1u);
                return false;
            };
            if(kind==1){ //00548B00.
                auto& action=resources.template current_action_as<HumanAIAction1Pool>()->state;
                if(wait_or_retry(action,0,1))return;
                if(action.template get<std::uint32_t>(8)==2){
                    if(resources.query().state.template get<std::uint32_t>(0x50)&0x80000000u){
                        if(!selected_emplacement_)throw std::logic_error("Guard emplacement arrival lost actual native148 actor");
                        if(!world.human_emplacement_claimed(selected_emplacement_,0)||
                           (world.human_seat_occupant(selected_emplacement_,0)&&human_frame_signed_word(world.human_seat_occupant(selected_emplacement_,0)->properties.stats_ec[0])<1)){
                            world.human_guard_enter_emplacement(*this,selected_emplacement_);return;
                        }
                        action.set(8,7u);
                    }else return;
                }
                if(action.template get<std::uint32_t>(8)==3){
                    const auto point=select_cover_point(timer_target_);
                    if(!request_cover_route(point,action,control,world))action.set(8,7u);
                    return;
                }
                if(action.template get<std::uint32_t>(8)==5){
                    if(resources.query().state.template get<std::uint32_t>(0x50)&0x80000000u)action.set(8,7u);
                    else{if(target_still_hostile())advance_attack(world,control);return;}
                }
                if(action.template get<std::uint32_t>(8)==6){
                    if(resources.query().state.template get<std::uint32_t>(0x50)&0x80000000u)action.set(8,7u);else return;
                }
                if(action.template get<std::uint32_t>(8)==7){
                    const auto distance=human_ai_closest_enemy_distance(*this->human(),this->state_frames());
                    action.set(0x10,distance>=10.f?6000u:3000u);
                    request_cover_posture(control,world);
                    resources.flag().request(2);
                    action.set(8,8u);
                    this->look_command(0,control);
                }
                if(action.template get<std::uint32_t>(8)==8){
                    if(!target_still_hostile()||!action.template get<std::uint32_t>(0x10)){
                        resources.target().bind_actor(nullptr);
                        action.set(8,1u);
                        resources.flag().request(0);
                    }else advance_attack(world,control);
                    return;
                }
                (void)advance_vehicle_action(action,world,control,1);return;
            }else{ //0054AC40.
                auto& action=resources.template current_action_as<HumanAIAction3Pool>()->state;
                if(wait_or_retry(action,5,0))return;
                const auto mode=[&]{return action.template get<std::uint32_t>(8);};
                auto& query=resources.query();
                const auto query_mask=[&]{return world.human_guard_root_sector(*this->human())?0x10008u:0x4008u;};
                const auto attack_hold=[&](std::uint32_t duration){resources.flag().request(2);action.set(8,4u);action.set(0x10,duration);};
                const auto route=[&](std::uint16_t point){
                    if(target_in_weapon_range(world)&&weapon_allows_crouch(world)){
                        resources.flag().request(2);query.request_point(point);query.state.set(0x54,0x4008u);this->look_command(0,control);request_posture(2,control,world.mode_flags());action.set(8,2u);
                    }else{
                        clear_item_ready(control,world);resources.flag().request(1);query.request_point(point);query.state.set(0x54,query_mask());this->look_command(0,control);request_posture(4,control,world.mode_flags());action.set(8,3u);
                    }
                };
                if(mode()==1){
                    if(hold_current_cover(world)){request_cover_posture(control,world);action.set(8,4u);return;}
                    const auto point=engagement_point(world);
                    if(point==0xffff){
                        if(preferred_range_status(world)<1){request_cover_posture(control,world);attack_hold(3000);return;}
                        clear_item_ready(control,world);resources.flag().request(1);query.request_position(frame_position(*timer_target_));query.state.set(0x54,query_mask());this->look_command(0,control);request_posture(4,control,world.mode_flags());action.set(8,6u);action.set(0xc,0u);action.set(0x10,1000u);return;
                    }
                    route(point);return;
                }
                if(mode()==6){
                    if(!timer_target_){resources.target().bind_actor(nullptr);action.set(8,0u);resources.flag().request(0);}
                    if((query.state.template get<std::uint32_t>(0x50)&0x10000000u)&&!action.template get<std::uint32_t>(0x10)){
                        if(!timer_target_)throw std::logic_error("Guard chase lost actual target before distance query");
                        const float distance=static_cast<float>(human_ai_actor_distance(*this->human(),*timer_target_,this->state_frames()));
                        if(distance>=static_cast<long double>(minimum_weapon_range(world))+10.f){
                            if(!preferred_range_status(world)){
                                const auto point=nearby_attack_point(world);if(point!=0xffff){route(point);return;}
                                auto* old=timer_target_;if(old==select_human_hostile<Human>(*this,this->state_frames(),world)){query.request_position(frame_position(*old));query.state.set(0x54,query_mask());}
                                const float age=static_cast<float>(action.template get<std::int32_t>(0xc));
                                const float deadline=static_cast<float>(static_cast<long double>(random_word())*random_scale*3000.f+4000.f);
                                if(deadline<=age){query.state.set(0x50,query.state.template get<std::uint32_t>(0x50)|0x2000u);resources.flag().request(2);action.set(8,4u);action.set(0x10,static_cast<std::uint32_t>(static_cast<long double>(random_word())*random_scale*3000.f+1000.f));request_cover_posture(control,world);}
                                action.set(0x10,1000u); //Native overwrites even a newly drawn hold deadline.
                            }else action.set(0x10,2000u);
                        }else{query.state.set(0x50,query.state.template get<std::uint32_t>(0x50)|0x2000u);attack_hold(3000);}
                    }
                    if(query.state.template get<std::uint32_t>(0x50)&0x80000000u)attack_hold(3000);
                }
                if(mode()==2){
                    if(query.state.template get<std::uint32_t>(0x50)&0x80000000u)attack_hold(3000);
                    if(target_still_hostile())advance_attack(world,control);
                }
                if(mode()==3&&(query.state.template get<std::uint32_t>(0x50)&0x80000000u))attack_hold(3000);
                if(mode()==4){
                    if(target_still_hostile())advance_attack(world,control);
                    else{resources.target().bind_actor(nullptr);action.set(8,0u);resources.flag().request(0);}
                    if(!action.template get<std::uint32_t>(0x10)){action.set(8,0u);resources.flag().request(0);}
                }
                if(mode()<=6)return;
                (void)advance_vehicle_action(action,world,control,0);return;
            }
        }
        if(kind==5){advance_panic_action(world,control);return;}
        if(!kind)return;
        throw std::runtime_error("Guard frame requires its actual positive action worker");
    }
    bool target_still_hostile()const{ //005582A0 and its caller's signed health gate.
        if(!timer_target_)return false;
        auto* target=dynamic_cast<Human*>(timer_target_);
        if(!target)throw std::logic_error("Guard attack requires the actual selected Human");
        if(human_frame_signed_word(target->properties.stats_ec[0])<1)return false;
        const auto& enemies=this->human()->perception.lists()[0];
        return std::find(enemies.rbegin(),enemies.rend(),timer_target_)!=enemies.rend();
    }
    template<class World,class Inline>void advance_attack(World& world,Inline& control){ //00558AE0.
        auto& resources=this->resources();
        const auto* idle=resources.template current_action_as<HumanAIAction5Pool>();
        if(idle&&(idle->state.template get<std::uint32_t>(8)&0x8000000u)&&timer_target_){ //00558960.
            resources.target().bind_actor(timer_target_);
            return;
        }
        if(state_.template get<std::uint32_t>(0x1a8)&&!state_.template get<std::uint32_t>(0x1b0)){
            if(this->words().template get<std::uint8_t>(0xad))resources.target().bind_actor(timer_target_);
            else{
                control.set_frame_item_ready(true,[&](std::uint32_t value){world.human_frame_first_person_state(*this->human(),value);});
                if((world.mode_flags()&7u)!=7u)throw std::runtime_error("Guard attack readiness requires actual command01 transport");
            }
            return;
        }
        if(select_human_guard_grenade(*this,world)){aim_human_ai_grenade(*this,world);state_.set(0x1ac,0u);return;} //005584B0 and00572DE0.
        if(timer_target_&&resources.state().template get<std::int32_t>(0xcc)<1){
            auto* target=dynamic_cast<Human*>(timer_target_);
            if(!target||!target->head_marker||!this->human()->head_marker)
                throw std::logic_error("Guard attack requires both actual head markers");
            auto& frames=this->state_frames();
            const auto from=frames.actor_world_matrix(this->human()->head_marker);
            const auto to=frames.actor_world_matrix(target->head_marker);
            auto direction=this->target_direction();
            for(unsigned axis=0;axis!=3;++axis)direction[axis]=static_cast<float>(static_cast<long double>(to[12+axis])-from[12+axis]);
            //00575670 stores XYZ differences before its (Z²+Y²)+X² normalization.
            const auto x=static_cast<long double>(direction[0]),y=static_cast<long double>(direction[1]),z=static_cast<long double>(direction[2]);
            const auto exact_square=(z*z+y*y)+x*x;
            const float square=static_cast<float>(exact_square);
            constexpr float epsilon=9.99999993922529e-09f;
            if(std::abs(exact_square-1)>=epsilon){
                if(square>=epsilon){
                    const auto scale=1/std::sqrt(static_cast<long double>(square));
                    for(unsigned axis=0;axis!=3;++axis)direction[axis]=static_cast<float>(direction[axis]*scale);
                }else{
                    const auto axis=x!=0?0u:z!=0?2u:1u;
                    direction[axis]=direction[axis]<0?-1.f:1.f;
                }
            }
            resources.direction().set_target(direction);
        }
        resources.target().bind_actor(nullptr);
    }
    static std::int32_t guard_health_percent(const Human& human){ //005A5FF0.
        const auto health=human_frame_signed_word(human.properties.stats_ec[0]);
        if(!health)return 0;
        const auto base=human_frame_signed_word(human.properties.stats_11c[0]);
        const auto percent=static_cast<long double>(health)*100.f/base;
        if(percent>100)return 100;
        if(percent>0&&percent<1)return 1;
        if(!std::isfinite(percent))return 0; //The x87 indefinite integer has a zero low DWORD.
        return human_frame_signed_word(static_cast<std::uint32_t>(static_cast<std::int64_t>(percent)));
    }
    template<class Inline>static bool can_interrupt_task(Inline& control){ //00553CC0→005AC2B0.
        const auto* animation=control.control();
        if(!animation)throw std::logic_error("Guard worker requires the actual DD0 owner");
        const auto state=animation->animation_state_word(0x1d8);
        return state!=8&&state!=14&&(state<106||state>108);
    }
    void select_mounted_action(){ //0054CB70.
        reset_frame_action();auto& resources=this->resources();auto& query=resources.query().state;query.set(0x50,query.template get<std::uint32_t>(0x50)|0x2000u);
        resources.current_action(actions_.kind6.acquire());state_.set(0x198,0u);resources.template current_action_as<HumanAIAction6Pool>()->state.set(8,0u);
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
        clone_frame_event(queue,event,retarget?static_cast<std::uint32_t>(target.base().id10()):event.words[4]);
    }
    void clone_frame_event(HumanAIEventQueue& queue,const HumanAIEvent& event,std::uint32_t target){
        selected_event_.reset(); //Native returns the previous clone BEFORE acquiring its replacement.
        auto copy=queue.pool().acquire();
        copy->words[0]=event.words[0];
        copy->words[1]=10000;
        copy->words[2]=event.words[2];
        copy->words[3]=event.words[3];
        copy->words[4]=target;
        std::copy_n(event.words.begin()+8,5,copy->words.begin()+8);
        selected_event_=std::move(copy);
    }
    template<class Inline>void request_posture(std::uint32_t posture,Inline& control,std::uint32_t mode){ //00559360→005767F0.
        const auto flags=this->words().template get<std::uint32_t>(0x50);
        if(flags&posture)return;
        if(posture!=1&&posture!=2&&posture!=4)return;
        const auto speed=posture==1?0.f:posture==2?.59f:.45f;
        state_.set(0x1a0,speed);
        speed_=speed;
        this->resources().state().set(0x10c,0u);
        this->request_navigation_posture(posture,control,mode);
    }
    static constexpr float random_scale=0x1.0002p-15f; //0080FF64, actual CRT rand divisor.
    std::uint32_t random_word(){const auto value=random_();if(value>0x7fff)throw std::runtime_error("Guard AI requires shared CRT random");return value;}
    static std::int32_t signed_stat(const Human& human,unsigned index){const auto bits=human.properties.stats_ec.at(index);std::int32_t value;std::memcpy(&value,&bits,4);return value;}
    HumanAIActionPools& actions_;Random& random_;Contexts& contexts_;HumanJointWords<0x1e0> state_;
    std::vector<std::uint16_t> primary_,secondary_;std::vector<void*> emplacements_;
    std::optional<float> speed_;
    HumanAIEventPool::Lease selected_event_{nullptr,HumanAIEventPool::Return{}}; //Native148, a separate clone from the processed queue.
    void* selected_emplacement_=nullptr; //Native148 actor borrower, independent of kind2 event clone.
    Actor* timer_target_=nullptr; //Actual19C; scalar guard words never publish a host pointer.
};
} //namespace hd2::game
#endif
