#ifndef HD2_FRONTEND_MENU_CONTENT_SETTINGS_H
#define HD2_FRONTEND_MENU_CONTENT_SETTINGS_H

#include "frontend/cleanup.h"
#include <array>
#include <cstdint>
#include <stdexcept>

namespace hd2::frontend {

//0066C5B0 saves NONZERO Boolean values (not raw DWORDs), selects content-load
// settings, constructs resources/screens, then restores in21/25/17 order.
// Native failures also attempt every restore and preserve the first error.
template<class Driver, class Load>
void load_menu_content(Driver& driver, std::uint32_t mode, Load&& load) {
    constexpr std::array<std::uint32_t, 3> states{0x15, 0x19, 0x11};
    std::array<std::uint32_t, 3> saved{};
    for (std::size_t i = 0; i < states.size(); ++i) saved[i] = driver.render_state(states[i]) != 0;
    const auto set = [&](std::uint32_t state, std::uint32_t value) {
        if (driver.set_render_state(state, value) < 0) throw std::runtime_error("Menu content driver state rejected");
    };
    CleanupErrors errors;
    errors.attempt([&] {
        set(0x15, 0); set(0x19, 0); set(0x11, mode != 11);
        load();
    });
    for (std::size_t i = 0; i < states.size(); ++i) errors.attempt([&] { set(states[i], saved[i]); });
    errors.rethrow();
}

}
#endif
