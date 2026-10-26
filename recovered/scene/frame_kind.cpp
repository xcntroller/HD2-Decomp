#include "scene/frame_kind.h"

#include <array>

namespace hd2::recovered {

std::optional<std::string_view> frame_kind_name(std::uint32_t kind) {
    static constexpr std::array<std::string_view, 16> names{
        "FRAME_VISUAL", "FRAME_LIGHT", "FRAME_CAMERA", "FRAME_SOUND",
        "FRAME_SECTOR", "FRAME_DUMMY", "FRAME_TARGET", "FRAME_USER",
        "FRAME_MODEL", "FRAME_JOINT", "FRAME_VOLUME", "FRAME_OCCLUDER",
        "FRAME_SCENE", "FRAME_AREA", "FRAME_SHADOW", "FRAME_LANDSCAPE",
    };
    if (kind == 0 || kind > names.size()) {
        return std::nullopt;
    }
    return names[kind - 1];
}

std::optional<std::string_view> frame_kind_name(FrameKind kind) {
    return frame_kind_name(static_cast<std::uint32_t>(kind));
}

}
