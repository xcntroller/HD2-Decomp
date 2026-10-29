#ifndef HD2_FRONTEND_SCREEN_NAVIGATION_H
#define HD2_FRONTEND_SCREEN_NAVIGATION_H

#include <cstdint>

namespace hd2::frontend {

// Client 006686f0. Caller supplies a valid candidate index. A leave veto is
// reported as handled and stops traversal, preserving the old focused index.
// The enter result is ignored; portable callbacks replace original control ABI.
template<class Screen>
bool screen_select_control(Screen& screen, int candidate) {
    if (!screen.control_eligible(candidate)) return false;
    const int focused = screen.focused_control();
    if (screen.valid_control(focused) && screen.leave_control(focused, 0) == 0) return true;
    screen.set_focused_control(candidate);
    screen.enter_control(candidate, 15);
    return true;
}

// Client 00668580. Search lower indices, then wrap from the final control.
// Bits 12..19 of control +60 exclude subordinate targets from this traversal.
template<class Screen>
bool screen_previous_control(Screen& screen) {
    for (int candidate = screen.focused_control() - 1; candidate >= 0; --candidate) {
        if ((screen.control_argument(candidate) & 0x000ff000u) == 0 &&
            screen_select_control(screen, candidate)) return true;
    }
    for (int candidate = screen.control_count() - 1; candidate >= 0; --candidate) {
        if ((screen.control_argument(candidate) & 0x000ff000u) == 0 &&
            screen_select_control(screen, candidate)) return true;
    }
    return false;
}

// Client 006685f0. Each forward-loop bound reads the current control count,
// because eligibility/focus callbacks may change the collection. Focus is -1
// or a valid index; this adapter does not model corrupted index arithmetic.
template<class Screen>
bool screen_next_control(Screen& screen) {
    for (int candidate = screen.focused_control() + 1;
         candidate < screen.control_count(); ++candidate) {
        if ((screen.control_argument(candidate) & 0x000ff000u) == 0 &&
            screen_select_control(screen, candidate)) return true;
    }
    for (int candidate = 0; candidate < screen.control_count(); ++candidate) {
        if ((screen.control_argument(candidate) & 0x000ff000u) == 0 &&
            screen_select_control(screen, candidate)) return true;
    }
    return false;
}

}

#endif
