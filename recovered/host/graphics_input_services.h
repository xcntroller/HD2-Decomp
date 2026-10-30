#ifndef HD2_HOST_GRAPHICS_INPUT_SERVICES_H
#define HD2_HOST_GRAPHICS_INPUT_SERVICES_H

#include "host/menu_scene.h"
#include "input/joystick_devices.h"
#include "input/binding_producer.h"

namespace hd2::host {

// Borrowed original LS3DF module and IJoy adapter. These methods forward to the
// real original interfaces; they neither own nor initialize the devices. The
// manager's attached graphics pointer is used as passed, not re-resolved.
// JoystickServices must outlive this adapter and all retained joystick refs.
template<class JoystickServices>
class GraphicsInputServices {
public:
    GraphicsInputServices(HMODULE engine, JoystickServices& joystick)
        : engine_(engine), joystick_(joystick) {
        static_assert(sizeof(void*) == 4, "Original input ABI requires i386");
        if (!engine_) throw std::invalid_argument("Input services require LS3DF");
    }
    std::uint32_t retain_joystick(void* device) { return joystick_.retain_joystick(device); }
    std::uint32_t release_joystick(void* device) { return joystick_.release_joystick(device); }
    std::uint8_t configure_joystick_binding(void* device, std::uint16_t selector, std::uint32_t flags) {
        return joystick_.configure_joystick_binding(device, selector, flags);
    }
    std::uint8_t poll_joystick(void* device, std::uint32_t flags) {
        return joystick_.poll_joystick(device, flags);
    }
    std::uint8_t read_joystick_current(void* device, input::JoystickInputState& output) {
        return joystick_.read_joystick_current(device, output);
    }
    std::uint8_t read_joystick_secondary(void* device, input::JoystickInputState& output) {
        return joystick_.read_joystick_secondary(device, output);
    }
    std::int32_t configure_keyboard_snapshot(void* graphics, const std::uint8_t* scans,
                                              std::uint32_t count) const {
        using Function = std::int32_t (WINAPI *)(EngineFrame*, const std::uint8_t*, std::uint32_t);
        auto* frame = static_cast<EngineFrame*>(graphics);
        return scene_method<Function>(frame, engine_, 0xb4, 0x6f4b0)(frame, scans, count);
    }
    void pump_graphics(void* graphics) const {
        using Function = void (WINAPI *)(EngineFrame*);
        auto* frame = static_cast<EngineFrame*>(graphics);
        scene_method<Function>(frame, engine_, 0x14, 0x6a470)(frame);
    }
    std::int32_t poll_mouse(void* graphics) const { return scalar(graphics, 0xe8, 0x6f8a0); }
    std::int32_t poll_keyboard(void* graphics) const { return scalar(graphics, 0x9c, 0x6f190); }
    std::uint64_t keyboard_snapshot(void* graphics) const {
        using Function = std::uint64_t (WINAPI *)(EngineFrame*);
        auto* frame = static_cast<EngineFrame*>(graphics);
        return scene_method<Function>(frame, engine_, 0xb8, 0x6f500)(frame);
    }
    std::uint32_t mouse_buttons(void* graphics) const {
        return static_cast<std::uint32_t>(scalar(graphics, 0x104, 0x6fb30));
    }
    std::int32_t mouse_x(void* graphics) const { return scalar(graphics, 0xec, 0x6fa10); }
    std::int32_t mouse_y(void* graphics) const { return scalar(graphics, 0xf0, 0x6fa20); }
    std::int32_t mouse_z(void* graphics) const { return scalar(graphics, 0xf4, 0x6fa30); }
private:
    std::int32_t scalar(void* graphics, std::size_t slot, std::uintptr_t rva) const {
        using Function = std::int32_t (WINAPI *)(EngineFrame*);
        auto* frame = static_cast<EngineFrame*>(graphics);
        return scene_method<Function>(frame, engine_, slot, rva)(frame);
    }
    HMODULE engine_;
    JoystickServices& joystick_;
};

// Startup 006db620 queries GetIGraph separately before keyboard initialization
// and attachment. The list is borrowed; no joystick factory call is implied.
template<class JoystickList>
class GraphicsInputSetup {
public:
    GraphicsInputSetup(HMODULE engine, JoystickList& joysticks)
        : engine_(engine), joysticks_(joysticks) {
        static_assert(sizeof(void*) == 4, "Original input ABI requires i386");
        const auto entry = engine ? GetProcAddress(engine, "GetIGraph") : nullptr;
        if (!entry || reinterpret_cast<std::uintptr_t>(entry) !=
                reinterpret_cast<std::uintptr_t>(engine) + 0x68f90)
            throw std::runtime_error("Unsupported input GetIGraph export");
        static_assert(sizeof(get_graphics_) == sizeof(entry));
        std::memcpy(&get_graphics_, &entry, sizeof(entry));
    }
    void* graphics() const { return get_graphics_(); }
    std::int32_t initialize_keyboard(void* graphics, std::uint32_t flags) const {
        using Function = std::int32_t (WINAPI *)(EngineFrame*, std::uint32_t);
        auto* frame = static_cast<EngineFrame*>(graphics);
        return scene_method<Function>(frame, engine_, 0x90, 0x6f040)(frame, flags);
    }
    input::JoystickDeviceView joysticks() const { return joysticks_.view(); }
private:
    using GetGraphics = EngineFrame* (__cdecl *)();
    HMODULE engine_;
    JoystickList& joysticks_;
    GetGraphics get_graphics_ = nullptr;
};
}
#endif
