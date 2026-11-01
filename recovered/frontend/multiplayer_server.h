#ifndef HD2_FRONTEND_MULTIPLAYER_SERVER_H
#define HD2_FRONTEND_MULTIPLAYER_SERVER_H

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace hd2::frontend {

// 00724EE0/00724EF0. Native fresh storage zeroes original indeterminate bytes;
// reset deliberately preserves host, address, versions and unknown fields.
struct MultiplayerServer {
    std::array<char, 51> host{}, map{};
    std::array<std::uint8_t, 2> padding66{};
    std::uint32_t ipv4 = 0, port = 0, style = 4, ping = 9999, players = 0, capacity = 0;
    std::uint8_t dedicated = 0, manual = 0, password = 0, padding83 = 0;
    std::uint32_t word84 = 0;
    std::uint8_t flag88 = 0;
    std::array<std::uint8_t, 3> padding89{};
    std::uint32_t minor = 0, major = 0, edition = 0;
    std::uint8_t listed = 0;
    std::array<std::uint8_t, 3> padding99{};

    MultiplayerServer() { reset(); }
    void reset() {
        constexpr char unknown[] = " ??? ";
        std::copy(std::begin(unknown), std::end(unknown), map.begin());
        style = 4; ping = 9999; players = capacity = 0;
        dedicated = manual = password = flag88 = listed = 0;
    }
};
static_assert(sizeof(MultiplayerServer) == 0x9c);
static_assert(offsetof(MultiplayerServer, map) == 0x33);
static_assert(offsetof(MultiplayerServer, ipv4) == 0x68);
static_assert(offsetof(MultiplayerServer, word84) == 0x84);
static_assert(offsetof(MultiplayerServer, minor) == 0x8c);
static_assert(offsetof(MultiplayerServer, listed) == 0x98);

inline std::string_view multiplayer_text(const std::array<char, 51>& text) {
    const auto end = std::find(text.begin(), text.end(), '\0');
    return {text.data(), static_cast<std::size_t>(end - text.begin())};
}

struct MultiplayerFilters {
    std::int32_t maximum_ping = 0, style = -1;
    bool responding = false, dedicated = false, not_empty = false;
    bool not_full = false, no_password = false, current_version = false;

    // 00635E40: profile-options+4 must equal1 to apply *any* filters.
    // The original not-empty check really is capacity, not players.
    bool accepts(const MultiplayerServer& server, std::uint32_t options_mode,
                 std::uint32_t major, std::uint32_t minor) const {
        if (options_mode != 1) return true;
        if (responding && server.ping > 9998) return false;
        if (maximum_ping > 0 && server.ping > static_cast<std::uint32_t>(maximum_ping)) return false;
        if (style != -1 && server.style != static_cast<std::uint32_t>(style)) return false;
        if (not_empty && server.capacity == 0) return false;
        if (not_full && server.capacity == server.players) return false;
        if (dedicated && !server.dedicated) return false;
        if (no_password && server.password) return false;
        return !current_version || (server.major == major && server.minor == minor);
    }
};

namespace multiplayer_archive {

// Checked byte/chunk view; malformed external input cannot reproduce the
// original 00725290 overlong-string stack overwrite or invalid chunk seeks.
class Reader {
public:
    Reader(const unsigned char* data, std::size_t size) : data_(data), size_(size) {
        if (size && !data) throw std::invalid_argument("Null multiplayer archive");
    }
    std::size_t remaining() const { return size_ - position_; }
    std::uint32_t word(unsigned count) {
        if (count > 4 || remaining() < count) throw std::runtime_error("Truncated multiplayer field");
        std::uint32_t value = 0;
        for (unsigned i = 0; i < count; ++i) value |= std::uint32_t{data_[position_++]} << (8 * i);
        return value;
    }
    Reader chunk(std::uint16_t& tag) {
        tag = static_cast<std::uint16_t>(word(2));
        const auto length = word(4);
        if (length < 6 || length - 6 > remaining()) throw std::runtime_error("Invalid multiplayer chunk bounds");
        Reader result(data_ + position_, length - 6);
        position_ += length - 6;
        return result;
    }
    void text(std::array<char, 51>& output) {
        const auto length = word(2);
        if (remaining() < length) throw std::runtime_error("Truncated multiplayer string");
        output[0] = 0;
        if (length) {
            const auto* start = data_ + position_;
            const auto* end = std::find(start, start + length, 0);
            if (end == start + length) throw std::runtime_error("Unterminated multiplayer string");
            const auto count = std::min<std::size_t>(end - start, length >= 51 ? 49 : 50);
            std::memcpy(output.data(), start, count);
            output[count] = 0;
        }
        position_ += length; // Consume once, including safely truncated strings.
    }
    void optional(std::uint32_t& value) { if (remaining() >= 4) value = word(4); }
    void optional(std::uint8_t& value) { if (remaining()) value = static_cast<std::uint8_t>(word(1)); }
private:
    const unsigned char* data_;
    std::size_t size_, position_ = 0;
};

inline void word(std::vector<unsigned char>& bytes, std::uint32_t value, unsigned count = 4) {
    for (unsigned i = 0; i < count; ++i) bytes.push_back(static_cast<unsigned char>(value >> (8 * i)));
}
inline std::size_t begin(std::vector<unsigned char>& bytes, std::uint16_t tag) {
    const auto start = bytes.size(); word(bytes, tag, 2); word(bytes, 0); return start;
}
inline void end(std::vector<unsigned char>& bytes, std::size_t start) {
    const auto length = bytes.size() - start;
    if (length > std::numeric_limits<std::uint32_t>::max()) throw std::length_error("Multiplayer archive too large");
    for (unsigned i = 0; i < 4; ++i) bytes[start + 2 + i] = static_cast<unsigned char>(length >> (8 * i));
}
inline void text(std::vector<unsigned char>& bytes, const std::array<char, 51>& value) {
    const auto view = multiplayer_text(value);
    if (view.size() == value.size()) throw std::invalid_argument("Unterminated multiplayer server name");
    word(bytes, static_cast<std::uint32_t>(view.size() + 1), 2);
    bytes.insert(bytes.end(), view.begin(), view.end()); bytes.push_back(0);
}

// 00725020: note persisted98 precedes94, unlike the native/original object.
inline void write_record(std::vector<unsigned char>& bytes, const MultiplayerServer& server) {
    const auto start = begin(bytes, 2000);
    text(bytes, server.host); text(bytes, server.map);
    for (auto value : {server.ipv4, server.port, server.style, server.ping, server.players, server.capacity}) word(bytes, value);
    word(bytes, server.dedicated, 1); word(bytes, server.manual, 1); word(bytes, server.password, 1);
    word(bytes, server.word84); word(bytes, server.flag88, 1); word(bytes, server.minor);
    word(bytes, server.major); word(bytes, server.listed, 1); word(bytes, server.edition);
    end(bytes, start);
}

// 00725290 scans record-wrapper children. Missing bounded scalar reads preserve
// initialized values. Invalid strings/chunk bounds explicitly reject input.
inline MultiplayerServer read_record(Reader wrapper) {
    MultiplayerServer server;
    while (wrapper.remaining()) {
        std::uint16_t tag = 0;
        auto record = wrapper.chunk(tag);
        if (tag != 2000) continue;
        record.text(server.host); record.text(server.map);
        record.optional(server.ipv4); record.optional(server.port); record.optional(server.style);
        record.optional(server.ping); record.optional(server.players); record.optional(server.capacity);
        record.optional(server.dedicated); record.optional(server.manual); record.optional(server.password);
        record.optional(server.word84); record.optional(server.flag88); record.optional(server.minor);
        record.optional(server.major); record.optional(server.listed); record.optional(server.edition);
    }
    return server;
}

} // namespace multiplayer_archive

// In-memory form of global PlayersProfiles\\ServerList.bin, not a new profile
// file or a discovery result. Caller owns actual file I/O and missing-file state.
struct MultiplayerServerLists {
    std::vector<MultiplayerServer> ordinary, favorites;

    // 00619D30. Persisted count is a DWORD hint; it cannot manufacture entries.
    std::vector<unsigned char> encode() const {
        using namespace multiplayer_archive;
        std::vector<unsigned char> bytes;
        const auto root = begin(bytes, 0x28a);
        const auto group = [&](std::uint16_t tag, const auto& records) {
            if (records.size() > std::numeric_limits<std::uint32_t>::max()) throw std::length_error("Too many multiplayer servers");
            const auto start = begin(bytes, tag), count = begin(bytes, 0x28d);
            word(bytes, static_cast<std::uint32_t>(records.size())); end(bytes, count);
            for (const auto& server : records) {
                const auto wrapper = begin(bytes, 0x296); write_record(bytes, server); end(bytes, wrapper);
            }
            end(bytes, start);
        };
        group(0x291, ordinary); group(0x295, favorites); end(bytes, root);
        return bytes;
    }

    // 0061A120 supported groups. Safe transactional native decode; caller clears
    // before attempting open to preserve the original missing-file behavior.
    static MultiplayerServerLists decode(const unsigned char* data, std::size_t size) {
        using namespace multiplayer_archive;
        Reader file(data, size);
        std::uint16_t tag = 0;
        auto root = file.chunk(tag);
        if (tag != 0x28a) throw std::runtime_error("Invalid multiplayer server-list root");
        MultiplayerServerLists result;
        while (root.remaining()) {
            auto group = root.chunk(tag);
            if (tag != 0x291 && tag != 0x295) continue;
            auto& target = tag == 0x291 ? result.ordinary : result.favorites;
            while (group.remaining()) {
                auto record = group.chunk(tag);
                if (tag == 0x28d) { record.word(4); } // Reserve hint ignored; no untrusted allocation.
                else if (tag == 0x296) target.push_back(read_record(record));
                else if (tag != 0x294 && !(tag == 0x28f && &target == &result.ordinary)) break;
            }
        }
        return result;
    }
};

} // namespace hd2::frontend
#endif
