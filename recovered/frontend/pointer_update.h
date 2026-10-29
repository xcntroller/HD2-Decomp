#ifndef HD2_FRONTEND_POINTER_UPDATE_H
#define HD2_FRONTEND_POINTER_UPDATE_H

#include "frontend/pointer_buttons.h"
#include "frontend/pointer_click_timing.h"
#include "frontend/pointer_cursor.h"

namespace hd2::frontend {

struct PointerUpdateSample {
    std::uint32_t buttons;
    PointerClick::Target target;
    std::uint32_t delta;
    float horizontal = 0;
    float vertical = 0;
    float wheel = 0;
    bool inhibit_click = false;
};

struct PointerUpdateResult {
    bool processed;
    PointerClick::Target captured;
    PointerClickResult click;
};

// Bounded 0066a220 sequencing. The caller supplies the already-picked target
// (0066b990 picks before updating input), button snapshot and relative deltas.
// Cursor storage is portable; original engine frame updates remain omitted.
class PointerUpdate {
public:
    PointerUpdateResult update(const PointerUpdateSample& sample) {
        buttons_.update(sample.buttons);
        timing_.advance(sample.delta);
        idle_ += sample.delta;
        cursor_.sample(sample.horizontal, sample.vertical, sample.wheel);
        PointerUpdateResult result{false, timing_.pressed(), {false, false}};
        // Original +44 is tested as signed; subtraction wraps, without clamping.
        if (suppression_ != 0 && (suppression_ & 0x80000000u) == 0) {
            suppression_ -= sample.delta;
            return result;
        }
        if (sample.horizontal == 0 && sample.vertical == 0 && sample.wheel == 0 &&
            buttons_.previous() == buttons_.current() && target_ == sample.target) {
            return result;
        }
        idle_ = 0;
        target_ = sample.target;
        cursor_.integrate();
        result.processed = true;
        if (!sample.inhibit_click) {
            if (buttons_.pressed(0)) timing_.press(target_);
            result.captured = timing_.pressed();
            if (buttons_.released(0)) result.click = timing_.release(target_);
        }
        return result;
    }

    void cancel_departed_press(PointerClick::Target target) {
        if (timing_.pressed() != target) timing_.cancel_press();
    }
    const PointerButtons& buttons() const { return buttons_; }
    PointerCursor& cursor() { return cursor_; }
    const PointerCursor& cursor() const { return cursor_; }
    std::uint32_t idle() const { return idle_; }
    void suppress_for(std::uint32_t delta) { suppression_ = delta; }
    void clear() {
        buttons_.clear();
        timing_.clear();
        target_ = 0;
        suppression_ = 0;
        idle_ = 0;
        cursor_.clear();
    }

private:
    PointerButtons buttons_;
    PointerClickTiming timing_;
    PointerCursor cursor_;
    std::uint32_t idle_ = 0;
    PointerClick::Target target_ = 0;
    std::uint32_t suppression_ = 0;
};

}

#endif
