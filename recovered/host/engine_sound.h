#ifndef HD2_HOST_ENGINE_SOUND_H
#define HD2_HOST_ENGINE_SOUND_H

#include "host/menu_scene.h"
#include "frontend/sound_startup.h"

namespace hd2::host {

// Original ISnd driver owns its DirectSound/COM resources. This host only
// borrows the DLL singleton, opens it once and closes after menu destruction.
class EngineSound {
public:
    EngineSound(HMODULE module, HWND window) {
        const auto entry = GetProcAddress(module, "ISndGetDriver");
        if (reinterpret_cast<std::uintptr_t>(entry) !=
                reinterpret_cast<std::uintptr_t>(module) + 0x68fa0) {
            throw std::runtime_error("Missing or unsupported ISndGetDriver export");
        }
        using GetDriver = EngineFrame* (__cdecl *)();
        GetDriver get_driver = nullptr;
        static_assert(sizeof(entry) == sizeof(get_driver));
        std::memcpy(&get_driver, &entry, sizeof(entry));
        driver_ = get_driver();
        initialize_ = scene_method<Initialize>(driver_, module, 4, 0x7b560);
        close_ = scene_method<Close>(driver_, module, 8, 0x7a850);
        initialized_ = scene_method<Initialized>(driver_, module, 0x0c, 0x7a840);
        if (initialized()) {
            throw std::runtime_error("Engine sound driver is already owned by another path");
        }
        // Bounded host defaults, not recovered user preferences: options clear,
        // quality index zero. Original 006db3b0 composes these as flags 0402.
        const auto flags = frontend::sound_startup_flags(false, false, 0);
        owns_ = frontend::start_menu_sound(*this, window, flags);
        std::printf("Original sound driver startup: status=%08x initialized=%u "
                    "rate=22050 channels=2 bits=16 flags=%04x\n",
                    static_cast<unsigned>(status_), static_cast<unsigned>(owns_),
                    static_cast<unsigned>(flags));
        if (!owns_) std::puts("Original sound driver unavailable; continuing without audio");
    }
    ~EngineSound() { if (owns_) close(); }
    EngineSound(const EngineSound&) = delete;
    EngineSound& operator=(const EngineSound&) = delete;

    std::int32_t initialize(HWND window, std::uint32_t rate, std::uint32_t channels,
                            std::uint32_t bits, std::uint32_t flags) {
        close_pending_ = true;
        status_ = initialize_(driver_, window, rate, channels, bits, flags);
        return status_;
    }
    bool initialized() const { return initialized_(driver_) != 0; }
    void close() noexcept {
        if (!close_pending_) return;
        close_pending_ = false;
        owns_ = false;
        close_(driver_);
        std::printf("Original sound driver closed: initialized=%u\n",
                    static_cast<unsigned>(initialized()));
    }

private:
    using Initialize = std::int32_t (WINAPI *)(EngineFrame*, HWND, std::uint32_t,
                                             std::uint32_t, std::uint32_t, std::uint32_t);
    using Close = void (WINAPI *)(EngineFrame*);
    using Initialized = std::uint8_t (WINAPI *)(EngineFrame*);
    EngineFrame* driver_ = nullptr;
    Initialize initialize_ = nullptr;
    Close close_ = nullptr;
    Initialized initialized_ = nullptr;
    bool owns_ = false;
    bool close_pending_ = false;
    std::int32_t status_ = 0;
};

}

#endif
