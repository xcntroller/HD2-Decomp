#ifndef HD2_FRONTEND_OPTIONS_DEFINITION_H
#define HD2_FRONTEND_OPTIONS_DEFINITION_H
#include "frontend/menu_definition.h"

namespace hd2::frontend {
// 0061CF60 and its five page builders. Dynamic binding groups4..8 are appended
// by0061F5A0's concrete model builder before the two layout buttons9/10.
inline MenuScreenDefinition options_definition(ControlObserver category, ControlObserver defaults,
                                               bool mission_active=false) {
    using K = MenuControlKind;
    MenuScreenDefinition result{"options", 0x18000000, "options", {}};
    auto control = [](std::uint32_t id, const char *frame, K kind, std::vector<ControlEvent> records = {}) {
        return MenuControlDefinition{id,
                                     frame,
                                     kind,
                                     kind == K::button ? "normal*" : nullptr,
                                     kind == K::button ? "actived*" : nullptr,
                                     MenuControlRole::none,
                                     std::move(records)};
    };
    auto text = [&](std::uint32_t id, const char *frame, std::uint32_t style, std::uint32_t label) {
        return control(id, frame, K::text, {caption_definition(style, label)});
    };
    result.controls = {text(0x18100000, "text_screen", 10, 0x961), text(0x18200000, "text_date00", 11, 0x962),
                       text(0x18300000, "text_date01", 12, 0x963),
                       text(0x18400000, "text_profile00", 11, 0x964)};
    auto tabs = control(0x18500000, "GROptions", K::container);
    const char *buttons[] = {"bvideo setup", "bsound setup",       "bcontrols",
                             "bgame setup",  "bmultiplayer setup", "bback"};
    const unsigned pages[] = {6, 7, 8, 10, 11};
    for (unsigned i = 0; i < 6; ++i) {
        auto button =
            control(0x18500000 | ((i + 1) << 12), buttons[i], K::button, {caption_definition(13, 0x965 + i)});
        button.initialization.push_back(
            i == 5 ? ControlEvent{0, 0x060010c4, 0, 0, 0, {}}
                   : ControlEvent{0x18000000u | (pages[i] << 20), 0x05001003, 0, 0, 0, {}});
        tabs.children.push_back(std::move(button));
    }
    result.controls.push_back(std::move(tabs));
    const char *names[] = {"graphics setup", "sound setup", "keysscreen",
                           "keysscreen01",   "gameoptions", "network settings"};
    for (unsigned page = 6; page <= 11; ++page) {
        auto group = control(0x18000000u | (page << 20), names[page - 6], K::container,
                             {{0, 0x01000004, 0, 0, 0, {}}});
        for (unsigned other = 6; other <= 11; ++other)
            if (other != page && (page != 9 || other == 8))
                group.initialization.push_back({0x18000000u | (other << 20), 0x0500d004, 0, 0, 0, {}});
        result.controls.push_back(std::move(group));
    }
    auto add = [&](unsigned page, const char *frame, K kind, std::uint32_t style = 0,
                   std::uint32_t label = 0) -> MenuControlDefinition & {
        auto &group = result.controls.at(page - 1);
        auto entry = control(
            group.identifier | ((static_cast<std::uint32_t>(group.children.size()) + 1) << 12), frame, kind);
        if (style)
            entry.initialization.push_back(caption_definition(style, label));
        group.children.push_back(std::move(entry));
        return group.children.back();
    };
    auto ruler = [&](unsigned page, const char *frame, bool inset = false) {
        auto &entry = add(page, frame, K::text);
        if (inset)
            entry.initialization.push_back({0, 0x02000073, 1, 0, 0, {}});
        for (unsigned i = 0; i < 3; ++i)
            entry.initialization.push_back({0, 0x01000017, 27 + i, 2030 + i, 0, {}});
    };
    auto slider = [&](unsigned page, const char *frame) {
        add(page, frame, K::slider).initialization.push_back({0, 0x01000020, 100, 0, 0, {}});
    };
    add(6, "text_graphics setup", K::text, 11, 0x96b);
    add(6, "text_gama correction", K::text, 11, 0x96c);
    ruler(6, "text_min norm max02");
    slider(6, "scrollbar03");
    auto &detail = add(6, "geometry detail", K::selector);
    for (unsigned i = 0; i < 4; ++i)
        detail.initialization.push_back({0, 0x01000067, 0x96e + i, 0, 0, {}});
    const char *features[] = {"GD00", "GD01", "GD02", "GD03", "GD04", "GD05", "GD06"};
    for (unsigned i = 0; i < 7; ++i) {
        auto &entry = add(6, features[i], K::toggle, 13, 0x972 + i);
        if(i==3 && mission_active) entry.initialization.push_back({0,0x01000001,0,0,0,{}});
        if (i == 6)
            entry.initialization.push_back({0, 0x01000004, 0, 0, 0, {}});
    }
    add(6, "text_modifications", K::text, 11, 0x97e);
    add(7, "text_sound setup", K::text, 11, 0x979);
    add(7, "text_sound volume", K::text, 11, 0x97a);
    ruler(7, "text_min norm max");
    add(7, "text_music volume", K::text, 11, 0x97c);
    ruler(7, "text_min norm max01", true);
    slider(7, "scrollbar01");
    slider(7, "scrollbar02");
    add(8, "text_controls setup", K::text, 11, 0x989);
    add(8, "bcontrols01", K::button, 13, 0x98a)
        .initialization.push_back({0x18900000, 0x05001003, 0, 0, 0, {}});
    add(8, "text_mouse setup", K::text, 11, 0x98b);
    add(8, "invert mouse", K::toggle, 13, 0x98c);
    add(8, "text_mouse sensitivity", K::text, 11, 0x98d);
    slider(8, "scrollbar");
    ruler(8, "text_minimal normal maximal");
    const char *hidden[] = {"text_joystick setup", "bcalibrate", "force feedback", "lock fire"};
    for (unsigned i = 0; i < 4; ++i)
        add(8, hidden[i],
            i == 0   ? K::text
            : i == 1 ? K::button
                     : K::toggle,
            i == 0 ? 11 : 13, 0x98f + i)
            .initialization.push_back({0, 0x01000004, 0, 0, 0, {}});
    add(9, "text_last save01", K::text, 11, 0x993);
    auto &categories = add(9, "controls", K::selector);
    for (unsigned i = 0; i < 5; ++i)
        categories.initialization.push_back({0, 0x01000067, 0x994 + i, 0, 0, {}});
    categories.initialization.push_back(
        observer_control_event(0, category.tag, category.context, category.callback));
    add(9, "scroll00", K::slider).initialization.push_back({0, 0x0100000b, 0x18904000, 0, 0, {}});
    add(10, "text_game options", K::text, 11, 0x99d);
    add(10, "subtitles", K::toggle, 13, 0x99e);
    auto& crosshair=add(10, "parental lock", K::toggle, 13, 0x4ef4);
    if(mission_active) crosshair.initialization.push_back({0, 0x01000001, 0, 0, 0, {}});
    add(11, "text_connection rate", K::text).initialization.push_back({0, 0x01000004, 0, 0, 0, {}});
    add(11, "connection", K::selector).initialization.push_back({0, 0x01000004, 0, 0, 0, {}});
    add(11, "text_voice-chat settings", K::text, 11, 0x981);
    add(11, "enable voice-chat", K::toggle, 13, 0x97f);
    add(11, "text_voice-chat volume", K::text, 11, 0x980);
    slider(11, "scrollbar04");
    ruler(11, "text_min norm max020", true);
    add(11, "text_network setup", K::text, 11, 0x9a9);
    result.controls.push_back(text(0x18c00000, "text_h&dversion", 40, 2006));
    // Defaults are inserted after the model builder has appended five lists.
    (void)defaults;
    return result;
}
} // namespace hd2::frontend
#endif
