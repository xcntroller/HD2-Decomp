#ifndef HD2_GAME_MISSION_SCRIPT_PROFILE_COMMANDS_H
#define HD2_GAME_MISSION_SCRIPT_PROFILE_COMMANDS_H
#include "game/mission_script_program.h"
#include <optional>
namespace hd2::game {
//0041E3F0 predicate1A: native DWORD wrapping, then signed0..10 admission.
inline std::uint32_t mission_script_game_type(std::uint32_t mode4,std::uint32_t mode8){
    const auto value=mode8+mode4*4u;return value<11u?value:0xffffffffu;
}
//00416E10 opcode89 ->00419D64: read integer BEFORE application20 presence.
//Repeat the actual application getter before selecting profile94/B8. A missing
//profile skips mutation and reaches the SAME guarded00416CF0 tail.
template<class Context,class Application,class Runtime>
std::optional<bool> execute_mission_script_profile_command(Context& context,const MissionScriptInstruction& instruction,
        Application& application,Runtime& runtime){
    if(instruction.opcode!=0x89u)return std::nullopt;
    if(!runtime.script_authority())return false;
    const auto value=context.program().integers.at(instruction.operands.at(0)).value;
    if(application.script_profile_owner()){
        auto* profile=application.script_profile_owner();
        if(!profile)throw std::logic_error("AddDiaryText lost its actual application profile owner");
        const auto root=application.script_profile_root();
        profile->progress.at(root==1u?1u:0u).append(value);
    }
    context.advance(runtime);return true;
}
} //namespace hd2::game
#endif
