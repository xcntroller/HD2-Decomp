#ifndef HD2_FRONTEND_POINTER_CLICK_TIMING_H
#define HD2_FRONTEND_POINTER_CLICK_TIMING_H

#include "frontend/pointer_click.h"

#include <cstdint>

namespace hd2::frontend {

struct PointerClickResult {
    bool click;
    bool double_click;
};

// Client 0066a220, using caller-supplied counter increments.
class PointerClickTiming {
public:
    void advance(std::uint32_t ticks) { elapsed_ += ticks; }
    void press(PointerClick::Target target) { click_.press(target); }
    PointerClick::Target pressed() const { return click_.pressed(); }

    PointerClickResult release(PointerClick::Target target) {
        const auto captured = click_.pressed();
        const bool matched = click_.release(target);
        // The original compares the wrapping counter as a signed 32-bit value.
        const bool within_threshold = elapsed_ < 400 || (elapsed_ & 0x80000000) != 0;
        const bool double_click = matched && within_threshold && previous_ == target;
        if (matched) previous_ = captured;
        if (double_click) previous_ = 0;
        elapsed_ = 0;
        return {matched, double_click};
    }

    void cancel_press() { click_.clear(); } // Host departure-safety adapter only.

    void clear() {
        click_.clear();
        previous_ = 0;
        elapsed_ = 0;
    }

private:
    PointerClick click_;
    PointerClick::Target previous_ = 0;
    std::uint32_t elapsed_ = 0;
};

}

#endif
