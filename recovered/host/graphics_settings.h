#ifndef HD2_HOST_GRAPHICS_SETTINGS_H
#define HD2_HOST_GRAPHICS_SETTINGS_H

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>

namespace hd2::host {

struct GraphicsSettings32 {
    std::uint32_t instance;
    std::uint32_t adapter;
    std::int32_t x, y, width, height;
    std::int32_t child_x, child_y, child_width, child_height;
    std::uint8_t color_bits, refresh_rate;
    std::uint16_t reserved;
    std::uint32_t flags;
    std::uint32_t menu;
    std::uint32_t opaque_callback;
};

static_assert(sizeof(GraphicsSettings32) == 56);
static_assert(offsetof(GraphicsSettings32, color_bits) == 0x28);
static_assert(offsetof(GraphicsSettings32, flags) == 0x2c);
static_assert(offsetof(GraphicsSettings32, opaque_callback) == 0x34);

inline GraphicsSettings32 windowed_settings(std::uint32_t instance) {
    return {instance, 0, 50, 50, 800, 600, 0, 0, 800, 600, 32, 0, 0, 0x80, 0, 0};
}

inline std::uint32_t parse_frame_limit(std::wstring_view text) {
    if (text.empty()) {
        throw std::invalid_argument("Frame limit must contain decimal digits (0 means interactive)");
    }
    std::uint32_t result = 0;
    for (wchar_t character : text) {
        if (character < L'0' || character > L'9') {
            throw std::invalid_argument("Frame limit must contain only decimal digits");
        }
        const auto digit = static_cast<std::uint32_t>(character - L'0');
        if (result > (std::numeric_limits<std::uint32_t>::max() - digit) / 10) {
            throw std::invalid_argument("Frame limit exceeds 32 bits");
        }
        result = result * 10 + digit;
    }
    return result;
}

}

#endif
