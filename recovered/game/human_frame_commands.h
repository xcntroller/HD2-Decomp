#ifndef HD2_GAME_HUMAN_FRAME_COMMANDS_H
#define HD2_GAME_HUMAN_FRAME_COMMANDS_H

#include "game/human_packet_commands.h"

namespace hd2::game {

//00577B30/Player0053F370: same streams and one atomic source append.
//Prepare rereads actual referenced inventory at serialization time for13/20.
template<class Commands,class Prepare>
void write_human_frame_commands(SessionBroadcastPackets& streams,std::uint32_t mode,std::int32_t identity,
        bool second,const Commands& commands,Prepare&& prepare){
    if(commands.empty())return;
    auto& stream=second?streams.broadcast_stream(0):streams.request_stream(0,mode);
    const auto before=stream.bit_count();
    const auto active=stream.word1c,priority=stream.word20,target=stream.word24,kind=stream.word28;
    const bool request=stream.flag18;
    try{
        stream.begin(13,0,0,second);stream.write(static_cast<std::uint32_t>(identity),10);
        for(const auto& command:commands)write_human_packet_command(stream,prepare(command),mode);
        stream.write(24,5);stream.finish_broadcast();
    }catch(...){
        stream.position(before/32,static_cast<unsigned>(before%32));
        stream.word1c=active;stream.word20=priority;stream.word24=target;stream.word28=kind;stream.flag18=request;
        throw;
    }
}

template<class Commands>
void write_human_frame_commands(SessionBroadcastPackets& streams,std::uint32_t mode,std::int32_t identity,
        bool second,const Commands& commands){
    write_human_frame_commands(streams,mode,identity,second,commands,[](const HumanPacketCommand& value)->const auto&{return value;});
}

} //namespace hd2::game
#endif
