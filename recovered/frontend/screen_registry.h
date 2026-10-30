#ifndef HD2_FRONTEND_SCREEN_REGISTRY_H
#define HD2_FRONTEND_SCREEN_REGISTRY_H

#include <array>
#include <cstddef>
#include <cstdint>

namespace hd2::frontend {

struct ScreenRegistration {
    const char* name;
    const char* model;
    std::uint32_t identifier;
};

// 00623dd0 registration order. Runtime registry index zero is a separately
// constructed modal screen (0066c1d0), not the first named definition.
// The two SINGLE_MISSION_2 registrations have DISTINCT IDs; keep both.
inline constexpr std::array<ScreenRegistration, 15> menu_screen_registry{{
    {"runtime-confirmation", "confirm", 0},
    {"MAIN MENU", "main_menu", 0x04000000},
    {"CHANGE PROFIL", "change_profile", 0x08000000},
    {"SINGLEPLAYER", "singleplayer", 0x0c000000},
    {"CONFIRM", "confirm", 0x10000000},
    {"SINGLE MISSION 2", "single mission 2", 0x14000000},
    {"OPTIONS", "options", 0x18000000},
    {"SINGLE MISSION 2", "single mission 2", 0x1c000000},
    {"multiplayer menu", "multiplayer menu", 0x20000000},
    {"create server", "create server", 0x24000000},
    {"PASSWORD", "password", 0x28000000},
    {"Server Filters", "server filters", 0x2c000000},
    {"Game info", "gameinfo", 0x30000000},
    {"MP Connect console", "confirm", 0x34000000},
    {"ADD SERVER", "password", 0x38000000}
}};

// Default registry view for the bounded host. The same stack can instead use
// OwnedScreens, whose successful construction order defines runtime indices.
struct MenuScreenRegistry {
    std::size_t size() const { return menu_screen_registry.size(); }
    const ScreenRegistration& registration(std::size_t index) const {
        return menu_screen_registry.at(index);
    }
};
inline const MenuScreenRegistry default_menu_screen_registry{};

}

#endif
