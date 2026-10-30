#ifndef HD2_INPUT_BINDING_PRODUCER_H
#define HD2_INPUT_BINDING_PRODUCER_H

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace hd2::input {

// Client binding record: 12 bytes at manager+bc. Native vector ownership below
// replaces the original vectors; their configuration/lifetime is separate work.
struct InputBinding {
    std::uint32_t direction;
    std::uint16_t device_kind, code;
    std::uint32_t device_index;
};
static_assert(sizeof(InputBinding) == 12 && offsetof(InputBinding, code) == 6);

// Joystick +50/+54 output, at record+04/+48. The final twelve bytes are not
// interpreted by this producer. This data layout is independent of COM ABI.
struct JoystickInputState {
    std::array<float, 8> axes{};
    std::array<float, 4> pov_angles{};
    std::array<std::uint8_t, 4> pov_unavailable{};
    std::uint32_t buttons = 0;
    std::array<std::uint32_t, 3> reserved{};
};
static_assert(sizeof(JoystickInputState) == 0x44);
static_assert(offsetof(JoystickInputState, buttons) == 0x34);
struct JoystickInput {
    JoystickInputState current, secondary;
};

// Caller must supply the recovered configuration before use. In particular,
// scan order, per-source binding counts and axis limits are not host defaults.
// first/second contain float bits as DWORDs, matching controller input views.
struct BindingInputState {
    std::vector<JoystickInput> joysticks;
    std::array<std::uint8_t, 64> scans{}, key_binding_counts{};
    std::array<std::uint8_t, 6> mouse_binding_counts{};
    std::array<float, 3> mouse_axis_limits{};
    std::vector<InputBinding> bindings;
    std::vector<std::uint32_t> first, second;
    std::uint64_t key_gate = 0;
    std::uint32_t mouse_gate = 0;
};

inline std::uint32_t input_float_word(float value) {
    std::uint32_t word;
    static_assert(sizeof(word) == sizeof(value));
    std::memcpy(&word, &value, sizeof(word));
    return word;
}

// 006c79bd..006c7a99: round signed motion to float before saturating/dividing.
// Negative/zero/NaN limits retain original arithmetic; do not invent defaults.
inline float normalize_binding_motion(std::int32_t motion, float limit) {
    auto value = static_cast<float>(motion);
    if (limit < std::fabs(value)) value = value > 0.0f ? limit : -limit;
    return value / limit;
}

// Both joystick POV branches use these exact single-precision radian bounds.
// Unordered comparisons follow the original x87 branches: NaN yields mask 4.
inline std::uint8_t binding_pov_mask(float angle) {
    if (angle > 3.16f) {
        if (angle > 4.72f) return 9;
        return angle < 4.69f ? 12 : 8;
    }
    if (!(angle < 3.12f)) return 4;
    if (angle < 1.5f) return angle > 0.1f ? 3 : 1;
    return angle > 1.6f ? 6 : 2;
}

inline float directed_binding_axis(float value, std::uint32_t direction) {
    if (direction == 1) return value > 0.0f ? value : 0.0f;
    if (direction == 2) return value < 0.0f ? -value : 0.0f;
    return value;
}

// Joystick kind three. Analog values use current state in BOTH output buffers;
// buttons and POV instead query current/secondary independently. Bad device
// indices and unrecognized analog selectors leave the two cleared outputs.
inline void produce_joystick_binding(BindingInputState& state, std::size_t index,
                                     const InputBinding& binding) {
    state.first[index] = state.second[index] = 0;
    if (binding.device_index >= state.joysticks.size()) return;
    const auto& device = state.joysticks[binding.device_index];
    if (binding.code > 0xff && binding.code < 0x200) {
        const auto bit = std::uint32_t{1} << (binding.code & 31u);
        if (device.current.buttons & bit) state.first[index] = 0x3f800000;
        if (device.secondary.buttons & bit) state.second[index] = 0x3f800000;
    } else if (binding.code >= 0x200) {
        const auto pov = (binding.code >> 4) & 15u;
        // Original indexes outside the four-POV arrays for malformed selectors.
        if (pov >= 4) throw std::out_of_range("Invalid joystick POV binding");
        const auto requested = binding.code & 15u;
        if (!device.current.pov_unavailable[pov] &&
            (binding_pov_mask(device.current.pov_angles[pov]) & requested))
            state.first[index] = 0x3f800000;
        if (!device.secondary.pov_unavailable[pov] &&
            (binding_pov_mask(device.secondary.pov_angles[pov]) & requested))
            state.second[index] = 0x3f800000;
    } else if (binding.code < device.current.axes.size()) {
        const auto value = directed_binding_axis(device.current.axes[binding.code], binding.direction);
        state.first[index] = state.second[index] = input_float_word(value);
    }
}

// Client 006c7870. Services implement the original graphics/joystick methods:
// joystick poll +58(1), snapshots +50/+54; graphics pump +14, mouse poll +e8,
// keyboard poll +9c, key snapshot +b8, buttons +104, signed x/y/z +ec/+f0/+f4.
// Return codes are deliberately ignored. Getters run even when polling is off.
// Backend callbacks must not invalidate borrowed joystick output storage.
template<class Services>
void produce_binding_input(BindingInputState& state, Services& services, bool poll) {
    for (auto count = state.joysticks.size(); count != 0; --count) {
        const auto index = count - 1;
        if (poll) services.poll_joystick(index, 1);
        services.read_joystick_current(index, state.joysticks.at(index).current);
        services.read_joystick_secondary(index, state.joysticks.at(index).secondary);
    }
    if (poll) {
        services.pump_graphics();
        services.poll_mouse();
        services.poll_keyboard();
    }
    const std::uint64_t keys = services.keyboard_snapshot();
    const std::uint32_t buttons = services.mouse_buttons();
    const auto x = services.mouse_x();
    const auto y = services.mouse_y();
    const auto z = services.mouse_z();
    auto key_counts = state.key_binding_counts;
    auto mouse_counts = state.mouse_binding_counts;
    state.mouse_gate = ~(state.mouse_gate & buttons) ^ buttons;
    state.key_gate = ~(state.key_gate & keys) ^ keys;
    const std::array<float, 3> axes{{
        normalize_binding_motion(x, state.mouse_axis_limits[0]),
        normalize_binding_motion(y, state.mouse_axis_limits[1]),
        normalize_binding_motion(z, state.mouse_axis_limits[2])}};

    const auto count = state.bindings.size();
    if (state.first.size() < count || state.second.size() < count)
        throw std::length_error("Input output buffers do not cover binding records");
    for (auto remaining = count; remaining != 0; --remaining) {
        const auto index = remaining - 1;
        const auto binding = state.bindings[index];
        if (binding.device_kind == 1) {
            std::size_t slot = 0;
            const auto scan = static_cast<std::uint8_t>(binding.code);
            while (slot < state.scans.size() && state.scans[slot] != scan) ++slot;
            // Original __allshl returns zero for shift 64. Do not mask to 63.
            const auto bit = slot < 64 ? std::uint64_t{1} << slot : 0;
            state.first[index] = (keys & bit) ? 0x3f800000 : 0;
            state.second[index] = 0;
            if ((keys & bit) && (state.key_gate & bit)) {
                if (--key_counts[slot] == 0) state.key_gate &= ~bit;
                state.second[index] = 0x3f800000;
            }
        } else if (binding.device_kind == 2) {
            state.first[index] = 0;
            std::size_t slot = 0;
            switch (binding.code) {
            case 1: slot = 0; break;
            case 2: slot = 1; break;
            case 4: slot = 2; break;
            case 8: slot = 3; break;
            case 16: slot = 4; break;
            case 0x100: case 0x200: case 0x300: {
                const auto value = directed_binding_axis(axes[(binding.code >> 8) - 1], binding.direction);
                state.first[index] = state.second[index] = input_float_word(value);
                continue;
            }
            default: continue; // Original preserves the previous second value.
            }
            const auto bit = std::uint32_t{1} << slot;
            state.first[index] = (buttons & bit) ? 0x3f800000 : 0;
            state.second[index] = 0;
            if ((buttons & bit) && (state.mouse_gate & bit)) {
                if (--mouse_counts[slot] == 0) state.mouse_gate &= ~bit;
                state.second[index] = 0x3f800000;
            }
        } else if (binding.device_kind == 3) {
            produce_joystick_binding(state, index, binding);
        } // Other kinds preserve both previous output words.
    }
}

}
#endif
