#ifndef HD2_NETWORKING_SERVER_PLAYER_SLOT_H
#define HD2_NETWORKING_SERVER_PLAYER_SLOT_H

#include <array>
#include <cstdint>
#include <stdexcept>

namespace hd2::networking {

// Owned data image for0077D4B0's0xC4-byte slot reset. This is neither a wire
// packet nor an ABI object containing usable native pointers. Unknown bytes are
// zero on fresh native construction; reset preserves bytes the original leaves.
class ServerPlayerSlot {
public:
    using Bytes = std::array<unsigned char,0xc4>;
    explicit ServerPlayerSlot(Bytes initial = {}) : bytes_(initial) { reset(); }
    void reset() noexcept {
        bytes_[8] = 0;
        for (unsigned at = 0x40; at < 0x64; at += 4) word(at,0);
        for (const auto at : {0xb4u,0x6cu,0x74u,0u,0x3cu,0x7cu,0xa4u,0xacu,0xb0u,0xbcu})
            word(at,0);
        bytes_[0xa1] = bytes_[0xa2] = 0;
        word(0x68,2); word(0x64,2); word(0x70,0xffffffffu); word(0xb8,50);
    }
    const Bytes& bytes() const noexcept { return bytes_; }
    std::uint32_t word(unsigned at) const {
        if (at > bytes_.size()-4) throw std::out_of_range("Player slot word outside record");
        return std::uint32_t{bytes_[at]} | (std::uint32_t{bytes_[at+1]} << 8) |
            (std::uint32_t{bytes_[at+2]} << 16) | (std::uint32_t{bytes_[at+3]} << 24);
    }
private:
    void word(unsigned at,std::uint32_t value) noexcept {
        for (unsigned byte = 0; byte < 4; ++byte)
            bytes_[at+byte] = static_cast<unsigned char>(value >> (byte*8));
    }
    Bytes bytes_;
};

} // namespace hd2::networking
#endif
