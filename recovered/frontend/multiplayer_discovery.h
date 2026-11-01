#ifndef HD2_FRONTEND_MULTIPLAYER_DISCOVERY_H
#define HD2_FRONTEND_MULTIPLAYER_DISCOVERY_H

#include "frontend/multiplayer_server.h"
#include <optional>

namespace hd2::frontend {

// Original0073D000, bounded and byte-based (no unaligned uint32 loads).
class MultiplayerBits {
public:
    MultiplayerBits(const unsigned char* data, std::size_t size) : data_(data), size_(size) {
        if (size && !data) throw std::invalid_argument("Null multiplayer response");
        if (size > std::numeric_limits<std::size_t>::max() / 8) throw std::length_error("Multiplayer response too large");
    }
    std::uint32_t read(unsigned count) {
        if (count > 32 || count > size_ * 8 - position_) throw std::runtime_error("Truncated multiplayer response");
        std::uint32_t value = 0;
        for (unsigned i = 0; i < count; ++i, ++position_)
            value |= std::uint32_t{(data_[position_ / 8] >> (position_ % 8)) & 1u} << i;
        return value;
    }
    std::size_t position() const { return position_; }
    void rewind(std::size_t position) {
        if (position > position_) throw std::invalid_argument("Invalid multiplayer bit rewind");
        position_ = position;
    }
private:
    const unsigned char* data_;
    std::size_t size_, position_ = 0;
};

struct MultiplayerPlayerInformation {
    std::int32_t score = 0;
    std::uint32_t word04 = 0, word08 = 0, word10 = 0, word14 = 0, word24 = 0;
    std::array<char, 51> name{};
};
struct MultiplayerInformation {
    std::vector<MultiplayerPlayerInformation> players;
    //00725C40 never transmits field12. Preserve that distinction explicitly.
    std::array<std::optional<std::int32_t>, 20> rules;
    std::array<std::uint32_t, 2> team_scores{};
};

inline void multiplayer_bit_string(MultiplayerBits& bits, std::array<char,51>& output) {
    for (std::size_t i = 0; i < 50; ++i) {
        const auto ch = static_cast<char>(bits.read(8));
        output[i] = ch;
        if (!ch) break;
    }
    output[49] = 0; //0073D0A0's original limit50 semantics.
}

//00725FB0/00725EC0/00725E60/00749AE0/00725C40. The bit reader is already
// positioned after the discovery record. Bound external counts before reserve;
// the original signed-negative player count reads no players.
inline MultiplayerInformation decode_multiplayer_information(MultiplayerBits& bits) {
    MultiplayerInformation result;
    const auto count = static_cast<std::int32_t>(bits.read(32));
    if (count > 64) throw std::runtime_error("Too many multiplayer information players");
    for (std::int32_t i = 0; i < count; ++i) {
        MultiplayerPlayerInformation player;
        const auto score = bits.read(11);
        player.score = score > 0x400 ? static_cast<std::int32_t>(score) - 0x800 : static_cast<std::int32_t>(score);
        player.word04 = bits.read(10); player.word08 = bits.read(10);
        player.word10 = bits.read(10); player.word14 = bits.read(5); player.word24 = bits.read(32);
        multiplayer_bit_string(bits, player.name);
        result.players.push_back(std::move(player));
    }
    const auto signed16 = [&] {
        const auto value = bits.read(16);
        return value & 0x8000u ? static_cast<std::int32_t>(value) - 0x10000 : static_cast<std::int32_t>(value);
    };
    result.rules[0] = static_cast<std::int32_t>(bits.read(8));
    for (unsigned i = 1; i <= 5; ++i) result.rules[i] = signed16();
    for (unsigned i = 6; i <= 11; ++i) result.rules[i] = static_cast<std::int32_t>(bits.read(1));
    result.rules[13] = static_cast<std::int32_t>(bits.read(1));
    result.rules[14] = static_cast<std::int32_t>(bits.read(8));
    result.rules[15] = signed16();
    for (unsigned i = 16; i <= 18; ++i) result.rules[i] = static_cast<std::int32_t>(bits.read(1));
    result.rules[19] = signed16();
    for (auto& score : result.team_scores) score = bits.read(10);
    return result;
}

// ENUM_HOSTS_RESPONSE branch of00722180; simple browser records only. Kind1
// game-info continuation00725FB0 is intentionally not treated as a browser row.
// Throws for malformed kind0 input; other kinds return nullopt. No connection
// success, UI callback or borrowed response/COM pointer escapes this function.
inline std::optional<MultiplayerServer> decode_multiplayer_discovery(
        const unsigned char* data, std::size_t size, std::string_view host,
        std::uint32_t ipv4, std::uint32_t ping,
        std::optional<MultiplayerInformation>* information = nullptr,
        std::uint32_t* response_kind = nullptr) {
    MultiplayerBits bits(data, size);
    const auto kind = bits.read(2);
    if (response_kind) *response_kind = kind;
    if (kind == 3 || (kind == 1 && !information) || (kind == 2 && !response_kind)) return std::nullopt;
    MultiplayerServer server;
    const auto name = host.substr(0, std::min<std::size_t>(host.find('\0'), 49));
    std::copy(name.begin(), name.end(), server.host.begin());
    server.ipv4 = ipv4; server.ping = ping;
    // 0073D0A0 reads at most50 bytes, then overwrites index49 with NUL.
    multiplayer_bit_string(bits, server.map);
    const auto start = bits.position();
    const auto common = [&](unsigned style_bits) {
        server.style = bits.read(style_bits); server.dedicated = static_cast<std::uint8_t>(bits.read(1));
        server.players = bits.read(6); server.capacity = bits.read(6);
        server.password = static_cast<std::uint8_t>(bits.read(1));
    };
    const auto tail = [&] {
        const auto port = bits.read(16);
        server.port = (port & 0x8000u) ? port | 0xffff0000u : port; // Original MOVSX.
        server.manual = 0; server.flag88 = static_cast<std::uint8_t>(bits.read(1));
    };
    common(2); server.minor = bits.read(8); server.major = bits.read(8); tail(); server.edition = 1;
    if (server.major != 1 || server.minor > 20) {
        bits.rewind(start);
        server.minor = bits.read(8); server.major = bits.read(8); common(3); tail();
        // 007E5A40 is strstr, confirmed by its body, not a prefix comparison.
        server.edition = multiplayer_text(server.host).find("DEMO") != std::string_view::npos ? 2 : bits.read(2);
    }
    if (information) {
        information->reset();
        if (kind == 1) *information = decode_multiplayer_information(bits);
    }
    return server;
}

} // namespace hd2::frontend
#endif
