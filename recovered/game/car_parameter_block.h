#ifndef HD2_GAME_CAR_PARAMETER_BLOCK_H
#define HD2_GAME_CAR_PARAMETER_BLOCK_H

#include "game/serialized_bytes.h"

namespace hd2::game {

//004D0110's COMP decode branch, used by004D55C0 before004D7D80 converts the
//packed parameter structure. This is a marker-byte RLE stream, not DTA LZSS.
//The encoder/editor branch and conversion/application of car physics parameters
//are separate bodies. A decoded block alone never establishes a loaded car.
inline std::vector<unsigned char> decode_car_parameter_block(SerializedBytes encoded) {
    if (encoded.word() != 0x504d4f43u) throw std::runtime_error("Invalid COMP parameter block");
    const auto decoded_size = encoded.word();
    const auto stored_size = encoded.word();
    const auto marker = encoded.byte();
    if (stored_size < 13 || stored_size - 13u > encoded.remaining() || decoded_size > 128u * 1024u * 1024u)
        throw std::runtime_error("Invalid COMP parameter block sizes");
    auto body = encoded.section(stored_size - 13u);
    std::vector<unsigned char> result;
    result.reserve(decoded_size);
    while (body.remaining()) {
        const auto first = body.byte();
        if (first != marker) {
            if (result.size() == decoded_size) throw std::runtime_error("COMP literal exceeds decoded size");
            result.push_back(first);
        } else {
            const auto value = body.byte();
            const auto count = body.byte();
            if (count > decoded_size - result.size()) throw std::runtime_error("COMP run exceeds decoded size");
            result.insert(result.end(), count, value);
        }
    }
    if (result.size() != decoded_size) throw std::runtime_error("COMP parameter block is truncated");
    return result;
}

//004D55C0's version3 outer framing. Later consumers must convert and apply the
//result through real vehicle initialization; retaining opaque attributes is not
//a successful replacement for those required consumers.
inline std::vector<unsigned char> read_car_parameter_block(SerializedBytes& properties) {
    if (properties.word() != 3) throw std::runtime_error("Unsupported Car actor properties version");
    const auto stored_size = properties.word();
    return decode_car_parameter_block(properties.section(stored_size));
}

} // namespace hd2::game
#endif
