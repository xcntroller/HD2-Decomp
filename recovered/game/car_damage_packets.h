#ifndef HD2_GAME_CAR_DAMAGE_PACKETS_H
#define HD2_GAME_CAR_DAMAGE_PACKETS_H
#include "game/car_contact_callbacks.h"
#include "game/session_packet_stream.h"
#include <cmath>
namespace hd2::game {
inline std::int32_t car_damage_integer(long double value){
    if(!std::isfinite(value)||value>=2147483648.L||value<-2147483648.L)
        throw std::runtime_error("Car damage exceeds the native signed event range");
    return static_cast<std::int32_t>(value); //Actual __ftol truncation.
}
//008108E0/08D8/08D0/08C0/08C8. Native unwritten padding has no meaning.
struct CarDamageEvent {
    std::uint32_t kind=0,index=0;std::int32_t health=0;
    CarContactVector first{},second{};float scalar=0;
    static CarDamageEvent explosion(std::uint32_t index){CarDamageEvent e;e.kind=5;e.index=index;return e;}
};
inline float read_car_fuel(SessionPacketStream& packet){ //004E0040:16 bits from DOUBLE49999.99925494195.
    const auto value=(static_cast<long double>(packet.read(16))/65535.L)*5000.f;
    return static_cast<float>(value<0?0.L:value>5000.f?5000.L:value);
}
inline void write_car_fuel(SessionPacketStream& packet,float value){ //004E00F0's original unspilled FLOAT factor then integer mask.
    const auto scaled=(static_cast<long double>(value)*.00019999999494757503f)*65535;
    if(!std::isfinite(scaled)||scaled<-9223372036854775808.L||scaled>=9223372036854775808.L)
        throw std::runtime_error("Car fuel serialization exceeds native FTOL range");
    packet.write(static_cast<std::uint32_t>(static_cast<std::int64_t>(std::trunc(scaled))),16);
}
inline void write_car_wheel_damage_payload(SessionPacketStream& packet,std::uint32_t index,std::int32_t health,const CarContactVector& velocity){ //004E05B0 after kind4.
    packet.write(index,8);packet.write(static_cast<std::uint32_t>(health),10);
    if(health<1)(void)packet.write_vector(velocity,packet.primary_quantizer(-1),7);
}
inline void write_car_contact_event(SessionPacketStream& packet,const CarWheelDamageEvent& event){ //004E05B0, RET4.
    packet.write(event.kind,4);write_car_wheel_damage_payload(packet,event.wheel,event.value,event.point);
}
inline void write_car_contact_event(SessionPacketStream& packet,const CarHumanDamageEvent& event){ //004E0270, RET4.
    packet.write(event.kind,4);packet.write(static_cast<std::uint32_t>(event.target),16);packet.write(event.amount,16);
    for(const auto* vector:{&event.impulse,&event.point})for(unsigned axis=0;axis!=3;++axis){
        std::uint32_t bits;std::memcpy(&bits,&(*vector)[axis],4);packet.write(bits,32);
    }
}
inline CarHumanDamageEvent read_car_human_contact(SessionPacketStream& packet){ //004E0180; kind already consumed.
    CarHumanDamageEvent event{};
    const auto signed_word=[](std::uint32_t word){return word&0x8000u?static_cast<std::int32_t>(word)-65536:static_cast<std::int32_t>(word);};
    event.target=signed_word(packet.read(16));event.amount=static_cast<std::uint32_t>(signed_word(packet.read(16)));
    for(auto* vector:{&event.impulse,&event.point})for(unsigned axis=0;axis!=3;++axis){const auto bits=packet.read(32);std::memcpy(&(*vector)[axis],&bits,4);}
    return event; //Both native fourth vector lanes are unproduced; source lanes remain initialized.
}
inline void write_car_contact_event(SessionPacketStream&,const CarDamageEvent&);
template<class Packets,class Event>void request_car_contact(Packets& streams,std::uint32_t mode,std::int32_t identity,const Event& event){ //004DB2E0→00475570(id,false).
    auto& packet=streams.request_stream(3,mode);packet.begin(13,3,0,false);
    packet.write(static_cast<std::uint32_t>(identity),10);write_car_contact_event(packet,event);packet.finish_broadcast();
}
inline CarDamageEvent read_car_damage_event(SessionPacketStream& packet,std::uint32_t kind){
    CarDamageEvent e;e.kind=kind;
    switch(kind){
    case 0:e.index=packet.read(10);e.health=static_cast<std::int32_t>(packet.read(10));e.first=packet.read_direction(-1);break; //004E0340.
    case 1:e.index=packet.read(10);e.health=static_cast<std::int32_t>(packet.read(10));
        if(e.health<1){e.first=packet.read_vector(packet.primary_quantizer(-1),7);e.second=packet.read_vector(packet.primary_quantizer(-1),7);}break; //004E0410.
    case 2:e.index=packet.read(8);e.health=static_cast<std::int32_t>(packet.read(10));
        if(e.health<1)e.first=packet.read_vector(packet.primary_quantizer(-1),7);
        break; //004E0540.
    case 3:e.index=packet.read(8);break; //004E0630.
    case 4:break; //004E0710 genuine RET4, no payload.
    case 5:e.index=packet.read(4);break; //004E06A0.
    case 10:e.scalar=read_car_fuel(packet);break; //004E0040, fuel.
    case 11:e.first=packet.read_vector(packet.primary_quantizer(-1),7);e.index=packet.read(10);break; //004E0800, point then other Car identity.
    case 6:break; //004E0760 genuine no-payload reader; containing reset is004DB2A0.
    case 7:throw std::logic_error("Car inventory packet requires incremental004DE1F0 dispatch");
    default:break; //004DB360 genuinely ignores8/9 and unassigned four-bit kinds.
    }
    return e;
}
inline void write_car_contact_event(SessionPacketStream& packet,const CarDamageEvent& e){
    packet.write(e.kind,4); //trunc(log2(12)/log2(2)+1), double00810860.
    switch(e.kind){
    case 0: //004E03A0, glass: direction is sent even when health remains positive.
        packet.write(e.index,10);packet.write(static_cast<std::uint32_t>(e.health),10);
        packet.write_direction(e.first,-1);break;
    case 1: //004E04B0, other part: positive health has no velocity/force payload.
        packet.write(e.index,10);packet.write(static_cast<std::uint32_t>(e.health),10);
        if(e.health<1){(void)packet.write_vector(e.first,packet.primary_quantizer(-1),7);
            (void)packet.write_vector(e.second,packet.primary_quantizer(-1),7);}break;
    case 2:write_car_wheel_damage_payload(packet,e.index,e.health,e.first);break; //004E05B0.
    case 3:packet.write(e.index,8);break; //004E0650.
    case 4:break; //004E0720.
    case 6:break; //004E0770 writes only the four-bit kind, already written above.
    case 5:packet.write(e.index,4);break; //004E06C0.
    case 10:write_car_fuel(packet,e.scalar);break; //004E00F0.
    case 11:(void)packet.write_vector(e.first,packet.primary_quantizer(-1),7);packet.write(e.index,10);break; //004E0830.
    default:throw std::runtime_error("Car damage event requires its actual derived writer");
    }
}
template<class Packets>void publish_car_damage(Packets& streams,std::int32_t identity,const CarDamageEvent& e){ //004DB2B0.
    auto& packet=streams.begin(13,3); //00475530(id,false): priority3's actual third stream.
    packet.write(static_cast<std::uint32_t>(identity),10);write_car_contact_event(packet,e);packet.finish_broadcast();
}
}
#endif
