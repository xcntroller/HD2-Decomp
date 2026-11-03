#ifndef HD2_FRONTEND_POINTER_CURSOR_H
#define HD2_FRONTEND_POINTER_CURSOR_H

#include "frontend/pointer_motion.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#if defined(_M_IX86)
#define HD2_CURSOR_CALL __thiscall
#define HD2_CURSOR_BODY __declspec(noinline)
#elif defined(__i386__)
#define HD2_CURSOR_CALL __attribute__((thiscall))
#define HD2_CURSOR_BODY __attribute__((noinline))
#else
#define HD2_CURSOR_CALL
#define HD2_CURSOR_BODY
#endif

namespace hd2::frontend {

struct PointerDeltaFields {
    std::array<std::uint32_t, 5> prefix{};
    float horizontal = 0, vertical = 0, wheel = 0;
};
static_assert(offsetof(PointerDeltaFields, horizontal) == 0x14);
static_assert(offsetof(PointerDeltaFields, vertical) == 0x18);
static_assert(offsetof(PointerDeltaFields, wheel) == 0x1c);

// The three query bodies use the original input prefix. Cursor globals,
// dimensions, accumulation and input-owner construction remain native storage.
class PointerCursor {
public:
    void initialize(float x, float y, float width, float height) {
        x_ = x;
        y_ = y;
        width_ = width;
        height_ = height;
    }
    void sample(float horizontal, float vertical, float wheel) {
        deltas_.horizontal = horizontal;
        deltas_.vertical = vertical;
        deltas_.wheel = wheel;
    }
    // Client 0066a220: integrate only after suppression/unchanged-input gates.
    void integrate() {
        x_ = static_cast<float>(static_cast<long double>(x_) + deltas_.horizontal);
        y_ = static_cast<float>(static_cast<long double>(y_) + deltas_.vertical);
        wheel_ = static_cast<float>(static_cast<long double>(wheel_) + deltas_.wheel);
        clip_axis(x_, deltas_.horizontal, width_);
        clip_axis(y_, deltas_.vertical, height_);
    }
    void clear() { *this = {}; }
    float x() const { return x_; }
    float y() const { return y_; }
    float horizontal() const { return deltas_.horizontal; }
    float vertical() const { return deltas_.vertical; }
    //00669FF0 /0066A020 /0066A030: ECX=this, AL or ST0 result, RET0.
    HD2_CURSOR_BODY bool HD2_CURSOR_CALL moved() const {
        return detail::pointer_delta_nonzero(deltas_.horizontal) || detail::pointer_delta_nonzero(deltas_.vertical);
    }
    HD2_CURSOR_BODY float HD2_CURSOR_CALL wheel_delta() const { return deltas_.wheel; }
    HD2_CURSOR_BODY bool HD2_CURSOR_CALL wheel_moved() const {
        return detail::pointer_delta_nonzero(deltas_.wheel);
    }
    float wheel() const { return wheel_; }

private:
    static void clip_axis(float& position, float& delta, float limit) {
        // Preserve original store boundaries and subtraction order: the upper
        // overflow stays in x87 precision until subtracted from the delta.
        // Do not replace with delta = clamped_position - previous_position.
        if (position < 0.0f) {
            delta = static_cast<float>(static_cast<long double>(delta) - position);
            position = 0.0f;
        } else if (position > limit) {
            delta = static_cast<float>(static_cast<long double>(delta) -
                (static_cast<long double>(position) - limit));
            position = limit;
        }
    }

    PointerDeltaFields deltas_;
    float x_ = 0, y_ = 0;
    float width_ = 0, height_ = 0;
    float wheel_ = 0;
};
static_assert(std::is_standard_layout_v<PointerCursor>);

}

#undef HD2_CURSOR_CALL
#undef HD2_CURSOR_BODY

#endif
