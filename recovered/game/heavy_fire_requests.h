#ifndef HD2_GAME_HEAVY_FIRE_REQUESTS_H
#define HD2_GAME_HEAVY_FIRE_REQUESTS_H
#include "game/session_broadcast_packets.h"
#include <memory>
namespace hd2::game {
//00472E50/00472CE0 values, keeping native Human loans out of scalar words.
struct HeavyFireAirRequest {
    HumanVector position{},direction{};
    std::int32_t type=0,count=0,side=0;
    void* human=nullptr;
};
struct HeavyFireMortarRequest {
    HumanVector position{};
    std::int32_t side=0;
    void* human=nullptr;
};
class HeavyFireSupportObject {
public:
    virtual ~HeavyFireSupportObject()=default;
    virtual bool advance(bool authority,std::uint32_t elapsed)=0;
};
//Native writes module16 before the subtype and actual scalar words. Selection
//metadata/cursor are restored on an append exception; created objects/flags
//are earlier effects and are not rolled back or silently replayed.
template<class Write>void write_heavy_fire_packet(SessionBroadcastPackets& streams,Write&& write){
    auto& target=streams.broadcast_stream(0);const auto cursor=target.bit_count();
    const auto active=target.word1c,priority=target.word20,destination=target.word24,kind=target.word28;const auto remote=target.flag18;
    try{auto& packet=streams.begin(16,0);write(packet);packet.finish_broadcast();}
    catch(...){target.position(cursor/32u,static_cast<unsigned>(cursor%32u));target.word1c=active;target.word20=priority;target.word24=destination;target.word28=kind;target.flag18=remote;throw;}
}
inline void write_heavy_fire_air_packet(SessionBroadcastPackets& streams,const HeavyFireAirRequest& request){
    write_heavy_fire_packet(streams,[&](auto& packet){
        packet.write(0,1);
        for(const auto* vector:{&request.position,&request.direction})
            for(unsigned axis=0;axis!=3;++axis){std::uint32_t word;std::memcpy(&word,&(*vector)[axis],4);packet.write(word,32);}
        packet.write(static_cast<std::uint32_t>(request.type),32);packet.write(static_cast<std::uint32_t>(request.count),32);
        //473018..29 and472DD0: the retail branch is inverted. Nonnull
        //Human writes -1; NULL reads absolute address10. Preserve valid bits,
        //reject the invalid native read rather than inventing an identity.
        if(!request.human)throw std::runtime_error("HeavyFire packet native NULL Human identity read");
        packet.write(0xffffffffu,32);
    });
}
inline void write_heavy_fire_mortar_packet(SessionBroadcastPackets& streams,const HeavyFireMortarRequest& request){
    write_heavy_fire_packet(streams,[&](auto& packet){
        packet.write(1,1);
        for(unsigned axis=0;axis!=3;++axis){std::uint32_t word;std::memcpy(&word,&request.position[axis],4);packet.write(word,32);}
        //473018..29 and472DD0: the retail branch is inverted. Nonnull
        //Human writes -1; NULL reads absolute address10. Preserve valid bits,
        //reject the invalid native read rather than inventing an identity.
        if(!request.human)throw std::runtime_error("HeavyFire packet native NULL Human identity read");
        packet.write(0xffffffffu,32);
    });
}
} //namespace hd2::game
#endif
