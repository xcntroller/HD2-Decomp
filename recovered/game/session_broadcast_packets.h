#ifndef HD2_GAME_SESSION_BROADCAST_PACKETS_H
#define HD2_GAME_SESSION_BROADCAST_PACKETS_H
#include "game/session_packet_stream.h"
namespace hd2::game {
//Original208/244/280 broadcast and2BC authority-request streams. Their
//lifetime belongs to the containing0072DE00 session. Targeted2F8 transport
//remains a separate required receiver.
struct SessionBroadcastPackets {
    explicit SessionBroadcastPackets(HumanInputShared& shared):first(shared),second(shared),third(shared),request(shared){}
    SessionPacketStream& begin(std::uint32_t kind,std::int32_t priority) { //0072BEB0→0072BEF0(0,kind,true,priority).
        if(priority<0)throw std::out_of_range("Negative session packet priority");
        auto& stream=broadcast_stream(priority);
        stream.begin(kind,priority,0,true);return stream;
    }
    SessionPacketStream& broadcast_stream(std::int32_t priority){
        if(priority<0)throw std::out_of_range("Negative session packet priority");
        return priority==0?first:priority==1?second:third;
    }
    SessionPacketStream& request_stream(std::int32_t priority,std::uint32_t mode){ //0072BED0/0072BEF0, target0 and local=false.
        return mode&1u?request:broadcast_stream(priority);
    }
    bool pending()const{return first.bit_count()||second.bit_count()||third.bit_count()||request.bit_count();}
    SessionPacketStream first,second,third,request;
};

} //namespace hd2::game
#endif
