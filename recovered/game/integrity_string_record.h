#ifndef HD2_GAME_INTEGRITY_STRING_RECORD_H
#define HD2_GAME_INTEGRITY_STRING_RECORD_H

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace hd2::game {

//009FFAF0: DWORD byte count, copied1024-byte scratch, descending adjacent XOR,
//then min(count+1,capacity) bytes to the caller. The actual original records and
//009FFBB0's shared process scratch must still come from their real owners.
//Explicit span sizes add guards for native invalid memory accesses only.
inline std::uint8_t* decode_integrity_string_record(const std::uint8_t* record,
    std::size_t record_size,std::uint8_t* destination,std::size_t destination_size,
    std::int32_t capacity) {
    if(!record||record_size<4)throw std::invalid_argument("Integrity string record has no actual DWORD count");
    if(capacity<0)throw std::invalid_argument("Negative integrity string capacity");
    const auto count=static_cast<std::uint32_t>(record[0])|
        (static_cast<std::uint32_t>(record[1])<<8)|
        (static_cast<std::uint32_t>(record[2])<<16)|
        (static_cast<std::uint32_t>(record[3])<<24);
    std::array<std::uint8_t,1024> scratch{};
    if(count>scratch.size()||count>record_size-4)
        throw std::invalid_argument("Integrity string payload exceeds its actual source/scratch");
    std::copy_n(record+4,count,scratch.begin());
    for(std::size_t n=count;n>1;--n)scratch[n-2]^=scratch[n-1];
    const auto copied=std::min<std::size_t>(static_cast<std::size_t>(count)+1,
        static_cast<std::size_t>(capacity));
    if(copied>scratch.size()||copied>destination_size||(!destination&&copied))
        throw std::invalid_argument("Integrity string output exceeds its actual destination/scratch");
    if(copied)std::copy_n(scratch.begin(),copied,destination);
    return destination;
}

} //namespace hd2::game
#endif
