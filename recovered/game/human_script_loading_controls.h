#ifndef HD2_GAME_HUMAN_SCRIPT_LOADING_CONTROLS_H
#define HD2_GAME_HUMAN_SCRIPT_LOADING_CONTROLS_H

#include "game/mission_script_actor_context.h"
#include <cstring>

namespace hd2::game {

//Actual005AF190/2D0/3B0/3D0/3F0/570 and005A5D50/7F50/7FA0 receivers.
//All lookups borrow the SAME Human composition; Sight/SOUND and Scene2 active
//identities are actual owners. Players/FPV still supply real presentation and
//attachment consumers; this bridge does not advertise application readiness.
template<class Frames,class Human,class Scene,class Sight,class Sound,class Players,class FPV,class Lookup>
class HumanScriptLoadingControls {
public:
    using Factory=std::decay_t<decltype(std::declval<Lookup&>().human_state_factory(std::declval<Human&>()))>;
    using Base=typename Factory::Base;using Script=typename Factory::Script;
    HumanScriptLoadingControls(Frames& frames,Scene& scene,Sight& sight,Sound& sound,Players& players,FPV& fpv,Lookup& lookup)
        :frames_(frames),scene_(scene),sight_(sight),sound_(sound),players_(players),fpv_(fpv),lookup_(lookup){}
    std::uint32_t mode_flags(){return scene_.mode_flags();}
    void set_alarm(Human& human,bool enabled){if(auto* state=script(human))state->set_alarm(enabled);}
    void guard_mode(Human& human){if(auto* state=script(human))state->request_guard_mode();}
    void zombie_mode(Human& human){if(auto* state=script(human))state->ai_mode(0x400);}
    void events(Human& human,bool enabled){if(auto* state=script(human))state->events(enabled,*this);}
    void attack(Human& human,typename Frames::Frame target,std::uint32_t duration,bool ill_fire,std::int32_t pc){ //005AF230/250.
        auto* state=script(human);if(!state)return;
        const auto* data=target?frames_.find_actor_frame_data(target):nullptr;
        auto* actor=data?reinterpret_cast<SceneActor<Frames>*>(data->words[0]):nullptr;
        if(actor)state->request_actor_attack(actor,duration,ill_fire,pc,lookup_);
        else state->request_frame_attack(target,duration,ill_fire,pc,lookup_);
    }
    bool vehicle_path(Human& human,const std::string& name,float speed,std::int32_t pc){ //00421C00 op26→00577A60.
        if(!human.field160||human.field160->base().type()!=4)return false;
        const auto point=lookup_.human_vehicle_named_point(name);
        if(auto* state=script(human))(void)lookup_.request_human_vehicle_point(*state,point,speed,point==0xffff?-1:pc);
        return point!=0xffff; //A missing named point stops the path then advances; valid point parks PC.
    }
    bool vehicle_chase(Human& human,typename Frames::Frame frame,float speed,std::int32_t pc){ //00421C00 op2B.
        if(!human.field160||human.field160->base().type()!=4||human.field164!=0)return false;
        const auto point=lookup_.human_vehicle_chase_point(human,frame);if(!point)return false;
        if(auto* state=script(human))(void)lookup_.request_human_vehicle_point(*state,*point,speed,*point==0xffff?-1:pc);
        return *point!=0xffff;
    }
    void turn_at(Human& human,typename Frames::Frame frame){if(auto* state=script(human))state->turn_at_frame(frame,lookup_);} //005AF660→005904C0.
    void working_mode(Human& human,std::int32_t mode) { //005AF5C0→0058FCC0.
        if(auto* state=script(human)){
            auto& factory=lookup_.human_state_factory(human);
            if(factory.control().human()!=&human)throw std::logic_error("Script working mode factory belongs to another Human");
            state->working_mode(mode,factory.control());
        }
    }
    void sniper_area(Human& human,bool enabled,float radius){ //005AF700→005AE610, does not change B0 here.
        const auto matrix=frames_.actor_world_matrix(human.base().frame());
        if(!human.command)throw std::logic_error("Human sniper area requires actual B4");
        human.command->sniper_area(enabled,radius,{{matrix[12],matrix[13],matrix[14],matrix[15]}});
    }
    void end_script(Human& human) { //005AF450→0058F680; no state borrower survives selection.
        auto* state=script(human);if(!state)return;
        if(state->words().template get<std::uint32_t>(0x50)&0x40000000u)state->events(true,*this);
        auto& owner=lookup_.human_state_owner(human);
        if(owner.get()!=state)throw std::logic_error("EndScript requires the SAME published B0 owner");
        switch(human.properties.field19c){
        case 1:{
            auto& factory=lookup_.human_state_factory(human);
            owner.select_script(false,factory.control(),players_,mode_flags(),scene_.human_current_player_id());return;
        }
        case 2:
            if(!human.command)throw std::logic_error("EndScript requires its actual Human command owner");
            owner.change(human.command->flag14?8u:4u);return;
        case 3:owner.change(6);return;
        case 4:case 5:owner.change(3);return;
        default:return;
        }
    }
    void suspend(Human& human,bool value){
        if(auto* state=script(human))state->suspend(value,*this);
        else {suspend_human(human,value);human_events(human,!value);}
    }
    void human_events(Human& human,bool enabled) {
        if(human.field258!=0xffffffffu)sight_.observer_enabled(signed_word(human.field258),enabled);
        if(enabled)human.field2a4=static_cast<std::uint32_t>(sound_.add_listener(human.base().frame()));
        else {
            if(human.field2a4!=0xffffffffu)sound_.remove_listener(signed_word(human.field2a4));
            human.field2a4=0xffffffffu;
        }
    }
    void suspend_human(Human& human,bool value) {
        auto& factory=lookup_.human_state_factory(human);
        if(factory.control().human()!=&human)throw std::logic_error("Script suspension factory belongs to another Human");
        scene_.set_scene_actor_active(human.base(),!value); //00475EC0, regardless of current active index.
        const bool hidden=value||(human.field160&&players_.attachment_hides_human(human));
        frames_.object_visible(human.base().frame(),!hidden);frames_.update_scene_object(human.base().frame());
        factory.control().suspend(value);
        if(human.loading_state) {
            auto* state=dynamic_cast<Base*>(human.loading_state);
            if(!state||state->human()!=&human)throw std::logic_error("Script suspension lost its actual current Human state");
            factory.suspend_human_state(*state,value);
        }
        if(human.field25c!=0xffffffffu)sight_.target_mode(signed_word(human.field25c),value?3u:4u);
        if(!value&&human.properties.field19c==1&&players_.current_player()==&human)fpv_.update_player_visibility();
    }
private:
    Script* script(Human& human) {
        auto* state=human.loading_state;
        if(!state)throw std::runtime_error("Human script control requires its actual B0 state");
        if(state->words().template get<std::uint32_t>(0x48)!=5)return nullptr;
        auto* result=dynamic_cast<Script*>(state);
        if(!result||result->human()!=&human)throw std::logic_error("Human script control has a different actual state5");
        return result;
    }
    static std::int32_t signed_word(std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;}
    Frames& frames_;Scene& scene_;Sight& sight_;Sound& sound_;Players& players_;FPV& fpv_;Lookup& lookup_;
};

//00421C00's initial loading commands. The containing runtime must try the
//actual base00416E10 receiver first, then this derived Human receiver.
template<class Frames,class Human,class Controls,class Runtime>std::optional<bool> execute_human_loading_script_command(
        MissionScriptContext<Frames>& context,const MissionScriptInstruction& instruction,Controls& controls,Runtime& runtime){
    switch(instruction.opcode){case 0x1d:case 0x21:case 0x26:case 0x2b:case 0x30:case 0x3e:case 0x2f:case 0xc7:case 0xcf:case 0xf8:case 0x44:break;default:return std::nullopt;}
    if(context.family()!=1)return std::nullopt;
    if(!runtime.script_authority())return false;
    auto* derived=dynamic_cast<ActorMissionScriptContext<Frames,1>*>(&context);
    auto* human=derived?dynamic_cast<Human*>(&derived->actor()):nullptr;
    if(!human)throw std::logic_error("Human script command requires the actual actor10C");
    switch(instruction.opcode){
    case 0x2b:{
        const auto bits=context.program().integers.at(instruction.operands.at(0)).value;std::int32_t speed;std::memcpy(&speed,&bits,4);
        const auto value=static_cast<float>(static_cast<long double>(static_cast<float>(speed))*.2800000011920929f);
        if(controls.vehicle_chase(*human,context.frame(),value,context.pc()))return true;
        break;
    }
    case 0x26:{
        const auto bits=context.program().integers.at(instruction.operands.at(1)).value;std::int32_t speed;std::memcpy(&speed,&bits,4);
        const auto value=static_cast<float>(static_cast<long double>(static_cast<float>(speed))*.2800000011920929f);
        if(controls.vehicle_path(*human,context.program().strings.at(instruction.operands.at(0)),value,context.pc()))return true;
        break;
    }
    case 0x3e:controls.turn_at(*human,context.slot(instruction.operands.at(0)));break;
    case 0x21:case 0x30:
        controls.attack(*human,context.slot(instruction.operands.at(0)),context.program().integers.at(instruction.operands.at(1)).value,instruction.opcode==0x30,context.pc());
        return true; //Original PC parks even if admission is rejected; timed completion advances it conditionally.
    case 0x44:{
        const auto enabled=context.program().integers.at(instruction.operands.at(0)).value!=0;
        const auto bits=context.program().integers.at(instruction.operands.at(1)).value;std::int32_t radius;std::memcpy(&radius,&bits,4);
        controls.sniper_area(*human,enabled,static_cast<float>(radius));break;
    }
    case 0x1d:controls.set_alarm(*human,context.program().integers.at(instruction.operands.at(0)).value!=0);break;
    case 0xc7:controls.guard_mode(*human);break;
    case 0xcf:controls.zombie_mode(*human);break;
    case 0xf8:controls.events(*human,context.program().integers.at(instruction.operands.at(0)).value!=0);break;
    case 0x2f:{
        const bool value=context.program().integers.at(instruction.operands.at(0)).value!=0;
        if(value&&!derived->flag108()){context.defer_actor_suspend();derived->flag108(true);return true;}
        controls.suspend(*human,value);context.advance(runtime);derived->flag108(false);return true;
    }
    default:throw std::logic_error("Unreachable Human loading command");
    }
    context.advance(runtime);return true;
}

} //namespace hd2::game
#endif
