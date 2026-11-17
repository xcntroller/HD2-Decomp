#ifndef HD2_GAME_MISSION_SCRIPT_PRESENTATION_COMMANDS_H
#define HD2_GAME_MISSION_SCRIPT_PRESENTATION_COMMANDS_H
#include "game/mission_script_context.h"
#include "game/session_broadcast_packets.h"
namespace hd2::game {
//00416E10 6C/73/FA/FB -> SAME module4 FIFO ->0042B650. Delayed speech
//uses SOUND94's actual duration even if the frame slot is null. No direct
//SOUND or subtitles call is substituted for the presentation packet receiver.
template<class Frames,class Runtime>class MissionScriptPresentationCommands {
public:
    MissionScriptPresentationCommands(Frames& frames,Runtime& runtime,SessionBroadcastPackets& packets)
        :frames_(frames),runtime_(runtime),packets_(packets){}
    template<class Context>std::optional<bool> execute(Context& context,const MissionScriptInstruction& instruction){
        const auto op=instruction.opcode;if(op!=0x6c&&op!=0x73&&op!=0xfa&&op!=0xfb&&op!=0xff&&op!=0x100&&op!=0x8f&&op!=0x7f)return std::nullopt;
        if(!runtime_.script_authority())return false;
        if(op==0x7f){ //0041A20C, begin/definition BEFORE slot/name; no native null-slot skip.
            auto& packet=packets_.begin(4,0);packet.write(op,8);
            packet.write(context.program().integers.at(instruction.operands.at(0)).value,8);
            const auto frame=context.slot(instruction.operands.at(1));if(!frame)throw std::runtime_error("FRM_CreateParticle reached native NULL frame-name receiver");
            packet.write_string(frames_.frame_name(frame));packet.finish_broadcast();
        }
        else if(op==0xff)runtime_.script_clear_hints();
        else if(op==0x100)runtime_.script_add_hint(context.program().integers.at(instruction.operands.at(0)).value);
        else if(op==0x8f)runtime_.script_play_music(context.program().integers.at(instruction.operands.at(0)).value);
        else if(op==0xfa||op==0xfb){
            auto& packet=packets_.begin(4,0);packet.write(op,8);
            const auto value=context.program().integers.at(instruction.operands.at(0)).value;
            packet.write(op==0xfa?std::uint32_t(value!=0):value,op==0xfa?1:32);packet.finish_broadcast();
        }else{
            if(const auto frame=context.slot(instruction.operands.at(0))){
                auto& packet=packets_.begin(4,0);packet.write(0x6c,8);packet.write_string(frames_.frame_name(frame));
                for(unsigned n=1;n!=4;++n)packet.write(context.program().integers.at(instruction.operands.at(n)).value,n==1?32:8);
                packet.finish_broadcast();
            }
            if(op==0x73){context.defer_speech_duration(runtime_.script_speech_length(context.program().integers.at(instruction.operands.at(1)).value));return true;}
        }
        context.advance(runtime_);return true;
    }
    bool receive_opcode(std::uint32_t op,SessionPacketStream& packet){
        if(op==0xfa){runtime_.script_subtitles_show(packet.read(1)!=0);return true;}
        if(op==0xfb){runtime_.script_subtitles_select(packet.read(32));return true;}
        if(op==0x7f){
            const auto definition=packet.read(8);const auto name=packet.read_string(0xff);
            if(!frames_.script_scene_available())throw std::runtime_error("Script particle decoder reached native NULL scene");
            if(const auto frame=frames_.find_scene_frame(name,0xffffu))runtime_.script_frame_particle(frame,definition);
            return true;
        }
        if(op!=0x6c)return false;
        const auto name=packet.read_string(0xff);const auto identity=packet.read(32);
        const float near_range=static_cast<float>(packet.read(8)),far_range=static_cast<float>(packet.read(8));
        if(!frames_.script_scene_available())throw std::runtime_error("Script speech decoder reached native NULL scene");
        if(const auto frame=frames_.find_scene_frame(name,0xffffu))runtime_.script_frame_speech(frame,identity,near_range,far_range);
        return true;
    }
private:
    Frames& frames_;Runtime& runtime_;SessionBroadcastPackets& packets_;
};
} //namespace hd2::game
#endif
