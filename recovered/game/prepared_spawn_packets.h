#ifndef HD2_GAME_PREPARED_SPAWN_PACKETS_H
#define HD2_GAME_PREPARED_SPAWN_PACKETS_H
#include "game/session_activation.h"

#include "frontend/control_startup.h"
#include "frontend/pre_mission_inventory.h"
#include "game/session_broadcast_packets.h"

namespace hd2::game {

template<class Bits>
void write_prepared_equipment_entry(Bits& bits,const frontend::PreMissionEquipmentEntry& entry) { //00727450.
    bits.write(static_cast<std::uint32_t>(static_cast<std::int32_t>(entry.position)),16);
    bits.write(static_cast<std::uint32_t>(static_cast<std::int32_t>(entry.item)),16);
    bits.write(static_cast<std::uint32_t>(static_cast<std::int32_t>(entry.count)),8);
}
template<class Bits>
void write_prepared_soldier(Bits& bits,frontend::PreMissionSoldier& soldier,bool equipment_only) { //00727480.
    if(!equipment_only) {
        bits.write_string(soldier.name);bits.write(static_cast<std::uint32_t>(soldier.specialization),1);
        if(soldier.human==-1)soldier.human=255;
        bits.write(static_cast<std::uint32_t>(soldier.human),8);
        if(soldier.network_identity==-1)soldier.network_identity=1023;
        bits.write(static_cast<std::uint32_t>(soldier.network_identity),10);
        bits.write(soldier.flag08,1);bits.write(soldier.flag09,1);bits.write(soldier.word0c,10);
    }
    constexpr std::array<unsigned,5> indices{{0,1,3,4,2}};
    for(const auto n:indices)write_prepared_equipment_entry(bits,soldier.equipment[n]);
    for(const auto* list:{&soldier.holsters,&soldier.backpack}) {
        if(list->size()>std::numeric_limits<std::uint32_t>::max())throw std::length_error("Prepared inventory exceeds native vector size");
        bits.write(static_cast<std::uint32_t>(list->size()),8); //Original count deliberately unmasked; no artificial255 clamp.
        for(std::size_t n=0;n<list->size();++n)write_prepared_equipment_entry(bits,list->at(n));
    }
    if(!equipment_only) {
        const auto float_word=[](float value){std::uint32_t word;std::memcpy(&word,&value,4);return word;};
        for(const auto word:std::array<std::uint32_t,8>{{static_cast<std::uint32_t>(soldier.capacity),float_word(soldier.load),
            float_word(soldier.pack_capacity),static_cast<std::uint32_t>(soldier.pack_available_volume),float_word(soldier.pack_load),
            static_cast<std::uint32_t>(soldier.pack_volume),float_word(soldier.holster_load),static_cast<std::uint32_t>(soldier.holster_volume)}})
            bits.write(word,32); //Native8C..A8 raw words, including FLOAT bit patterns.
    }
}

template<class Bits>
frontend::PreMissionEquipmentEntry read_prepared_equipment_entry(Bits& bits) { //00727420, native WORD stores, no catalogue lookup.
    frontend::PreMissionEquipmentEntry value;
    const auto position=static_cast<std::uint16_t>(bits.read(16)),item=static_cast<std::uint16_t>(bits.read(16));
    std::memcpy(&value.position,&position,2);std::memcpy(&value.item,&item,2);
    value.count=static_cast<std::int16_t>(bits.read(8));return value;
}
template<class Bits>
void read_prepared_soldier(Bits& bits,frontend::PreMissionSoldier& soldier,bool equipment_only) { //00727660, carried records APPEND.
    if(!equipment_only) {
        soldier.name=bits.read_string(0xff);soldier.specialization=static_cast<std::int32_t>(bits.read(1));
        soldier.human=static_cast<std::int32_t>(bits.read(8));soldier.network_identity=static_cast<std::int32_t>(bits.read(10));
        soldier.flag08=bits.read(1)!=0;soldier.flag09=bits.read(1)!=0;soldier.word0c=bits.read(10);
    }
    for(auto n:{0u,1u,3u,4u,2u})soldier.equipment[n]=read_prepared_equipment_entry(bits);
    for(auto* list:{&soldier.holsters,&soldier.backpack}) {
        const auto count=bits.read(8);
        for(std::uint32_t n=0;n<count;++n)list->push_back(read_prepared_equipment_entry(bits));
    }
    if(!equipment_only) {
        const auto integer=[&]{const auto word=bits.read(32);std::int32_t value;std::memcpy(&value,&word,4);return value;};
        const auto real=[&]{const auto word=bits.read(32);float value;std::memcpy(&value,&word,4);return value;};
        soldier.capacity=integer();soldier.load=real();soldier.pack_capacity=real();soldier.pack_available_volume=integer();
        soldier.pack_load=real();soldier.pack_volume=integer();soldier.holster_load=real();soldier.holster_volume=integer();
    }
}

//00611C90's actual offline publication, sharing the startup record owner and
//session locks. Direct is the genuine006123A0 receiver for game1C==1; the normal
//new-session value0 appends the kind13 message without inventing queue dispatch.
template<class Locks,class Scene,class Direct>
class PreparedSpawnPackets {
public:
    PreparedSpawnPackets(frontend::ControlStartupState& state,SessionBroadcastPackets& streams,
                         Locks& locks,Scene& scene,Direct& direct,std::uint32_t& mode,SessionActivationState& game)
        :state_(state),streams_(streams),locks_(locks),scene_(scene),direct_(direct),mode_(mode),game_(game){}
    void bind_spawned_player(std::uint32_t player,std::uint32_t value,const std::string& name) { //0072E940.
        if(!(mode_&2u))return;
        Lock lock(locks_);
        if(name.size()>=frontend::ControlStartupRecord::size-8)throw std::length_error("Spawn player name exceeds native record");
        if(state_.records.size()<=player) {
            frontend::ControlStartupRecord record;frontend::initialize_control_record(record);
            record.word(0,0);record.byte(8,0); //Actual additional writes to the expansion exemplar.
            while(state_.records.size()<=player)state_.records.push_back(record);
        }
        auto& record=state_.records.at(player);record.word(0,value);
        for(std::size_t n=0;n<name.size();++n)record.byte(8+n,static_cast<std::uint8_t>(name[n]));
        record.byte(8+name.size(),0);
    }
    template<class Spawn,class Human>
    void publish_prepared_spawn(Spawn& spawn,Human& human,frontend::PreMissionSoldier& soldier,
                                const HumanVector& position,const HumanVector& direction) {
        if((mode_&7u)!=7u)throw std::runtime_error("Prepared packet requires actual nonoffline stat serialization");
        if(game_.words.template get<std::uint32_t>(0x1c)==1) {
            Lock lock(locks_);
            const auto record=state_.records.at(0); //0072EC70 lock remains held through direct receiver.
            if(mode_&2u)bind_spawned_player(0,record.word(0),record.name()); //Native recursive critical section.
            direct_.publish_prepared_spawn(0,spawn,human,soldier,position,direction);
            return;
        }
        auto& packet=streams_.begin(13,0); //00475530(spawn identity,true).
        packet.write(static_cast<std::uint32_t>(spawn.base().id10()),10);
        packet.write(0,2);packet.write(0,8);
        const auto identity=human.base().id10();
        packet.write(static_cast<std::uint32_t>(identity),8);
        (void)packet.write_vector(position,packet.primary_quantizer(-1),7);
        packet.write_direction(direction,-1);
        packet.write(static_cast<std::uint32_t>(scene_.player_generation(identity)),8);
        {Lock lock(locks_);const auto& record=state_.records.at(0);
            packet.write_string(record.name());packet.write(record.word(0),32);}
        write_prepared_soldier(packet,soldier,false);
        packet.finish_broadcast(); //004755B0→0072BF90(target0), leaves bits pending.
    }
private:
    struct Lock {
        Locks& locks;explicit Lock(Locks& value):locks(value){locks.enter_control_lock(0);}
        ~Lock(){locks.leave_control_lock(0);}
    };
    frontend::ControlStartupState& state_;SessionBroadcastPackets& streams_;Locks& locks_;Scene& scene_;Direct& direct_;
    std::uint32_t& mode_;const SessionActivationState& game_;
};
} //namespace hd2::game
#endif
