#ifndef HD2_GAME_SERIALIZED_BYTES_H
#define HD2_GAME_SERIALIZED_BYTES_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>
#include <optional>

namespace hd2::game {

// Bounded native reads for client mission serializers. The file's DWORDs are
// always little endian; neither host pointer width nor native struct padding
// participates in serialization.
class SerializedBytes {
public:
    SerializedBytes(const unsigned char* data, std::size_t size) : data_(data), size_(size) {
        if (size && !data) throw std::invalid_argument("Null mission data");
    }
    explicit SerializedBytes(const std::vector<unsigned char>& data) : SerializedBytes(data.data(), data.size()) {}
    std::size_t remaining() const { return size_ - offset_; }
    std::size_t position() const { return offset_; }
    const unsigned char* take(std::size_t count) {
        if (count > remaining()) throw std::runtime_error("Truncated mission data");
        const auto* result = data_ ? data_ + offset_ : nullptr;
        offset_ += count;
        return result;
    }
    SerializedBytes section(std::size_t count) { return {take(count), count}; }
    std::uint8_t byte() { return *take(1); }
    std::uint16_t half() {
        const auto* p = take(2);
        return static_cast<std::uint16_t>(p[0] | (std::uint16_t(p[1]) << 8));
    }
    std::uint32_t word() {
        const auto* p = take(4);
        return std::uint32_t(p[0]) | (std::uint32_t(p[1]) << 8) |
            (std::uint32_t(p[2]) << 16) | (std::uint32_t(p[3]) << 24);
    }
    float real() {
        const auto bits = word();
        float result;
        static_assert(sizeof(result) == sizeof(bits));
        std::memcpy(&result, &bits, sizeof(result));
        return result;
    }
    std::string text() {
        const auto* first = data_ ? data_ + offset_ : nullptr;
        const auto* end = remaining() ? static_cast<const unsigned char*>(std::memchr(first, 0, remaining())) : nullptr;
        if (!end) throw std::runtime_error("Unterminated mission reference name");
        const auto length = static_cast<std::size_t>(end - first);
        offset_ += length + 1;
        return {reinterpret_cast<const char*>(first), length};
    }
    void exact_end() const {
        if (remaining()) throw std::runtime_error("Mission record has an inconsistent serialized length");
    }
private:
    const unsigned char* data_;
    std::size_t size_, offset_ = 0;
};

struct MissionChunk {
    std::uint16_t id;
    SerializedBytes bytes;
};
inline std::optional<MissionChunk> next_mission_chunk(SerializedBytes& input) {
    if (!input.remaining()) return std::nullopt;
    const auto id = input.half();
    const auto size = input.word();
    if (size < 6) throw std::runtime_error("Mission chunk is smaller than its header");
    return MissionChunk{id, input.section(size - 6u)};
}

} // namespace hd2::game
#endif
