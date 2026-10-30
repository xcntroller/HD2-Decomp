#ifndef HD2_HOST_OPENING_TEXT_GEOMETRY_H
#define HD2_HOST_OPENING_TEXT_GEOMETRY_H

#include "host/menu_text_geometry.h"
#include <cstring>

namespace hd2::host {

// Seven floats copied by00644B80. Native fields replace original text-object ABI.
struct OpeningTextRectangle { float left, bottom, right, top, width, height, depth; };
inline OpeningTextRectangle opening_text_rectangle(int width, int height) {
    if (width <= 0 || height <= 0) throw std::invalid_argument("Invalid opening text surface");
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

//00641E30 style8 +00646820/00646A10/00648860/006404C0, bounded ASCII single-line
// consumer. Actual installed opening strings fit; unsupported wrapping rejects.
inline std::vector<CursorVertex> opening_subtitle_vertices(
        const frontend::TextObject& object, const FontMetrics& metrics,
        const OpeningTextRectangle& rectangle, const frontend::TextStyle& style,
        int surface_width, int surface_height) {
    if (object.records().empty()) return {};
    if (object.records().size() != 1 || style.font != 3 || style.horizontal != 2 || style.vertical != 1)
        throw std::logic_error("Unsupported opening subtitle paragraph/style");
    const auto& record = object.records().front();
    if (record.style != 8) throw std::logic_error("Unsupported opening subtitle style ID");
    if (!record.text || record.text->empty()) return {};
    const auto& text = *record.text;
    if (text.size() > 4096 || surface_width <= 0 || surface_height <= 0)
        throw std::invalid_argument("Invalid opening subtitle geometry");
    const auto glyph_height = static_cast<int>(static_cast<long double>(surface_height) * style.height + .5f);
    const auto padding = static_cast<int>(static_cast<long double>(surface_width) * (1.0 / 1024) * object.inset);
    int line_width = 0;
    for (unsigned char character : text) {
        if (character < 32 || character > 126) throw std::logic_error("Opening subtitles require printable English ASCII");
        line_width += glyph_advance(metrics[character], glyph_height);
    }
    if (line_width > rectangle.width - 2 * padding || glyph_height > rectangle.height)
        throw std::logic_error("Opening subtitle requires unreconstructed paragraph wrapping");
    // Centering uses integer division of the remaining line width before ftol.
    const auto available = static_cast<int>(rectangle.width) - 2 * padding;
    float x = static_cast<float>(static_cast<int>(rectangle.left + padding + (available - line_width) / 2));
    const float bottom = static_cast<float>(static_cast<int>(rectangle.top) + glyph_height);
    const float rhw = 1.0f / rectangle.depth;
    std::vector<CursorVertex> vertices;
    vertices.reserve(text.size() * 6);
    for (unsigned char character : text) {
        const auto& metric = metrics[character];
        const auto advance = glyph_advance(metric, glyph_height);
        if (metric.aspect != 0) {
            const CursorVertex bl{x, bottom, rectangle.depth, rhw, style.color, 0, metric.u_left, metric.v_bottom};
            const CursorVertex tl{x, bottom - glyph_height, rectangle.depth, rhw, style.color, 0, metric.u_left, metric.v_top};
            const CursorVertex tr{x + advance, bottom - glyph_height, rectangle.depth, rhw, style.color, 0, metric.u_right, metric.v_top};
            const CursorVertex br{x + advance, bottom, rectangle.depth, rhw, style.color, 0, metric.u_right, metric.v_bottom};
            vertices.insert(vertices.end(), {bl, tl, tr, br, bl, tr});
        }
        x += advance;
    }
    return vertices;
}

} // namespace hd2::host
#endif
