#ifndef HD2_GAME_OFFLINE_SPAWN_PRESENTATION_H
#define HD2_GAME_OFFLINE_SPAWN_PRESENTATION_H

#include "game/prepared_spawn_packets.h"

namespace hd2::game {

//006123A0 offline7: use the SAME SpawnPoint A8 Human already created by
//authority. Camera commands precede roster append, even for the first player.
//Decoded pose/stats never create or reposition another offline Human.
template<class Human,class Control,class Camera,class Refresh>
class OfflineSpawnPresentation {
public:
    OfflineSpawnPresentation(Control& control,Camera& camera,Refresh& refresh,
                              std::uint32_t& mode,std::int32_t& local_player)
        :control_(control),camera_(camera),refresh_(refresh),mode_(mode),local_player_(local_player){}
    template<class Spawn>void present(std::uint32_t player,Spawn& spawn) {
        if(!spawn.base().frame())return;
        if((mode_&7u)!=7u)throw std::runtime_error("Spawn presentation requires actual nonoffline creation/control receivers");
        if(!spawn.spawned_human)return;
        auto* human=dynamic_cast<Human*>(spawn.spawned_human);
        if(!human)throw std::runtime_error("SpawnPoint A8 is not its actual Human");
        if(player==static_cast<std::uint32_t>(local_player_)&&!spawn.properties.fielda0) {
            camera_.command_offline(0,true,0,mode_,refresh_); //Param3 presence1, param4NULL, not bool(true).
            camera_.command_offline(2,true,0,mode_,refresh_); //Native offline7 command2 does nothing.
            if(!control_.human_roster().contains(human))control_.add_human(human);
        }
    }
    template<class Spawn>void publish_prepared_spawn(std::uint32_t player,Spawn& spawn,Human&,
            frontend::PreMissionSoldier&,const HumanVector&,const HumanVector&){present(player,spawn);} //Direct game1C==1 adapter.
private:
    Control& control_;Camera& camera_;Refresh& refresh_;std::uint32_t& mode_;const std::int32_t& local_player_;
};

//006125A0: actual SpawnPoint virtual1C presentation decoder, ordinary offline
//subtype0. Full decode/record publication precedes006123A0, including on a
//SpawnPoint without a frame or without its previously-created A8 Human.
template<class PacketRecords,class Presentation>
class OfflineSpawnPacketDecoder {
public:
    OfflineSpawnPacketDecoder(PacketRecords& records,Presentation& presentation,std::uint32_t& mode)
        :records_(records),presentation_(presentation),mode_(mode){}
    template<class Spawn>void receive(Spawn& spawn,SessionPacketStream* packet) {
        if(!(mode_&2u)||!packet)return;
        if((mode_&7u)!=7u)throw std::runtime_error("Spawn decoder requires actual nonoffline generation/stats consumers");
        if(packet->read(2)!=0)return;
        const auto player=packet->read(8),identity=packet->read(8);
        const auto position=packet->read_vector(packet->primary_quantizer(-1),7);
        const auto direction=packet->read_direction(-1);
        const auto generation=packet->read(8);const auto name=packet->read_string(0xff);const auto value=packet->read(32);
        frontend::PreMissionSoldier soldier;read_prepared_soldier(*packet,soldier,false);
        //Native offline authority skips00475510(generation-1) and passes NULL
        //explicit stats. These decoded locals still consume the original bits.
        (void)identity;(void)position;(void)direction;(void)generation;
        if(mode_&2u)records_.bind_spawned_player(player,value,name);
        presentation_.present(player,spawn);
    }
private:
    PacketRecords& records_;Presentation& presentation_;std::uint32_t& mode_;
};

//00475630 (module13 virtual24): physical Scene2 identity selects the actual
//actor receiver. Reserved0..31 goes through005A5FB0, including NULL; it must
//consume through its real player-state/005418E0 receiver, never skip payload.
template<class Scene,class Actors,class Reserved>
bool receive_scene2_presentation_packet(Scene& scene,SessionPacketStream& packet,Actors& actors,Reserved& reserved) {
    const auto identity=packet.read(10);
    auto* actor=scene.scene_actor_identity(static_cast<std::int32_t>(identity));
    if(identity<32)reserved.receive_player_packet(actor,packet);
    else {
        if(!actor)throw std::runtime_error("Scene2 packet has no actual actor at its identity");
        actors.receive_actor_packet(*actor,packet);
    }
    return true;
}
} //namespace hd2::game
#endif
