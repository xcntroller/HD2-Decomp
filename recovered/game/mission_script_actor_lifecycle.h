#ifndef HD2_GAME_MISSION_SCRIPT_ACTOR_LIFECYCLE_H
#define HD2_GAME_MISSION_SCRIPT_ACTOR_LIFECYCLE_H

#include "game/mission_script_context.h"
#include "game/model_scene_actors.h"

namespace hd2::game {

//Actual00416DB0 Human cleanup AFTER MissionScriptContext stores104=0/D4=1,
//plus00416E10 F9 EndScript. Frame EC actor0 is independent of context family.
//Controls is the SAME HumanScriptLoadingControls used by derived commands.
template<class Frames,class Human,class Controls,class Players>class MissionScriptActorLifecycle {
public:
    using Context=MissionScriptContext<Frames>;using Actor=SceneActor<Frames>;
    MissionScriptActorLifecycle(Frames& frames,Controls& controls,Players& players):frames_(frames),controls_(controls),players_(players){}
    void return_end_human(Context& context,bool authority) { //00416DB0, no health gate.
        if(!authority)return;
        auto* value=actor(context);if(!value||value->base().type()!=3)return;
        auto& human=as_human(*value);
        if(!human.loading_state)throw std::runtime_error("Script ReturnEnd requires its actual Human B0");
        if(human.loading_state->words().template get<std::uint32_t>(0x48)==5){controls_.working_mode(human,0);controls_.suspend(human,false);}
    }
    template<class Runtime>std::optional<bool> execute(Context& context,const MissionScriptInstruction& instruction,Runtime& runtime){
        if(instruction.opcode!=0xf9u)return std::nullopt;
        if(!runtime.script_authority())return false;
        context.begin_end_script();
        if(auto* value=actor(context)){
            if(value->base().type()==3){
                auto& human=as_human(*value);
                if(positive(human.properties.stats_ec[0])){
                    controls_.working_mode(human,0);controls_.suspend(human,false);controls_.end_script(human);
                    if(human.field160&&human.field160->base().type()==0x22&&players_.current_player()==&human)
                        throw std::runtime_error("Attached current-player EndScript requires its actual0045DA40 receiver");
                }
            }else if(value->base().type()==30){
                const auto* dog=dynamic_cast<DogSceneActor<Frames>*>(value);
                if(!dog)throw std::logic_error("EndScript Dog EC has the wrong actual actor family");
                if(positive(dog->properties.fields4a0[0]))
                    throw std::runtime_error("Live Dog EndScript requires actual005D5870/0041D790 control receivers");
            }
        }
        context.advance_end_script();return true;
    }
private:
    Actor* actor(Context& context){
        const auto* data=context.frame()?frames_.find_actor_frame_data(context.frame()):nullptr;
        return data?reinterpret_cast<Actor*>(data->words[0]):nullptr;
    }
    static Human& as_human(Actor& actor){auto* human=dynamic_cast<Human*>(&actor);if(!human)throw std::logic_error("Script lifecycle Human EC has the wrong actual actor family");return *human;}
    static bool positive(std::uint32_t value){std::int32_t signed_value;std::memcpy(&signed_value,&value,4);return signed_value>0;}
    Frames& frames_;Controls& controls_;Players& players_;
};

} //namespace hd2::game
#endif
