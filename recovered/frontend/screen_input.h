#ifndef HD2_FRONTEND_SCREEN_INPUT_H
#define HD2_FRONTEND_SCREEN_INPUT_H

#include <cstdint>

namespace hd2::frontend {

// Client 00656dd0.
template<class Screen>
int screen_click(Screen& screen, std::uint32_t target) {
    const int focused = screen.focused_control();
    if (!screen.valid_control(focused)) return 7;
    return screen.click_control(focused, target);
}

}

#endif
