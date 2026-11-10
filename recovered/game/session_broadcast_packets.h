#ifndef HD2_GAME_SESSION_BROADCAST_PACKETS_H
#define HD2_GAME_SESSION_BROADCAST_PACKETS_H
#include "game/session_packet_stream.h"
namespace hd2::game {
//The three original208/244/280 broadcast streams. Their lifetime belongs to
//the containing0072DE00 session, not a transient spawn callback. Targeted and
//request streams2BC/2F8 are separate required transport work.
struct SessionBroadcastPackets {
    explicit SessionBroadcastPackets(HumanInputShared& shared):first(shared),second(shared),third(shared){}
    SessionPacketStream& begin(std::uint32_t kind,std::int32_t priority) { //0072BEB0→0072BEF0(0,kind,true,priority).
        if(priority<0)throw std::out_of_range("Negative session packet priority");
        auto& stream=priority==0?first:priority==1?second:third;
        stream.begin(kind,priority,0,true);return stream;
    }
    bool pending()const{return first.bit_count()||second.bit_count()||third.bit_count();}
    SessionPacketStream first,second,third;
};

} //namespace hd2::game
#endif
