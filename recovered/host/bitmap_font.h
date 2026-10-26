#ifndef HD2_HOST_BITMAP_FONT_H
#define HD2_HOST_BITMAP_FONT_H

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace hd2::host {

struct GlyphMetrics {
    float u_left, v_top, u_right, v_bottom, aspect;
};

using FontMetrics = std::array<GlyphMetrics, 256>;

// Original little-endian TBL: 256 records of five float32s; UVs in [0,1],
// ordered corners and aspect in [0,16]. Invalid inputs throw invalid_argument.
FontMetrics decode_font_metrics(const std::vector<std::uint8_t>& bytes);

// H2MT, LE32 count (1..256), then LE32 ID/length and printable ASCII bytes.
// Labels are 1..4096 bytes; total input is at most 1 MiB, without trailing data.
std::map<std::uint32_t, std::string> decode_menu_labels(
    const std::vector<std::uint8_t>& bytes);

}

#endif
