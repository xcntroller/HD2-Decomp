#include "scene/model_file_header.h"

#include <fstream>
#include <iomanip>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: inspect_model_header model.4ds\n";
        return 2;
    }
    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
        std::cerr << "Cannot open the model file\n";
        return 1;
    }
    try {
        const auto header = hd2::recovered::read_model_file_header(input);
        std::cout << "{\"version\":" << header.version << ",\"opaque_metadata_hex\":\"";
        for (const auto byte : header.opaque_metadata) {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<unsigned int>(byte);
        }
        std::cout << "\",\"first_table_entry_count\":" << std::dec
                  << header.first_table_entry_count << "}\n";
    } catch (const hd2::recovered::ModelHeaderError& error) {
        std::cerr << "Model header inspection failed: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
