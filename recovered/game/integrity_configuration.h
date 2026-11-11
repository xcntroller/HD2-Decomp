#ifndef HD2_GAME_INTEGRITY_CONFIGURATION_H
#define HD2_GAME_INTEGRITY_CONFIGURATION_H

#include <array>
#include <cstdint>

namespace hd2::game {

//00A0DB60, called by009F0A60 on the embedded 84hex-byte configuration.
//This is decoding only: callers must still perform009F0A60's digest and flags
//checks before using the configuration or publishing the private WORD owner.
using IntegrityConfiguration=std::array<std::uint8_t,0x84>;
inline void decode_integrity_configuration(IntegrityConfiguration* configuration) noexcept {
    if(!configuration)return;
    auto& bytes=*configuration;
    bytes[0]=0x41;bytes[1]=0x64;bytes[2]=0x64;bytes[3]=0x44;
    bytes[5]=bytes[6]=bytes[7]=0;
    //Unaligned little-endian DWORD at56; byte operations preserve native width.
    bytes[0x56]^=0xca;bytes[0x57]^=0xdd;bytes[0x58]^=0xdd;bytes[0x59]^=0xac;
    for(unsigned n=8;n<0x4a;++n)bytes[n]^=static_cast<std::uint8_t>(n<<(n%3));
}

} //namespace hd2::game
#endif
