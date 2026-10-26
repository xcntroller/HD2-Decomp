#include "scene/model_file_header.h"
#include "scene/model_first_table.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string_view>

namespace {

template <typename Bytes>
void print_hex(const Bytes& bytes) {
    std::cout << '"';
    for (const auto byte : bytes) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<unsigned int>(static_cast<unsigned char>(byte));
    }
    std::cout << std::dec << '"';
}

void print_optional_name(const std::optional<std::string>& name) {
    if (name) {
        print_hex(*name);
    } else {
        std::cout << "null";
    }
}

std::size_t record_size(const hd2::recovered::ModelTableRecord& record) {
    return 4 + record.opaque_payload.size() + 1 + record.primary_name_bytes.size()
        + (record.extra_name_bytes ? 1 + record.extra_name_bytes->size() : 0)
        + (record.secondary_name_bytes ? 1 + record.secondary_name_bytes->size() : 0)
        + (record.sequence ? 18 : 0);
}

void print_record(const hd2::recovered::ModelTableRecord& record, std::size_t offset) {
    std::cout << "{\"offset\":" << offset << ",\"end\":" << offset + record_size(record)
              << ",\"flags\":" << record.serialized_flags << ",\"payload_hex\":";
    print_hex(record.opaque_payload);
    std::cout << ",\"extra_name_hex\":";
    print_optional_name(record.extra_name_bytes);
    std::cout << ",\"primary_name_hex\":";
    print_hex(record.primary_name_bytes);
    std::cout << ",\"secondary_name_hex\":";
    print_optional_name(record.secondary_name_bytes);
    std::cout << ",\"sequence\":";
    if (record.sequence) {
        std::cout << "{\"counts\":[" << static_cast<unsigned int>(record.sequence->counts[0])
                  << ',' << static_cast<unsigned int>(record.sequence->counts[1])
                  << "],\"parameters_hex\":";
        print_hex(record.sequence->opaque_parameters);
        std::cout << '}';
    } else {
        std::cout << "null";
    }
    std::cout << '}';
}

}

int main(int argc, char** argv) {
    const bool details = argc == 3 && std::string_view(argv[2]) == "--records";
    if (argc != 2 && !details) {
        std::cerr << "Usage: scan_model_first_table model.4ds [--records]\n";
        return 2;
    }
    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
        std::cerr << "Cannot open the model file\n";
        return 1;
    }
    try {
        const auto header = hd2::recovered::read_model_file_header(input);
        const auto records = hd2::recovered::read_model_first_table(
            input, header.first_table_entry_count);
        std::size_t end = 16;
        std::size_t sequence_count = 0;
        std::size_t extra_count = 0;
        std::size_t secondary_count = 0;
        std::size_t max_name = 0;
        std::map<std::uint32_t, std::size_t> flags;
        for (const auto& record : records) {
            end += record_size(record);
            sequence_count += record.sequence.has_value();
            extra_count += record.extra_name_bytes.has_value();
            secondary_count += record.secondary_name_bytes.has_value();
            max_name = std::max(max_name, record.primary_name_bytes.size());
            if (record.extra_name_bytes) {
                max_name = std::max(max_name, record.extra_name_bytes->size());
            }
            if (record.secondary_name_bytes) {
                max_name = std::max(max_name, record.secondary_name_bytes->size());
            }
            ++flags[record.serialized_flags];
        }
        if (input.tellg() != static_cast<std::streamoff>(end)) {
            throw hd2::recovered::ModelTableReadError("Model table byte-count mismatch");
        }
        std::cout << "{\"header\":{\"version\":" << header.version
                  << ",\"opaque_metadata_hex\":";
        print_hex(header.opaque_metadata);
        std::cout << ",\"first_table_entry_count\":" << header.first_table_entry_count
                  << "},\"records\":" << records.size() << ",\"table_end\":" << end
                  << ",\"plain_records\":" << records.size() - sequence_count
                  << ",\"sequence_records\":" << sequence_count
                  << ",\"extra_name_fields\":" << extra_count
                  << ",\"secondary_name_fields\":" << secondary_count
                  << ",\"max_name_length\":" << max_name << ",\"flags\":{";
        bool first = true;
        for (const auto& [flag, count] : flags) {
            std::cout << (first ? "" : ",") << "\"0x" << std::hex << std::setw(8)
                      << std::setfill('0') << flag << "\":" << std::dec << count;
            first = false;
        }
        std::cout << '}';
        if (details) {
            std::cout << ",\"record_data\":[";
            std::size_t offset = 16;
            first = true;
            for (const auto& record : records) {
                std::cout << (first ? "" : ",");
                print_record(record, offset);
                offset += record_size(record);
                first = false;
            }
            std::cout << ']';
        }
        std::cout << "}\n";
    } catch (const hd2::recovered::ModelHeaderError& error) {
        std::cerr << "Model table header failed: " << error.what() << '\n';
        return 1;
    } catch (const hd2::recovered::ModelTableReadError& error) {
        std::cerr << "Model table scan failed: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
