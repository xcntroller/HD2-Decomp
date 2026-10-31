#ifndef HD2_FRONTEND_MAIN_MENU_STATE_H
#define HD2_FRONTEND_MAIN_MENU_STATE_H
#include "frontend/control_events.h"
#include <array>
#include <optional>
#include <stdexcept>
#include <string>
namespace hd2::frontend {
// 006B3AC0 parses profile+08 (ProfilePayload::strings[2]), not a file timestamp.
//  Retain the third field's remainder, including further '$' characters.
inline std::optional<std::array<std::string, 3>> main_save_caption(const std::string &stored) {
    const std::string value(stored.c_str());
    if (value.empty())
        return {};
    if (value.size() >= 1024)
        throw std::length_error("Original last-save caption buffer exceeded");
    const auto first = value.find('$');
    if (first == std::string::npos)
        return {};
    const auto second = value.find('$', first + 1);
    if (second == std::string::npos)
        return {};
    return std::array<std::string, 3>{value.substr(0, first), value.substr(first + 1, second - first - 1),
                                      value.substr(second + 1)};
}
// 00644200 style40: initialized version globals00844F88=1,00844F8C=12;
// 009820F4 is zero-initialized BSS (only the formatter references it).
inline std::string main_version_text(const std::string &format) {
    const std::array<unsigned, 3> values{1, 12, 0};
    unsigned argument = 0;
    std::string result;
    for (std::size_t i = 0; i < format.size(); ++i) {
        if (format[i] != '%') {
            result += format[i];
            continue;
        }
        if (++i == format.size())
            throw std::runtime_error("Incomplete version format");
        if (format[i] == '%') {
            result += '%';
            continue;
        }
        bool padded = false;
        if (format.compare(i, 3, "02d") == 0) {
            padded = true;
            i += 2;
        } else if (format[i] != 'd')
            throw std::runtime_error("Unsupported version format");
        if (argument == values.size())
            throw std::runtime_error("Too many version arguments");
        auto number = std::to_string(values[argument++]);
        if (padded && number.size() < 2)
            result += '0';
        result += number;
    }
    if (argument != values.size())
        throw std::runtime_error("Incomplete version arguments");
    return result;
}
// 00627520 Main shown callback. Control availability (real receiving owners)
//  stays separate from these original enabled/disabled state messages.
inline void refresh_main_menu(ControlEventDestination &queue, const std::string &profile,
                              const std::string &stored_caption) {
    const bool active = !profile.empty();
    const auto caption = active ? main_save_caption(stored_caption) : std::nullopt;
    const auto label = [&](unsigned target, unsigned style, unsigned id) {
        queue.enqueue({target, 0x01000015, style, id, 0, {}});
    };
    const auto text = [&](unsigned target, unsigned style, const std::string &value,
                          unsigned code = 0x01000014) {
        ControlEvent event{target, code, style, 0, 0, {}};
        event.text = std::make_shared<const std::string>(value);
        queue.enqueue(event);
    };
    if (active)
        text(0x04500000, 12, profile);
    else
        label(0x04500000, 12, 2020);
    queue.enqueue({0x04700000, 0x02000018, 0, 0, 0, {}});
    if (caption)
        for (const auto &line : *caption)
            text(0x04700000, 7, line, 0x01000016);
    else
        label(0x04700000, 12, 2021);
    queue.enqueue({0x04800000, caption ? 0x01000002u : 0x01000001u, 0, 0, 0, {}});
    label(0x04900000, 13, active ? 2106 : 2117);
    for (unsigned id = 0x04a00000; id <= 0x04f00000; id += 0x00100000)
        queue.enqueue({id, active ? 0x01000002u : 0x01000001u, 0, 0, 0, {}});
}
} // namespace hd2::frontend
#endif
