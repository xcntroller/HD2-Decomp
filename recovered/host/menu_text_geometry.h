#ifndef HD2_HOST_MENU_TEXT_GEOMETRY_H
#define HD2_HOST_MENU_TEXT_GEOMETRY_H

#include "host/bitmap_font.h"
#include "frontend/text_object.h"
#include "host/cursor_geometry.h"

#include <algorithm>
#include <cmath>
#include <string_view>
#include <vector>
#include <optional>

namespace hd2::host {

struct MenuRectangle {
    float left, top, right, bottom;

    bool contains(CursorPoint point) const {
        return point.x >= left && point.x < right && point.y >= top && point.y < bottom;
    }
};

inline MenuRectangle menu_rectangle(
        const std::array<float, 8>& bounds, int width, int height, std::uint8_t inset = 0) {
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
    // Text object +90 is in 1/1024-screen-width units, truncated before projection.
    const int padding = static_cast<int>(static_cast<double>(width) * (1.0 / 1024.0) * inset);
    const auto left = static_cast<float>(static_cast<double>(width) * bounds[0] + padding);
    const auto right = static_cast<float>(static_cast<double>(width) * bounds[4] - padding);
    if (left >= right) {
        throw std::invalid_argument("Original menu text inset leaves an empty rectangle");
    }
    return {left, 1.0f - height * bounds[5] * (4.0f / 3.0f),
            right, -height * bounds[1] * (4.0f / 3.0f) - 1.0f};
}

struct MenuLabelBinding {
    const char* frame;
    const char* visual;
    const char* active_visual;
    std::uint32_t id;
    float height;
    bool small;
    bool button;
};

inline constexpr std::array<MenuLabelBinding, 13> menu_label_bindings{{
    {"text_screen", "text_screen", nullptr, 2101, .05f, false, false},
    {"text_date00", "text_date00", nullptr, 2102, .02f, true, false},
    {"text_profile00", "text_profile00", nullptr, 2103, .02f, true, false},
    {"text_last save00", "text_last save00", nullptr, 2104, .02f, true, false},
    {"bresume game", "normal00", "actived00", 2105, .04f, false, true},
    {"bchange profile", "normal01", "actived01", 2106, .04f, false, true},
    {"bsingleplayer", "normal02", "actived02", 2107, .04f, false, true},
    {"bcinematics", "normal04", "actived04", 2109, .04f, false, true},
    {"boptions", "normal05", "actived05", 2110, .04f, false, true},
    {"bcredits", "normal06", "actived06", 2111, .04f, false, true},
    {"bexit", "normal07", "actived07", 2112, .04f, false, true},
    {"bmultiplayerlan", "normal03", "actived03", 2115, .04f, false, true},
    {"bmultiplayernet", "normal08", "actived08", 2116, .04f, false, true},
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

struct MenuTextSubmission {
    bool small;
    std::vector<CursorVertex> vertices;
};

// Bounded glyph submission for one attached ASCII record. Bounds are lazy so
// empty dynamic labels/deferred version text do not request unused visuals.
// General paragraph wrapping and word_94 color semantics remain unresolved;
// current hover/unavailable colors preserve the desktop-tested host policy.
template<class Bounds>
std::optional<MenuTextSubmission> menu_text_submission(
        const frontend::TextObject& object, Bounds bounds, bool button,
        bool available, bool hovered, const FontMetrics& big, const FontMetrics& small) {
    if (object.records().empty()) return std::nullopt;
    if (object.records().size() != 1) throw std::logic_error("Multiple text records require original paragraph layout");
    const auto& record = object.records().front();
    if (!record.text || record.text->empty()) return std::nullopt;
    const auto* style = frontend::menu_text_style(record.style);
    if (!style || !((style->horizontal == 1 && style->vertical == 2) ||
                    (style->horizontal == 3 && style->vertical == 3))) {
        throw std::logic_error("Unsupported original text style layout");
    }
    std::uint32_t color = object.disabled || (button && !available) ? 0x60000000 : style->color;
    if (hovered && available && !object.disabled) color = 0xffffffff;
    return MenuTextSubmission{style->font == 3,
        menu_label_vertices(*record.text, style->font == 3 ? small : big,
            menu_rectangle(bounds(), 800, 600, object.inset), style->height, 600,
            style->horizontal == 3, color)};
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
