#ifndef HD2_GAME_MISSION_SCRIPT_FRAME_COMMANDS_H
#define HD2_GAME_MISSION_SCRIPT_FRAME_COMMANDS_H

#include "game/mission_script_context.h"
#include "game/simple_scene_actors.h"

namespace hd2::game {

//00416E10 ORIGINAL opcodes1/5E/66/87. This receiver uses the actual scene path
//resolver, retained pool90 and SAME EC-bound context owner. Result nullopt is
//a different command; false is the original base VM's unhandled result, which
//must still pass through actual Human/Dog derived dispatch when appropriate.
template<class Frames,class Contexts>class MissionScriptFrameCommands {
public:
    using Frame=typename Frames::Frame;
    using Context=MissionScriptContext<Frames>;
    MissionScriptFrameCommands(Frames& frames,Contexts& contexts):frames_(frames),contexts_(contexts){}
    template<class Runtime>std::optional<bool> execute(Context& context,const MissionScriptInstruction& instruction,Runtime& runtime) {
        if(!runtime.script_authority())return false;
        switch(instruction.opcode) {
        case 0x5e: {
            const auto destination=instruction.operands.at(0),name=instruction.operands.at(1);
            if(destination<context.slot_count()) {
                context.slot(destination,Frame{}); //Release old reference even if the name index is invalid.
                if(name<context.program().strings.size()) {
                    const auto text=context.program().strings[name];
                    auto frame=frames_.resolve_scene_path(text); //Actual driver+88 first.
                    if(!frame)frame=frames_.find_scene_frame(text,0xcffff); //Then actual scene+7C, same mask as native.
                    if(frame)context.slot(destination,frame);
                }
            }
            context.advance(runtime);return true;
        }
        case 0x87: {
            auto frame=context.slot(instruction.operands.at(0));
            if(!frame)return false; //Native returns false WITHOUT advancing on this exact null-frame branch.
            if(auto* target=contexts_.context_for_frame(frame))target->signal(signed_word(integer(context,instruction,1)),runtime);
            context.advance(runtime);return true; //A real frame with no context still advances.
        }
        case 0x66: {
            const auto destination=instruction.operands.at(0),name=instruction.operands.at(1);
            if(destination<context.slot_count()) {
                context.slot(destination,Frame{}); //Also before reading the parent: aliasing clears it.
                if(name<context.program().strings.size()) {
                    const auto text=context.program().strings[name];
                    const auto parent=context.slot(instruction.operands.at(2));
                    if(parent)if(const auto found=frames_.actor_find(parent,text.c_str(),0xffff))context.slot(destination,found);
                }
            }
            context.advance(runtime);return true;
        }
        case 1: {
            auto frame=context.slot(instruction.operands.at(0));
            const auto text=context.program().strings.at(instruction.operands.at(1)); //Copy before binding can retire a program.
            if(!frame&&context.frame()) {
                const auto* data=frames_.find_actor_frame_data(context.frame());
                auto* actor=data?reinterpret_cast<SceneActor<Frames>*>(data->words[0]):nullptr;
                if(actor&&actor->base().type()==10) {
                    auto* spawn=dynamic_cast<SpawnPointSceneActor<Frames>*>(actor);
                    if(!spawn)throw std::runtime_error("Script SpawnPoint EC has the wrong actual actor owner");
                    if(spawn->spawned_human)frame=spawn->spawned_human->base().frame();
                }
            }
            if(frame) {
                if(contexts_.context_for_frame(frame)==&context)throw std::runtime_error("Script self-assignment would retire the executing context");
                (void)contexts_.bind(frame,text.c_str()); //Native deliberately ignores the binding result.
            }
            context.advance(runtime);return true;
        }
        default:return std::nullopt;
        }
    }
private:
    static std::uint32_t integer(const Context& context,const MissionScriptInstruction& instruction,std::size_t n){return context.program().integers.at(instruction.operands.at(n)).value;}
    static std::int32_t signed_word(std::uint32_t word){std::int32_t result;std::memcpy(&result,&word,4);return result;}
    Frames& frames_;Contexts& contexts_;
};
} //namespace hd2::game
#endif
