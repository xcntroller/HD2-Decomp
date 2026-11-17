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
#include "game/mission_script_door_commands.h"
#include "game/mission_script_model_animations.h"
#include "game/mission_script_weather_commands.h"
#include "game/mission_script_cutscenes.h"
#include "game/mission_script_presentation_commands.h"

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
        :registration_(registry,4,"Script"),frames_(frames),files_(files),runtime_(runtime),resource_(std::move(resource)),mode_(mode),packets_(packets),
         contexts_(frames,files,runtime,resource_),frame_commands_(frames,contexts_),snapshots_(frames),animations_(frames,snapshots_,models,runtime),rotations_(frames,snapshots_),visibility_(frames,snapshots_,models,packets,mode),weather_(frames,runtime,packets,mode),presentation_(frames,runtime,packets),cutscenes_(frames,contexts_,snapshots_,animations_,runtime,fields824_[0],timer_),variables_(variables){}
    MissionScriptModuleLoading(frontend::ClientModuleRegistry& registry,Frames& frames,Files& files,Runtime& runtime,
                                Models& models,SessionBroadcastPackets& packets,std::uint32_t& mode,std::string resource,MissionScriptVariables& variables,
                                frontend::ClientModuleRegistration& shared_registration)
        :registration_(registry,shared_registration,4,"Script"),frames_(frames),files_(files),runtime_(runtime),resource_(std::move(resource)),mode_(mode),packets_(packets),
         contexts_(frames,files,runtime,resource_),frame_commands_(frames,contexts_),snapshots_(frames),animations_(frames,snapshots_,models,runtime),rotations_(frames,snapshots_),visibility_(frames,snapshots_,models,packets,mode),weather_(frames,runtime,packets,mode),presentation_(frames,runtime,packets),cutscenes_(frames,contexts_,snapshots_,animations_,runtime,fields824_[0],timer_),variables_(variables){}
    auto& registration(){return registration_.get();}auto& contexts(){return contexts_;}auto& variables(){return variables_;}
    Runtime& runtime(){return runtime_;} //Containing source identity check, no new original-function credit.
    Frames& frames(){return frames_;} //SAME containing registry for checked Human/VM borrowers.
    auto& snapshots(){return snapshots_;}auto& visibility(){return visibility_;}
    auto& rotations(){return rotations_;}
    bool loaded()const{return loaded_;}const auto& failed_bindings()const{return failed_bindings_;}
    void reset_round(std::uint32_t configuration_kind,bool& advance_override){ //00424900.
        advance_override=true;
        if(mode_&2u)finish(false);
        if(mode_&1u){finish(true);variables_.restore_previous();if(mode_&1u)activate(true,configuration_kind);}
        if(mode_&2u)activate(false,configuration_kind);
        advance_override=false; //Original normal-return tail; no invented failure rollback.
    }
    void activate(bool authority,std::uint32_t configuration_kind) { //004270D0.
        if(!authority||loaded_)return;
        activation_started_=true; //Source guard: bound contexts can exist before native loaded10 publication.
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
        cutscenes_.tick_timer(elapsed);
        if(loaded_)contexts_.tick_contexts(elapsed,fields824_[0]!=0);
    }
    bool receive(SessionPacketStream* packet){ //00424590 reads opcode once, then0042B650.
        if(!packet)return false;
        const auto opcode=packet->read(8);if(!(mode_&2u))return true;
        if(weather_.receive_opcode(opcode,*packet))return true;
        if(presentation_.receive_opcode(opcode,*packet))return true;
        return visibility_.receive_opcode(opcode,*packet);
    }
    void end_cutscene(){ //00423EB0: first native BYTE824 gate, before camera/engine/context calls.
        cutscenes_.end();
    }
    void frame_update(std::int32_t elapsed) { //00428B30; not gated by loaded10.
        rotations_.update(elapsed);
        if(fields824_[0])frames_.script_cutscene_viewport(); //Actual IGraph dimensions→scene+148 receiver.
    }
    std::optional<bool> execute(typename Contexts::Context& context,const MissionScriptInstruction& instruction) {
        if(const auto result=cutscenes_.execute(context,instruction))return result;
        if(const auto result=frame_commands_.execute(context,instruction,runtime_))return result;
        if(const auto result=execute_mission_script_door_command(frames_,context,instruction,packets_,runtime_))return result;
        if(const auto result=execute_mission_script_output(context,instruction,runtime_))return result;
        if(const auto result=variables_.execute(context,instruction,runtime_))return result;
        if(const auto result=snapshots_.execute(context,instruction,runtime_))return result;
        if(const auto result=animations_.execute(context,instruction,runtime_,fields824_[0]!=0))return result;
        if(const auto result=rotations_.execute(context,instruction,runtime_))return result;
        if(const auto result=weather_.execute(context,instruction))return result;
        if(const auto result=presentation_.execute(context,instruction))return result;
        return visibility_.execute(context,instruction,runtime_);
    }
    void finish(bool authority) { //00427B80 supported loading/SetOn owners, real restore BEFORE exit.
        if((!loaded_&&!activation_started_)||!authority)return;
        visibility_.clear_lights();flag825_=false;
        std::exception_ptr failure;
        try{snapshots_.clear([&](auto frame){animations_.restore(frame);});}catch(...){failure=std::current_exception();}
        rotations_.clear(); //Native restores snapshot rotations BEFORE releasing84C record frame references.
        try{contexts_.finish_contexts();}catch(...){if(!failure)failure=std::current_exception();}
        try{animations_.clear();}catch(...){if(!failure)failure=std::current_exception();}
        try{cutscenes_.end();}catch(...){if(!failure)failure=std::current_exception();}
        loaded_=false;activation_started_=false;
        //Original always notifies GEffects+8C(0,false,nullptr), even with empty queues.
        try{runtime_.script_loading_finish();}catch(...){if(!failure)failure=std::current_exception();}
        if(failure)std::rethrow_exception(failure);
    }
    void unload(bool authority) { //004285D0: finish first, then authority binding retirement.
        if(loaded_||activation_started_)finish(authority);
        if(authority)bindings_.clear();
    }
private:
    frontend::ClientModuleBinding registration_;
    Frames& frames_;Files& files_;Runtime& runtime_;std::string resource_;std::uint32_t& mode_;SessionBroadcastPackets& packets_;
    std::array<std::uint32_t,5> fields824_{};std::uint32_t timer_=0; //Must outlive cutscene borrowers.
    Contexts contexts_;MissionScriptFrameCommands<Frames,Contexts> frame_commands_;Snapshots snapshots_;MissionScriptModelAnimations<Frames,Snapshots,Models,Runtime> animations_;MissionScriptRotations<Frames,Snapshots> rotations_;Visibility visibility_;MissionScriptWeatherCommands<Frames,Runtime> weather_;MissionScriptPresentationCommands<Frames,Runtime> presentation_;MissionScriptCutscenes<Frames,Contexts,Snapshots,MissionScriptModelAnimations<Frames,Snapshots,Models,Runtime>,Runtime> cutscenes_;MissionScriptVariables& variables_;MissionScriptBindings bindings_; //SAME process module4 bank, not reset by world destruction/finish.
    std::vector<MissionScriptBindings::Binding> failed_bindings_;
    bool loaded_=false,flag11_=false,flag825_=false,activation_started_=false;
};
} //namespace hd2::game
#endif
