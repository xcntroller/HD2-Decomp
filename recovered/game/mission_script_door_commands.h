#ifndef HD2_GAME_MISSION_SCRIPT_DOOR_COMMANDS_H
#define HD2_GAME_MISSION_SCRIPT_DOOR_COMMANDS_H

#include "game/geometry_scene_actors.h"
#include "game/mission_script_program.h"

namespace hd2::game {
//00416E10 opcode9A -> dispatch41/00417992. Resolve actual pool90 frame/EC
//actor0 BEFORE integerA0. Missing/wrong actor skips the mutation but reaches
//the SAME guarded00416CF0 tail at0041C8C6. Actual Door dispatch ignores the
//key receiver's bool; only an unknown opcode returns nullopt.
template<class Frames,class Context,class Runtime>
std::optional<bool> execute_mission_script_door_command(Frames& frames,Context& context,
        const MissionScriptInstruction& instruction,SessionBroadcastPackets& packets,Runtime& runtime){
    if(instruction.opcode!=0x9au)return std::nullopt;
    if(!runtime.script_authority())return false;
    const auto frame=context.slot(instruction.operands.at(0));
    const auto* data=frame?frames.find_actor_frame_data(frame):nullptr;
    auto* actor=data?reinterpret_cast<SceneActor<Frames>*>(data->words[0]):nullptr;
    if(actor&&actor->base().type()==6){
        auto* door=dynamic_cast<DoorSceneActor<Frames>*>(actor);
        if(!door)throw std::logic_error("Script DOOR_Lock requires its actual Door actor");
        const bool lock=context.program().integers.at(instruction.operands.at(1)).value!=0;
        (void)door->key_lock(door->properties.field230,lock,packets);
    }
    context.advance(runtime);return true;
}
} //namespace hd2::game
#endif
