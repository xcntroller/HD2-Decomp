#include "host/bitmap_font.h"

#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>

namespace {

void append_u32(std::vector<std::uint8_t>& bytes, std::uint32_t value) {
    for (unsigned shift = 0; shift < 32; shift += 8) {
        bytes.push_back(static_cast<std::uint8_t>(value >> shift));
    }
}

void set_float(std::vector<std::uint8_t>& bytes, std::size_t offset, float value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    for (unsigned i = 0; i < 4; ++i) {
        bytes.at(offset + i) = static_cast<std::uint8_t>(bits >> (i * 8));
    }
}

std::vector<std::uint8_t> labels(std::uint32_t id = 2101, const std::string& text = "Menu") {
    std::vector<std::uint8_t> bytes{'H', '2', 'M', 'T'};
    append_u32(bytes, 1);
    append_u32(bytes, id);
    append_u32(bytes, static_cast<std::uint32_t>(text.size()));
    bytes.insert(bytes.end(), text.begin(), text.end());
    return bytes;
}

template<class F> void rejects(F operation) {
    bool rejected = false;
    try {
        operation();
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    assert(rejected);
}

std::vector<std::uint8_t> read_file(const std::string& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        throw std::runtime_error("Cannot read test asset: " + path);
    }
    return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}

}

int main(int argc, char** argv) {
    using namespace hd2::host;
    std::vector<std::uint8_t> bytes(5120);
    const std::size_t offset = 'A' * 20;
    set_float(bytes, offset, 0.125f);
    set_float(bytes, offset + 4, 0.25f);
    set_float(bytes, offset + 8, 0.5f);
    set_float(bytes, offset + 12, 0.75f);
    set_float(bytes, offset + 16, 0.375f);
    const auto decoded = decode_font_metrics(bytes);
    assert(decoded[0].aspect == 0);
    assert(decoded['A'].u_left == 0.125f && decoded['A'].v_top == 0.25f);
    assert(decoded['A'].u_right == 0.5f && decoded['A'].v_bottom == 0.75f);
    assert(decoded['A'].aspect == 0.375f);
    for (std::size_t size : {0u, 5119u, 5121u}) {
        rejects([&] { decode_font_metrics(std::vector<std::uint8_t>(size)); });
    }
    for (std::size_t field = 0; field < 5; ++field) {
        for (float value : {std::numeric_limits<float>::infinity(),
                            -std::numeric_limits<float>::infinity(),
                            std::numeric_limits<float>::quiet_NaN(), -0.1f}) {
            auto bad = bytes;
            set_float(bad, offset + field * 4, value);
            rejects([&] { decode_font_metrics(bad); });
        }
    }
    for (std::size_t field = 0; field < 4; ++field) {
        auto bad = bytes;
        set_float(bad, offset + field * 4, 1.1f);
        rejects([&] { decode_font_metrics(bad); });
    }
    for (auto field : {0u, 1u}) {
        auto bad = bytes;
        set_float(bad, offset + field * 4, 0.9f);
        rejects([&] { decode_font_metrics(bad); });
    }
    auto bad = bytes;
    set_float(bad, offset + 16, 16.1f);
    rejects([&] { decode_font_metrics(bad); });

    const auto valid = labels();
    assert(decode_menu_labels(valid).at(2101) == "Menu");
    for (std::size_t length = 0; length < valid.size(); ++length) {
        const std::vector<std::uint8_t> truncated(valid.begin(), valid.begin() + length);
        rejects([&] { decode_menu_labels(truncated); });
    }
    for (const auto& text : {std::string(), std::string("a\0b", 3),
                            std::string("\x80"), std::string("\n"), std::string(4097, 'x')}) {
        rejects([&] { decode_menu_labels(labels(2101, text)); });
    }
    assert(decode_menu_labels(labels(1, std::string(4096, 'x'))).at(1).size() == 4096);
    auto malformed = valid;
    malformed.push_back(0);
    rejects([&] { decode_menu_labels(malformed); });
    malformed = valid;
    malformed[0] = 'X';
    rejects([&] { decode_menu_labels(malformed); });
    for (std::uint32_t count : {0u, 257u, 0xffffffffu}) {
        malformed = {'H', '2', 'M', 'T'};
        append_u32(malformed, count);
        rejects([&] { decode_menu_labels(malformed); });
    }
    malformed = valid;
    malformed[4] = 2;
    malformed.insert(malformed.end(), valid.begin() + 8, valid.end());
    rejects([&] { decode_menu_labels(malformed); });
    malformed = valid;
    for (std::size_t i = 12; i < 16; ++i) {
        malformed[i] = 0xff;
    }
    rejects([&] { decode_menu_labels(malformed); });
    malformed.assign(1024 * 1024 + 1, 0);
    rejects([&] { decode_menu_labels(malformed); });

    // Optional original/staged paths keep synthetic tests independent of game data.
    if (argc > 1) {
        const std::string root = argv[1];
        for (const auto* name : {"BIG", "SMALL"}) {
            const auto font = decode_font_metrics(read_file(
                root + "/Text/Fonts/ANSI/Set2/" + name + ".tbl"));
            assert(font['A'].aspect > 0 && font[' '].aspect > 0);
            assert(font['A'].u_right > font['A'].u_left);
        }
    }
    if (argc > 2) {
        const auto original = decode_menu_labels(read_file(argv[2]));
        assert(original.size() == 13 && original.at(2101) == "MAIN MENU");
        assert(original.count(2116) == 1);
    }
    std::cout << "Bitmap font and menu label decoding, bounds and malformed inputs passed\n";
}
