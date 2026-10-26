#ifndef HD2_HOST_CURSOR_GEOMETRY_H
#define HD2_HOST_CURSOR_GEOMETRY_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>

namespace hd2::host {

struct CursorVertex {
    float x, y, z, reciprocal_w;
    std::uint32_t diffuse, specular;
    float u, v;
};

static_assert(sizeof(CursorVertex) == 32);
static_assert(offsetof(CursorVertex, diffuse) == 16);
static_assert(offsetof(CursorVertex, u) == 24);

struct CursorPoint {
    float x, y;
};

inline std::optional<CursorPoint> cursor_position(
        std::int32_t x, std::int32_t y, std::int32_t client_width,
        std::int32_t client_height, std::int32_t render_width, std::int32_t render_height) {
    if (client_width <= 0 || client_height <= 0 || render_width <= 0 || render_height <= 0) {
        throw std::invalid_argument("Cursor viewport dimensions must be positive");
    }
    if (x < 0 || y < 0 || x >= client_width || y >= client_height) {
        return std::nullopt;
    }
    return CursorPoint{
        static_cast<float>(x) / static_cast<float>(client_width) * render_width,
        static_cast<float>(y) / static_cast<float>(client_height) * render_height,
    };
}

inline std::array<CursorVertex, 4> cursor_quad(
        CursorPoint point, std::int32_t width, std::int32_t height) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Cursor render dimensions must be positive");
    }
    const float right = point.x + static_cast<float>(width) * (1.0f / 1024.0f) * 30.0f;
    const float bottom = point.y + static_cast<float>(height) * (1.0f / 768.0f) * 30.0f;
    constexpr float z = 0x1.d17e0cp-6f;
    constexpr float reciprocal_w = 0x1.1993bep+5f;
    constexpr std::uint32_t white = 0xffffffff;
    return {{
        {point.x, point.y, z, reciprocal_w, white, 0, 0, 0},
        {right, point.y, z, reciprocal_w, white, 0, 0, 1},
        {right, bottom, z, reciprocal_w, white, 0, 1, 1},
        {point.x, bottom, z, reciprocal_w, white, 0, 1, 0},
    }};
}

}

#endif
