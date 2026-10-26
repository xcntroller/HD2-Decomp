#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace hd2::recovered {

enum class FrameKind : std::uint32_t {
    visual = 1,
    light = 2,
    camera = 3,
    sound = 4,
    sector = 5,
    dummy = 6,
    target = 7,
    user = 8,
    model = 9,
    joint = 10,
    volume = 11,
    occluder = 12,
    scene = 13,
    area = 14,
    shadow = 15,
    landscape = 16,
};

// Unknown values remain numeric; the engine's fallback formatter is not reproduced.
std::optional<std::string_view> frame_kind_name(std::uint32_t kind);
std::optional<std::string_view> frame_kind_name(FrameKind kind);

}
