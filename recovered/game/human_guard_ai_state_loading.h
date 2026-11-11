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
            if(attachment){
                const auto type=attachment->base().type();
                if(type==4||type==0x14||type==0x15||type==0x22)
                    throw std::runtime_error("Guard attachment frame requires its actual command receiver");
            }
            this->resources().reconcile_direction_flags();
            this->advance_frame_query(control,world,elapsed);
            const auto was_firing=this->words().template get<std::uint32_t>(0x50)&0x200000u;
            this->advance_frame_aim(world); //00548080 ->00576350.
            if(was_firing&&!(this->words().template get<std::uint32_t>(0x50)&0x200000u))
                throw std::runtime_error("Guard aim stop requires its actual 00548080 reaction and item receivers");
            this->advance_frame_direction(elapsed,control);
            this->apply_frame_movement(control);
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
        const auto require_attachment=[&]{
            if(const auto* attachment=human->field160){
                const auto type=attachment->base().type();
                if(type==4||type==0x14||type==0x15||type==0x22)
                    throw std::runtime_error("Guard task requires its actual 00548590 attachment query");
            }
        };
        auto& resources=this->resources();
        auto kind=resources.current_action_kind();
        if(!kind){ //00548520.
            if(state_.template get<std::uint32_t>(0x144)&4u)return false;
            (void)world.prepare_guard_frame_weapon(*this);
            require_attachment();
            if(hostile)select_hostile_action();else select_frame_task(world,control);
            return resources.current_action_kind()!=0;
        }
        drain_guard_frame_events(world,control); //005A8480, via current Guard F4.
        if(!can_interrupt_task(control))return true;
        kind=resources.current_action_kind();
        if(!kind)return false;
        const auto skill=signed_stat(*human,3);
        if(kind==5){
            if(skill>40&&equip_missing_frame_weapon(world))
                throw std::runtime_error("Guard armed task requires the actual inline weapon command");
            return true;
        }
        require_attachment();
        if(kind==6){
            state_.set(0x144,state_.template get<std::uint32_t>(0x144)&~0x20u);
            if(hostile)select_hostile_action();else select_frame_task(world,control);
            return resources.current_action_kind()!=0;
        }
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
            if(kind==2){ //005482A0 ->00549940.
                if(auto* target=world.human_ai_resolve_actor(event->words[4])){
                    auto* action=resources.template current_action_as<HumanAIAction2Pool>();
                    const auto priority=code&0xf0000000u;
                    if(!selected_event_&&priority){
                        clone_frame_event(incoming,*event,event->words[4]);
                        action->state.set(8,4u);
                    }else if(selected_event_&&priority<0x40000000u&&
                             world.human_ai_resolve_actor(selected_event_->words[4])==target){
                        if(action->state.template get<std::uint32_t>(8)==7)
                            throw std::runtime_error("Guard target refresh requires the actual 00549760 route receiver");
                    }else if(code==0x30000001u){
                        if(can_interrupt_task(control))select_hostile_action();
                    }else if(code==0x20000013u||(code>=0x3000000bu&&code<=0x3000000du)||
                             code==0x3000000fu||code==0x40000010u||code==0x40000011u||code==0x40000015u)
                        throw std::runtime_error("Guard wait event requires its actual positive route receiver");
                }
            }else{
                const bool damage=code==0x40000010u||code==0x40000011u||code==0x40000015u;
                const bool report=code==0x3000000bu||code==0x3000000cu;
                const auto& hostiles=human.perception.lists()[0];
                if(kind==6)throw std::runtime_error("Guard attachment event requires its actual current action receiver");
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
                            if(damage||code==0x30000001u){
                                if(const auto* vehicle=source->field160){
                                    const auto type=vehicle->base().type();
                                    if(type==4||type==0x15)throw std::runtime_error("Guard event requires the actual target vehicle query");
                                }
                                auto& action=resources.template current_action_as<HumanAIAction1Pool>()->state;
                                const auto mode=action.template get<std::uint32_t>(8);
                                if(damage&&!hostile)look();
                                if(mode==0||mode==0x12eu)action.set(8,1u);
                            }else if(!hostile)look();
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
                    state_.set(0x1c4,static_cast<std::uint32_t>((static_cast<long double>(random_word())*random_scale+1)*2000.f));
                    if(!doors_.empty())throw std::runtime_error("Guard target requires the actual populated 0054E600 Door search");
                }
                auto& target=static_cast<Human&>(*timer_target_);
                if(const auto* attachment=target.field160){
                    const auto type=attachment->base().type();
                    if(type==4||type==0x15)
                        throw std::runtime_error("Guard target requires the actual vehicle seat capability");
                }
                action.set(8,kind==1?3u:1u);
                return false;
            };
            if(kind==1){ //00548B00.
                auto& action=resources.template current_action_as<HumanAIAction1Pool>()->state;
                if(wait_or_retry(action,0,1))return;
                if(action.template get<std::uint32_t>(8)==3){
                    if(timer_target_&&!secondary_.empty())
                        throw std::runtime_error("Guard target requires actual 005579E0 cover selection");
                    action.set(8,7u); //00553D10 returns false for the missing cover point.
                    return;
                }
                if(action.template get<std::uint32_t>(8)==7){
                    const auto distance=human_ai_closest_enemy_distance(*this->human(),this->state_frames());
                    action.set(0x10,distance>=10.f?6000u:3000u);
                    if(this->point()&&timer_target_)
                        throw std::runtime_error("Guard target requires actual 00552970 cover posture");
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
            }else{ //0054AC40.
                auto& action=resources.template current_action_as<HumanAIAction3Pool>()->state;
                if(wait_or_retry(action,5,0))return;
                if(action.template get<std::uint32_t>(8)==4){
                    if(target_still_hostile())advance_attack(world,control);
                    else{
                        resources.target().bind_actor(nullptr);
                        action.set(8,0u);
                        resources.flag().request(0);
                    }
                    if(!action.template get<std::uint32_t>(0x10)){
                        action.set(8,0u);
                        resources.flag().request(0);
                    }
                    return;
                }
            }
        }
        if(kind==5&&!hostile){
            auto& action=resources.template current_action_as<HumanAIAction5Pool>()->state;
            const auto mode=action.template get<std::uint32_t>(8);
            if(mode==0x1000000u)return;
            if(mode==0x1000001u){
                resources.flag().request(0);
                action.set(8,0x1000000u);
                return;
            }
        }
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
        const auto grenade=world.human_feedback_kind9(*this->human()); //005584B0 first queries the real holder.
        const auto age=state_.template get<std::int32_t>(0x1ac);
        const auto flags=this->words().template get<std::uint32_t>(0x50);
        if(grenade&&age>=10000&&state_.template get<std::uint32_t>(0x1a8)&&!(flags&0x200001u)){
            const auto count=this->human()->perception.lists()[0].size();
            if(!count||static_cast<long double>(age)>=40.L/count*1000.f)
                throw std::runtime_error("Guard attack requires the actual grenade selection and trajectory receivers");
        }
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
