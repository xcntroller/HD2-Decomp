#ifndef HD2_FRONTEND_SCREEN_TARGET_H
#define HD2_FRONTEND_SCREEN_TARGET_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace hd2::frontend {

// Client 0065c830.
template<std::size_t N>
int find_screen_control(std::uint32_t screen_id,
                        const std::array<std::uint32_t, N>& controls,
                        std::uint32_t target) {
    static_assert(N <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    const std::uint32_t mask = (screen_id & 0x000ff000) != 0 ? 0x00000ff0 :
                               (screen_id & 0x03f00000) != 0 ? 0x000ff000 :
                                                               0x03f00000;
    for (std::size_t i = 0; i < controls.size(); ++i) {
        if ((controls[i] & mask) == (target & mask)) return static_cast<int>(i);
    }
    return -1;
}

}

#endif
