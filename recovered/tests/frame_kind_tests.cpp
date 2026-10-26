#include "scene/frame_kind.h"

#include <array>
#include <iostream>
#include <limits>
#include <string_view>
#include <utility>

int main() {
    using hd2::recovered::FrameKind;
    using hd2::recovered::frame_kind_name;
    constexpr std::array<std::pair<FrameKind, std::string_view>, 16> cases{{
        {FrameKind::visual, "FRAME_VISUAL"}, {FrameKind::light, "FRAME_LIGHT"},
        {FrameKind::camera, "FRAME_CAMERA"}, {FrameKind::sound, "FRAME_SOUND"},
        {FrameKind::sector, "FRAME_SECTOR"}, {FrameKind::dummy, "FRAME_DUMMY"},
        {FrameKind::target, "FRAME_TARGET"}, {FrameKind::user, "FRAME_USER"},
        {FrameKind::model, "FRAME_MODEL"}, {FrameKind::joint, "FRAME_JOINT"},
        {FrameKind::volume, "FRAME_VOLUME"}, {FrameKind::occluder, "FRAME_OCCLUDER"},
        {FrameKind::scene, "FRAME_SCENE"}, {FrameKind::area, "FRAME_AREA"},
        {FrameKind::shadow, "FRAME_SHADOW"}, {FrameKind::landscape, "FRAME_LANDSCAPE"},
    }};
    std::size_t failures = 0;
    for (std::size_t index = 0; index < cases.size(); ++index) {
        const auto [kind, expected] = cases[index];
        const auto value = static_cast<std::uint32_t>(index + 1);
        if (static_cast<std::uint32_t>(kind) != value
            || frame_kind_name(kind) != expected || frame_kind_name(value) != expected) {
            std::cerr << "FAIL frame kind " << value << '\n';
            ++failures;
        }
    }
    for (const auto value : {0U, 17U, std::numeric_limits<std::uint32_t>::max()}) {
        if (frame_kind_name(value) || frame_kind_name(static_cast<FrameKind>(value))) {
            std::cerr << "FAIL unknown frame kind " << value << '\n';
            ++failures;
        }
    }
    std::cout << (19 - failures) << "/19 frame-kind tests passed\n";
    return failures == 0 ? 0 : 1;
}
