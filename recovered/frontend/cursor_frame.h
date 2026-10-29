#ifndef HD2_FRONTEND_CURSOR_FRAME_H
#define HD2_FRONTEND_CURSOR_FRAME_H

#include <array>
#include <cmath>
#include <stdexcept>

namespace hd2::frontend {

// Client 0066a220 tail: final float stores follow x87 division/multiplication.
// This is the frame position, not the picking ray's later +0.1 Z offset.
inline std::array<float, 3> cursor_frame_position(float x, float y, float width, float height) {
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(width) ||
        !std::isfinite(height) || width <= 0 || height <= 0) {
        throw std::invalid_argument("Invalid cursor frame coordinates or range");
    }
    return {{static_cast<float>(static_cast<long double>(x) / width),
             static_cast<float>((-1.0L / height) * y * 0.75L), -1.0f}};
}

// Original frame +08 position setter precedes +24 world/transform update.
template<class Frame>
void update_cursor_frame(Frame& frame, float x, float y, float width, float height) {
    frame.set_position(cursor_frame_position(x, y, width, height));
    frame.update();
}

}

#endif
