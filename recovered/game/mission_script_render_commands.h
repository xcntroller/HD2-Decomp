#ifndef HD2_GAME_MISSION_SCRIPT_RENDER_COMMANDS_H
#define HD2_GAME_MISSION_SCRIPT_RENDER_COMMANDS_H
#include "game/mission_script_program.h"
#include <optional>
namespace hd2::game {
//00416E10 opcodeAD (dispatch byte53), CSC_RunCLouder. Resolve SAME integer
//pool before GEffectsB8, then00416CF0. A configuration failure must not advance.
//Also original opcodeBB, THUNDERSTORM_SetOn, dispatch5F/00418576;
//A0/A1 CSC_FadeIn/Out, dispatch47/48 -> SAME GEffects8C fade owner.
template<class Context,class Effects,class Runtime>
std::optional<bool> execute_mission_render_command(Context& context,const MissionScriptInstruction& instruction,
        Effects& effects,Runtime& runtime){
    if(instruction.opcode!=0xadu&&instruction.opcode!=0xbbu&&
       instruction.opcode!=0xa0u&&instruction.opcode!=0xa1u)return std::nullopt;
    if(!runtime.script_authority())return false;
    const auto value=context.program().integers.at(instruction.operands.at(0)).value;
    if(instruction.opcode==0xa0u||instruction.opcode==0xa1u){ //0041BFD1/0041BFF7 ->0041C8BC.
        std::int32_t duration;std::memcpy(&duration,&value,sizeof(duration));
        effects.fade().configure(duration,instruction.opcode==0xa1u,nullptr);
        context.advance(runtime);return true;
    }
    const bool enabled=value!=0;
    if(instruction.opcode==0xadu)effects.configure_procedural(enabled);
    else{
        auto frame=context.slot(instruction.operands.at(1));
        if(frame&&effects.frames().frame_kind(frame)!=2)frame={};
        effects.configure_special_model(enabled,frame,false);
    }
    context.advance(runtime);return true;
}
}
#endif
