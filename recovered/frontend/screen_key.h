#ifndef HD2_FRONTEND_SCREEN_KEY_H
#define HD2_FRONTEND_SCREEN_KEY_H

#include <cstdint>

namespace hd2::frontend {

// Client 00668670, screen +20. Portable callbacks replace control ABI/storage.
// Offer the full key word to enabled focused controls first. Activation checks
// only its low byte and deliberately ignores the activation callback's result.
template<class Screen>
bool screen_key(Screen& screen, std::uint32_t key) {
    int focused = screen.focused_control();
    if (screen.valid_control(focused) && !screen.control_disabled(focused) &&
        screen.key_control(focused, key)) return true;

    const auto code = static_cast<std::uint8_t>(key);
    if (code != 0x1c && code != 0x39 && code != 0x9c) return false;
    focused = screen.focused_control(); // The key callback may change focus.
    if (!screen.valid_control(focused)) return false;
    screen.activate_control(focused, screen.control_argument(focused));
    return true;
}

}

#endif
