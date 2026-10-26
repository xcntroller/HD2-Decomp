#include "io/chunk_reader.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>

using hd2::recovered::ChunkReadError;
using hd2::recovered::ChunkReader;

namespace {

void print_counts(const char* label, const std::map<std::uint16_t, std::size_t>& counts) {
    std::cout << ",\n  \"" << label << "\": {";
    bool first = true;
    for (const auto& [id, count] : counts) {
        std::cout << (first ? "\n" : ",\n") << "    \"0x"
                  << std::hex << std::setw(4) << std::setfill('0') << id
                  << "\": " << std::dec << count;
        first = false;
    }
    std::cout << (counts.empty() ? "}" : "\n  }");
}

}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: scan_scene_chunks scene2.bin\n";
        return 2;
    }
    try {
        std::ifstream input(argv[1], std::ios::binary);
        if (!input) {
            throw ChunkReadError("Cannot open the scene file");
        }
        ChunkReader reader(input);
        const auto root = reader.enter_chunk();
        if (!root || root->id != 0x4c53 || root->end_offset() != reader.file_size()) {
            throw ChunkReadError("Expected one 0x4c53 root covering the entire file");
        }
        std::size_t top_level_chunks = 0;
        std::size_t object_containers = 0;
        std::size_t object_records = 0;
        std::map<std::uint16_t, std::size_t> field_counts;
        std::map<std::uint16_t, std::size_t> unknown_record_kinds;
        while (const auto section = reader.enter_chunk()) {
            ++top_level_chunks;
            if (section->id == 0x4000) {
                ++object_containers;
                while (const auto record = reader.enter_chunk()) {
                    if (record->id == 0x4010) {
                        ++object_records;
                        while (const auto field = reader.enter_chunk()) {
                            ++field_counts[field->id];
                            reader.leave_chunk();
                        }
                    } else {
                        ++unknown_record_kinds[record->id];
                    }
                    reader.leave_chunk();
                }
            }
            reader.leave_chunk();
        }
        reader.leave_chunk();
        std::cout << "{\n  \"size\": " << reader.file_size()
                  << ",\n  \"top_level_chunks\": " << top_level_chunks
                  << ",\n  \"object_containers\": " << object_containers
                  << ",\n  \"object_records\": " << object_records;
        print_counts("record_field_counts", field_counts);
        print_counts("uninterpreted_record_kinds", unknown_record_kinds);
        std::cout << "\n}\n";
    } catch (const std::exception& error) {
        std::cerr << "Scene scan failed: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
