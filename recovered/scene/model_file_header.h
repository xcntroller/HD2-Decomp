#pragma once

#include <array>
#include <cstdint>
#include <istream>
#include <stdexcept>
#include <string>

namespace hd2::recovered {

enum class ModelHeaderFailure {
    incomplete_read,
    invalid_signature,
    unsupported_version,
    entry_limit_exceeded,
};

class ModelHeaderError : public std::runtime_error {
public:
    ModelHeaderError(ModelHeaderFailure reason, const std::string& message);
    ModelHeaderFailure reason() const noexcept;

private:
    ModelHeaderFailure reason_;
};

struct ModelFileHeader {
    std::uint16_t version;
    std::array<std::uint8_t, 8> opaque_metadata;
    std::uint16_t first_table_entry_count;
};

// Read only the 16-byte prefix at the current stream position, not its table records.
ModelFileHeader read_model_file_header(std::istream& input);

}
