#ifndef HD2_GAME_MISSION_SCRIPT_VARIABLES_H
#define HD2_GAME_MISSION_SCRIPT_VARIABLES_H

#include "game/mission_script_program.h"
#include <optional>

namespace hd2::game {

//Module4's256 DWORDs at native24 and their previous snapshot424. Original
//00428C80 copies CURRENT→PREVIOUS, not the reverse. The containing004270D0
//passes the actual configuration+4 kind after creating/beginning contexts.
class MissionScriptVariables {
public:
    void activate(std::uint32_t configuration_kind) {
        if(configuration_kind==1)current_.fill(0);
        else previous_=current_; //00428C80 backwards copy has the same nonoverlapping value result.
    }
    const auto& current()const{return current_;}const auto& previous()const{return previous_;}
    template<class Context,class Runtime>std::optional<bool> execute(Context& context,const MissionScriptInstruction& instruction,Runtime& runtime) {
        if(instruction.opcode!=0x8d)return std::nullopt; //00416E10 case34, SaveGameValue.
        if(!runtime.script_authority())return false;
        const auto index=context.program().integers.at(instruction.operands.at(0)).value;
        const auto value=context.program().integers.at(instruction.operands.at(1)).value;
        if(index<current_.size())current_[index]=value;
        context.advance(runtime);return true;
    }
    std::optional<std::uint32_t> evaluate(std::uint32_t opcode,const std::array<std::uint32_t,4>& values,
                                         const MissionScriptProgram& program,bool authority)const {
        if(!authority)return 0;
        if(opcode!=0xc)return std::nullopt; //0041E3F0 case0C, _LoadGameValue.
        const auto index=program.integers.at(values[0]).value;
        return index<current_.size()?current_[index]:0u;
    }
private:
    std::array<std::uint32_t,256> current_{},previous_{};
};
} //namespace hd2::game
#endif
