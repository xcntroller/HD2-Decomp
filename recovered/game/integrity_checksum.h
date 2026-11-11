#ifndef HD2_GAME_INTEGRITY_CHECKSUM_H
#define HD2_GAME_INTEGRITY_CHECKSUM_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace hd2::game {

//00A0D960's WORD table and00A0DAF0's byte accumulator. This calculates bytes
//from the real image/file reader; no expected checksum or healthy value is
//manufactured.00A0DA00's file-open/seek/read owner is a separate dependency.
class IntegrityChecksumTable {
public:
    IntegrityChecksumTable() noexcept {
        for(unsigned n=0;n<256;++n){
            std::uint16_t bit_value=0xc0c1;
            for(unsigned bit=1;bit<256;bit<<=1){
                if(n&bit)table_[n]^=bit_value;
                bit_value=static_cast<std::uint16_t>((bit_value<<1)^0x4003);
            }
        }
    }
    std::uint16_t accumulate(std::uint16_t crc,const std::uint8_t* bytes,std::size_t count)const {
        if(count&&!bytes)throw std::invalid_argument("Integrity checksum requires actual file bytes");
        for(std::size_t n=0;n<count;++n)
            crc=static_cast<std::uint16_t>((crc>>8)^table_[(crc&0xffu)^bytes[n]]);
        return crc;
    }
    const auto& words()const noexcept{return table_;}
private:
    std::array<std::uint16_t,256> table_{};
};

} //namespace hd2::game
#endif
