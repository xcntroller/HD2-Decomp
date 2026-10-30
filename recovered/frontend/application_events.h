#ifndef HD2_FRONTEND_APPLICATION_EVENTS_H
#define HD2_FRONTEND_APPLICATION_EVENTS_H

#include "frontend/control_events.h"

#include <array>
#include <optional>

namespace hd2::frontend {

// Ordered callback services at entry to 00626b20. Their screen-specific bodies
// remain dependencies, not implementations supplied by this routing helper.
enum class MenuApplicationHandler { profiles, multiplayer, mission, options, connection };
inline constexpr std::array<MenuApplicationHandler, 5> menu_application_handlers{{
    MenuApplicationHandler::profiles,    // 006211e0
    MenuApplicationHandler::multiplayer, // 006333c0
    MenuApplicationHandler::mission,     // 00638d30
    MenuApplicationHandler::options,     // 0061bea0
    MenuApplicationHandler::connection   // 006322c0
}};

// 00626b20's common routing after the five screen-specific services decline.
// Reacquire the current screen for each family test, as the original does.
// Unknown derived events are ignored by the original; false lets the bounded
// host keep its explicit unsupported-event diagnostic instead.
template<class Services>
bool dispatch_menu_application_event(Services& services, ControlEvent& event) {
    for (const auto handler : menu_application_handlers) {
        if (services.dispatch_handler(handler, event)) return true;
    }
    const auto family_is = [&](std::uint32_t family) {
        const auto current = services.current_identifier();
        return current && ((*current >> 26) & 0x3fu) == family;
    };
    switch (event.code & 0x03000fffu) {
    case 0x0200000a:
        if (services.current_identifier()) {
            if (family_is(2)) {
                services.profile_back_cleanup(); // 00622a90(0,0,0,0).
                services.enqueue({0, 0x02000003, 0, 0, 0, {}});
                return true;
            }
            if (family_is(14) || family_is(10)) {
                services.enqueue({0, 0x10000200, 1, 0, 0, {}});
                services.enqueue({0, 0x02000005, 0, 0, 0, {}});
                return true;
            }
            if (family_is(4)) {
                services.enqueue({0, 0x14001200, 1, 0, 0, {}});
                services.enqueue({0, 0x06001005, 0, 0, 0, {}});
                return true;
            }
        }
        services.enqueue({0, 0x02000003, 0, 0, 0, {}});
        return true;
    case 0x0200003a:
        services.select_application_state(12);
        return true;
    case 0x02000075:
        services.resume_last_save(); // Save/path/application body is a service.
        return true;
    default:
        return false;
    }
}

}

#endif
