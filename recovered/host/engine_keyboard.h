#ifndef HD2_HOST_ENGINE_KEYBOARD_H
#define HD2_HOST_ENGINE_KEYBOARD_H

#include "host/menu_scene.h"
#include "input/keyboard.h"
#include <mmsystem.h>

namespace hd2::host {

// Original engine owns DirectInput and device configuration. Recovered native
// polling/scan/translation owns its state; no original mutable globals are used
// for key selection. Foreground nonexclusive/no-repeat are explicit host flags.
class EngineKeyboard {
public:
    EngineKeyboard(EngineFrame* graphics, HMODULE module) : graphics_(graphics), module_(module) {
        using Initialize = std::int32_t (WINAPI *)(EngineFrame*, std::uint32_t);
        using Enabled = std::uint8_t (WINAPI *)(EngineFrame*);
        const auto initialize = scene_method<Initialize>(graphics, module, 0x90, 0x6f040);
        const auto enabled = scene_method<Enabled>(graphics, module, 0x94, 0x6f150);
        close_ = scene_method<Close>(graphics, module, 0x98, 0x6f160);
        pump_ = scene_method<Pump>(graphics, module, 0x14, 0x6a470);
        if (enabled(graphics) || device())
            throw std::runtime_error("Engine keyboard is already owned by another path");
        // Copy the immutable, fingerprinted 63-entry scan table plus sentinel.
        // Runtime table lookup is reconstructed in native source.
        static_assert(sizeof(input::KeyboardCharacter) == 3);
        std::memcpy(table_.data(), reinterpret_cast<const unsigned char*>(module) + 0xa7868,
                    sizeof(table_));
        if (table_.back().scan != 0)
            throw std::runtime_error("Unsupported original keyboard character table");
        const auto result = initialize(graphics, 0);
        if (result < 0 || !enabled(graphics) || !device()) {
            close_(graphics);
            throw std::runtime_error("Original nonexclusive keyboard initialization failed");
        }
        try {
            state_.debug_flags = global<std::uint32_t>(0xd1d44);
            Device adapter{*this};
            input::poll_keyboard(adapter, state_);
            state_.previous = state_.current; // Opening owner snapshots already held keys.
        } catch (...) {
            close_(graphics);
            throw;
        }
        std::puts("Recovered keyboard polling enabled: foreground nonexclusive, no repeat");
    }
    ~EngineKeyboard() { close_(graphics_); std::puts("Keyboard closed"); }
    EngineKeyboard(const EngineKeyboard&) = delete;
    EngineKeyboard& operator=(const EngineKeyboard&) = delete;

    std::uint32_t poll() {
        Device adapter{*this};
        input::poll_keyboard(adapter, state_);
        return input::next_keyboard_scan(state_, *this);
    }
    std::uint32_t take_character() { return input::take_keyboard_character(state_); }
    std::uint32_t modifiers(std::uint32_t update) {
        if (update) {
            Device adapter{*this};
            input::poll_keyboard(adapter, state_);
        }
        return input::keyboard_modifiers(state_);
    }
    std::uint8_t translate(std::uint32_t scan, std::uint32_t modifiers) const {
        return input::translate_keyboard_character(table_, scan, static_cast<std::uint8_t>(modifiers));
    }
    std::uint32_t milliseconds() const { return timeGetTime(); }

private:
    struct Device {
        EngineKeyboard& owner;
        bool available() const { return owner.device() != nullptr; }
        std::int32_t read(std::array<std::uint8_t, 256>& state) const {
            auto* value = owner.device();
            using Read = std::int32_t (WINAPI *)(EngineFrame*, std::uint32_t, void*);
            return reinterpret_cast<Read>(value->methods[0x24 / 4])(value, 256, state.data());
        }
        std::int32_t acquire() const {
            auto* value = owner.device();
            using Acquire = std::int32_t (WINAPI *)(EngineFrame*);
            return reinterpret_cast<Acquire>(value->methods[0x1c / 4])(value);
        }
        void pump() const { owner.pump_(owner.graphics_); }
    };
    template<class T> T global(std::uintptr_t rva) const {
        T result{};
        std::memcpy(&result, reinterpret_cast<const unsigned char*>(module_) + rva, sizeof(result));
        return result;
    }
    EngineFrame* device() const { return global<EngineFrame*>(0xd19c8); }
    using Close = void (WINAPI *)(EngineFrame*);
    using Pump = void (WINAPI *)(EngineFrame*);
    EngineFrame* graphics_;
    HMODULE module_;
    Close close_{};
    Pump pump_{};
    input::KeyboardState state_;
    std::array<input::KeyboardCharacter, 64> table_{};
};

} // namespace hd2::host
#endif
