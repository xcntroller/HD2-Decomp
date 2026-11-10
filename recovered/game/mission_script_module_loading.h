#ifndef HD2_GAME_MISSION_SCRIPT_MODULE_LOADING_H
#define HD2_GAME_MISSION_SCRIPT_MODULE_LOADING_H

#include "frontend/module_registration.h"
#include "game/mission_script_bindings.h"
#include "game/mission_script_contexts.h"
#include "game/mission_script_frame_commands.h"
#include "game/mission_script_visibility.h"
#include "game/mission_script_variables.h"
#include "game/mission_script_rotations.h"
#include "game/mission_script_output_commands.h"

namespace hd2::game {

//Containing module4 loading composition. Actual command/predicate factory,
//model/effect/world and clock endpoints remain required receivers. Deferred
//model/clip/cutscene producers are not provided by successful empty services.
template<class Frames,class Files,class Runtime,class Models>class MissionScriptModuleLoading {
public:
    using Contexts=MissionScriptContexts<Frames,Files,Runtime>;
    using Snapshots=MissionScriptFrameSnapshots<Frames>;
    using Visibility=MissionScriptVisibility<Frames,Snapshots,Models>;
    MissionScriptModuleLoading(frontend::ClientModuleRegistry& registry,Frames& frames,Files& files,Runtime& runtime,
                                Models& models,SessionBroadcastPackets& packets,std::uint32_t& mode,std::string resource,MissionScriptVariables& variables)
        :registration_(registry,4,"Script"),frames_(frames),files_(files),runtime_(runtime),resource_(std::move(resource)),mode_(mode),
         contexts_(frames,files,runtime,resource_),frame_commands_(frames,contexts_),snapshots_(frames),rotations_(frames,snapshots_),visibility_(frames,snapshots_,models,packets,mode),variables_(variables){}
    MissionScriptModuleLoading(frontend::ClientModuleRegistry& registry,Frames& frames,Files& files,Runtime& runtime,
                                Models& models,SessionBroadcastPackets& packets,std::uint32_t& mode,std::string resource,MissionScriptVariables& variables,
                                frontend::ClientModuleRegistration& shared_registration)
        :registration_(registry,shared_registration,4,"Script"),frames_(frames),files_(files),runtime_(runtime),resource_(std::move(resource)),mode_(mode),
         contexts_(frames,files,runtime,resource_),frame_commands_(frames,contexts_),snapshots_(frames),rotations_(frames,snapshots_),visibility_(frames,snapshots_,models,packets,mode),variables_(variables){}
    auto& registration(){return registration_.get();}auto& contexts(){return contexts_;}auto& variables(){return variables_;}
    Runtime& runtime(){return runtime_;} //Containing source identity check, no new original-function credit.
    Frames& frames(){return frames_;} //SAME containing registry for checked Human/VM borrowers.
    auto& snapshots(){return snapshots_;}auto& visibility(){return visibility_;}
    auto& rotations(){return rotations_;}
    bool loaded()const{return loaded_;}const auto& failed_bindings()const{return failed_bindings_;}
    void activate(bool authority,std::uint32_t configuration_kind) { //004270D0.
        if(!authority||loaded_)return;
        flag825_=false;failed_bindings_.clear();
        (void)bindings_.load(files_,frames_,resource_,(mode_&7u)!=7u);
        const auto& bindings=bindings_.values();
        for(std::size_t n=bindings.size();n;) {
            const auto binding=bindings[--n];
            if(const auto frame=frames_.resolve_scene_path(binding.frame))
                if(!contexts_.bind(frame,binding.script.c_str()))failed_bindings_.push_back(binding);
        }
        bindings_.clear();loaded_=true;timer_=0;fields824_.fill(0);
        variables_.activate(configuration_kind);flag11_=false;
        //Native loaded10 latches even when00427730 returnsfalse. The containing
        //world must inspect failed_bindings before claiming required init success.
    }
    void tick(std::uint32_t elapsed) { //00428490's reached branch with no deferred-frame producer.
        if(timer_)throw std::runtime_error("Script timer requires its actual game6E producer/receiver");
        if(loaded_)contexts_.tick_contexts(elapsed,fields824_[0]!=0);
    }
    bool receive(SessionPacketStream* packet){return visibility_.receive(packet);}
    void frame_update(std::int32_t elapsed) { //00428B30; not gated by loaded10.
        rotations_.update(elapsed);
        if(fields824_[0])frames_.script_cutscene_viewport(); //Actual IGraph dimensions→scene+148 receiver.
    }
    std::optional<bool> execute(typename Contexts::Context& context,const MissionScriptInstruction& instruction) {
        if(const auto result=frame_commands_.execute(context,instruction,runtime_))return result;
        if(const auto result=execute_mission_script_output(context,instruction,runtime_))return result;
        if(const auto result=variables_.execute(context,instruction,runtime_))return result;
        if(const auto result=snapshots_.execute(context,instruction,runtime_))return result;
        if(const auto result=rotations_.execute(context,instruction,runtime_))return result;
        return visibility_.execute(context,instruction,runtime_);
    }
    void finish(bool authority) { //00427B80 supported loading/SetOn owners, real restore BEFORE exit.
        if(!loaded_||!authority)return;
        visibility_.clear_lights();flag825_=false;
        std::exception_ptr failure;
        try{snapshots_.clear();}catch(...){failure=std::current_exception();}
        rotations_.clear(); //Native restores snapshot rotations BEFORE releasing84C record frame references.
        try{contexts_.finish_contexts();}catch(...){if(!failure)failure=std::current_exception();}
        loaded_=false;
        //Original always notifies GEffects+8C(0,false,nullptr), even with empty queues.
        try{runtime_.script_loading_finish();}catch(...){if(!failure)failure=std::current_exception();}
        if(failure)std::rethrow_exception(failure);
    }
    void unload(bool authority) { //004285D0: finish first, then authority binding retirement.
        if(loaded_)finish(authority);
        if(authority)bindings_.clear();
    }
private:
    frontend::ClientModuleBinding registration_;
    Frames& frames_;Files& files_;Runtime& runtime_;std::string resource_;std::uint32_t& mode_;
    Contexts contexts_;MissionScriptFrameCommands<Frames,Contexts> frame_commands_;Snapshots snapshots_;MissionScriptRotations<Frames,Snapshots> rotations_;Visibility visibility_;MissionScriptVariables& variables_;MissionScriptBindings bindings_; //SAME process module4 bank, not reset by world destruction/finish.
    std::vector<MissionScriptBindings::Binding> failed_bindings_;
    std::array<std::uint32_t,5> fields824_{};std::uint32_t timer_=0;bool loaded_=false,flag11_=false,flag825_=false;
};
} //namespace hd2::game
#endif
