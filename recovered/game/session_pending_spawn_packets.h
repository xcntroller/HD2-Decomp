#ifndef HD2_GAME_SESSION_PENDING_SPAWN_PACKETS_H
#define HD2_GAME_SESSION_PENDING_SPAWN_PACKETS_H
#include "game/prepared_spawn_packets.h"
#include "game/session_peer_records.h"
#include "game/session_pending_spawn.h"
namespace hd2::game {
template<class Packet,class Locks>
void write_session_pending_spawn(Packet& packet,SessionPendingSpawn& pending,frontend::ControlStartupState& records,Locks& locks){ //0072A0B0 case3.
    if(pending.serialized||!pending.soldier||!pending.stats)throw std::logic_error("Spawn command has no live original transferred soldier/stat allocations");
    const auto side=static_cast<std::uint32_t>(pending.soldier->specialization);
    set_session_peer_side(records,locks,pending.peer,side,false);set_session_peer_side(records,locks,pending.peer,side,true);
    packet.write(pending.peer,32);(void)packet.write_vector(pending.position,packet.primary_quantizer(-1),7);
    packet.write_direction(pending.direction,-1);write_prepared_soldier(packet,*pending.soldier,false);
    const auto& stats=*pending.stats;packet.write(stats[0],10);
    for(auto index:{2u,1u,3u,4u,5u,6u,7u,8u,9u,11u,10u})packet.write(stats[index],7);
    pending.soldier.reset();pending.stats.reset();pending.serialized=true; //Native frees BEFORE finalization; failed finalize cannot safely replay freed input.
}
}
#endif
