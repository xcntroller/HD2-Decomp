#ifndef HD2_FRONTEND_SCREEN_INPUT_H
#define HD2_FRONTEND_SCREEN_INPUT_H

#include "frontend/pointer_dispatch.h"

#include <cstdint>

namespace hd2::frontend {

namespace detail {

template<ScreenPointerCallback Callback, int InvalidResult, class Screen>
int screen_focused_pointer(Screen& screen, std::uint32_t target) {
    if (!screen.valid_control(screen.focused_control())) return InvalidResult;
    return screen.pointer_control(screen.focused_control(), Callback, target);
}

}

// Client 00656ca0, screen +48 -> focused control +48; invalid focus returns zero.
template<class Screen>
int screen_press(Screen& screen, std::uint32_t target) {
    return detail::screen_focused_pointer<ScreenPointerCallback::press, 0>(screen, target);
}

// Client 00656cd0, screen +44. Unlike press/drag, release must hit the focus.
template<class Screen>
int screen_release(Screen& screen, std::uint32_t target) {
    if (!screen.valid_control(screen.focused_control())) return 0;
    const int focused = screen.focused_control();
    if (focused != screen.control_for_target(target)) return 0;
    return screen.pointer_control(focused, ScreenPointerCallback::release, target);
}

// Client 00656d10, 00656d40 and 00656d70; invalid focus returns zero.
template<class Screen>
int screen_drag(Screen& screen, std::uint32_t target) {
    return detail::screen_focused_pointer<ScreenPointerCallback::drag, 0>(screen, target);
}

template<class Screen>
int screen_drag_end(Screen& screen, std::uint32_t target) {
    return detail::screen_focused_pointer<ScreenPointerCallback::drag_end, 0>(screen, target);
}

template<class Screen>
int screen_drag_begin(Screen& screen, std::uint32_t target) {
    return detail::screen_focused_pointer<ScreenPointerCallback::drag_begin, 0>(screen, target);
}

// Client 00656e00, screen +60 -> focused control +60; invalid focus returns seven.
template<class Screen>
int screen_double_click(Screen& screen, std::uint32_t target) {
    return detail::screen_focused_pointer<ScreenPointerCallback::double_click, 7>(screen, target);
}

// Client 00656dd0.
template<class Screen>
int screen_click(Screen& screen, std::uint32_t target) {
    const int focused = screen.focused_control();
    if (!screen.valid_control(focused)) return 7;
    return screen.click_control(focused, target);
}

}

#endif
