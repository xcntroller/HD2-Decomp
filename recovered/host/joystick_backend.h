#ifndef HD2_HOST_JOYSTICK_BACKEND_H
#define HD2_HOST_JOYSTICK_BACKEND_H

#include "input/binding_producer.h"
#include <windows.h>
#include <cstring>
#include <stdexcept>

namespace hd2::host {

// Concrete delegated IJoy boundary, not a reconstruction of its DirectInput
// internals. Both loaded modules are borrowed and must outlive all device refs.
// Source-only: the original client's lifecycle callers remain unresolved.
class JoystickBackend {
public:
    JoystickBackend(HMODULE engine, HMODULE joy) : engine_(engine), joy_(joy) {
        static_assert(sizeof(void*) == 4, "Original joystick ABI requires i386");
        get_graphics_ = exported<GetGraphics>(engine, "GetIGraph", 0x68f90);
        factory_ = exported<Factory>(joy, "_CreateJoyBase@20", 0x1040);
    }
    void* graphics() const { return get_graphics_(); }
    std::uintptr_t window(void* graphics) const {
        using Window = HWND (WINAPI *)(void*);
        return reinterpret_cast<std::uintptr_t>(method<Window>(graphics, engine_, 0x48,
                                                              0x6a240)(graphics));
    }
    std::uint8_t create_base(void*& output, std::uintptr_t instance,
                              std::uintptr_t window, void* direct_input,
                              std::uint32_t flags) const {
        return factory_(&output, reinterpret_cast<HINSTANCE>(instance),
                        reinterpret_cast<HWND>(window), direct_input, flags);
    }
    std::int32_t joystick_count(void* base) const {
        using Count = std::int32_t (__thiscall *)(void*);
        return method<Count>(base, joy_, 0x18, 0x14c0)(base);
    }
    std::uint8_t create_joystick(void* base, void*& output, std::int32_t index,
                                 std::uint32_t flags) const {
        using Create = std::uint8_t (__thiscall *)(void*, void**, std::int32_t, std::uint32_t);
        return method<Create>(base, joy_, 0x2c, 0x18b0)(base, &output, index, flags);
    }
    std::uint32_t release_joystick_base(void* base) const {
        return method<Reference>(base, joy_, 0x08, 0x12c0)(base);
    }
    std::uint32_t retain_joystick(void* device) const {
        return method<Reference>(device, joy_, 0x04, 0x40a0)(device);
    }
    std::uint32_t release_joystick(void* device) const {
        return method<Reference>(device, joy_, 0x08, 0x4070)(device);
    }
    std::uint8_t configure_joystick_binding(void* device, std::uint32_t selector,
                                            std::uint32_t flags) const {
        using Configure = std::uint8_t (__thiscall *)(void*, std::uint32_t, std::uint32_t);
        return method<Configure>(device, joy_, 0x30, 0x2a00)(device, selector, flags);
    }
    std::uint8_t poll_joystick(void* device, std::uint32_t flags) const {
        using Poll = std::uint8_t (__thiscall *)(void*, std::uint32_t);
        return method<Poll>(device, joy_, 0x58, 0x2c00)(device, flags);
    }
    std::uint8_t read_joystick_current(void* device, input::JoystickInputState& output) const {
        return method<Read>(device, joy_, 0x50, 0x3870)(device, &output);
    }
    std::uint8_t read_joystick_secondary(void* device, input::JoystickInputState& output) const {
        return method<Read>(device, joy_, 0x54, 0x38a0)(device, &output);
    }

private:
    using GetGraphics = void* (__cdecl *)();
    using Factory = std::uint8_t (WINAPI *)(void**, HINSTANCE, HWND, void*, std::uint32_t);
    using Reference = std::uint32_t (__thiscall *)(void*);
    using Read = std::uint8_t (__thiscall *)(void*, input::JoystickInputState*);

    template<class Function>
    static Function exported(HMODULE module, const char* name, std::uintptr_t rva) {
        const auto address = module ? GetProcAddress(module, name) : nullptr;
        if (!address || reinterpret_cast<std::uintptr_t>(address) !=
                reinterpret_cast<std::uintptr_t>(module) + rva)
            throw std::runtime_error("Unsupported joystick dependency export");
        Function result = nullptr;
        static_assert(sizeof(result) == sizeof(address));
        std::memcpy(&result, &address, sizeof(result));
        return result;
    }
    template<class Function>
    static Function method(void* object, HMODULE module, std::size_t slot, std::uintptr_t rva) {
        if (!object || !module) throw std::runtime_error("Null joystick dependency");
        const auto* methods = *static_cast<const std::uintptr_t* const*>(object);
        if (!methods || methods[slot / 4] != reinterpret_cast<std::uintptr_t>(module) + rva)
            throw std::runtime_error("Unsupported joystick dependency method");
        return reinterpret_cast<Function>(methods[slot / 4]);
    }
    HMODULE engine_;
    HMODULE joy_;
    GetGraphics get_graphics_ = nullptr;
    Factory factory_ = nullptr;
};

}
#endif
