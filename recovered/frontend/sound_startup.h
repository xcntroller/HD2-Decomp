#ifndef HD2_FRONTEND_SOUND_STARTUP_H
#define HD2_FRONTEND_SOUND_STARTUP_H

#include <cstdint>

namespace hd2::frontend {

// Client 006db3b0: byte 008aed06 chooses 2/12, byte 008aed08 adds 20,
// byte 008aed09 selects one quality bit. Meanings of the option bytes are
// unresolved; this helper does not read the original settings globals.
constexpr std::uint32_t sound_startup_flags(bool option_06, bool option_08,
                                           std::uint8_t quality) {
    std::uint32_t flags = option_06 ? 0x12u : 0x02u;
    if (option_08) flags |= 0x20u;
    switch (quality) {
    case 0: flags |= 0x0400u; break;
    case 1: flags |= 0x0800u; break;
    case 2: flags |= 0x1000u; break;
    case 3: flags |= 0x2000u; break;
    default: break;
    }
    return flags;
}

// Host lifecycle policy around the original startup call. Backend must be
// uninitialized. Failed/partial initialization is closed before silent fallback.
// Success leaves shutdown to the owner, after scene/frame resources are gone.
template<class Backend, class Window>
bool start_menu_sound(Backend& backend, Window window, std::uint32_t flags) {
    const auto status = backend.initialize(window, 22050, 2, 16, flags);
    if (status < 0 || !backend.initialized()) {
        backend.close();
        return false;
    }
    return true;
}

}

#endif
