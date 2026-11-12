#ifndef HD2_GAME_OFFLINE_SPAWN_PRESENTATION_H
#define HD2_GAME_OFFLINE_SPAWN_PRESENTATION_H

#include "game/prepared_spawn_packets.h"

namespace hd2::game {

//006123A0 shares the authority's actual Scene2 Human. Only a client creates
//through00612A60, and only offline authority reads SpawnPoint A8.
template<class Human,class Runtime>class SessionSpawnPresentation {
public:
    explicit SessionSpawnPresentation(Runtime& runtime):r_(runtime){}
    template<class Spawn>void present(std::uint32_t peer,std::uint32_t identity,Spawn& spawn,
            const HumanVector& position,const HumanVector& direction,frontend::PreMissionSoldier& soldier,
            const std::array<std::uint32_t,12>* stats){
        if(!spawn.base().frame())return;
        Human* human=nullptr;
        if(!(r_.mode_flags()&1u))human=r_.create_session_spawn_human(peer,identity,position,direction,soldier,stats,spawn.properties.fielda0!=0);
        else if((r_.mode_flags()&7u)==7u){
            human=dynamic_cast<Human*>(spawn.spawned_human);
            if(spawn.spawned_human&&!human)throw std::logic_error("SpawnPoint A8 lost its actual Human");
        }else human=r_.spawn_scene_human(identity);
        if(!human)return;
        if((r_.mode_flags()&7u)!=7u&&peer==static_cast<std::uint32_t>(r_.human_current_player_id())&&r_.spawn_result()!=3)
            r_.set_spawn_match_phase(1);
        if(!(r_.mode_flags()&1u))human->activate(false);
        if(peer==static_cast<std::uint32_t>(r_.human_current_player_id())&&!spawn.properties.fielda0){
            r_.spawn_camera_command(0,true,0);r_.spawn_camera_command(2,true,0);
            if(!r_.spawn_roster_contains(*human))r_.append_spawn_human(*human);
        }
        if((r_.mode_flags()&7u)!=7u){
            const auto kind=r_.spawn_state_kind(*human);
            if(kind==1){
                r_.spawn_human_side(*human,static_cast<std::uint32_t>(soldier.specialization)); //005A8800 uses SCENE id, not peer slot.
                if(r_.human_is_player(*human))r_.select_spawn_human(*human);
                r_.spawn_selection_suppressed(*human,static_cast<std::uint8_t>(soldier.flag08));
                r_.spawn_player_flag(*human,static_cast<std::uint8_t>(soldier.flag09));
                r_.present_local_spawn(peer,*human);
            }else if(kind==7){
                r_.spawn_remote_flag(*human,static_cast<std::uint8_t>(soldier.flag09));
                r_.spawn_selection_suppressed(*human,static_cast<std::uint8_t>(soldier.flag08));
            }
        }
        if((r_.mode_flags()&7u)!=7u&&!(r_.mode_flags()&1u)&&r_.spawn_voice_enabled())r_.update_human_transmit_targets();
    }
    template<class Spawn>void publish_prepared_spawn(std::uint32_t peer,Spawn& spawn,Human& human,
            frontend::PreMissionSoldier& soldier,const HumanVector& position,const HumanVector& direction){
        present(peer,static_cast<std::uint32_t>(human.base().id10()),spawn,position,direction,soldier,nullptr);
    }
private:Runtime& r_;
};

//006125A0: generation is written BEFORE soldier decode on clients. Every mode
//test remains live, including the final decision to pass the explicit stats.
template<class PacketRecords,class Presentation>class SessionSpawnPacketDecoder {
public:
    SessionSpawnPacketDecoder(PacketRecords& records,Presentation& presentation,std::uint32_t& mode)
        :records_(records),presentation_(presentation),mode_(mode){}
    template<class Spawn>void receive(Spawn& spawn,SessionPacketStream* packet){
        if(!(mode_&2u)||!packet)return;
        if(packet->read(2)!=0)return;
        const auto peer=packet->read(8),identity=packet->read(8);
        const auto position=packet->read_vector(packet->primary_quantizer(-1),7);
        const auto direction=packet->read_direction(-1);
        const auto generation=packet->read(8);const auto name=packet->read_string(0xff);const auto transport=packet->read(32);
        if(!(mode_&1u))records_.set_player_generation(static_cast<std::int32_t>(identity),generation-1u);
        frontend::PreMissionSoldier soldier;read_prepared_soldier(*packet,soldier,false);
        std::array<std::uint32_t,12> stats{{800,80,80,80,80,80,50,30,20,50,0,80}};
        if((mode_&7u)!=7u)for(auto n:{0u,2u,1u,3u,4u,5u,6u,7u,8u,9u,11u,10u})stats[n]=packet->read(n==0?10:7);
        if(mode_&2u)records_.bind_spawned_player(peer,transport,name);
        presentation_.present(peer,identity,spawn,position,direction,soldier,(mode_&7u)!=7u?&stats:nullptr);
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
//004755D0 (module13 virtual28): preserve reserved005A5F80 versus actor20.
template<class Scene,class Actors,class Reserved>
bool receive_scene2_authority_packet(Scene& scene,SessionPacketStream& packet,Actors& actors,Reserved& reserved){
    const auto identity=packet.read(10);auto* actor=scene.scene_actor_identity(static_cast<std::int32_t>(identity));
    if(identity<32)reserved.receive_player_authority_packet(actor,packet);
    else {
        if(!actor)throw std::runtime_error("Scene2 authority packet has no actual actor at its identity");
        actors.receive_actor_authority_packet(*actor,packet);
    }
    return true;
}
} //namespace hd2::game
#endif
