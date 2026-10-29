#ifndef HD2_FRONTEND_SCREEN_TARGET_H
#define HD2_FRONTEND_SCREEN_TARGET_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace hd2::frontend {

// Client 0065c830. Controls contains at most INT_MAX identifiers; fixed arrays
// and definition-backed portable vectors share the original mask/first-match rule.
template<class Controls>
int find_screen_control(std::uint32_t screen_id,
                        const Controls& controls,
                        std::uint32_t target) {
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
