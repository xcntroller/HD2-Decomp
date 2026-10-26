#ifndef HD2_HOST_MENU_TEXT_GEOMETRY_H
#define HD2_HOST_MENU_TEXT_GEOMETRY_H

#include "host/bitmap_font.h"
#include "host/cursor_geometry.h"

#include <algorithm>
#include <cmath>
#include <string_view>
#include <vector>

namespace hd2::host {

struct MenuRectangle {
    float left, top, right, bottom;

    bool contains(CursorPoint point) const {
        return point.x >= left && point.x < right && point.y >= top && point.y < bottom;
    }
};

inline MenuRectangle menu_rectangle(const std::array<float, 8>& bounds, int width, int height) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Menu projection requires positive dimensions");
    }
    for (const auto index : {0u, 1u, 4u, 5u}) {
        if (!std::isfinite(bounds[index])) {
            throw std::invalid_argument("Nonfinite original menu bounds");
        }
    }
    if (bounds[0] >= bounds[4] || bounds[1] >= bounds[5]) {
        throw std::invalid_argument("Empty or inverted original menu bounds");
    }
    return {width * bounds[0], 1.0f - height * bounds[5] * (4.0f / 3.0f),
            width * bounds[4], -height * bounds[1] * (4.0f / 3.0f) - 1.0f};
}

struct MenuLabelBinding {
    const char* frame;
    const char* visual;
    std::uint32_t id;
    float height;
    bool small;
    bool button;
};

inline constexpr std::array<MenuLabelBinding, 13> menu_label_bindings{{
    {"text_screen", "text_screen", 2101, .05f, false, false},
    {"text_date00", "text_date00", 2102, .02f, true, false},
    {"text_profile00", "text_profile00", 2103, .02f, true, false},
    {"text_last save00", "text_last save00", 2104, .02f, true, false},
    {"bresume game", "normal00", 2105, .04f, false, true},
    {"bchange profile", "normal01", 2106, .04f, false, true},
    {"bsingleplayer", "normal02", 2107, .04f, false, true},
    {"bcinematics", "normal04", 2109, .04f, false, true},
    {"boptions", "normal05", 2110, .04f, false, true},
    {"bcredits", "normal06", 2111, .04f, false, true},
    {"bexit", "normal07", 2112, .04f, false, true},
    {"bmultiplayerlan", "normal03", 2115, .04f, false, true},
    {"bmultiplayernet", "normal08", 2116, .04f, false, true},
}};

inline int glyph_advance(const GlyphMetrics& metric, int height) {
    const double aspect = metric.aspect == 0 ? .4 : static_cast<double>(metric.aspect);
    return static_cast<int>(height * aspect + .5);
}

inline std::vector<CursorVertex> menu_label_vertices(
        std::string_view text, const FontMetrics& metrics, const MenuRectangle& rectangle,
        float normalized_height, int render_height, bool title, std::uint32_t color) {
    if (render_height <= 0 || !std::isfinite(normalized_height) ||
            normalized_height <= 0 || normalized_height > 1 || text.size() > 4096) {
        throw std::invalid_argument("Invalid bounded menu label dimensions");
    }
    const int height = static_cast<int>(static_cast<double>(render_height) * normalized_height + .5);
    int line_width = 0;
    for (unsigned char character : text) {
        if (character < 32 || character > 126) {
            throw std::invalid_argument("Static English menu labels require printable ASCII");
        }
        line_width += glyph_advance(metrics[character], height);
    }
    // Styles 11/13 are left/vertically centered; style 10 is right/bottom aligned.
    float x = static_cast<float>(static_cast<int>(
        title ? rectangle.right - line_width : rectangle.left));
    const float bottom = static_cast<float>(static_cast<int>(title ? rectangle.bottom :
        rectangle.top + std::max(0.0f, (rectangle.bottom - rectangle.top - height) * .5f)) +
        (title ? 0 : height));
    std::vector<CursorVertex> result;
    result.reserve(text.size() * 6);
    for (unsigned char character : text) {
        const auto& metric = metrics[character];
        const int advance = glyph_advance(metric, height);
        if (metric.aspect != 0) {
            constexpr float z = 0x1.d17e0cp-6f;
            constexpr float rhw = 0x1.1993bep+5f;
            const CursorVertex bl{x, bottom, z, rhw, color, 0, metric.u_left, metric.v_bottom};
            const CursorVertex tl{x, bottom - height, z, rhw, color, 0, metric.u_left, metric.v_top};
            const CursorVertex tr{x + advance, bottom - height, z, rhw, color, 0,
                                  metric.u_right, metric.v_top};
            const CursorVertex br{x + advance, bottom, z, rhw, color, 0,
                                  metric.u_right, metric.v_bottom};
            result.insert(result.end(), {bl, tl, tr, br, bl, tr});
        }
        x += advance;
    }
    return result;
}

class ExitButton {
public:
    bool release(bool inside) {
        const bool activated = pressed_ && inside;
        pressed_ = false;
        return activated;
    }
    void press(bool inside) { pressed_ = inside; }
    void cancel() { pressed_ = false; }

private:
    bool pressed_ = false;
};

}

#endif
