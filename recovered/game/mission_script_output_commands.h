#ifndef HD2_GAME_MISSION_SCRIPT_OUTPUT_COMMANDS_H
#define HD2_GAME_MISSION_SCRIPT_OUTPUT_COMMANDS_H

#include "game/mission_script_context.h"
#include <optional>
#include <string>

namespace hd2::game {

//0069BFE0 in this retail client is a genuine one-byte RET. The string and
//game arguments are unused; there is no missing console/HUD service here.
inline void retail_mission_script_output(const std::string&)noexcept{}

//00416E10 original opcodes8C/B7. Obtain the actual string/integer operands,
//perform PrintfNumber's signed "%s %d" formatting, then the genuine output
//body and the ordinary00416CF0 advance gate. No diagnostic output is invented.
template<class Frames,class Runtime>std::optional<bool> execute_mission_script_output(
        MissionScriptContext<Frames>& context,const MissionScriptInstruction& instruction,Runtime& runtime){
    if(instruction.opcode!=0x8cu&&instruction.opcode!=0xb7u)return std::nullopt;
    if(!runtime.script_authority())return false;
    auto text=context.program().strings.at(instruction.operands.at(0));
    if(instruction.opcode==0xb7u){
        const auto word=context.program().integers.at(instruction.operands.at(1)).value;
        std::int32_t value;std::memcpy(&value,&word,4);text+=' ';text+=std::to_string(value);
    }
    retail_mission_script_output(text);context.advance(runtime);return true;
}

} //namespace hd2::game
#endif
