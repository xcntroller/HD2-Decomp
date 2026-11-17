#ifndef HD2_GAME_SESSION_PEER_STATISTICS_WIRE_H
#define HD2_GAME_SESSION_PEER_STATISTICS_WIRE_H
#include "game/human_state_frame_timers.h"
namespace hd2::game {
//00749A70,46 bits from native relative0C/04/08/10/14. Ping10 clamps by
//MUTATING the original owner before writing; negative scores add2048 once.
template<class Bits,class Read,class Write>void write_session_peer_statistics(Bits& bits,Read&& read,Write&& write){
    auto score=read(3);if(human_frame_signed_word(score)<0)score+=2048u;
    bits.write(score,11);bits.write(read(1),10);bits.write(read(2),10);
    const auto ping=human_frame_signed_word(read(4));if(ping<0||ping>1023)write(4,1023u);
    bits.write(read(4),10);bits.write(read(5),5);
}
}
#endif
