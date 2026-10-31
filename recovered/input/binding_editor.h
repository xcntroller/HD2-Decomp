#ifndef HD2_INPUT_BINDING_EDITOR_H
#define HD2_INPUT_BINDING_EDITOR_H
#include "input/binding_configuration.h"
#include <optional>

namespace hd2::input {
// 006A8230, literal NUL-terminated scan list008424F0.
inline bool reserved_binding_scan(std::uint16_t scan) {
    const auto byte = static_cast<std::uint8_t>(scan);
    return (byte >= 0x3b && byte <= 0x44) || byte == 0x57 || byte == 0x58 || byte == 1 || byte == 0xc5;
}
// 006A8480: only overlapping modes conflict. Hidden commands never lose their
// binding; the two original P commands (captions2848/2856) may share a key.
inline void remove_binding_conflicts(BindingConfiguration &configuration, std::size_t index) {
    const auto selected = configuration.settings.at(index);
    for (std::size_t i = 0; i < configuration.settings.size(); ++i) {
        auto &other = configuration.settings[i];
        if (i == index || other.group == 0xffffffffu || other.device_kind != selected.device_kind ||
            other.code != selected.code || other.direction != selected.direction)
            continue;
        if ((other.caption == 0xb28 && selected.caption == 0xb20) ||
            (other.caption == 0xb20 && selected.caption == 0xb28))
            continue;
        const bool overlap = selected.mode == 1 ? other.mode >= 1 && other.mode <= 3
                             : selected.mode == 2
                                 ? other.mode == 1 || other.mode == 2
                                 : selected.mode == 3 && (other.mode == 1 || other.mode == 3);
        if (overlap) {
            other.device_kind = 0;
            other.code = 0;
            other.dirty = 1;
        }
    }
}
// 006A83C0. Direction survives duplicate clearing; no implicit manager apply.
inline bool edit_binding(BindingConfiguration &configuration, std::size_t index, std::uint16_t kind,
                         std::uint16_t code, std::uint32_t direction) {
    if (index >= configuration.settings.size() || (kind == 1 && reserved_binding_scan(code)) ||
        (kind == 2 && (code == 0x100 || code == 0x200)))
        return false;
    auto &record = configuration.settings[index];
    record.device_kind = kind;
    record.code = code;
    record.direction = direction;
    record.dirty = 1;
    remove_binding_conflicts(configuration, index);
    return true;
}
// 006A85B0 includes hidden records, in vector order.
inline std::optional<std::size_t> first_unassigned_binding(const BindingConfiguration &configuration) {
    for (std::size_t i = 0; i < configuration.settings.size(); ++i)
        if (!configuration.settings[i].device_kind && !configuration.settings[i].code)
            return i;
    return {};
}
// 006A8290 returns a ONE-based index within the preceding contiguous group.
inline std::size_t binding_group_ordinal(const BindingConfiguration &configuration, std::size_t index) {
    const auto group = configuration.settings.at(index).group;
    auto first = index;
    while (first && configuration.settings[first - 1].group == group)
        --first;
    return index - first + 1;
}
struct BindingCandidate {
    std::uint16_t kind, code;
    std::uint32_t direction;
};
// The Options mask0x9A in006C84A0 excludes all joystick paths. Keyboard
// modifiers precede the scan; mouse axes precede wheel, although edit rejects X/Y.
inline std::optional<BindingCandidate> options_binding_candidate(std::uint32_t modifiers, std::uint32_t scan,
                                                                 std::uint32_t buttons, std::int32_t x,
                                                                 std::int32_t y, std::int32_t z) {
    if (modifiers || (scan & 0xff)) {
        for (const auto pair : {std::pair<std::uint32_t, std::uint16_t>{0x20, 0x2a},
                                {0x40, 0x36},
                                {0x80, 0x1d},
                                {0x100, 0x9d},
                                {0x200, 0x38},
                                {0x400, 0xb8}})
            if (modifiers & pair.first)
                return BindingCandidate{1, pair.second, 0};
        return BindingCandidate{1, static_cast<std::uint16_t>(scan & 0xff), 0};
    }
    for (std::uint16_t bit = 1; bit <= 16; bit <<= 1)
        if (buttons & bit)
            return BindingCandidate{2, bit, 0};
    for (const auto pair : {std::pair<std::uint16_t, std::int32_t>{0x100, x}, {0x200, y}, {0x300, z}})
        if (pair.second > 10 || pair.second < -10)
            return BindingCandidate{2, pair.first, pair.second > 0 ? 1u : 2u};
    return {};
}
} // namespace hd2::input
#endif
