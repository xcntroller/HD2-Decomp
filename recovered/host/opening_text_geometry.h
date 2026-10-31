#ifndef HD2_HOST_OPENING_TEXT_GEOMETRY_H
#define HD2_HOST_OPENING_TEXT_GEOMETRY_H

#include "host/menu_text_geometry.h"
#include <cstring>

namespace hd2::host {

// Seven floats copied by00644B80. Native fields replace original text-object ABI.
struct OpeningTextRectangle {
    float left, bottom, right, top, width, height, depth;
};
inline OpeningTextRectangle opening_text_rectangle(int width, int height) {
    if (width <= 0 || height <= 0)
        throw std::invalid_argument("Invalid opening text surface");
    const long double left = static_cast<long double>(width) * .125;
    OpeningTextRectangle result{};
    result.left = static_cast<float>(left);
    result.right = static_cast<float>(width - left);
    result.bottom = static_cast<float>(static_cast<long double>(height) * .98);
    result.top = static_cast<float>(static_cast<long double>(height) * .82);
    result.width = result.right - result.left;
    result.height = result.bottom - result.top;
    const std::uint32_t depth = 0x3f43acc5;
    std::memcpy(&result.depth, &depth, 4);
    return result;
}

// 00641E30 style8 +00646820/00646A10/00648860/006404C0, bounded printable English
//  consumer with single-style word wrapping for Cinematics subtitles.
inline std::vector<CursorVertex> opening_subtitle_vertices(const frontend::TextObject &object,
                                                           const FontMetrics &metrics,
                                                           const OpeningTextRectangle &rectangle,
                                                           const frontend::TextStyle &style,
                                                           int surface_width, int surface_height) {
    if (object.records().empty())
        return {};
    if (object.records().size() != 1 || style.font != 3 || style.horizontal != 2 || style.vertical != 1)
        throw std::logic_error("Unsupported opening subtitle paragraph/style");
    const auto &record = object.records().front();
    if (record.style != 8)
        throw std::logic_error("Unsupported opening subtitle style ID");
    if (!record.text || record.text->empty())
        return {};
    const auto &text = *record.text;
    if (text.size() > 4096 || surface_width <= 0 || surface_height <= 0)
        throw std::invalid_argument("Invalid opening subtitle geometry");
    const auto glyph_height = static_cast<int>(static_cast<long double>(surface_height) * style.height + .5f);
    // 00645F20 style8 has +2d=1: retain the last ASCII whitespace and
    //  wrap before it when the next glyph exceeds the actual rectangle width.
    //  The installed movie subtitles use words narrower than that interval.
    const int available = static_cast<int>(rectangle.width);
    struct Line {
        std::string_view text;
        int width;
    };
    std::vector<Line> lines;
    std::string_view remaining = text;
    while (true) {
        int width = 0, space_width = 0;
        std::size_t space = std::string_view::npos, index = 0;
        for (; index < remaining.size(); ++index) {
            const auto character = static_cast<unsigned char>(remaining[index]);
            if (character < 32 || character > 126)
                throw std::logic_error("Movie subtitles require printable English ASCII");
            const auto advance = glyph_advance(metrics[character], glyph_height);
            if (width + advance > available)
                break;
            if (character == ' ') {
                space = index;
                space_width = width;
            }
            width += advance;
        }
        if (index == remaining.size()) {
            lines.push_back({remaining, width});
            break;
        }
        if (space == std::string_view::npos || space == 0)
            throw std::logic_error("Movie subtitle contains a word wider than its rectangle");
        lines.push_back({remaining.substr(0, space), space_width});
        remaining.remove_prefix(space + 1);
    }
    if (lines.size() * static_cast<std::size_t>(glyph_height) > rectangle.height)
        throw std::logic_error("Movie subtitle exceeds its text rectangle");
    const float rhw = 1.0f / rectangle.depth;
    std::vector<CursorVertex> vertices;
    vertices.reserve(text.size() * 6);
    unsigned row = 0;
    for (const auto &line : lines) {
        float x = static_cast<float>(static_cast<int>(rectangle.left + (available - line.width) / 2));
        const float bottom = static_cast<float>(static_cast<int>(rectangle.top) + (++row) * glyph_height);
        for (unsigned char character : line.text) {

            const auto &metric = metrics[character];
            const auto advance = glyph_advance(metric, glyph_height);
            if (metric.aspect != 0) {
                const CursorVertex bl{x,           bottom, rectangle.depth, rhw,
                                      style.color, 0,      metric.u_left,   metric.v_bottom};
                const CursorVertex tl{x, bottom - glyph_height, rectangle.depth, rhw, style.color,
                                      0, metric.u_left,         metric.v_top};
                const CursorVertex tr{x + advance, bottom - glyph_height, rectangle.depth, rhw, style.color,
                                      0,           metric.u_right,        metric.v_top};
                const CursorVertex br{x + advance, bottom, rectangle.depth, rhw,
                                      style.color, 0,      metric.u_right,  metric.v_bottom};
                vertices.insert(vertices.end(), {bl, tl, tr, br, bl, tr});
            }
            x += advance;
        }
    }
    return vertices;
}

} // namespace hd2::host
#endif
