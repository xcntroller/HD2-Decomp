#ifndef HD2_INPUT_KEYBOARD_H
#define HD2_INPUT_KEYBOARD_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace hd2::input {

struct KeyboardState {
    std::array<std::uint8_t, 256> current{}, previous{};
    std::uint32_t flags = 0, debug_flags = 0;
    std::array<std::uint8_t, 3> debug_toggles{};
    std::uint32_t pending_scan = 0, pending_character = 0, character = 0;
    std::uint32_t repeat_scan = 0xffffffffu, repeat_start = 0, repeat_last = 0;
};

// 1006f550/570/590 and 1006f5b0. Keep separate left/right flag bits, in
// addition to each modifier family's aggregate bit.
inline std::uint32_t keyboard_modifiers(const KeyboardState& state) {
    const auto down = [&](std::size_t scan) { return (state.current[scan] & 0x80u) != 0; };
    std::uint32_t result = 0;
    if (down(0x2a) || down(0x36)) {
        result = 1;
        if (down(0x2a)) result = 0x21;
        if (down(0x36)) result |= 0x40;
    }
    if (down(0x1d) || down(0x9d)) {
        result |= 4;
        if (down(0x1d)) result |= 0x80;
        if (down(0x9d)) result |= 0x100;
    }
    if (down(0x38) || down(0xb8)) {
        result |= 8;
        if (down(0x38)) result |= 0x200;
        if (down(0xb8)) result |= 0x400;
    }
    return result;
}

struct KeyboardCharacter {
    std::uint8_t scan, normal, shifted;
};

// 1006f660: first matching scan byte wins; only modifier bit zero selects
// the shifted column. Caps Lock, Ctrl and Alt do not change this lookup.
template<std::size_t Size>
std::uint8_t translate_keyboard_character(const std::array<KeyboardCharacter, Size>& table,
                                           std::uint32_t scan, std::uint8_t modifiers) {
    for (const auto& entry : table) {
        if (entry.scan == 0) break;
        if (entry.scan == static_cast<std::uint8_t>(scan))
            return (modifiers & 1u) ? entry.shifted : entry.normal;
    }
    return 0;
}

// LS3DF 1006f190. Keyboard retry failure deliberately returns success with a
// cleared current state. Hotkey toggles run only on initial-read success, not
// on successful retry. Lost acquisition pumps graphics before trying again.
template<class Device>
std::uint32_t poll_keyboard(Device& device, KeyboardState& state) {
    if (!device.available()) return 0x80000009u;
    state.previous = state.current;
    state.current = {};
    if (device.read(state.current) < 0) {
        auto result = device.acquire();
        while (static_cast<std::uint32_t>(result) == 0x8007001eu) {
            device.pump();
            result = device.acquire();
        }
        if (device.read(state.current) < 0) state.current = {};
    } else if (!(state.debug_flags & 0x10000u) && (state.current[0x2a] & 0x80u) &&
               (state.current[0x38] & 0x80u) && (state.current[0x1d] & 0x80u)) {
        constexpr std::array<std::uint8_t, 3> keys{{0x58, 0x57, 0x44}};
        for (std::size_t i = 0; i < keys.size(); ++i) {
            const auto key = keys[i];
            if ((state.current[key] & 0x80u) && !(state.previous[key] & 0x80u))
                state.debug_toggles[i] = state.debug_toggles[i] == 0;
        }
    }
    return 0;
}

inline std::int32_t signed_keyboard_time(std::uint32_t word) {
    std::int32_t result;
    std::memcpy(&result, &word, sizeof(result));
    return result;
}

// LS3DF 1006f320. Translation/modifier services correspond to IGraph +c8/+cc;
// the concrete keyboard device, character tables and message-mode producer are
// separate dependencies. Repeat testing occurs INSIDE the scan loop, so a due
// repeat can precede a new key at a higher scan index. Do not reorder it.
template<class Services>
std::uint32_t next_keyboard_scan(KeyboardState& state, Services& services) {
    if (state.flags & 2u) {
        const auto result = state.pending_scan;
        state.pending_scan = 0;
        state.character = state.pending_character;
        state.pending_character = 0;
        return result;
    }
    for (std::uint32_t scan = 0; scan < state.current.size(); ++scan) {
        if ((state.current[scan] & 0x80u) && state.previous[scan] == 0 &&
                scan != 0x1d && scan != 0x9d && scan != 0x2a &&
                scan != 0x36 && scan != 0x38 && scan != 0xb8) {
            const auto modifiers = services.modifiers(0);
            state.character = static_cast<std::uint8_t>(services.translate(scan, modifiers & 0xdu));
            if (state.flags & 4u) {
                state.repeat_scan = scan;
                state.repeat_start = services.milliseconds();
                state.repeat_last = 0;
            }
            return scan;
        }
        if ((state.flags & 4u) && state.repeat_scan != 0xffffffffu) {
            if (state.repeat_scan >= state.current.size())
                throw std::out_of_range("Invalid native keyboard repeat scan");
            if (state.current[state.repeat_scan] == 0) {
                state.repeat_scan = 0xffffffffu;
                return 0;
            }
            const std::uint32_t now = services.milliseconds();
            const auto elapsed = state.repeat_last ? now - state.repeat_last : now - state.repeat_start;
            const auto threshold = state.repeat_last ? 34 : 350;
            if (signed_keyboard_time(elapsed) > threshold) {
                state.repeat_last = now;
                return state.repeat_scan;
            }
        }
    }
    return 0;
}

// LS3DF 1006f470: destructive character read; repeats do not retranslate it.
inline std::uint32_t take_keyboard_character(KeyboardState& state) {
    const auto result = state.character;
    state.character = 0;
    return result;
}

} // namespace hd2::input
#endif
