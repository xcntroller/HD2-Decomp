#ifndef HD2_GAME_MISSION_SCRIPT_ACTOR_BINDINGS_H
#define HD2_GAME_MISSION_SCRIPT_ACTOR_BINDINGS_H

#include "game/mission_script_actor_context.h"
#include "game/model_scene_actors.h"

namespace hd2::game {

//00427730's notifications surround compilation, rather than being an opaque
//part of context construction. Lookup owns the actual Human compositions and
//Players is their SAME module11 control. Cold binding before Human activation
//genuinely skips005A3DF0/005907C0 because B0 is still null.
template<class Composition,class Lookup,class Players>class MissionScriptActorBindings {
public:
    using Frames=typename Composition::Frames;using Human=typename Composition::Human;
    using Context=MissionScriptContext<Frames>;using Actor=SceneActor<Frames>;
    struct Target {Actor* actor=nullptr;bool notify_human=false,dog=false;};
    MissionScriptActorBindings(Frames& frames,Lookup& lookup,Players& players)
        :frames_(frames),lookup_(lookup),players_(players){}
    std::unique_ptr<Context> make_script_context(Frames& frames,typename Frames::Frame frame){
        if(&frames!=&frames_)throw std::logic_error("Script context factory requires the SAME actor frame registry");
        return make_mission_script_context(frames_,frame);
    }
    Target prepare_script_binding(Context& context){
        if(context.family()==1){
            auto* derived=dynamic_cast<ActorMissionScriptContext<Frames,1>*>(&context);
            auto* human=derived?dynamic_cast<Human*>(&derived->actor()):nullptr;
            if(!human)throw std::logic_error("Human script context requires its actual Human actor");
            if((human->properties.field19c==1||human->properties.field19c==4)&&human->loading_state){
                auto& owner=lookup_.template human_loading_owner<Composition>(*human);
                owner.states().select_script(true,owner.control(),players_,lookup_.mode_flags(),lookup_.human_current_player_id());
            }
            return {human,human->loading_state&&human->loading_state->words().template get<std::uint32_t>(0x48)==5,false};
        }
        if(context.family()==2){
            auto* derived=dynamic_cast<ActorMissionScriptContext<Frames,2>*>(&context);
            if(!derived||!dynamic_cast<DogSceneActor<Frames>*>(&derived->actor()))
                throw std::logic_error("Dog script context requires its actual Dog actor");
            return {&derived->actor(),false,true};
        }
        if(context.family()!=0)throw std::runtime_error("Script context has no original family");
        return {};
    }
    void script_binding_completed(const Target& target,Context* context){
        if(target.notify_human){
            auto* human=dynamic_cast<Human*>(target.actor);
            if(!human||!human->loading_state)throw std::runtime_error("Human script completion lost its actual B0 owner");
            auto& owner=lookup_.template human_loading_owner<Composition>(*human);
            auto* state=owner.states().get();
            if(!state)throw std::runtime_error("Human script completion requires current state5");
            owner.state_factory().script_context(*state,context); //005907C0; rereads current B0 after compilation.
        }else if(target.dog){
            auto* dog=dynamic_cast<DogSceneActor<Frames>*>(target.actor);
            if(!dog)throw std::logic_error("Dog script completion lost its actual actor");
            if(dog->field534)throw std::runtime_error("Dog script binding requires actual534 control virtual40");
        }
    }
private:
    Frames& frames_;Lookup& lookup_;Players& players_;
};

} //namespace hd2::game
#endif
