#ifndef HD2_FRONTEND_CHILD_NAVIGATION_H
#define HD2_FRONTEND_CHILD_NAVIGATION_H
#include <cstdint>

namespace hd2::frontend {

// 006553a0 / 00655430. Lists do not wrap or skip subordinate identifiers.
// Both leave and enter receive fifteen; leave veto still reports handled.
template<class List>
bool list_select_control(List& list, int candidate) {
    if (list.valid_control(list.focused_control()) && list.leave_control(list.focused_control(), 15) == 0) return true;
    list.set_focused_control(candidate);
    list.enter_control(candidate, 15);
    return true;
}
template<class List>
bool list_previous_control(List& list) {
    int candidate = list.focused_control();
    if (candidate == -1) candidate = list.control_count();
    while (--candidate >= 0) {
        if (list.valid_control(candidate) && list.control_eligible(candidate)) return list_select_control(list, candidate);
    }
    return false;
}
template<class List>
bool list_next_control(List& list) {
    for (int candidate = list.focused_control() + 1; candidate < list.control_count(); ++candidate) {
        if (list.valid_control(candidate) && list.control_eligible(candidate)) return list_select_control(list, candidate);
    }
    return false;
}

// 006554b0. Offer the full word to the eligible focused child first. Scroll
// geometry is 00656700's widget service, supplied by the actual list backend.
// Home/End return false after reaching a boundary. Page returns true only when
// a recursive step fails early, and false after completing its saved span.
template<class List>
bool list_key(List& list, std::uint32_t word) {
    if (list.valid_control(list.focused_control()) && list.control_eligible(list.focused_control()) &&
        list.key_control(list.focused_control(), word)) return true;
    const auto code = static_cast<std::uint8_t>(word);
    if (code == 0xc8 || code == 0xcb || code == 0xcd || code == 0xd0) {
        const bool up = code == 0xc8 || code == 0xcb;
        if (list.focused_control() == (up ? list.first_visible_control() : list.last_visible_control())) {
            if (!list.scroll(up ? 1 : 2)) return false;
            if (list.scroll_target() != 0) list.enqueue_scroll(list.scroll_target(), list.first_visible_control());
        }
        return up ? list_previous_control(list) : list_next_control(list);
    }
    if (code == 0xc7 || code == 0xcf) {
        while (list.key(code == 0xc7 ? 0xc8 : 0xd0)) {}
    } else if (code == 0xc9 || code == 0xd1) {
        const auto span = static_cast<std::int64_t>(list.last_visible_control()) - list.first_visible_control();
        for (std::int64_t i = 0; i < span; ++i) {
            if (!list.key(code == 0xc9 ? 0xc8 : 0xd0)) return true;
        }
    }
    return false;
}

// 0064e770. Capturing a binding swallows every scan; Escape asks its internal
// capture control to deactivate. 0064efd0 forwards to that child's key method.
template<class Binding>
bool binding_capture_key(Binding& binding, std::uint32_t word) {
    if (!binding.capture_active()) return false;
    if (static_cast<std::uint8_t>(word) == 1) binding.deactivate_capture(0);
    return true;
}
template<class Binding>
bool binding_child_key(Binding& binding, std::uint32_t word) { return binding.child_key(word); }

// 00657880 / 00658050 omit the ordinary group's eligibility check.
template<class Group>
bool focused_child_key(Group& group, std::uint32_t word) {
    if (!group.valid_control(group.focused_control())) return false;
    return group.key_control(group.focused_control(), word);
}
// 006502c0: minus/equal and keypad subtract/add consume regardless of result.
template<class Spinner>
bool spinner_key(Spinner& spinner, std::uint32_t word) {
    switch (static_cast<std::uint8_t>(word)) {
    case 0x0c: case 0x4a: spinner.decrement(); return true;
    case 0x0d: case 0x4e: spinner.increment(); return true;
    default: return false;
    }
}

}
#endif
