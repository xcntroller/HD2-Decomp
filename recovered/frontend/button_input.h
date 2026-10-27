#ifndef HD2_FRONTEND_BUTTON_INPUT_H
#define HD2_FRONTEND_BUTTON_INPUT_H

#include <cstdint>

namespace hd2::frontend {

// Client 0064dbc0, for buttons whose activation ignores the pointer target.
template<class Button>
bool button_click(Button& button) {
    button.emit_event(std::uint32_t{0x04012000}, std::uint32_t{0});
    return button.activate_control();
}

}

#endif
