#ifndef HD2_GAME_DOOR_LOADING_PACKETS_H
#define HD2_GAME_DOOR_LOADING_PACKETS_H

#include "game/prepared_spawn_packets.h"

namespace hd2::game {
//00601F40→00475530/004755B0. Door commands use ceil(log2(7))=3 bits,
//following the actual module13 and physical Scene2 identity headers. They stay
//pending in the same broadcast stream until the normal local FIFO flush.
inline void write_door_loading_command(SessionBroadcastPackets& streams,std::int32_t identity,std::uint32_t command) {
    auto& packet=streams.begin(13,0);packet.write(static_cast<std::uint32_t>(identity),10);
    packet.write(command,3);packet.finish_broadcast();
}
} //namespace hd2::game
#endif
