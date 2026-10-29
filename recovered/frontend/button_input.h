#ifndef HD2_FRONTEND_BUTTON_INPUT_H
#define HD2_FRONTEND_BUTTON_INPUT_H

#include "frontend/pointer_dispatch.h"

#include <cstdint>
#include <stdexcept>

namespace hd2::frontend {

// Client 0064db00/20/60/80/a0/e0. These callbacks emit a tagged event and
// return seven; they do not activate the button. Pointer target is unused.
template<class Button>
int button_pointer_event(Button& button, ScreenPointerCallback callback) {
    std::uint32_t event = 0;
    switch (callback) {
    case ScreenPointerCallback::release: event = 0x04007000; break;
    case ScreenPointerCallback::press: event = 0x04006000; break;
    case ScreenPointerCallback::drag: event = 0x04008000; break;
    case ScreenPointerCallback::drag_end: event = 0x04009000; break;
    case ScreenPointerCallback::drag_begin: event = 0x0400a000; break;
    case ScreenPointerCallback::double_click: event = 0x04013000; break;
    default: throw std::invalid_argument("Unsupported button pointer event");
    }
    button.emit_event(event, std::uint32_t{0});
    return 7;
}

// Client 0064db40 ignores target and wheel amount, emitting a zero argument.
template<class Button>
int button_wheel(Button& button) {
    button.emit_event(std::uint32_t{0x04011000}, std::uint32_t{0});
    return 7;
}

// Client 0064dbc0, for buttons whose activation ignores the pointer target.
template<class Button>
bool button_click(Button& button) {
    button.emit_event(std::uint32_t{0x04012000}, std::uint32_t{0});
    return button.activate_control();
}

}

#endif
