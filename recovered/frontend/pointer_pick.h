#ifndef HD2_FRONTEND_POINTER_PICK_H
#define HD2_FRONTEND_POINTER_PICK_H

#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace hd2::frontend {

struct alignas(16) PointerRay {
    std::array<float, 4> origin;
    std::array<float, 4> direction;
};

// Client 0066a120 and the ray setup in 0066a7c0.
inline PointerRay menu_pointer_ray(float x, float y, float width, float height) {
    if (!std::isfinite(x) || !std::isfinite(y) ||
        !std::isfinite(width) || !std::isfinite(height) || width <= 0 || height <= 0) {
        throw std::invalid_argument("Invalid pointer-picking coordinates or dimensions");
    }
    return {{
        static_cast<float>(static_cast<double>(x) / width),
        static_cast<float>(-static_cast<double>(y) / height * 0.75),
        -1.0f + 0.1f, 0.0f
    }, {0.0f, 0.0f, 1.0f, 0.0f}};
}

// Client 0066a7c0, with the input-state store supplied by the caller.
template<class PickFrame, class FrameIdentifier>
std::uint32_t menu_pointer_target(const PointerRay& ray, PickFrame pick_frame,
                                  FrameIdentifier frame_identifier) {
    auto* frame = pick_frame(ray, std::uint32_t{0x00140000});
    return frame == nullptr ? 0 : frame_identifier(frame);
}

}

#endif
