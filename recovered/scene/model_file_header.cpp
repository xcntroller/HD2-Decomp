#include "scene/model_file_header.h"

#include <cstddef>

namespace hd2::recovered {
namespace {

template <std::size_t Size>
std::array<char, Size> read_exact(std::istream& input) {
    std::array<char, Size> bytes{};
    input.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    if (!input) {
        throw ModelHeaderError(ModelHeaderFailure::incomplete_read,
                               "Cannot read the complete model header field");
    }
    return bytes;
}

std::uint16_t read_u16(std::istream& input) {
    const auto bytes = read_exact<2>(input);
    return static_cast<std::uint16_t>(
        static_cast<unsigned char>(bytes[0])
        | (static_cast<unsigned char>(bytes[1]) << 8));
}

}

ModelHeaderError::ModelHeaderError(ModelHeaderFailure reason, const std::string& message)
    : std::runtime_error(message), reason_(reason) {}

ModelHeaderFailure ModelHeaderError::reason() const noexcept {
    return reason_;
}

ModelFileHeader read_model_file_header(std::istream& input) {
    if (read_exact<4>(input) != std::array<char, 4>{'4', 'D', 'S', '\0'}) {
        throw ModelHeaderError(ModelHeaderFailure::invalid_signature,
                               "Expected the four-byte 4DS signature");
    }
    const auto version = read_u16(input);
    if (version != 0x29) {
        throw ModelHeaderError(ModelHeaderFailure::unsupported_version,
                               "Only model format version 0x29 is supported");
    }
    ModelFileHeader result{version, {}, 0};
    const auto metadata = read_exact<8>(input);
    for (std::size_t index = 0; index < metadata.size(); ++index) {
        result.opaque_metadata[index] = static_cast<std::uint8_t>(metadata[index]);
    }
    result.first_table_entry_count = read_u16(input);
    if (result.first_table_entry_count > max_model_first_table_entries) {
        throw ModelHeaderError(ModelHeaderFailure::entry_limit_exceeded,
                               "Model first-table count exceeds 6000");
    }
    return result;
}

}
