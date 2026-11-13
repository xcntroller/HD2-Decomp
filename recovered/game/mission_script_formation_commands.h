#ifndef HD2_GAME_MISSION_SCRIPT_FORMATION_COMMANDS_H
#define HD2_GAME_MISSION_SCRIPT_FORMATION_COMMANDS_H
#include "game/mission_script_context.h"
#include "game/scene_actor.h"
#include "game/human_joint_math.h"
namespace hd2::game {
//00416E10 original opcodes2..C, actual pool90 frameEC Human and module11.
//A failed base receiver returns false; containing Human/Dog default owns advancement.
template<class Frames,class Human,class Players,class Runtime>
std::optional<bool> execute_formation_script_command(Frames& frames,Players& players,
        MissionScriptContext<Frames>& context,const MissionScriptInstruction& instruction,Runtime& runtime){
    if(instruction.opcode<2||instruction.opcode>0xc)return std::nullopt;
    if(!runtime.script_authority())return false;
    const auto human_at=[&](std::size_t operand)->Human*{
        const auto frame=context.slot(instruction.operands.at(operand));if(!frame)return nullptr;
        const auto* data=frames.find_actor_frame_data(frame);
        auto* actor=data?reinterpret_cast<SceneActor<Frames>*>(data->words[0]):nullptr;
        if(!actor||actor->base().type()!=3)return nullptr;
        auto* human=dynamic_cast<Human*>(actor);if(!human)throw std::logic_error("Formation frameEC kind3 has a different actual owner");return human;
    };
    auto* human=human_at(0);if(!human)return false;
    if(instruction.opcode==2){(void)players.subgroup(human,true);context.advance(runtime);return true;}
    if(instruction.opcode==5){(void)players.remove_from_subgroups(human);context.advance(runtime);return true;}
    auto* leader=instruction.opcode==4?human_at(1):human;if(!leader)return false;
    auto* group=players.subgroup(leader,false);if(!group)return false;
    switch(instruction.opcode){
    case 3:(void)players.remove_subgroup(group);break;
    case 4:(void)players.remove_from_subgroups(human);(void)group->add(human);break;
    case 6:group->mode(6);break;
    case 7:group->mode(5);break;
    case 8:group->mode(7);break;
    case 9:group->mode(8);break;
    case 0xa:group->mode(9);break;
    case 0xb:group->mode(10);break;
    case 0xc:{const auto raw=context.program().integers.at(instruction.operands.at(1)).value;std::int32_t integer;std::memcpy(&integer,&raw,4);
        const float scale=static_cast<float>(static_cast<long double>(integer)*.009999999776482582f);group->scale(HumanVector{{scale,scale,scale,0}});break;}
    default:throw std::logic_error("Formation opcode dispatch changed unexpectedly");
    }
    context.advance(runtime);return true;
}
} //namespace hd2::game
#endif
