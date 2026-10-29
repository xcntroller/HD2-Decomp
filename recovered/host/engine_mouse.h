#ifndef HD2_HOST_ENGINE_MOUSE_H
#define HD2_HOST_ENGINE_MOUSE_H

#include "host/menu_scene.h"
#include "input/mouse_poll.h"

namespace hd2::host {

// Bounded bridge to LS3DF's existing DirectInput ownership. No borrowed device
// is released here: engine mouse shutdown owns Unacquire/Release and clearing.
class EngineMouse {
public:
    EngineMouse(EngineFrame* graphics, HMODULE module) : graphics_(graphics), module_(module) {
        using Initialize = std::int32_t (WINAPI *)(EngineFrame*, std::int32_t);
        const auto initialize = scene_method<Initialize>(graphics, module, 0xdc, 0x6f710);
        close_ = scene_method<Close>(graphics, module, 0xe4, 0x6f850);
        using Enabled = std::uint8_t (WINAPI *)(EngineFrame*);
        const auto enabled = scene_method<Enabled>(graphics, module, 0xe0, 0x6f840);
        if (enabled(graphics) != 0 || device() != nullptr) {
            throw std::runtime_error("Engine mouse is already owned by another path");
        }
        const auto status = initialize(graphics, 0); // Foreground, nonexclusive (flags 6).
        if (status < 0 || device() == nullptr) {
            close_(graphics);
            throw std::runtime_error("Original nonexclusive mouse initialization failed");
        }
        QueryPerformanceFrequency(&frequency_);
        if (frequency_.QuadPart <= 0) {
            close_(graphics);
            throw std::runtime_error("Performance-counter frequency is unavailable");
        }
        try {
            baseline_ = microseconds();
        } catch (...) {
            close_(graphics);
            throw;
        }
        std::puts("Rebuilt engine mouse polling enabled: nonexclusive foreground device");
    }
    ~EngineMouse() { close_(graphics_); }
    EngineMouse(const EngineMouse&) = delete;
    EngineMouse& operator=(const EngineMouse&) = delete;

    struct Snapshot {
        std::uint32_t status, buttons, delta;
        input::MouseState state;
    };
    Snapshot poll() {
        Device adapter{device()};
        const auto status = input::poll_mouse(adapter, state_,
            global<std::uint32_t>(0xd1dbc), global<std::uint8_t>(0xd1890) != 0);
        const auto now = microseconds();
        const auto elapsed = (now - baseline_) / 1000u;
        baseline_ = now;
        // Same units/bounds as original caller; host Sleep supplies pacing.
        const auto delta = elapsed < 10 ? 10u : elapsed > 100 ? 100u : elapsed;
        return {status, input::mouse_button_mask(state_), delta, state_};
    }

private:
    struct Device {
        EngineFrame* value;
        bool available() const { return value != nullptr; }
        std::int32_t read(input::MouseState& state) const {
            using Read = std::int32_t (WINAPI *)(EngineFrame*, std::uint32_t, void*);
            return reinterpret_cast<Read>(value->methods[0x24 / 4])(value, sizeof(state), &state);
        }
        void acquire() const {
            using Acquire = std::int32_t (WINAPI *)(EngineFrame*);
            reinterpret_cast<Acquire>(value->methods[0x1c / 4])(value);
        }
    };
    template<class T> T global(std::uintptr_t rva) const {
        T value{};
        std::memcpy(&value, reinterpret_cast<const unsigned char*>(module_) + rva, sizeof(value));
        return value;
    }
    EngineFrame* device() const { return global<EngineFrame*>(0xd2148); }
    std::uint32_t microseconds() const {
        LARGE_INTEGER counter{};
        if (!QueryPerformanceCounter(&counter)) {
            throw std::runtime_error("Performance counter is unavailable");
        }
        return static_cast<std::uint32_t>(
            static_cast<std::uint64_t>(counter.QuadPart) * 1000000u /
            static_cast<std::uint64_t>(frequency_.QuadPart));
    }
    using Close = std::int32_t (WINAPI *)(EngineFrame*);
    EngineFrame* graphics_;
    HMODULE module_;
    Close close_;
    input::MouseState state_;
    LARGE_INTEGER frequency_{};
    std::uint32_t baseline_ = 0;
};

}

#endif
