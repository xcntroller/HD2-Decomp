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
    CarContactVector first{},second{};
    static CarDamageEvent explosion(std::uint32_t index){CarDamageEvent e;e.kind=5;e.index=index;return e;}
};
template<class Packets>void publish_car_damage(Packets& streams,std::int32_t identity,const CarDamageEvent& e){ //004DB2B0.
    auto& packet=streams.begin(13,3); //00475530(id,false): priority3's actual third stream.
    packet.write(static_cast<std::uint32_t>(identity),10);
    packet.write(e.kind,4); //trunc(log2(12)/log2(2)+1), double00810860.
    switch(e.kind){
    case 0: //004E03A0, glass: direction is sent even when health remains positive.
        packet.write(e.index,10);packet.write(static_cast<std::uint32_t>(e.health),10);
        packet.write_direction(e.first,-1);break;
    case 1: //004E04B0, other part: positive health has no velocity/force payload.
        packet.write(e.index,10);packet.write(static_cast<std::uint32_t>(e.health),10);
        if(e.health<1){(void)packet.write_vector(e.first,packet.primary_quantizer(-1),7);
            (void)packet.write_vector(e.second,packet.primary_quantizer(-1),7);}break;
    case 3:packet.write(e.index,8);break; //004E0650.
    case 4:break; //004E0720.
    case 5:packet.write(e.index,4);break; //004E06C0.
    default:throw std::runtime_error("Car damage event requires its actual derived writer");
    }
    packet.finish_broadcast();
}
}
#endif
