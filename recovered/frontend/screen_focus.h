#ifndef HD2_FRONTEND_SCREEN_FOCUS_H
#define HD2_FRONTEND_SCREEN_FOCUS_H

#include <cstdint>
#include "frontend/container_input.h"

namespace hd2::frontend {

// Client 00656b90.
template<class Screen>
int screen_focus_leave(Screen& screen, std::uint32_t target) {
    if (screen.valid_control(screen.focused_control())) {
        if (screen.leave_control(screen.focused_control(), target) == 0) return 0;
        screen.set_focused_control(-1);
    }
    return 1;
}

// Client 00656b20. Share ordinary lookup/eligibility semantics with groups;
// target fifteen calls this screen's +20 override instead of ID lookup.
template<class Screen>
int screen_focus_enter(Screen& screen, std::uint32_t target) {
    return container_focus_enter(screen, target);
}

// Client 00656c30.
template<class Screen>
int screen_pointer_focus(Screen& screen, std::uint32_t target) {
    return container_pointer_focus(screen, target);
}

}

#endif
