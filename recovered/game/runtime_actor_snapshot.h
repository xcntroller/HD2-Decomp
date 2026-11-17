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
//004790C0 owner=-1 branch, using SAME Items and Runtime factory owners.
template<class Frames,class Registry,class Items>auto present_runtime_snapshot_loose_item(Frames& frames,Registry& registry,Items& items,
        std::int32_t identity,std::int32_t definition,std::int32_t item_identity,const HumanVector& position,const HumanVector& direction)
        ->decltype(registry.identity(identity)){
    if(registry.mode_flags()&1u)return registry.identity(identity);
    const auto entry=items.create_unheld_item(definition,item_identity);
    if(!entry)throw std::runtime_error("Snapshot loose-item definition produced no actual item");
    auto& item=items.runtime_item(entry);item.flag1c=true;const auto model=item.model();if(!model)return nullptr;
    frames.human_loose_item_pose(model,position,direction);auto* actor=registry.create_loose_item(model,identity);
    if(!actor)throw std::runtime_error("Snapshot loose-item factory produced no actual owner");
    actor->bind_item(entry,{{0,-.1f,0,0}});return actor; //Native W is unwritten, no padding value claim.
}
//00478520 consumes type/id for every row; only2/11 construct actors and35
//finds a retained owner. Other native types genuinely have no following data.
template<class Registry,class Queries>void read_runtime_actor_snapshot(Registry& registry,SessionPacketStream& packet,Queries& q){
    const auto signed16=[&]{const auto value=packet.read(16);return value&0x8000u?static_cast<std::int32_t>(value)-65536:static_cast<std::int32_t>(value);};
    registry.receive_next_identity(signed16());
    for(auto type=packet.read(6);type;type=packet.read(6)){
        const auto identity=signed16();
        if(type==2||type==11){
            const auto first=signed16(),second=signed16();
            const auto position=packet.read_vector(packet.primary_quantizer(-1),7),direction=packet.read_direction(-1);
            auto* actor=type==2?q.present_mine(identity,first,second,position,direction):q.present_snapshot_loose_item(identity,first,second,position,direction);
            if(!actor)throw std::runtime_error("Runtime snapshot factory returned no actual actor for virtual54");
            q.receive_runtime_snapshot_tail(*actor,packet);
        }else if(type==35){
            auto* actor=registry.identity(identity);
            if(!actor)throw std::runtime_error("Runtime snapshot lost its actual retained identity for virtual54");
            q.receive_runtime_snapshot_tail(*actor,packet);
        }
    }
}
}
#endif
