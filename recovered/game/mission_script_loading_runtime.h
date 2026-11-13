#ifndef HD2_GAME_MISSION_SCRIPT_LOADING_RUNTIME_H
#define HD2_GAME_MISSION_SCRIPT_LOADING_RUNTIME_H

#include "game/mission_script_module_loading.h"
#include "game/mission_script_actor_context.h"
#include "game/human_script_loading_controls.h"
#include "game/mission_script_actor_lifecycle.h"
#include "game/mission_script_view_commands.h"
#include "game/mission_objective_status.h"
#include "game/mission_script_formation_commands.h"

namespace hd2::game {

//Containing source bridge for the reconstructed module/context receivers.
//Dependencies borrows actual mode/session/advance_override/clock/bindings,
//views/objectives/lifecycle/controls/predicates/world/effects owners. No default
//command, predicate, healthy guard or successful missing World service exists.
//All those owners and the SAME Frames must outlive this bridge and its module.
template<class Dependencies>class MissionScriptLoadingRuntime {
public:
    using Frames=typename Dependencies::Frames;using Files=typename Dependencies::Files;
    using Models=typename Dependencies::Models;using Human=typename Dependencies::Human;
    using Context=MissionScriptContext<Frames>;
    using Module=MissionScriptModuleLoading<Frames,Files,MissionScriptLoadingRuntime,Models>;
    MissionScriptLoadingRuntime(Frames& frames,Dependencies dependencies):frames_(frames),dependencies_(dependencies){}
    MissionScriptLoadingRuntime(const MissionScriptLoadingRuntime&)=delete;
    MissionScriptLoadingRuntime& operator=(const MissionScriptLoadingRuntime&)=delete;
    void bind_module(Module& module,std::uint32_t& mode){
        if(module_||&module.runtime()!=this||&mode!=&dependencies_.mode)
            throw std::logic_error("Script runtime requires its SAME module/runtime/process mode owner");
        module_=&module;
    }
    void unbind_module(Module& module)noexcept{if(module_==&module)module_=nullptr;}
    Module& module(){if(!module_)throw std::logic_error("Script runtime has no containing module owner");return *module_;}
    std::uint32_t mode_flags()const{return dependencies_.mode;}
    bool script_authority()const{return (mode_flags()&1u)!=0;}
    bool script_can_advance()const{ //00416CF0, actual008692D0/game28 gate.
        return dependencies_.advance_override||dependencies_.session.words.template get<std::uint32_t>(0x28)!=0;
    }
    std::uint32_t script_microseconds(){return dependencies_.clock.microseconds();} //004167B0 same QPF/QPC wrapping clock.
    auto make_script_context(Frames& frames,typename Frames::Frame frame){return dependencies_.bindings.make_script_context(frames,frame);}
    auto prepare_script_binding(Context& context){return dependencies_.bindings.prepare_script_binding(context);}
    template<class Target>void script_binding_completed(const Target& target,Context* context){dependencies_.bindings.script_binding_completed(target,context);}
    bool script_accept_signal(Context& context){ //00416610 Human admission, not health/roster membership.
        const auto* data=context.frame()?frames_.find_actor_frame_data(context.frame()):nullptr;
        auto* actor=data?reinterpret_cast<SceneActor<Frames>*>(data->words[0]):nullptr;
        if(!actor||actor->base().type()!=3)return true;
        auto* human=dynamic_cast<Human*>(actor);
        if(!human||!human->loading_state)throw std::runtime_error("Script signal requires its actual Human/B0 owner");
        return human->loading_state->words().template get<std::uint32_t>(0x48)==5;
    }
    void fail_human_mission(){dependencies_.objectives.fail_all(false);} //005A0220→0045E790→0069BED0.
    void set_human_objective_status(std::int32_t identity,std::int32_t status){dependencies_.objectives.set_status(identity,status);} //0053BD00→actual0069BA60 owner.
    void receive_human_objective_status(std::int32_t identity,std::int32_t status){dependencies_.objectives.receive_status(identity,status);}
    void script_return_end(Context& context){dependencies_.lifecycle.return_end_human(context,script_authority());}
    void script_loading_finish(){dependencies_.effects.fade().configure(0,false,nullptr);} //00427B80→GEffects8C/00486B30.
    bool human_remote_ai_enabled(){return dependencies_.world.human_remote_ai_enabled();}
    bool human_selected_suppressed(Human& human){return dependencies_.world.human_selected_suppressed(human);}
    std::uint32_t evaluate_script_predicate(Context& context,std::uint32_t opcode,
            const std::array<std::uint32_t,4>& values,const std::array<std::uint32_t,4>&,
            const MissionScriptProgram& program){
        if(!script_authority())return 0;
        if(const auto result=module().variables().evaluate(opcode,values,program,true))return *result;
        if(const auto result=dependencies_.predicates.evaluate(context,opcode,values,program,*this))return *result;
        throw std::runtime_error("Required script predicate is not reconstructed: "+std::to_string(opcode));
    }
    void execute_script_command(Context& context,const MissionScriptInstruction& instruction){
        if(!script_authority())return;
        //00421C00/00421970 call base00416E10 BEFORE their derived switch.
        if(accept_base(module().execute(context,instruction),context))return;
        if(accept_base(execute_formation_script_command<Frames,Human>(frames_,dependencies_.players,context,instruction,*this),context))return;
        if(accept_base(dependencies_.views.execute(context,instruction,*this),context))return;
        if(accept_base(execute_mission_objective_command(context,instruction,dependencies_.objectives,*this),context))return;
        if(accept_base(dependencies_.lifecycle.execute(context,instruction,*this),context))return;
        if(const auto result=execute_human_loading_script_command<Frames,Human>(context,instruction,dependencies_.controls,*this)){
            if(*result)return;
            throw std::runtime_error("Actual Human script receiver declined a required command");
        }
        throw std::runtime_error("Required script command is not reconstructed: "+std::to_string(instruction.opcode));
    }
private:
    bool accept_base(std::optional<bool> result,Context& context){
        if(!result)return false;
        if(!*result&&context.family()!=0){
            //Only a KNOWN base command's genuine false return reaches here.
            //Native Human/Dog default then advances. Unknown commands above
            //remain nullopt and must reach a real derived receiver or reject.
            if(context.family()!=1&&context.family()!=2)throw std::runtime_error("Script context has no original family");
            context.advance(*this);
        }
        return true;
    }
    Frames& frames_;Dependencies dependencies_;Module* module_=nullptr;
};

//Own runtime BEFORE module, and finish module while all borrowed dependencies
//and frame identities still exist. Reuse the process registration when supplied.
//This does not construct the containing world or enable application loading.
template<class Dependencies>class MissionScriptLoadingComposition {
public:
    using Runtime=MissionScriptLoadingRuntime<Dependencies>;using Module=typename Runtime::Module;
    using Frames=typename Runtime::Frames;using Files=typename Runtime::Files;using Models=typename Runtime::Models;
    MissionScriptLoadingComposition(frontend::ClientModuleRegistry& registry,Frames& frames,Files& files,Models& models,
            SessionBroadcastPackets& packets,std::uint32_t& mode,std::string resource,MissionScriptVariables& variables,Dependencies dependencies)
        :frames_(frames),runtime_(frames,dependencies),module_(registry,frames,files,runtime_,models,packets,mode,std::move(resource),variables){
        runtime_.bind_module(module_,mode);
    }
    MissionScriptLoadingComposition(frontend::ClientModuleRegistry& registry,Frames& frames,Files& files,Models& models,
            SessionBroadcastPackets& packets,std::uint32_t& mode,std::string resource,MissionScriptVariables& variables,Dependencies dependencies,
            frontend::ClientModuleRegistration& shared)
        :frames_(frames),runtime_(frames,dependencies),module_(registry,frames,files,runtime_,models,packets,mode,std::move(resource),variables,shared){
        runtime_.bind_module(module_,mode);
    }
    ~MissionScriptLoadingComposition(){
        try{module_.finish(runtime_.script_authority());}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
        runtime_.unbind_module(module_);
    }
    MissionScriptLoadingComposition(const MissionScriptLoadingComposition&)=delete;
    MissionScriptLoadingComposition& operator=(const MissionScriptLoadingComposition&)=delete;
    Module& module(){return module_;}Runtime& runtime(){return runtime_;}
private:
    Frames& frames_;Runtime runtime_;Module module_;
};

} //namespace hd2::game
#endif
