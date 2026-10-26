#include "scene/model_first_table.h"
#include "scene/model_file_header.h"

#include <cstddef>
#include <utility>

namespace hd2::recovered {
namespace {

void read_field(std::istream& input, void* destination, std::size_t size) {
    input.read(static_cast<char*>(destination), static_cast<std::streamsize>(size));
    if (!input) {
        throw ModelTableReadError("Incomplete model first-table field");
    }
}

std::string read_name_bytes(std::istream& input) {
    std::uint8_t size = 0;
    read_field(input, &size, 1);
    std::string name(size, '\0');
    if (size != 0) {
        read_field(input, name.data(), size);
    }
    return name;
}

std::uint32_t read_flags(std::istream& input) {
    std::array<std::uint8_t, 4> bytes{};
    read_field(input, bytes.data(), bytes.size());
    std::uint32_t flags = 0;
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        flags |= static_cast<std::uint32_t>(bytes[index]) << (index * 8);
    }
    return flags;
}

}

std::vector<ModelTableRecord> read_model_first_table(std::istream& input, std::uint16_t count) {
    if (count > max_model_first_table_entries) {
        throw ModelTableReadError("Model first-table count exceeds 6000");
    }
    std::vector<ModelTableRecord> records;
    records.reserve(count);
    for (std::uint16_t index = 0; index < count; ++index) {
        ModelTableRecord record{};
        record.serialized_flags = read_flags(input);
        const bool has_extra = (record.serialized_flags & 0x00080000U) != 0;
        record.opaque_payload.resize(has_extra ? 76 : 72);
        read_field(input, record.opaque_payload.data(), record.opaque_payload.size());
        if (has_extra) {
            record.extra_name_bytes = read_name_bytes(input);
        }
        record.primary_name_bytes = read_name_bytes(input);
        if (!record.primary_name_bytes.empty()
            && (record.serialized_flags & 0x40000000U) != 0
            && (record.serialized_flags & 0xa1000000U) == 0) {
            record.secondary_name_bytes = read_name_bytes(input);
        }
        if ((record.serialized_flags & 0x04000000U) != 0) {
            ModelRecordSequence sequence{};
            read_field(input, sequence.counts.data(), sequence.counts.size());
            read_field(input, sequence.opaque_parameters.data(), sequence.opaque_parameters.size());
            record.sequence = sequence;
        }
        records.push_back(std::move(record));
    }
    return records;
}

}
