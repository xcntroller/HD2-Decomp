#ifndef HD2_FRONTEND_BUTTON_FOCUS_H
#define HD2_FRONTEND_BUTTON_FOCUS_H

#include <cstdint>

namespace hd2::frontend {

// Client 00655250; the adapter supplies the original frame and event operations.
template<class Button>
bool button_focus_enter(Button& button) {
    if (button.focus_blocked()) return false;
    button.set_active_visible(true);
    button.set_normal_visible(false);
    button.update_active();
    button.update_normal();
    button.emit_event(std::uint32_t{0x04003000}, std::uint32_t{0});
    return true;
}

// Client 006552a0.
template<class Button>
bool button_focus_leave(Button& button) {
    button.set_normal_visible(true);
    button.set_active_visible(false);
    button.update_normal();
    button.update_active();
    button.emit_event(std::uint32_t{0x04004000}, std::uint32_t{0});
    return true;
}

// Client 006552f0.
template<class Button>
bool button_activate(Button& button) {
    if (button.focus_blocked()) return false;
    button.emit_event(std::uint32_t{0x04001000}, std::uint32_t{0});
    return true;
}

}

#endif
