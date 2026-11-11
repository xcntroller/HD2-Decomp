#ifndef HD2_GAME_MISSION_SCRIPT_WORLD_LOADING_H
#define HD2_GAME_MISSION_SCRIPT_WORLD_LOADING_H

#include "game/mission_script_loading_runtime.h"
#include "game/mission_script_actor_bindings.h"
#include "game/mission_script_player_predicates.h"
#include "game/mission_script_model_visibility.h"
#include "game/mission_script_world_link.h"
#include "game/human_objective_experience.h"

namespace hd2::game {

//One containing source lifetime for the SAME Human and VM world receivers.
//HumanComposition can use MissionScriptWorldLink as both context lookup and
//authority provider, so no successful VM declaration or duplicate context owner
//is needed while forming the mutually borrowing types. Close this module before
//retiring Humans; all borrowed dependencies and the link outlive this owner.
//Construction alone supplies no guard observations, loading or first frame.
template<class HumanComposition,class FileBackend,class Lookup,class Players,class FirstPlayers,
         class Sight,class Hearing,class View,class Camera,class Effects,class Random,
         class Hint,class Collisions,class Clock>
class MissionScriptWorldLoading {
public:
    using Frames=typename HumanComposition::Frames;using Human=typename HumanComposition::Human;
    using Link=MissionScriptWorldLink<Frames>;
    using Bindings=MissionScriptActorBindings<HumanComposition,Lookup,typename Players::Control>;
    using Controls=HumanScriptLoadingControls<Frames,Human,Lookup,Sight,Hearing,FirstPlayers,View,Lookup>;
    using Lifecycle=MissionScriptActorLifecycle<Frames,Human,Controls,typename Players::Control>;
    using Views=MissionScriptViewCommands<Frames,Human,Camera,View,Effects>;
    using Predicates=MissionScriptPlayerPredicates<Frames,Players>;
    using Objectives=MissionObjectiveStatus<Random,typename Players::Control,HumanObjectiveExperience,Hint,Lookup>;
    using Animation=AnimationActorVisibility<Frames,Collisions>;
    using Models=MissionScriptModelVisibility<Frames,Human,Effects,Lookup,Animation,Players,View>;
    struct Dependencies{
        using Frames=typename HumanComposition::Frames;using Files=FileBackend;
        using Models=typename MissionScriptWorldLoading::Models;using Human=typename HumanComposition::Human;
        std::uint32_t& mode;bool& advance_override;SessionActivationState& session;Clock& clock;
        Bindings& bindings;Views& views;Objectives& objectives;Lifecycle& lifecycle;Controls& controls;
        Predicates& predicates;Lookup& world;Effects& effects;
    };
    using Composition=MissionScriptLoadingComposition<Dependencies>;
    MissionScriptWorldLoading(frontend::ClientModuleRegistry& registry,frontend::ClientModuleRegistration& registration,
            Frames& frames,FileBackend& files,
            Lookup& lookup,Players& players,FirstPlayers& first_players,Sight& sight,Hearing& hearing,
            View& view,Camera& camera,Effects& effects,Random& random,Hint& hint,Collisions& collisions,
            Clock& clock,SessionActivationState& session,MissionIntegrityTables& guard,MissionScriptVariables& variables,
            SessionBroadcastPackets& packets,std::uint32_t& mode,bool& advance_override,
            std::string resource,Link& link)
        :frames_(frames),link_(link),bindings_(frames,lookup,players.control()),
         controls_(frames,lookup,sight,hearing,first_players,view,lookup),
         lifecycle_(frames,controls_,players.control()),views_(frames,camera,view,effects,session,mode),
         predicates_(frames,players),experience_(mode),objectives_(session,mode,guard,random,players.control(),experience_,hint,lookup),
         animation_(frames,collisions),models_(frames,effects,lookup,animation_,players,view,mode),
         script_(registry,frames,files,models_,packets,mode,std::move(resource),variables,
             Dependencies{mode,advance_override,session,clock,bindings_,views_,objectives_,lifecycle_,controls_,predicates_,lookup,effects},registration){
        link_.bind(script_.module());
    }
    ~MissionScriptWorldLoading(){close();}
    MissionScriptWorldLoading(const MissionScriptWorldLoading&)=delete;
    MissionScriptWorldLoading& operator=(const MissionScriptWorldLoading&)=delete;
    void close()noexcept{
        if(closed_)return;
        closed_=true;
        try{script_.module().finish(script_.runtime().script_authority());}
        catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
        link_.unbind(script_.module()); //Finish contexts while Human callbacks can still consult this actual module.
    }
    auto& module(){if(closed_)throw std::logic_error("Script world owner is closed");return script_.module();}
    auto& runtime(){if(closed_)throw std::logic_error("Script world owner is closed");return script_.runtime();}
private:
    Frames& frames_;Link& link_;Bindings bindings_;Controls controls_;Lifecycle lifecycle_;Views views_;
    Predicates predicates_;HumanObjectiveExperience experience_;Objectives objectives_;
    Animation animation_;Models models_;Composition script_;bool closed_=false;
};
} //namespace hd2::game
#endif
