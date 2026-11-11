#ifndef HD2_GAME_HUMAN_FRAME_COMMANDS_H
#define HD2_GAME_HUMAN_FRAME_COMMANDS_H

#include "game/human_packet_commands.h"
#include "game/human_loading_stats.h"
#include "game/human_damage_commands.h"

namespace hd2::game {
template<class Packet>HumanDamageCommand read_human_damage_command(Packet& packet){return human_damage_command_value(read_human_packet_command(packet,1,7));}
template<class Packet>HumanDropCommand read_human_drop_command(Packet& packet){return human_drop_command_value(read_human_packet_command(packet,10,7));}
template<class Packet,class Damage,class Drop,class Rank>void receive_human_presentation_commands(Packet& packet,Damage&& damage,Drop&& drop,Rank&& rank){ //005406C0/005418E0 reached damage/drop/rank grammar.
    for(;;){const auto kind=packet.read(5);if(kind==24)return;
        if(kind==1)damage(read_human_damage_command(packet));
        else if(kind==10)drop(read_human_drop_command(packet));
        else if(kind==16)rank(packet.read(4)); //005B0E50→005B0EC0.
        else throw std::runtime_error("Human presentation packet requires its actual command payload receiver");
    }
}
template<class Packet,class Damage,class Drop>void receive_human_presentation_commands(Packet& packet,Damage&& damage,Drop&& drop){
    receive_human_presentation_commands(packet,std::forward<Damage>(damage),std::forward<Drop>(drop),[](std::uint32_t){throw std::runtime_error("Rank packet requires its actual presentation receiver");});
}

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
