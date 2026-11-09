#ifndef HD2_GAME_HUD_SOUND_DIRECTION_H
#define HD2_GAME_HUD_SOUND_DIRECTION_H

#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace hd2::game {

//004947E0 →007885B0. Input is world X/Z; no camera rotation or W read.
//Caller must first resolve actual Players0045D1D0 and return if absent.
inline unsigned hud_sound_direction_sector(float x,float z) {
    const auto length=std::sqrt(static_cast<long double>(z)*z+static_cast<long double>(x)*x);
    if(length>9.99999993922529e-09f) {
        const auto scale=1/length;x=static_cast<float>(x*scale);z=static_cast<float>(z*scale);
    }
    auto angle=std::atan2(1.L,0.L)-std::atan2(static_cast<long double>(z),static_cast<long double>(x));
    constexpr float pi=3.1415927410125732f,tau=6.2831854820251465f;
    if(angle<-pi)angle+=tau;
    if(angle>pi)angle-=tau;
    if(angle<0)angle+=tau;
    const auto value=angle*1.2732394933700562f; //008104C0, eight compass sectors.
    constexpr auto minimum=static_cast<long double>(std::numeric_limits<std::int64_t>::min());
    const auto result=!std::isfinite(value)||value<minimum||value>=-minimum?0u:
        static_cast<std::uint32_t>(static_cast<std::int64_t>(value)); //007E5E00 keeps low DWORD.
    if(result>=8)throw std::runtime_error("HUD sound direction exceeds original compass rows");
    return result;
}
} //namespace hd2::game
#endif
