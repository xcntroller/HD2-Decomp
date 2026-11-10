#ifndef HD2_GAME_SESSION_PACKET_WORLD_H
#define HD2_GAME_SESSION_PACKET_WORLD_H
#include "game/session_packet_stream.h"
#include "game/world_loading_dependency_link.h"
namespace hd2::game {
//Source binding of native current game00698500 to actual0048DC50/70 packet
//receivers. Queue/stream owners remain process/control-lived across world exit.
class SessionPacketWorld {
public:
    virtual ~SessionPacketWorld()=default;
    virtual bool world_loaded()const=0;
    virtual bool dispatch_packet(std::uint32_t,SessionPacketStream&,bool)=0;
};
class SessionPacketWorldLink {
public:
    void bind(SessionPacketWorld& world){world_.bind(world);}
    void unbind(SessionPacketWorld& world)noexcept{world_.unbind(world);}
    bool bound()const noexcept{return world_.bound();}
    bool world_loaded()const{return world_.bound()&&world_.get().world_loaded();} //Native absent/unloaded game is genuinelyfalse.
    bool dispatch_packet(std::uint32_t kind,SessionPacketStream& packet,bool authority){
        return world_.bound()&&world_.get().dispatch_packet(kind,packet,authority);
    }
private:
    WorldLoadingDependencyLink<SessionPacketWorld> world_;
};
} //namespace hd2::game
#endif
