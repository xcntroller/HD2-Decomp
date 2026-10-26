#pragma once

#include <array>
#include <cstdint>
#include <istream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace hd2::recovered {

class ModelTableReadError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct ModelRecordSequence {
    std::array<std::uint8_t, 2> counts;
    std::array<std::uint8_t, 16> opaque_parameters;
};

struct ModelTableRecord {
    std::uint32_t serialized_flags;
    std::vector<std::uint8_t> opaque_payload;
    std::optional<std::string> extra_name_bytes;
    std::string primary_name_bytes;
    std::optional<std::string> secondary_name_bytes;
    std::optional<ModelRecordSequence> sequence;
};

// Read exactly count records at the current position; names are raw length-prefixed bytes.
std::vector<ModelTableRecord> read_model_first_table(std::istream& input, std::uint16_t count);

}
