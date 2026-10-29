#ifndef HD2_INPUT_MOUSE_POLL_H
#define HD2_INPUT_MOUSE_POLL_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace hd2::input {

struct MouseState {
    std::int32_t x = 0, y = 0, z = 0;
    std::array<std::uint8_t, 8> buttons{};
};
static_assert(sizeof(MouseState) == 20);
static_assert(offsetof(MouseState, buttons) == 12);

// LS3DF 1006f8a0: bounded device-state branch. The caller supplies the borrowed
// input device, capability axis count and swap setting. Window metric queries
// and exclusive-mode absolute-coordinate accumulation remain outside this slice.
template<class Device>
std::uint32_t poll_mouse(Device& device, MouseState& state,
                         std::uint32_t axes, bool swap_buttons) {
    if (!device.available()) return 0x80000009u;
    if (device.read(state) < 0) {
        device.acquire(); // Its result is ignored by the original; retry once.
        if (device.read(state) < 0) {
            state = {};
            return 0x80000005u;
        }
    }
    if (axes < 3) state.z = 0;
    if (swap_buttons) std::swap(state.buttons[0], state.buttons[1]);
    return 0;
}

// LS3DF 1006fb30: only the first five high bits contribute, despite eight bytes.
inline std::uint32_t mouse_button_mask(const MouseState& state) {
    std::uint32_t result = 0;
    for (std::uint32_t index = 0; index < 5; ++index) {
        if ((state.buttons[index] & 0x80) != 0) result |= 1u << index;
    }
    return result;
}

}

#endif
