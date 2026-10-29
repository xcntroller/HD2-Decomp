#ifndef HD2_FRONTEND_SCREEN_EVENT_H
#define HD2_FRONTEND_SCREEN_EVENT_H

#include "frontend/control_events.h"

namespace hd2::frontend {

// 006684f0. Zero target and tagged messages go to the base SCREEN handler.
// Other messages go to the child selected by hierarchical ID. The focus
// request is applied before lookup, even if the eventual target is missing.
template<class Screen>
bool dispatch_screen_event(Screen& screen, ControlEvent& event) {
    if (event.target == 0 || (event.code & 0x040ff000u) != 0) {
        return screen.dispatch_base_event(event);
    }
    if ((event.code & 0x03000fffu) == 0x02000016) {
        if (screen.leave_focus(static_cast<std::uint32_t>(screen.focused_control())) != 0) {
            screen.enter_focus(event.target);
        }
    }
    if (screen.control_for_target(event.target) < 0) return false;
    // Original resolves the child again after the successful first lookup.
    const int index = screen.control_for_target(event.target);
    screen.dispatch_child_event(index, event);
    return true; // Child result is ignored.
}

}

#endif
