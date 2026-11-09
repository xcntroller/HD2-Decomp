#ifndef HD2_GAME_CAR_PARAMETERS_H
#define HD2_GAME_CAR_PARAMETERS_H

#include "game/car_parameter_block.h"
#include <array>
#include <bitset>

namespace hd2::game {

// Native representation of004D7D80's runtime (third argument zero) output.
// This pointer-free value preserves overlapping inline-name writes as well as
// scalar fields. In particular Tutorial's Jeep has a27-byte name at source8F7
// which is partly overwritten by a later name copy. Separate strings lose that
// behavior. The defined-byte mask keeps unwritten padding out of real consumers.
class CarParameters {
public:
    std::uint32_t word(std::size_t index) const {
        std::array<unsigned char, 4> value{};
        for (std::size_t i = 0; i != value.size(); ++i) value[i] = flag(index * 4 + i);
        return SerializedBytes(value.data(), value.size()).word();
    }
    float real(std::size_t index) const {
        const auto bits = word(index);
        float value;
        std::memcpy(&value, &bits, sizeof(value));
        return value;
    }
    std::uint8_t flag(std::size_t byte_offset) const {
        if (!defined_.test(byte_offset)) throw std::runtime_error("Uninitialized Car parameter byte");
        return bytes_.at(byte_offset);
    }
    std::string name(std::size_t word_index) const {
        std::string result;
        for (auto offset = word_index * 4; ; ++offset) {
            const auto value = flag(offset);
            if (!value) return result;
            result.push_back(static_cast<char>(value));
        }
    }
private:
    friend CarParameters convert_car_parameters(SerializedBytes);
    friend CarParameters default_car_parameters(std::size_t);
    //Highest written byte is123C. This is the parameter value, not a raw actor
    //object or a guessed stack allocation, and contains no engine/client pointer.
    std::array<unsigned char, 0x123d> bytes_{};
    std::bitset<0x123d> defined_;
    void put(std::size_t offset, const unsigned char* bytes, std::size_t count) {
        if (offset > bytes_.size() || count > bytes_.size() - offset)
            throw std::runtime_error("Car parameter copy exceeds its destination");
        for (std::size_t i = 0; i != count; ++i) {
            bytes_[offset + i] = bytes[i];
            defined_.set(offset + i);
        }
    }
};

inline CarParameters convert_car_parameters(SerializedBytes packed) {
    //The serialized structure has four trailing bytes not read by004D7D80.
    //All actual loads pass a4687-byte decoded block. Do not read native padding
    //or reproduce lstrcpy writes outside the parameter value on malformed names.
    const auto size = packed.remaining();
    if (size < 0x124b) throw std::runtime_error("Truncated packed Car parameters");
    const auto* data = packed.take(size);
    CarParameters result;
    auto source = [&](std::size_t offset, std::size_t count) {
        if (offset > size || count > size - offset)
            throw std::runtime_error("Truncated packed Car parameter field");
        return SerializedBytes(data + offset, count);
    };
    auto words = [&](std::size_t offset, std::size_t index, std::size_t count) {
        auto bytes = source(offset, count * 4);
        result.put(index * 4, bytes.take(count * 4), count * 4);
    };
    auto flag = [&](std::size_t offset, std::size_t destination) {
        result.put(destination, source(offset, 1).take(1), 1);
    };
    auto name = [&](std::size_t offset, std::size_t destination) {
        const auto text = source(offset, size - offset).text();
        result.put(destination * 4, reinterpret_cast<const unsigned char*>(text.c_str()), text.size() + 1);
    };

    words(0, 0, 20);
    for (std::size_t row = 0; row != 16; ++row) {
        for (std::size_t column = 0; column != 3; ++column)
            words(0x50 + row * 12 + column * 4, 0x14 + row + column * 16, 1);
    }
    struct ScalarRange { std::size_t source, destination, count; };
    constexpr ScalarRange ranges[] = {
        {0x69c,0x19b,7}, {0x6b9,0x1a2,3},
        {0x6c8,0x1a5,1}, {0x6cd,0x1a7,11}, {0x6fa,0x1b2,25},
        {0x76e,0x1cb,21}, {0x7e2,0x1e8,50}
    };
    words(0x110,0x44,19);

    //Twelve packed112-byte records become columns with stride12. The editor
    //index-to-ID remap is deliberately excluded from this runtime entry point.
    for (std::size_t reverse = 12; reverse; --reverse) {
        const auto row = reverse - 1;
        const auto base = 0x15c + row * 112;
        for (std::size_t column = 0; column != 22; ++column)
            words(base + column * 4, 0x57 + row + column * 12, 1);
        flag(base + 88, 0x11ec + row);
        flag(base + 89, 0x11f8 + row);
        flag(base + 90, 0x1204 + row);
        for (std::size_t column = 0; column != 5; ++column)
            words(base + 91 + column * 4, 0x15f + row + column * 12, 1);
        flag(base + 111, 0x1210 + row);
    }
    for (const auto& range : ranges) words(range.source, range.destination, range.count);
    flag(0x6b8,0x121c);
    flag(0x6c5,0x121d); flag(0x6c6,0x121e); flag(0x6c7,0x121f);
    flag(0x6cc,0x698); //Only one byte of native word1A6 is initialized.
    flag(0x6f9,0x1220);

    struct NamedGroup {
        std::size_t source, stride, names, scalars, flags;
    };
    constexpr NamedGroup groups[] = {
        {0x907,49,0x231,0x2c5,0x1221}, {0x9fc,49,0x245,0x2ed,0x1226},
        {0xaf1,48,0x259,0x315,0}, {0xdc1,49,0x358,0x3f0,0x122f},
        {0xeb6,49,0x36c,0x418,0x1234}, {0xfab,48,0x380,0x440,0}
    };
    auto named_group = [&](const NamedGroup& group) {
        for (std::size_t reverse = 5; reverse; --reverse) {
            const auto row = reverse - 1;
            const auto base = group.source + row * group.stride;
            name(base, group.names + row * 4);
            for (std::size_t column = 0; column != 8; ++column)
                words(base + 16 + column * 4, group.scalars + row + column * 5, 1);
            if (group.flags) flag(base + 48, group.flags + row);
        }
    };
    struct PairGroup { std::size_t source, first, second, scalars, flags; };
    constexpr PairGroup pairs[] = {
        {0xcbd,0x29d,0x2ad,0x344,0x122b}, {0x1197,0x3c4,0x3d4,0x477,0x1239}
    };
    auto pair_group = [&](const PairGroup& group) {
        for (std::size_t reverse = 4; reverse; --reverse) {
            const auto row = reverse - 1;
            const auto base = group.source + row * 37;
            name(base, group.first + row * 4);
            name(base + 16, group.second + row * 4);
            words(base + 32, group.scalars + row, 1);
            flag(base + 36, group.flags + row);
        }
    };
    struct Name { std::size_t source, destination; };
    constexpr Name names[] = {
        {0x7c2,0x1e0},{0x7d2,0x1e4},{0x8d7,0x225},{0x8e7,0x229},{0x8f7,0x22d},
        {0xbe1,0x26d},{0xbf9,0x271},{0xc1d,0x275},{0xc2d,0x279},{0xc3d,0x27d},
        {0xc4d,0x281},{0xc5d,0x285},{0xc6d,0x289},{0xc7d,0x28d},{0xc8d,0x291},
        {0xc9d,0x295},{0xcad,0x299},{0xd51,0x2bd},{0xd61,0x2c1},{0xd71,0x348},
        {0xd81,0x34c},{0xd91,0x350},{0xda1,0x354},{0x109b,0x394},{0x10c3,0x398},
        {0x10f7,0x39c},{0x1107,0x3a0},{0x1117,0x3a4},{0x1127,0x3a8},{0x1137,0x3ac},
        {0x1147,0x3b0},{0x1157,0x3b4},{0x1167,0x3b8},{0x1177,0x3bc},{0x1187,0x3c0},
        {0x122b,0x3e4},{0x123b,0x3e8}
    };
    //Keep original copy order: inline names may overlap their neighboring fields.
    for (std::size_t i = 0; i != 2; ++i) name(names[i].source, names[i].destination);
    words(0x8aa,0x21a,11);
    for (std::size_t i = 2; i != 17; ++i) name(names[i].source, names[i].destination);
    for (std::size_t i = 0; i != 3; ++i) named_group(groups[i]);
    words(0xbf1,0x33d,2); words(0xc09,0x33f,5);
    pair_group(pairs[0]);
    for (std::size_t i = 17; i != 35; ++i) name(names[i].source, names[i].destination);
    words(0xdb1,0x3ec,4);
    for (std::size_t i = 3; i != 6; ++i) named_group(groups[i]);
    words(0x10ab,0x468,6); words(0x10d3,0x46e,9);
    pair_group(pairs[1]);
    for (std::size_t i = 35; i != 37; ++i) name(names[i].source, names[i].destination);
    return result;
}

//004D55C0's decode/conversion portion only. A real loaded actor also requires
//004D10A0/004E2E30 geometry and004ED270 parameter application to that geometry.
inline CarParameters read_car_parameters(SerializedBytes& properties) {
    const auto packed = read_car_parameter_block(properties);
    return convert_car_parameters(SerializedBytes(packed));
}

} // namespace hd2::game
#endif
