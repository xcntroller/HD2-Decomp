#ifndef HD2_GAME_RUNTIME_ACTOR_PACKETS_H
#define HD2_GAME_RUNTIME_ACTOR_PACKETS_H
#include "game/session_packet_stream.h"
namespace hd2::game {
//004787E0→00478CC0→00478880.0072BEB0 selects the SAME process broadcast
//stream. Priority3 selects the third stream and is clamped to2 in its header.
template<class Packets>auto& begin_runtime_actor_command(Packets& packets,std::uint32_t command,bool priority){
    auto& packet=packets.begin(12,priority?0:3);packet.write(1,2);packet.write(command,2);return packet;
}
template<class Packets>void publish_runtime_mine(Packets& packets,std::int32_t identity,std::int32_t human,
        std::int32_t definition,const HumanVector& position,const HumanVector& direction){
    auto& packet=begin_runtime_actor_command(packets,0,true);
    packet.write(static_cast<std::uint32_t>(identity),16);packet.write(static_cast<std::uint32_t>(human),16);
    packet.write(static_cast<std::uint32_t>(definition),16);
    (void)packet.write_vector(position,packet.primary_quantizer(-1),7);packet.write_direction(direction,-1);
    packet.finish_broadcast();
}
inline std::int32_t runtime_packet_signed16(std::uint32_t word){
    const auto low=word&0xffffu;return low&0x8000u?static_cast<std::int32_t>(low)-0x10000:static_cast<std::int32_t>(low);
}
template<class Runtime>bool receive_runtime_actor_packet(Runtime& runtime,SessionPacketStream& packet,bool authority){
    if(authority){ //004788A0: missing token returnsfalse, no actor payload discard.
        auto* actor=runtime.identity(static_cast<std::int32_t>(packet.read(10)));
        if(!actor)return false;
        runtime.receive_runtime_authority(*actor,packet);return true;
    }
    const auto selector=packet.read(2); //004788E0. Other selectors genuinely consume no more.
    if(selector==0){
        if(auto* actor=runtime.identity(static_cast<std::int32_t>(packet.read(10))))runtime.receive_runtime_presentation(*actor,packet);
    }else if(selector==1&&packet.read(2)==0){
        const auto identity=runtime_packet_signed16(packet.read(16));
        const auto human=runtime_packet_signed16(packet.read(16));
        const auto definition=runtime_packet_signed16(packet.read(16));
        const auto position=packet.read_vector(packet.primary_quantizer(-1),7);
        const auto direction=packet.read_direction(-1);
        (void)runtime.present_mine(identity,human,definition,position,direction);
    }
    return true;
}
}
#endif
