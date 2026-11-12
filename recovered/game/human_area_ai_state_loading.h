#ifndef HD2_GAME_HUMAN_AREA_AI_STATE_LOADING_H
#define HD2_GAME_HUMAN_AREA_AI_STATE_LOADING_H

#include "game/human_ai_state_loading.h"
#include "game/human_ai_frame_timers.h"
#include "game/human_ai_cover.h"

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
    template<class Queries,class Inline,class World,class Runtime>
    bool advance_authority_frame(std::uint32_t elapsed,Queries& queries,Inline& control,World& world,Runtime& runtime){ //00559DE0.
        this->publish_authority_input_gate(control);
        if(this->words().template get<std::uint32_t>(0x50)&0x4000800u){
            this->process_second_frame_commands();
            return false;
        }
        advance_frame_prefix(elapsed,queries);
        if(!advance_frame_decision(elapsed)){
            this->process_second_frame_commands();
            return true; //005AF680 runs after dispatch returns to the owning composition.
        }
        advance_frame_action(control,world);
        this->resources().reconcile_direction_flags();
        this->advance_frame_query(control,world,elapsed,runtime);
        this->advance_frame_direction(elapsed,control);
        const auto was_firing=this->words().template get<std::uint32_t>(0x50)&0x200000u;
        this->advance_frame_aim(world); //00559D40 ->00576350.
        if(was_firing&&!(this->words().template get<std::uint32_t>(0x50)&0x200000u)&&this->item()){
            const auto& item=world.human_frame_output_item(this->item());
            if(item.definition().identifier==0x26&&item.amount()==0)
                drop_empty_frame_hand(world);
        }
        this->apply_frame_movement(control);
        this->apply_frame_items(control,world,runtime,elapsed,0);
        this->process_second_frame_commands();
        return false;
    }
    template<class World>void drop_empty_frame_hand(World& world){ //00559CF0, SAME hand3/virtual70/005A2C50 protocol as00558AA0.
        auto* storage=this->human()->base().storage();if(!storage||!storage->holder(3))throw std::logic_error("Area empty weapon requires actual hand3");
        const auto entry=storage->holder(3)->entry();if(entry==this->item())this->clear_cached_item();
        if(entry)world.human_guard_drop_hand(*this->human(),entry,this->target_direction());
    }
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
    template<class Inline,class World>void advance_frame_action(Inline& control,World& world){ //0055A100/0055A1C0→0055AD50/0055A6E0.
        const auto kind=this->resources().current_action_kind();if(kind==0||kind==5)return;
        if(kind!=3)throw std::logic_error("Area worker requires actual kind3 action");
        auto* action=this->resources().template current_action_as<HumanAIAction3Pool>();
        if(!action||!this->human()||!this->command())throw std::logic_error("Area worker requires actual Human, command and action owners");
        auto& words=action->state;auto& resources=this->resources();auto& query=resources.query();
        const auto mode=[&]{return words.template get<std::uint32_t>(8);};
        const auto hostile=[&]{return !this->human()->perception.lists()[0].empty();};
        const auto active=[&]{return hostile()||this->command()->second.events.size()!=0;}; //0055B860.
        const bool short_range=(this->words().template get<std::uint32_t>(0x50)&0x400u)!=0;
        if(mode()==0){if(active())words.set(8,3u);else if(!words.template get<std::uint32_t>(0x10))state_.set(0x154,state_.template get<std::uint32_t>(0x154)|4u);}
        if(!short_range&&mode()==2&&!words.template get<std::uint32_t>(0x10))words.set(8,3u);
        if(!short_range&&mode()==1){
            const auto selected=human_ai_cover::defensive_point(*this,this->state_frames(),route_);
            if(selected==0xffff)words.set(8,3u);
            else{
                auto* navigation=this->navigation();const auto current=this->point()?navigation->index_of(*this->point()):0xffffu;
                if(current==selected){request_defensive_posture(this->point(),control,world);words.set(0x10,random_interval(8000.,9000.));}
                else{request_defensive_posture(navigation->record(selected),control,world);query.state.set(0x54,0x4100u);query.request_point(selected);(void)random_word();words.set(0x10,3000u);}
                words.set(8,2u);
            }
        }
        const auto attack_mode=[&]{
            if(target_is_hostile()){resources.flag().request(2);words.set(8,6u);words.set(0x10,random_interval(2000.,4000.));}
            else{resources.flag().request(0);words.set(8,4u);words.set(0x10,1000u);}
        };
        if(mode()==3){
            if(!active()){words.set(8,0u);words.set(0x10,random_interval(short_range?6000.:8000.,short_range?8000.:16000.));}
            else{
                const bool enemies=hostile();
                if(enemies){const auto& list=this->human()->perception.lists()[0];timer_target_=short_range&&list.size()==1?list.front():select_human_hostile<Human>(*this,this->state_frames(),world);}
                else{auto event=this->command()->second.events.pop_front();timer_target_=event?world.human_ai_resolve_actor(event->words[4]):nullptr;} //0055B920 returns the processed lease after resolution.
                if(short_range){
                    resources.flag().request(enemies?2u:0u);words.set(8,enemies?6u:4u);if(!enemies)words.set(0x10,1000u);
                }else{
                    const auto selected=human_ai_cover::attack_point(*this,this->state_frames(),kind10_,timer_target_,enemies);
                    const auto current=this->point()&&this->navigation()?this->navigation()->index_of(*this->point()):0xffffu;
                    if(selected!=0xffff&&selected!=current){request_attack_posture(this->navigation()->record(selected),control,world);query.state.set(0x54,0x4000u);query.request_point(selected);words.set(8,5u);return;}
                    request_attack_posture(this->point(),control,world);attack_mode();
                }
            }
        }
        if(!short_range&&mode()==5&&(query.state.template get<std::uint32_t>(0x50)&0x80000000u))attack_mode();
        if(mode()==4){
            if(timer_target_&&!target_is_hostile()&&target_alive()&&words.template get<std::uint32_t>(0x10)){
                using namespace human_ai_cover;
                if(!this->human()->head_marker)throw std::logic_error("Area look requires actual head marker");
                const auto matrix=this->state_frames().actor_world_matrix(this->human()->head_marker);HumanVector facing{{matrix[8],matrix[9],matrix[10],matrix[11]}};
                normalize_human_ai_shot(facing,square(facing));
                const auto self=actor_position(this->state_frames(),*this->human());const auto target=actor_position(this->state_frames(),*timer_target_);auto direction=difference(target,self);normalize_human_ai_shot(direction,square(direction));
                normalize_human_ai_shot(facing,square(facing));
                const auto dot=(static_cast<long double>(facing[0])*direction[0]+static_cast<long double>(facing[1])*direction[1])+static_cast<long double>(facing[2])*direction[2];
                if(dot<=.7599999904632568f){
                    if(resources.direction().request_look(direction,[&](const auto& a,const auto& b){return this->state_frames().car_vector_angle(a,b);}))words.set(8,3u);
                }else{world.human_area_force_sight(*this->human(),*timer_target_);words.set(8,3u);}
            }else words.set(8,3u);
        }
        if(mode()==6){
            if(!timer_target_||!target_alive()||!target_is_hostile()){resources.target().bind_actor(nullptr);words.set(8,3u);resources.flag().request(0);}
            else resources.target().bind_actor(timer_target_);
            if(!short_range&&!words.template get<std::uint32_t>(0x10)){words.set(8,3u);resources.flag().request(0);}
        }
    }
private:
    bool target_alive()const{const auto* human=dynamic_cast<const Human*>(timer_target_);if(!human)throw std::logic_error("Area target requires actual Human health");return human_frame_signed_word(human->properties.stats_ec[0])>0;}
    bool target_is_hostile()const{const auto& enemies=this->human()->perception.lists()[0];return std::find(enemies.rbegin(),enemies.rend(),timer_target_)!=enemies.rend();} //0055B8D0.
    std::uint32_t random_word(){const auto value=contexts_.human_script_random();if(value>0x7fffu)throw std::logic_error("Area requires shared CRT random source");return value;}
    std::uint32_t random_interval(double range,double base){std::uint32_t total=0;for(unsigned n=0;n<4;++n)total+=random_word()>>2;return static_cast<std::uint32_t>((static_cast<long double>(total)*0x1.0002p-15f-.5)*range+base);}
    template<class Inline,class World>void request_defensive_posture(const MissionTableRecord* point,Inline& control,World& world){ //0055B6F0.
        if(!point)return;
        const auto& list=this->human()->perception.lists()[0];
        const auto exposed=[&](std::uint8_t posture){for(auto i=list.size();i;)if(!human_ai_cover::blocked_toward(*point,human_ai_cover::actor_position(this->state_frames(),*list[--i]),posture))return true;return false;};
        if(exposed(4)&&!exposed(2)&&(this->words().template get<std::uint32_t>(0x50)&4u))this->request_navigation_posture(2,control,world.mode_flags());
    }
    template<class Inline,class World>void request_attack_posture(const MissionTableRecord* point,Inline& control,World& world){ //0055BC40.
        if(!point||!timer_target_)return;
        if(!human_ai_cover::blocked_toward(*point,human_ai_cover::actor_position(this->state_frames(),*timer_target_),2)){this->request_navigation_posture(2,control,world.mode_flags());return;}
        if(!human_ai_cover::blocked_toward(*point,human_ai_cover::actor_position(this->state_frames(),*timer_target_),4))this->request_navigation_posture(4,control,world.mode_flags());
    }
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
