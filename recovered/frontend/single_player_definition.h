#ifndef HD2_FRONTEND_SINGLE_PLAYER_DEFINITION_H
#define HD2_FRONTEND_SINGLE_PLAYER_DEFINITION_H

#include "frontend/menu_definition.h"

namespace hd2::frontend {

// 00623DD0 static screen3; callbacks0063B590/0063B740 are supplied by the
//  owning flow. Hidden declarations and the side-image order stay original.
inline MenuScreenDefinition single_player_definition(void *owner, ObserverCallback shown,
                                                     ObserverCallback mode) {
    MenuScreenDefinition result{"SINGLEPLAYER", 0x0c000000, "singleplayer", {}};
    result.controls.push_back({0x0c100000,
                               "text_screen",
                               MenuControlKind::text,
                               nullptr,
                               nullptr,
                               MenuControlRole::none,
                               {{0, 0x01000015, 0x0000000a, 0x00000899, 0, {}}}});
    result.controls.push_back(
        {0x0c200000,
         "single side",
         MenuControlKind::image_selector,
         nullptr,
         nullptr,
         MenuControlRole::none,
         {{0, 0x01000032, 0, 0, 0, {}, {}, std::make_shared<const std::string>("ss1")},
          {0, 0x01000032, 0, 0, 0, {}, {}, std::make_shared<const std::string>("ss2")},
          {0, 0x01000032, 0, 0, 0, {}, {}, std::make_shared<const std::string>("ss3")},
          {0, 0x01000032, 0, 0, 0, {}, {}, std::make_shared<const std::string>("ss4")},
          {0, 0x01000032, 0, 0, 0, {}, {}, std::make_shared<const std::string>("ss5")},
          {0, 0x01000032, 0, 0, 0, {}, {}, std::make_shared<const std::string>("ss6")},
          {0, 0x01000032, 0, 0, 0, {}, {}, std::make_shared<const std::string>("ss8")},
          {0, 0x01000032, 0, 0, 0, {}, {}, std::make_shared<const std::string>("ss7")}}});
    result.controls.push_back(
        {0x0c300000, "text00", MenuControlKind::text, nullptr, nullptr, MenuControlRole::none, {}});
    result.controls.push_back({0x0c400000,
                               "bcampaign",
                               MenuControlKind::button,
                               "normal*",
                               "actived*",
                               MenuControlRole::single_player,
                               {{0, 0x01000015, 0x0000000d, 0x0000089a, 0, {}},
                                observer_control_event(0, 0x04003000, owner, mode),
                                observer_control_event(0, 0x04001000, owner, mode)}});
    result.controls.push_back({0x0c500000,
                               "blone wolf",
                               MenuControlKind::button,
                               "normal*",
                               "actived*",
                               MenuControlRole::single_player,
                               {{0, 0x01000015, 0x0000000d, 0x0000089b, 0, {}},
                                observer_control_event(0, 0x04003000, owner, mode),
                                observer_control_event(0, 0x04001000, owner, mode)}});
    result.controls.push_back({0x0c600000,
                               "carnage",
                               MenuControlKind::button,
                               "normal*",
                               "actived*",
                               MenuControlRole::single_player,
                               {{0, 0x01000015, 0x0000000d, 0x0000089c, 0, {}},
                                observer_control_event(0, 0x04003000, owner, mode),
                                observer_control_event(0, 0x04001000, owner, mode)}});
    result.controls.push_back({0x0c700000,
                               "bcampaign01",
                               MenuControlKind::button,
                               "normal*",
                               "actived*",
                               MenuControlRole::single_player,
                               {{0, 0x01000015, 0x0000000d, 0x0000089d, 0, {}},
                                observer_control_event(0, 0x04003000, owner, mode),
                                observer_control_event(0, 0x04001000, owner, mode)}});
    result.controls.push_back({0x0c800000,
                               "blone wolf01",
                               MenuControlKind::button,
                               "normal*",
                               "actived*",
                               MenuControlRole::single_player,
                               {{0, 0x01000015, 0x0000000d, 0x0000089e, 0, {}},
                                observer_control_event(0, 0x04003000, owner, mode),
                                observer_control_event(0, 0x04001000, owner, mode)}});
    result.controls.push_back({0x0c900000,
                               "carnage01",
                               MenuControlKind::button,
                               "normal*",
                               "actived*",
                               MenuControlRole::single_player,
                               {{0, 0x01000004, 0x00000000, 0x0, 0, {}}}});
    result.controls.push_back({0x0ca00000,
                               "bsingle mission",
                               MenuControlKind::button,
                               "normal*",
                               "actived*",
                               MenuControlRole::single_player,
                               {{0, 0x01000004, 0x00000000, 0x0, 0, {}}}});
    result.controls.push_back({0x0cb00000,
                               "bsingle mission- carnage",
                               MenuControlKind::button,
                               "normal*",
                               "actived*",
                               MenuControlRole::single_player,
                               {{0, 0x01000004, 0x00000000, 0x0, 0, {}}}});
    result.controls.push_back({0x0cc00000,
                               "bback",
                               MenuControlKind::button,
                               "normal*",
                               "actived*",
                               MenuControlRole::single_player,
                               {{0, 0x01000015, 0x0000000d, 0x000008a0, 0, {}},
                                observer_control_event(0, 0x04003000, owner, mode),
                                observer_control_event(0, 0x04001000, owner, mode)}});
    result.controls.push_back({0x0cd00000,
                               "text_h&dversion",
                               MenuControlKind::text,
                               nullptr,
                               nullptr,
                               MenuControlRole::none,
                               {{0, 0x01000015, 0x00000028, 0x000007d6, 0, {}}}});
    result.controls.push_back(
        {0x0ce00000,
         "text_HD2original",
         MenuControlKind::text,
         nullptr,
         nullptr,
         MenuControlRole::none,
         {{0, 0x01000015, 0x0000000d, 0x00004fb0, 0, {}}, {0, 0x01000004, 0x00000000, 0x0, 0, {}}}});
    result.controls.push_back(
        {0x0cf00000,
         "text_HD2datadisk",
         MenuControlKind::text,
         nullptr,
         nullptr,
         MenuControlRole::none,
         {{0, 0x01000015, 0x0000000d, 0x00004fb1, 0, {}}, {0, 0x01000004, 0x00000000, 0x0, 0, {}}}});
    result.initialization = {observer_control_event(0, 0x0400d000, owner, shown)};
    return result;
}

} // namespace hd2::frontend
#endif
