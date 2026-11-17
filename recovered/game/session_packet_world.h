#ifndef HD2_GAME_SESSION_PACKET_WORLD_H
#define HD2_GAME_SESSION_PACKET_WORLD_H
#include "game/session_packet_stream.h"
#include "game/session_pending_spawn.h"
#include "game/world_loading_dependency_link.h"
#include "game/human_input_callbacks.h"
namespace hd2::game {
//Source binding of native current game00698500 to actual0048DC50/70 packet
//receivers. Queue/stream owners remain process/control-lived across world exit.
class SessionRoundResetReceiver {
public:
    virtual ~SessionRoundResetReceiver()=default;
    virtual const void* round_reset_world_identity()const noexcept=0;
    virtual void reset_round(bool)=0;
};
class SessionRoundResetLink {
public:
    explicit SessionRoundResetLink(const void* world):world_(world){if(!world_)throw std::invalid_argument("Round reset requires actual World identity");}
    void bind(SessionRoundResetReceiver& receiver){
        if(receiver.round_reset_world_identity()!=world_)throw std::logic_error("FRAME round reset borrower belongs to a different World");
        receiver_.bind(receiver);
    }
    void unbind(SessionRoundResetReceiver& receiver)noexcept{receiver_.unbind(receiver);}
    void reset(){receiver_.get().reset_round(true);}
private:
    const void* world_;WorldLoadingDependencyLink<SessionRoundResetReceiver> receiver_;
};
class SessionPacketWorld {
public:
    virtual ~SessionPacketWorld()=default;
    virtual bool world_loaded()const=0;
    virtual bool dispatch_packet(std::uint32_t,SessionPacketStream&,bool)=0;
    virtual void reset_round_from_packet()=0;
    virtual void read_application_world_snapshot(SessionPacketStream&)=0;
    virtual void receive_application_statistics_clock(std::uint32_t,std::uint32_t,std::uint32_t)=0;
    virtual void receive_application_respawn(std::uint32_t,const HumanVector&,const HumanVector&,frontend::PreMissionSoldier&,const std::array<std::uint32_t,12>&)=0;
    virtual void admit_application_spawn(std::uint32_t,frontend::PreMissionSoldier&,const std::array<std::uint32_t,12>*)=0;
    virtual void receive_application_objective(std::uint32_t,std::uint32_t)=0;
    virtual void receive_application_peer_departure(std::uint32_t)=0;
    virtual void receive_application_local_removal(std::uint32_t)=0;
    virtual void receive_application_spawn_rejection(std::uint32_t)=0;
    virtual void append_application_message(const std::string&)=0;
    virtual HumanInputCallbacks* input_default_receiver(std::int32_t,bool)=0;
    virtual std::uint32_t frame_statistics_word(unsigned)const=0; //Actual current game30/28/84, never a copied clock.
    virtual void write_application_world_snapshot(SessionPacketStream&)=0; //Actual004782C0 runtime owner, distinct from whole-module initial-sync38.
    virtual void queue_application_pending_spawn(std::uint32_t,std::unique_ptr<frontend::PreMissionSoldier>&,std::unique_ptr<std::array<std::uint32_t,12>>&)=0;
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
    void reset_round_from_packet(){world_.get().reset_round_from_packet();}
    void read_application_world_snapshot(SessionPacketStream& packet){world_.get().read_application_world_snapshot(packet);}
    void receive_application_statistics_clock(std::uint32_t round,std::uint32_t game,std::uint32_t losses){world_.get().receive_application_statistics_clock(round,game,losses);}
    void receive_application_respawn(std::uint32_t peer,const HumanVector& position,const HumanVector& direction,frontend::PreMissionSoldier& soldier,const std::array<std::uint32_t,12>& stats){world_.get().receive_application_respawn(peer,position,direction,soldier,stats);}
    void admit_application_spawn(std::uint32_t peer,frontend::PreMissionSoldier& soldier,const std::array<std::uint32_t,12>* stats){world_.get().admit_application_spawn(peer,soldier,stats);}
    void receive_application_objective(std::uint32_t identity,std::uint32_t status){world_.get().receive_application_objective(identity,status);}
    void receive_application_peer_departure(std::uint32_t transport){world_.get().receive_application_peer_departure(transport);}
    void receive_application_local_removal(std::uint32_t reason){world_.get().receive_application_local_removal(reason);}
    void receive_application_spawn_rejection(std::uint32_t reason){world_.get().receive_application_spawn_rejection(reason);}
    void append_application_message(const std::string& message){world_.get().append_application_message(message);}
    HumanInputCallbacks* input_default_receiver(std::int32_t identity,bool secondary){return world_.get().input_default_receiver(identity,secondary);}
    std::uint32_t frame_statistics_word(unsigned offset)const{return world_.bound()?world_.get().frame_statistics_word(offset):0u;} //0072F880's genuine absent-current-game0.
    void write_application_world_snapshot(SessionPacketStream& packet){world_.get().write_application_world_snapshot(packet);}
    void queue_application_pending_spawn(std::uint32_t peer,std::unique_ptr<frontend::PreMissionSoldier>& soldier,
            std::unique_ptr<std::array<std::uint32_t,12>>& stats){world_.get().queue_application_pending_spawn(peer,soldier,stats);}
private:
    WorldLoadingDependencyLink<SessionPacketWorld> world_;
};
} //namespace hd2::game
#endif
