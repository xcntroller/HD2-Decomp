#ifndef HD2_FRONTEND_SCREEN_FOCUS_H
#define HD2_FRONTEND_SCREEN_FOCUS_H

#include <cstdint>
#include <stdexcept>

namespace hd2::frontend {

// Client 00656b90.
template<class Screen>
int screen_focus_leave(Screen& screen, std::uint32_t target) {
    const int focused = screen.focused_control();
    if (screen.valid_control(focused)) {
        if (screen.leave_control(focused, target) == 0) return 0;
        screen.set_focused_control(-1);
    }
    return 1;
}

// Client 00656b20, excluding its special target-15 branch.
template<class Screen>
int screen_focus_enter(Screen& screen, std::uint32_t target) {
    if (target == 15) throw std::invalid_argument("Special screen focus target is unsupported");
    const int candidate = screen.control_for_target(target);
    if (!screen.valid_control(candidate) || !screen.control_eligible(candidate)) return 0;
    screen.set_focused_control(candidate);
    return screen.enter_control(candidate, target);
}

// Client 00656c30.
template<class Screen>
int screen_pointer_focus(Screen& screen, std::uint32_t target) {
    const int candidate = screen.control_for_target(target);
    if (!screen.valid_control(candidate)) return screen_focus_leave(screen, target);
    if (candidate == screen.focused_control()) return screen.move_control(candidate, target);
    if (screen_focus_leave(screen, target) == 0) return 1;
    return screen_focus_enter(screen, target);
}

}

#endif
