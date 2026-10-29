#ifndef HD2_HOST_POINTER_DISPATCH_PROBE_H
#define HD2_HOST_POINTER_DISPATCH_PROBE_H

#include "frontend/pointer_dispatch.h"
#include "frontend/pointer_motion.h"
#include "frontend/pointer_update.h"
#include "frontend/screen_focus.h"
#include "frontend/screen_input.h"
#include "frontend/screen_target.h"

#include <array>
#include <cstdio>
#include <optional>
#include <stdexcept>

namespace hd2::host {

// Observational adapter. Supplied picked IDs and message-derived deltas are
// snapshots, not original engine polling. Screen callbacks only write logs.
class PointerDispatchProbe {
public:
    frontend::PointerUpdateResult observe(const frontend::PointerUpdateSample& sample) {
        sample_ = sample;
        frontend::dispatch_menu_pointer(state_, *this, sample.delta);
        return update_;
    }
    void initialize_cursor(float x, float y, float width, float height) {
        pointer_.cursor().initialize(x, y, width, height);
    }
    void clear() {
        pointer_.clear();
        state_ = {};
        input_ = {};
        update_ = {};
        last_focus_.reset();
        focused_ = -1;
    }
    const frontend::PointerButtons& buttons() const { return pointer_.buttons(); }

    std::uint32_t pick_target() const { return sample_.target; }
    bool update_input(std::uint32_t delta, std::uint32_t target) {
        auto sample = sample_;
        sample.delta = delta;
        sample.target = target;
        update_ = pointer_.update(sample);
        const auto& buttons = pointer_.buttons();
        input_ = {buttons.pressed(3), buttons.released(3), buttons.currently_selected(3),
                  update_.click.click, update_.click.double_click,
                  frontend::pointer_moved(pointer_.cursor().horizontal(), pointer_.cursor().vertical()),
                  pointer_.cursor().wheel_delta() != 0};
        return update_.processed;
    }
    const frontend::PointerDispatchInput& input() const { return input_; }
    std::optional<std::uint32_t> captured_control_target() const { return std::nullopt; }
    int captured_control_event(std::uint32_t) {
        throw std::logic_error("Probe has no captured screen control");
    }
    void clear_captured_control() {
        throw std::logic_error("Probe has no captured screen control");
    }
    int focused_control() const { return focused_; }
    void set_focused_control(int index) { focused_ = index; }
    bool valid_control(int index) const { return index == 1 || index == 2; }
    bool control_eligible(int index) const { return valid_control(index); }
    int control_for_target(std::uint32_t target) const {
        constexpr std::array<std::uint32_t, 4> ids{{
            0x10100000, 0x10200000, 0x10300000, 0x10400000}};
        return frontend::find_screen_control(0x10000000, ids, target);
    }
    int enter_control(int, std::uint32_t) const { return 1; }
    int leave_control(int, std::uint32_t) const { return 1; }
    int move_control(int index, std::uint32_t target) const {
        return pointer_control(index, frontend::ScreenPointerCallback::focus, target);
    }
    int click_control(int, std::uint32_t) const { return 7; }
    int pointer_control(int, frontend::ScreenPointerCallback, std::uint32_t) const {
        return 7; // Observational control callback; never activates or emits events.
    }
    int screen_event(frontend::ScreenPointerCallback callback, std::uint32_t target) {
        const int result = route_screen_event(callback, target);
        // Suppress consecutive identical motion/drag diagnostics.
        // The dispatcher still invokes every callback.
        const auto slot = static_cast<unsigned>(callback);
        if (callback == frontend::ScreenPointerCallback::focus ||
            callback == frontend::ScreenPointerCallback::drag) {
            const auto key = (static_cast<std::uint64_t>(slot) << 32) | target;
            if (last_focus_ && *last_focus_ == key) return result;
            last_focus_ = key;
        } else {
            last_focus_.reset();
        }
        std::printf("Confirmation dispatch probe: screen+%02x target=%08x "
                    "focus=%d result=%d cursor=(%.1f,%.1f) motion=(%.1f,%.1f) "
                    "(observational)\n",
                    slot, static_cast<unsigned>(target), focused_, result,
                    pointer_.cursor().x(), pointer_.cursor().y(),
                    pointer_.cursor().horizontal(), pointer_.cursor().vertical());
        return result;
    }
    void accepted_click() {
        throw std::logic_error("Observational probe cannot accept a click");
    }
    void wheel_and_refresh(std::uint32_t) {
        throw std::logic_error("Original wheel dispatch is not reconstructed");
    }

private:
    int route_screen_event(frontend::ScreenPointerCallback callback, std::uint32_t target) {
        using Callback = frontend::ScreenPointerCallback;
        switch (callback) {
        case Callback::focus: return frontend::screen_pointer_focus(*this, target);
        case Callback::press: return frontend::screen_press(*this, target);
        case Callback::release: return frontend::screen_release(*this, target);
        case Callback::drag: return frontend::screen_drag(*this, target);
        case Callback::drag_end: return frontend::screen_drag_end(*this, target);
        case Callback::drag_begin: return frontend::screen_drag_begin(*this, target);
        case Callback::click: return frontend::screen_click(*this, target);
        case Callback::double_click: return frontend::screen_double_click(*this, target);
        }
        throw std::logic_error("Unknown observational screen callback");
    }
    int focused_ = -1;
    frontend::PointerUpdate pointer_;
    frontend::PointerDispatchState state_;
    frontend::PointerDispatchInput input_;
    frontend::PointerUpdateSample sample_{};
    frontend::PointerUpdateResult update_{};
    std::optional<std::uint64_t> last_focus_;
};

}

#endif
