#ifndef HD2_FRONTEND_MENU_DEFINITION_H
#define HD2_FRONTEND_MENU_DEFINITION_H

#include "frontend/control_events.h"

#include <initializer_list>

namespace hd2::frontend {

enum class MenuControlKind : std::uint32_t {
    image = 4, container = 5, movie_row = 6, slider = 8, binding_list = 0x0b, binding_row = 0x0d, selector = 0x10, text = 0x13, button = 0x14, video = 0x22, movie_list = 0x23, toggle = 0x24
};
// Host semantic roles identify the three implemented actions without relying
// on array positions. These role names are not original client fields.
enum class MenuControlRole { none, exit, yes, no, ok, credits, cinematics, play, back, movie_list, options };

struct MenuControlDefinition {
    std::uint32_t identifier;
    const char* frame;
    MenuControlKind kind;
    const char* normal;
    const char* active;
    MenuControlRole role;
    std::vector<ControlEvent> initialization;
    std::vector<MenuControlDefinition> children{};
};

struct MenuScreenDefinition {
    const char* label;
    std::uint32_t identifier;
    const char* model;
    std::vector<MenuControlDefinition> controls;
    std::vector<ControlEvent> initialization{};
};

inline MenuControlDefinition text_definition(std::uint32_t id, const char* frame,
                                             std::initializer_list<ControlEvent> initialization = {}) {
    return {id, frame, MenuControlKind::text, nullptr, nullptr, MenuControlRole::none, initialization};
}

inline MenuControlDefinition button_definition(
        std::uint32_t id, const char* frame, const char* normal, const char* active,
        MenuControlRole role, std::initializer_list<ControlEvent> initialization) {
    return {id, frame, MenuControlKind::button, normal, active, role, initialization};
}

inline ControlEvent caption_definition(std::uint32_t style, std::uint32_t label) {
    return {0, 0x01000015, style, label, 0, {}};
}

// Bounded reconstruction of 00623dd0 definitions: exact order, identifiers,
// kind/name, caption/style and activation bindings. Original observer and
// screen callback lists remain incomplete. Models/visual names also
// use the previously verified engine/resource evidence.
inline const MenuScreenDefinition& main_menu_definition() {
    static const MenuScreenDefinition definition{"main-menu", 0x04000000, "main_menu", {
        text_definition(0x04100000, "text_screen", {caption_definition(10, 2101)}),
        text_definition(0x04200000, "text_date00", {caption_definition(11, 2102)}),
        text_definition(0x04300000, "text_date01", {caption_definition(12, 2002)}),
        text_definition(0x04400000, "text_profile00", {caption_definition(11, 2103)}),
        text_definition(0x04500000, "text_profile01", {caption_definition(12, 2002)}),
        text_definition(0x04600000, "text_last save00", {caption_definition(11, 2104)}),
        text_definition(0x04700000, "text_last save01", {caption_definition(12, 2002)}),
        button_definition(0x04800000, "bresume game", "normal00", "actived00",
                          MenuControlRole::none, {caption_definition(13, 2105), {0, 0x06001075, 0, 0, 0, {}}}),
        button_definition(0x04900000, "bchange profile", "normal01", "actived01",
                          MenuControlRole::none, {caption_definition(13, 2106), {0, 0x06001001, 2, 0, 0, {}}}),
        button_definition(0x04a00000, "bsingleplayer", "normal02", "actived02",
                          MenuControlRole::none, {caption_definition(13, 2107), {0, 0x06001001, 3, 0, 0, {}}}),
        button_definition(0x04b00000, "bmultiplayerlan", "normal03", "actived03",
                          MenuControlRole::none, {caption_definition(13, 2115), {0, 0x06001099, 0, 0, 0, {}}}),
        button_definition(0x04c00000, "bmultiplayernet", "normal08", "actived08",
                          MenuControlRole::none, {caption_definition(13, 2116), {0, 0x0600109a, 0, 0, 0, {}}}),
        button_definition(0x04d00000, "bcinematics", "normal04", "actived04",
                          MenuControlRole::cinematics, {caption_definition(13, 2109), {0, 0x06001001, 7, 0, 0, {}}}),
        button_definition(0x04e00000, "boptions", "normal05", "actived05",
                          MenuControlRole::options, {caption_definition(13, 2110), {0, 0x06001071, 0, 0, 0, {}}}),
        button_definition(0x04f00000, "bcredits", "normal06", "actived06",
                          MenuControlRole::credits, {caption_definition(13, 2111), {0, 0x0600103a, 0, 0, 0, {}}}),
        button_definition(0x05000000, "bexit", "normal07", "actived07",
                          MenuControlRole::exit, {caption_definition(13, 2112), {0, 0x06001003, 0x04000000, 0, 0, {}}}),
        text_definition(0x05100000, "text_h&dversion", {caption_definition(40, 2006)})
    }};
    return definition;
}

inline const MenuScreenDefinition& confirmation_definition() {
    static const MenuScreenDefinition definition{"confirmation", 0x10000000, "confirm", {
        text_definition(0x10100000, "text", {caption_definition(12, 2004)}),
        button_definition(0x10200000, "byes?", "normal02", "actived03",
                          MenuControlRole::yes, {caption_definition(13, 2001), {0, 0x14001100, 0, 0, 0, {}},
                                                 {0, 0x06001005, 0, 0, 0, {}}}),
        button_definition(0x10300000, "bno?", "normal01", "actived01",
                          MenuControlRole::no, {caption_definition(13, 2000), {0, 0x14001200, 1, 0, 0, {}},
                                                {0, 0x06001005, 0, 0, 0, {}}}),
        button_definition(0x10400000, "bok", "normal03", "actived04",
                          MenuControlRole::ok, {{0, 0x01000004, 0, 0, 0, {}}})
    }};
    return definition;
}

}

#endif
