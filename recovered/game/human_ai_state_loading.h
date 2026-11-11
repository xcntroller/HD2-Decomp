#ifndef HD2_GAME_HUMAN_AI_STATE_LOADING_H
#define HD2_GAME_HUMAN_AI_STATE_LOADING_H

#include "game/human_ai_resources.h"
#include "game/human_input_callbacks.h"
#include "game/human_state_base_loading.h"
#include "game/human_ai_event_queue.h"
#include "game/scene_actor.h"
#include "game/human_ai_direction_frame.h"
#include "game/human_ai_scene_steering.h"
#include "game/human_ai_frame_query.h"
#include "game/human_ai_frame_output.h"
#include "game/human_ai_ground_projection.h"
#include "game/human_ai_light_observation.h"

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
    bool project_ground_position(HumanVector& position)override{
        return project_human_ai_ground_position(*this,this->state_frames(),position); //0057FBC0.
    }
    bool observe_navigation_light(float& shadowed,float& plain)override{
        return observe_human_ai_light(*this,this->state_frames(),shadowed,plain); //0057F880.
    }
    bool frame_actor_contact(std::uint32_t kind,std::int32_t,std::uint32_t)override{return kind!=8u;} //005765F0, genuine AI Ladder rejection, absent frozen.
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
    //0056F630 returns path jobs before resetting the derived current action.
    //Positive Base84/94 command retirement still needs the actual visitor.
    void close_loading() {
        resources_.finish_navigation_jobs();
        if(resources_.state().template get<std::uint32_t>(0xf0))
            throw std::runtime_error("Human AI close requires actual action-pool retirement");
        reset_frame_action();
        bound_=false; //Native does not call00583690 or remove the shared input.
    }
    virtual void reset_frame_action(){resources_.retire_current_action();} //AI virtual10C:00575820.
    auto& resources(){return resources_;}const auto& resources()const{return resources_;}
    void publish_frame_command_flags(std::uint32_t command)override{ //0056FC30.
        const auto previous=this->words().template get<std::uint32_t>(0x50)&7u;
        Base::publish_frame_command_flags(command);
        const auto posture=this->words().template get<std::uint32_t>(0x50)&7u;
        if(posture!=previous)resources_.direction().state().set(0x44,posture&1u?.94f:.7f);
    }
    HumanVector current_direction()const override{return resources_.direction().state().template get<HumanVector>(0);} //0056FBF0.
    HumanVector target_direction()const override{return resources_.direction().state().template get<HumanVector>(0x10);} //0056FC00.
    bool aims_at_human(const Human& target)const override{return resources_.aimed_actor()==&target;}
    bool selected_hostile_is(const Human& target)const override{return resources_.selected_actor()==&target;} //00524DD0.
    std::vector<HumanAINavigationJobPool::Lease>* scene_navigation_jobs()override{return &resources_.navigation_jobs();} //00524DC0.
    bool scene_movement_enabled()const override{return !(this->words().template get<std::uint32_t>(0x50)&1u);} //00589290.
    bool scene_movement_interaction()const override{ //00524DE0.
        return (resources_.query().state.template get<std::uint32_t>(0x50)&0x30u)!=0;
    }
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
    template<class Inline>void apply_frame_movement(Inline& control){apply_human_ai_frame_movement(*this,control);}
    template<class Inline,class World,class Runtime>void apply_frame_items(Inline& control,World& world,
            Runtime& runtime,std::uint32_t elapsed,std::uint32_t fire_argument){
        apply_human_ai_frame_items(*this,control,world,runtime,elapsed,fire_argument);
    }
    void advance_frame_presentation(std::uint32_t mode){ //Guard00548130 and Area00559DA0.
        if(!(this->words().template get<std::uint32_t>(0x50)&0x4000800u)&&(mode&7u)!=7u)
            throw std::runtime_error("AI presentation requires the actual 0056F710 nonoffline receiver");
        this->process_second_frame_commands();
    }
    template<class Inline>void publish_authority_input_gate(Inline& control){
        auto* input=this->input();
        if(!input)return;
        auto* animation=control.control();
        if(!animation)throw std::logic_error("AI authority input gate requires actual DD0");
        input->word38=animation->animation_state_word(0x1ac)||animation->animation_state_word(0x9c0)?7u:0u;
    }
    template<class World>void advance_frame_aim(World& world){ //00576350.
        auto& target=resources_.target();
        const auto delta=target.state.template get<std::array<float,3>>(0x10);
        if(target.state.template get<std::uint8_t>(0x35)||target.state.template get<std::uint8_t>(0x34))
            throw std::runtime_error("AI aim frame requires its actual special-item target receiver");
        const bool firing=(this->words().template get<std::uint32_t>(0x50)&0x200000u)!=0;
        if(!target.actor&&human_scene_steering_square(delta)<1.e-16f){
            if(firing)stop_frame_aim(100,world);
            target.state.set(0x38,8u);
            return;
        }
        if(!firing){
            resources_.publish_aimed_actor(target.actor);
            resources_.state().set(0xd8,target.actor?0u:2u);
        }
        std::uint32_t result;
        if(!this->item()){
            if(firing){stop_frame_aim(400,world);stop_frame_aim(100,world);} //00575D50, then 00576350's status-10 mask.
            result=0x10;
        }else{
            const auto& item=world.human_frame_output_item(this->item());
            const auto now=world.game_word28();
            if(now<item.field14||human_frame_signed_word(now-item.field58)<0)result=0x40;
            else if(now<item.field5c)result=1;
            else if(resources_.state().template get<std::int32_t>(0xc8)>0)result=4;
            else if(firing&&!resources_.state().template get<std::uint32_t>(0xc4)){
                stop_frame_aim(static_cast<std::uint32_t>(static_cast<long double>(world.human_sound_random())*400.f+300.f),world);
                result=4;
            }
            else throw std::runtime_error("AI aim frame requires actual weapon alignment and line-of-fire receivers");
            if(firing){
                if(result==1)stop_frame_aim(100,world);
                else if(target.actor!=resources_.aimed_actor()){stop_frame_aim(100,world);result=0x80;}
            }
        }
        target.state.set(0x38,result);
    }
    void advance_scene_steering(){advance_human_ai_scene_steering(*this);} //0057F610.
    template<class Graph>std::uint32_t advance_frame_query(Graph& graph,std::uint32_t elapsed){
        return advance_human_ai_frame_query(*this,this->state_frames(),graph,elapsed);
    }
    template<class Inline,class World>std::uint32_t advance_frame_query(Inline& control,World& world,std::uint32_t elapsed){
        struct Graph {
            Inline& control;
            World& world;
            bool create_ai_navigation_path(HumanAIStateLoading& ai){
                return ai.create_frame_navigation_path(world.human_ai_navigation_jobs(),control,world.mode_flags());
            }
            void reroute_ai_navigation_point(HumanAIStateLoading& ai){
                ai.reroute_frame_navigation_point(world.human_ai_navigation_jobs(),control,world.mode_flags());
            }
            void advance_ai_navigation_path(HumanAIStateLoading& ai,std::uint32_t){
                ai.advance_frame_navigation_path(control,world);
            }
        } graph{control,world};
        return advance_frame_query(graph,elapsed);
    }
    template<class Inline>bool create_frame_navigation_path(HumanAINavigationJobPool& pool,Inline& control,std::uint32_t mode){ //0057CA70.
        auto& receiver=resources_.query();
        auto& query=receiver.state;
        const auto from=frame_navigation_origin();
        const bool built=(query.template get<std::uint32_t>(0x50)&4u)?
            build_human_ai_escape_path(*this,this->state_frames(),pool,from):
            build_human_ai_table_path(*this,pool,from,control.frame_navigation_floor());
        if(!built){
            query.set(0x50,(query.template get<std::uint32_t>(0x50)&~7u)|0x80000000u);
            return false;
        }
        receiver.begin_path();
        const bool floor_search=control.frame_navigation_floor();
        if(floor_search)query.set(0x50,(query.template get<std::uint32_t>(0x50)&~0x1000u)|0x700u);
        else if(this->words().template get<std::uint32_t>(0x50)&0x8000000u){
            control.clear_frame_item_action();
            if((mode&7u)!=7u)throw std::runtime_error("AI route item action requires its actual nonoffline packet receiver");
        }
        resources_.state().set(0x10c,0u);
        this->words().set(0x58,0.f);
        const auto origin=frame_navigation_origin();
        const auto& jobs=resources_.navigation_jobs();
        const auto first=jobs.front()->words.template get<HumanVector>(0);
        if(origin[0]!=first[0]||origin[2]!=first[2])
            human_ai_navigation::aim_along_job(resources_.direction(),origin,first,floor_search);
        else if(jobs.size()>1)
            human_ai_navigation::aim_along_job(resources_.direction(),origin,jobs[1]->words.template get<HumanVector>(0),floor_search);
        return true;
    }
    template<class Inline>void reroute_frame_navigation_point(HumanAINavigationJobPool& pool,Inline& control,std::uint32_t mode){ //0057B270.
        auto* navigation=this->navigation();
        if(!navigation||!this->point())return;
        const auto index=navigation->escape_restricted_point(this->point());
        if(index==0xffff)return;
        auto& receiver=resources_.query();
        receiver.request_point(index);
        receiver.state.set(0x54,0x4000u);
        receiver.state.set(0x50,receiver.state.template get<std::uint32_t>(0x50)|0x4000u);
        human_ai_navigation::finish_path(*this,4u);
        create_frame_navigation_path(pool,control,mode);
    }
    template<class Inline,class World>void advance_frame_navigation_path(Inline& control,World& world){ //0057F670.
        using namespace human_ai_navigation;
        auto& query=resources_.query().state;
        const auto flags=[&]{return query.template get<std::uint32_t>(0x50);};
        if(flags()&0x80000000u)return;
        const auto origin=frame_navigation_origin();
        const bool floor_search=control.frame_navigation_floor();
        this->navigation_point(path_navigation_point(*this,origin,floor_search));
        auto& jobs=resources_.navigation_jobs();
        auto* command=this->command();
        if(!command)throw std::logic_error("AI path requires actual Human B4");
        const auto kind=this->words().template get<std::uint32_t>(0x48);
        if(kind!=3&&!jobs.empty())update_navigation_control(origin,control,world);
        if(flags()&0x800u){advance_navigation_door(origin,control,world,floor_search);return;}
        if(flags()&0x30u)throw std::runtime_error("AI interaction traversal requires its actual 0057CC50 receiver");
        if(jobs.empty()){finish_path(*this,4u);return;}
        const bool floor_path=flags()&0x700u;
        const bool reached=reach_path_job(*this,origin);
        if(flags()&0x80000000u)return;
        auto& movement=frame_navigation_movement();
        if(floor_path){
            if(!command->lists[0].empty())throw std::runtime_error("AI floor path requires its actual 00581E30 actor-list receiver");
            if(jobs.size()>1&&jobs.front()->words.template get<std::uint8_t>(0x26)!=15)
                query.set(0x50,flags()&~0x700u);
        }else check_navigation_transition(reached,origin,control,world);

        if(reached){
            resources_.advance_navigation_job(); //Old104 returns first, then transfer frontF8.
            if(!floor_path)movement.flag104=true;
            if(jobs.empty()){finish_path(*this,4u);return;}
            if(!floor_path&&!(flags()&0x4000u)){
                const auto index=jobs.front()->words.template get<std::uint32_t>(0x20);
                const auto* point=this->navigation()->record(static_cast<std::uint16_t>(index));
                if(point&&(point->flags&0x2000u)){
                    query.set(0x50,flags()|0x8000u);
                    const auto random=world.human_frame_random_word(); //00560F90, same CRT stream.
                    query.set(0x34,static_cast<std::uint32_t>((1+static_cast<long double>(random)*0x1.0002p-15f)*5000.f));
                    return;
                }
            }
        }
        if(!floor_path)navigation_posture_near_point(control,world.mode_flags(),origin);
        const auto model=this->state_frames().actor_world_matrix(this->human()->base().frame());
        output_path_inputs(*this,movement,model,floor_search);
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
    void sight_enemy_killed(Human& actor)override{ //00573FB0, SAME actual first command event queue.
        auto* command=this->command();if(!command)throw std::logic_error("Human kill notification requires its actual command owner");
        const auto world=this->state_frames().actor_world_matrix(actor.base().frame());
        auto& queue=command->first.events;auto event=queue.pool().acquire();
        event->words[0]=0x10000003u;event->words[1]=10000;event->words[2]=0;
        event->words[3]=0xffffffffu;event->words[4]=static_cast<std::uint32_t>(actor.base().id10());event->words[12]=0;
        const HumanVector point{{world[12],world[13],world[14],world[15]}};
        std::memcpy(event->words.data()+8,point.data(),16);queue.push(std::move(event));
    }
    void sight_dead_acquired(Human& actor) override {
        const auto kind=this->words().template get<std::uint32_t>(0x48);
        if(kind!=4&&kind!=6&&kind!=8)throw std::runtime_error("Human dead-sight requires its actual derived B0 receiver");
        emit_sight(actor,2); //005741F0.
    }
protected:
    template<class Inline>void request_navigation_posture(std::uint32_t posture,Inline& control,std::uint32_t mode){ //005767F0.
        if(this->human()->field174)return;
        auto* animation=control.control();
        if(!animation)throw std::logic_error("AI posture requires the actual DD0 owner");
        if(human_frame_signed_word(animation->animation_state_word(0xa30))>0||
           animation->animation_state_word(0x1d8)==0x5fu)return;
        if(this->words().template get<std::uint32_t>(0x48)==3)
            throw std::runtime_error("Active AI posture requires the actual localized frame-text receiver");
        const auto flags=this->words().template get<std::uint32_t>(0x50);
        if(flags&posture||(posture!=1&&posture!=2&&posture!=4))return;
        const auto point_kind=this->point()?this->point()->byte16:0;
        if(posture!=1)this->words().set(0x4c,posture);
        if((posture!=1&&point_kind==8)||(posture==4&&point_kind==7))return;
        if(posture!=2&&(flags&0x8000000u)){
            control.clear_frame_item_action();
            if((mode&7u)!=7u)throw std::runtime_error("AI posture requires its actual nonoffline packet receiver");
        }
        if(posture==1)this->words().set(0x4c,posture);
        const unsigned offset=posture==2||(posture==4&&(flags&2u))?0x20u:
            posture==1||(flags&1u)?0x24u:0u;
        if(offset){
            auto* input=this->second_frame_inputs();
            if(!input)throw std::logic_error("AI posture requires the actual second frame input buffer");
            input->set(offset,1.f);
        }
        if(posture==1||(flags&1u)){
            auto& words=resources_.state();
            if(human_frame_signed_word(words.template get<std::uint32_t>(0xc8))<300)words.set(0xc8,300u);
        }
    }
    void emit_sight(Human& actor,unsigned kind) {
        auto* command=this->command();if(!command)throw std::logic_error("Human sight callback has no bound command owner");
        queue_human_ai_sight_event(this->state_frames(),command->first.events,actor,kind);
    }
private:
    template<class World>void stop_frame_aim(std::uint32_t delay,World& world){ //00572C20, no inline action-kind 4 stop.
        auto& target=resources_.target().state;
        const auto base=target.template get<std::int32_t>(0x44);
        if(base!=-1){
            const auto random=static_cast<long double>(world.human_sound_random());
            const auto span=target.template get<std::int32_t>(0x48);
            delay=static_cast<std::uint32_t>(static_cast<std::int64_t>((random-.5L)*span*2+base));
        }
        const auto flags=this->words().template get<std::uint32_t>(0x50);
        this->words().set(0x50,flags&~0x200000u);
        if(flags&0x8000000u)throw std::runtime_error("AI aim stop requires the actual inline action and B0 command receivers");
        resources_.publish_aimed_actor(nullptr);
        resources_.state().set(0xd8,0u);
        if(this->item()){
            const auto& item=world.human_frame_output_item(this->item());
            const auto now=world.game_word28();
            const auto interval=std::max(human_frame_signed_word(item.field14-now),human_frame_signed_word(item.field58-now));
            if(interval>0&&static_cast<std::uint32_t>(interval)>delay)delay=static_cast<std::uint32_t>(interval);
        }
        resources_.state().set(0xc8,delay);
    }
    HumanVector frame_navigation_origin(){
        auto* human=this->human();
        if(!human||!human->base().frame())throw std::logic_error("AI route requires the actual Human model");
        const auto matrix=this->state_frames().actor_world_matrix(human->base().frame());
        return {{matrix[12],matrix[13],matrix[14],matrix[15]}};
    }
    auto& frame_navigation_movement(){
        auto* human=this->human();
        if(!human||!human->scene_movement||human->scene_movement->owner!=human)
            throw std::logic_error("AI path requires the bound Human+2C4 movement owner");
        return *human->scene_movement;
    }
    template<class Inline,class World>void update_navigation_control(const HumanVector& origin,Inline& control,World& world){ //0057EC60.
        using namespace human_ai_navigation;
        auto& query=resources_.query().state;
        const auto kind=this->words().template get<std::uint32_t>(0x48);
        const bool guard_or_area=kind==4||kind==8;
        const auto first_kind=resources_.navigation_jobs().front()->words.template get<std::uint8_t>(0x26);
        const auto ordinary=[](unsigned value){return value==0||value>6;}; //0057EDC0.
        const auto clear_of_actors=[&]{ //0057EE00 ->005AE890, reverse actual B4 list.
            const auto& ids=this->command()->lists[0];
            for(auto it=ids.rbegin();it!=ids.rend();++it){
                auto* actor=world.human_ai_resolve_actor(*it);
                if(!actor)continue;
                if(!actor->base().frame())throw std::logic_error("AI path control requires actual nearby actor models");
                const auto matrix=this->state_frames().actor_world_matrix(actor->base().frame());
                if(distance_squared(position_xyz(origin),{{matrix[12],matrix[13],matrix[14]}})<4.f)return false;
            }
            return true;
        };
        const auto flags=query.template get<std::uint32_t>(0x50);
        const auto saved=query.template get<std::uint32_t>(0x30);
        if(this->point()&&ordinary(this->point()->byte16)&&ordinary(first_kind)&&clear_of_actors()){
            if(!(flags&0x10000u)||(!guard_or_area&&saved==0xffffffffu))return;
            control.clear_control_flags(guard_or_area?4u:6u);
            control.add_control_flags(guard_or_area?2u:saved&6u);
            if(!guard_or_area)query.set(0x30,0xffffffffu);
            query.set(0x50,flags&~0x10000u);
        }else{
            if((flags&0x10000u)||(!guard_or_area&&saved!=0xffffffffu))return;
            if(!guard_or_area)query.set(0x30,control.frame_navigation_control_mode());
            control.clear_control_flags(6);
            query.set(0x50,flags|0x10000u);
        }
    }
    template<class Inline,class World>void check_navigation_transition(bool reached,const HumanVector& origin,Inline& control,World& world){ //0057DBD0.
        auto& jobs=resources_.navigation_jobs();
        auto& first=jobs.front()->words;
        auto& receiver=resources_.query();
        auto& query=receiver.state;
        const auto first_kind=first.template get<std::uint8_t>(0x26);
        const auto index=static_cast<std::uint16_t>(first.template get<std::uint32_t>(0x20));
        const auto flags=[&]{return query.template get<std::uint32_t>(0x50);};
        const auto climb=[&]{ //0057DB70 -> 00586200 subtype1F.
            if((world.mode_flags()&7u)!=7u){
                auto& state=resources_.state();
                if(human_frame_signed_word(state.template get<std::uint32_t>(0x120))>0)return;
                state.set(0x120,1000u);
                throw std::runtime_error("AI path climb requires the actual nonoffline command receiver");
            }
            control.frame_movement_animation(0x6a,[&](std::uint32_t flags){publish_frame_command_flags(flags);});
        };
        if(!reached){
            const auto* current=this->point();
            if(current&&current!=this->navigation()->record(index)){
                if((current->byte16==6&&first_kind==5&&static_cast<long double>(first.template get<float>(4))-origin[1]>.3f)||
                   (current->byte16==4&&first_kind==4))climb();
            }
            return;
        }
        if(jobs.size()<2)return;
        const auto& next=jobs[1]->words;
        const auto second_kind=next.template get<std::uint8_t>(0x26);
        if(first_kind==15)query.set(0x50,flags()|0x700u);
        else if(first_kind==1&&second_kind==1){
            auto* door=this->navigation()->object_for_reference(index);
            if(door){
                const auto status=world.human_navigation_door(door).properties.fields220[1];
                if(status==0||status==3){receiver.door=door;query.set(0x50,flags()|0x800u);}
            }
        }else if(first_kind==3&&second_kind==3){
            receiver.actor=this->navigation()->object_for_reference(index);
            query.set(0x48,index);
            query.set(0x4a,static_cast<std::uint16_t>(next.template get<std::uint32_t>(0x20)));
            if(receiver.actor)query.set(0x50,flags()|0x10u);
        }else if((first_kind==4&&second_kind==4)||(first_kind==6&&second_kind==5))climb();
        else if(first_kind==5){
            if(second_kind==6){
                if(jobs.size()>2&&jobs[2]->words.template get<std::uint8_t>(0x26)==5){
                    const auto separation=human_ai_navigation::distance_squared(
                        first.template get<human_ai_navigation::Position>(0),jobs[2]->words.template get<human_ai_navigation::Position>(0));
                    if(separation>.25f&&separation<9.f){
                        auto* input=this->second_frame_inputs();
                        if(!input)throw std::logic_error("AI path transition requires the actual second frame input buffer");
                        input->set(0x30,1.f);
                    }
                }
            }else if(static_cast<long double>(first.template get<float>(4))-(second_kind==5?next.template get<float>(4):origin[1])>.3f)
                climb();
        }
    }
    template<class Inline,class World>
    void advance_navigation_door(const HumanVector& origin,Inline& control,World& world,bool floor_search){ //0057CF80.
        using namespace human_ai_navigation;
        auto& receiver=resources_.query();
        auto& query=receiver.state;
        if(query.template get<std::uint32_t>(0x34))return;
        auto& door=world.human_navigation_door(receiver.door);
        const auto status=door.properties.fields220[1];
        auto& direction=resources_.direction();
        const auto mode=resources_.flag().state.template get<std::uint32_t>(0);
        const auto matrix=[&]{return this->state_frames().actor_world_matrix(this->human()->base().frame());};
        if(status==1){
            if(mode==4&&direction.target_locked()){direction.unlock_target();direction.lock_current();}
            receiver.door=nullptr;
            query.set(0x50,query.template get<std::uint32_t>(0x50)&0xdffff7ffu);
            output_path_inputs(*this,frame_navigation_movement(),matrix(),floor_search);
            return;
        }
        if(status==2){
            if(door.navigation_side(origin)==1){
                auto* input=this->first_frame_inputs();
                if(!input)throw std::logic_error("AI Door approach requires the actual first frame input buffer");
                input->set(4,1.f);
            }
            return;
        }
        if(status!=0&&status!=3)return;
        if(this->words().template get<std::uint32_t>(0x48)==3&&door.properties.flag234){finish_path(*this,0xfffffff5u);return;}
        if(mode==2){finish_path(*this,0xffffffffu);return;}
        if(mode==4&&!direction.target_locked()){direction.unlock_current();direction.lock_target();}
        const auto world_matrix=matrix();
        auto forward=Position{{world_matrix[8],world_matrix[9],world_matrix[10]}};
        forward=unit(forward,square_length(forward));
        forward=unit(forward,square_length(forward));
        const auto& jobs=resources_.navigation_jobs();
        if(jobs.empty())throw std::logic_error("AI Door approach requires the actual next path job");
        auto delta=difference(jobs.front()->words.template get<Position>(0),position_xyz(origin));
        delta=unit(delta,square_length(delta));
        if((product(forward[0],delta[0])+product(forward[1],delta[1]))+product(forward[2],delta[2])<.86f){
            direction.set_current({{delta[0],delta[1],delta[2],direction.state().template get<float>(0x0c)}});
            query.set(0x34,300u);
        }
        const auto flags=this->words().template get<std::uint32_t>(0x50);
        if(this->item()&&this->words().template get<std::uint8_t>(0xad)){
            control.clear_frame_item_ready([&](std::uint32_t value){world.human_frame_first_person_state(*this->human(),value);});
            if((world.mode_flags()&7u)!=7u)throw std::runtime_error("AI Door readiness requires its actual nonoffline command receiver");
            query.set(0x34,500u);
        }
        if(!query.template get<std::uint32_t>(0x34))world.human_navigation_request_door(receiver.door,flags);
    }
    template<class Inline>void navigation_posture_near_point(Inline& control,std::uint32_t mode,const HumanVector& origin){ //00576610.
        using namespace human_ai_navigation;
        const auto* point=this->point();
        if(!point)return;
        const auto& jobs=resources_.navigation_jobs();
        const auto kind=jobs.front()->words.template get<std::uint8_t>(0x26);
        const auto xyz=this->navigation()->position(this->navigation()->index_of(*point));
        const auto squared=distance_squared(xyz,position_xyz(origin));
        const auto flags=this->words().template get<std::uint32_t>(0x50);
        const auto set_input=[&](unsigned offset){
            auto* input=this->second_frame_inputs();
            if(!input)throw std::logic_error("AI path posture requires the actual second frame input buffer");
            input->set(offset,1.f);
        };
        if(((point->byte16==4&&kind==4)||(point->byte16==6&&kind==5))&&length(static_cast<float>(squared))<1.5f){
            if(flags&4u)return;
            if(flags&2u)set_input(0x20);
            else if(flags&1u)set_input(0x24);
            return;
        }
        const auto radius=static_cast<unsigned char>(point->prefix[0x13])*.0625f;
        if(!(squared<product(radius,radius)))return;
        if(point->byte16==7){if(!(flags&2u))set_input(0x20);}
        else if(point->byte16==8){if(!(flags&1u))set_input(0x24);}
        else{
            const auto posture=this->words().template get<std::uint32_t>(0x4c);
            if(!(flags&posture))request_navigation_posture(posture,control,mode);
        }
    }
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
