#ifndef HD2_FRONTEND_MENU_DEFINITION_H
#define HD2_FRONTEND_MENU_DEFINITION_H

#include "frontend/control_events.h"

#include <initializer_list>

namespace hd2::frontend {

enum class MenuControlKind : std::uint32_t { text = 0x13, button = 0x14 };
// Host semantic roles identify the three implemented actions without relying
// on array positions. These role names are not original client fields.
enum class MenuControlRole { none, exit, yes, no, ok };

struct MenuControlDefinition {
    std::uint32_t identifier;
    const char* frame;
    MenuControlKind kind;
    const char* normal;
    const char* active;
    MenuControlRole role;
    std::vector<ControlEvent> initialization;
};

struct MenuScreenDefinition {
    const char* label;
    std::uint32_t identifier;
    const char* model;
    std::vector<MenuControlDefinition> controls;
};

inline MenuControlDefinition text_definition(std::uint32_t id, const char* frame) {
    return {id, frame, MenuControlKind::text, nullptr, nullptr, MenuControlRole::none, {}};
}

inline MenuControlDefinition button_definition(
        std::uint32_t id, const char* frame, const char* normal, const char* active,
        MenuControlRole role, std::initializer_list<ControlEvent> initialization) {
    return {id, frame, MenuControlKind::button, normal, active, role, initialization};
}

// Bounded reconstruction of 00623dd0 definitions: exact order, identifiers,
// kind/name and activation bindings. Caption/style/observer lists remain in
// the existing text adapter; an empty text initialization here is not a claim
// that the original text definition has no events. Models/visual names also
// use the previously verified engine/resource evidence.
inline const MenuScreenDefinition& main_menu_definition() {
    static const MenuScreenDefinition definition{"main-menu", 0x04000000, "main_menu", {
        text_definition(0x04100000, "text_screen"),
        text_definition(0x04200000, "text_date00"),
        text_definition(0x04300000, "text_date01"),
        text_definition(0x04400000, "text_profile00"),
        text_definition(0x04500000, "text_profile01"),
        text_definition(0x04600000, "text_last save00"),
        text_definition(0x04700000, "text_last save01"),
        button_definition(0x04800000, "bresume game", "normal00", "actived00",
                          MenuControlRole::none, {{0, 0x06001075, 0, 0, 0, {}}}),
        button_definition(0x04900000, "bchange profile", "normal01", "actived01",
                          MenuControlRole::none, {{0, 0x06001001, 2, 0, 0, {}}}),
        button_definition(0x04a00000, "bsingleplayer", "normal02", "actived02",
                          MenuControlRole::none, {{0, 0x06001001, 3, 0, 0, {}}}),
        button_definition(0x04b00000, "bmultiplayerlan", "normal03", "actived03",
                          MenuControlRole::none, {{0, 0x06001099, 0, 0, 0, {}}}),
        button_definition(0x04c00000, "bmultiplayernet", "normal08", "actived08",
                          MenuControlRole::none, {{0, 0x0600109a, 0, 0, 0, {}}}),
        button_definition(0x04d00000, "bcinematics", "normal04", "actived04",
                          MenuControlRole::none, {{0, 0x06001001, 7, 0, 0, {}}}),
        button_definition(0x04e00000, "boptions", "normal05", "actived05",
                          MenuControlRole::none, {{0, 0x06001071, 0, 0, 0, {}}}),
        button_definition(0x04f00000, "bcredits", "normal06", "actived06",
                          MenuControlRole::none, {{0, 0x0600103a, 0, 0, 0, {}}}),
        button_definition(0x05000000, "bexit", "normal07", "actived07",
                          MenuControlRole::exit, {{0, 0x06001003, 0x04000000, 0, 0, {}}}),
        text_definition(0x05100000, "text_h&dversion")
    }};
    return definition;
}

inline const MenuScreenDefinition& confirmation_definition() {
    static const MenuScreenDefinition definition{"confirmation", 0x10000000, "confirm", {
        text_definition(0x10100000, "text"),
        button_definition(0x10200000, "byes?", "normal02", "actived03",
                          MenuControlRole::yes, {{0, 0x14001100, 0, 0, 0, {}},
                                                 {0, 0x06001005, 0, 0, 0, {}}}),
        button_definition(0x10300000, "bno?", "normal01", "actived01",
                          MenuControlRole::no, {{0, 0x14001200, 1, 0, 0, {}},
                                                {0, 0x06001005, 0, 0, 0, {}}}),
        button_definition(0x10400000, "bok", "normal03", "actived04",
                          MenuControlRole::ok, {{0, 0x01000004, 0, 0, 0, {}}})
    }};
    return definition;
}

}

#endif
