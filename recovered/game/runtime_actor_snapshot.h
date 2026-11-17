#ifndef HD2_GAME_RUNTIME_ACTOR_SNAPSHOT_H
#define HD2_GAME_RUNTIME_ACTOR_SNAPSHOT_H
#include "game/session_packet_stream.h"
namespace hd2::game {
//004782C0: SAME runtime ownership order/70 counter; fresh row access after
//every writer. Type2/11 virtual50 are genuine RET4;35 alone adds motion/yaw.
template<class Registry,class Queries>void write_runtime_actor_snapshot(Registry& registry,SessionPacketStream& packet,Queries& q){
    packet.write(static_cast<std::uint32_t>(registry.next_identity()),16);
    for(std::size_t n=0;n<registry.size();++n){
        packet.write(registry.owned_at(n)->base().type(),6); //DOUBLE39 -> trunc(log2(39)+1)=6.
        packet.write(static_cast<std::uint32_t>(registry.owned_at(n)->base().id10()),16);
        const auto kind=registry.owned_at(n)->base().type();
        if(kind==2){
            packet.write(static_cast<std::uint32_t>(q.snapshot_mine_human_identity(n)),16);
            packet.write(static_cast<std::uint32_t>(q.snapshot_mine_definition(n)),16);
        }else if(kind==11){
            packet.write(q.snapshot_item_definition(n),16);
            packet.write(static_cast<std::uint32_t>(q.snapshot_item_identity(n)),16);
        }else if(kind==35){q.snapshot_placed_motion(n,packet);continue;}
        else continue; //Original writes ONLY type/id for all other actor kinds.
        const auto position=registry.owned_at(n)->base().world_position();
        (void)packet.write_vector(position,packet.primary_quantizer(-1),7);
        const auto direction=registry.owned_at(n)->base().world_direction();
        packet.write_direction(direction,-1);
        //Actual type2:005B8880 and type11:005B70F0, both C2 04 00. No payload.
    }
    packet.write(0,6); //Terminal type has NO following identity.
}
}
#endif
