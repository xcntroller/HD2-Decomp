#ifndef HD2_GAME_HUMAN_FRAME_COMMANDS_H
#define HD2_GAME_HUMAN_FRAME_COMMANDS_H

#include "game/session_broadcast_packets.h"

namespace hd2::game {

//00577B30/Player0053F370's actual kind6 producer/005B0F90 serializer. Remaining command
//payload families require their own recovered producers and serializers.
template<class Commands>
void write_human_frame_commands(SessionBroadcastPackets& streams,std::uint32_t mode,std::int32_t identity,
        bool second,const Commands& commands){
    if(commands.empty())return;
    for(const auto& command:commands)if(command.kind!=6)
        throw std::runtime_error("Human frame command requires its actual payload serializer");
    auto& stream=second?streams.broadcast_stream(0):streams.request_stream(0,mode);
    const auto before=stream.bit_count();
    const auto active=stream.word1c,priority=stream.word20,target=stream.word24,kind=stream.word28;
    const bool request=stream.flag18;
    try{
        stream.begin(13,0,0,second);stream.write(static_cast<std::uint32_t>(identity),10);
        for(const auto& command:commands){stream.write(6,5);stream.write(static_cast<std::uint32_t>(command.actor_identity),10);}
        stream.write(24,5);stream.finish_broadcast();
    }catch(...){
        stream.position(before/32,static_cast<unsigned>(before%32));
        stream.word1c=active;stream.word20=priority;stream.word24=target;stream.word28=kind;stream.flag18=request;
        throw;
    }
}

//00540DB0 case6, reached through actual Scene2 authority dispatch. Reserved
//missing/nonplayer owners use00541CB0 and still consume the actor identity.
template<class Packet,class Request>
void receive_human_ladder_commands(Packet& packet,Request& request){
    for(;;){
        const auto kind=packet.read(5);
        if(kind==24)return;
        if(kind!=6)throw std::runtime_error("Human authority packet requires its actual command payload receiver");
        request(packet.read(10));
    }
}

} //namespace hd2::game
#endif
