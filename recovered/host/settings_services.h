#ifndef HD2_HOST_SETTINGS_SERVICES_H
#define HD2_HOST_SETTINGS_SERVICES_H

#include "host/menu_scene.h"
#include "frontend/profile_settings.h"
#include "frontend/settings_sound.h"
#include <cstdlib>

namespace hd2::host {

// Borrowed module, real singleton getters and verified LS3DF slots. Neither
// starts devices nor owns their lifetime; original application startup must do so.
class SettingsServices {
public:
    explicit SettingsServices(HMODULE engine) : engine_(engine) {
        static_assert(sizeof(void*) == 4, "Original settings ABI requires i386");
        get_driver_ = getter(engine, "I3DGetDriver", 0x68fb0);
        get_graphics_ = getter(engine, "GetIGraph", 0x68f90);
        get_sound_ = getter(engine, "ISndGetDriver", 0x68fa0);
    }
    EngineFrame* driver() const { return get_driver_(); }
    void driver_state(EngineFrame* driver, std::uint32_t state, std::uint32_t value) const {
        using Function = std::int32_t (WINAPI *)(EngineFrame*, std::uint32_t, std::uint32_t);
        scene_method<Function>(driver, engine_, 0x18, 0x15770)(driver, state, value);
    }
    void gamma(const frontend::SettingsGammaRamp& ramp, std::uint32_t flags) const {
        using Function = std::uint8_t (WINAPI *)(EngineFrame*, const std::uint16_t*, std::uint32_t);
        auto* graphics = get_graphics_();
        scene_method<Function>(graphics, engine_, 0x12c, 0x700a0)(graphics, ramp.data(), flags);
    }
    void sound_driver_volume(float volume) const {
        using Function = std::int32_t (WINAPI *)(EngineFrame*, float);
        auto* sound = get_sound_();
        scene_method<Function>(sound, engine_, 0x14, 0x7a9b0)(sound, volume);
    }
    bool sound_driver_initialized() const {
        using Function = std::uint8_t (WINAPI *)(EngineFrame*);
        auto* sound = get_sound_();
        return scene_method<Function>(sound, engine_, 0x0c, 0x7a840)(sound) != 0;
    }
    bool movie_opened(void* movie) const {
        using Function = std::uint8_t (WINAPI *)(EngineFrame*);
        auto* frame = static_cast<EngineFrame*>(movie);
        return scene_method<Function>(frame, engine_, 8, 0x71b70)(frame) != 0;
    }
    void movie_volume(void* movie, float volume) const {
        using Function = std::int32_t (WINAPI *)(EngineFrame*, float);
        auto* frame = static_cast<EngineFrame*>(movie);
        scene_method<Function>(frame, engine_, 0x3c, 0x724d0)(frame, volume);
    }
    void* create_sound_movie() const {
        using Function = EngineFrame* (WINAPI *)(EngineFrame*);
        auto* graphics = get_graphics_();
        return scene_method<Function>(graphics, engine_, 0x38, 0x69e80)(graphics);
    }
    void stop_sound_movie(void* movie) const { movie_operation(movie, 0x18, 0x71d10); }
    void close_sound_movie(void* movie) const { movie_operation(movie, 0x10, 0x71b80); }
    void play_sound_movie(void* movie) const { movie_operation(movie, 0x14, 0x71c50); }
    void release_sound_movie(void* movie) const { movie_operation(movie, 4, 0x70fd0); }
    std::int32_t sound_movie_status(void* movie) const {
        using Function = std::int32_t (WINAPI *)(EngineFrame*);
        auto* frame = static_cast<EngineFrame*>(movie);
        return scene_method<Function>(frame, engine_, 0x2c, 0x72160)(frame);
    }
    std::int32_t query_sound_movie_position(void* movie, std::uint32_t& position) const {
        using Function = std::int32_t (WINAPI *)(EngineFrame*, std::uint32_t*);
        auto* frame = static_cast<EngineFrame*>(movie);
        return scene_method<Function>(frame, engine_, 0x38, 0x72400)(frame, &position);
    }
    void seek_sound_movie(void* movie, std::uint32_t position) const {
        using Function = std::int32_t (WINAPI *)(EngineFrame*, std::uint32_t);
        auto* frame = static_cast<EngineFrame*>(movie);
        scene_method<Function>(frame, engine_, 0x30, 0x722a0)(frame, position);
    }
    std::uint32_t sound_random_word() const { return static_cast<std::uint32_t>(std::rand()); }
    void catalog_diagnostic(const char* message) const { OutputDebugStringA(message); }
    // 0043A630 releases each supplied sample through its own slot0, self on
    // stack. Sample creation/type-specific vtables remain separate research;
    // unlike IShow calls above this boundary does not assert a guessed target.
    void release_catalog_sound(void* sound) const {
        using Function = std::int32_t (WINAPI *)(EngineFrame*);
        auto* frame = static_cast<EngineFrame*>(sound);
        if (!frame || !frame->methods || !frame->methods[0])
            throw std::invalid_argument("Invalid original catalog sound reference");
        reinterpret_cast<Function>(frame->methods[0])(frame);
    }
    std::int32_t open_sound_movie(void* movie, const char* path, std::uint32_t flags) const {
        using Function = std::int32_t (WINAPI *)(EngineFrame*, const char*, std::uint32_t);
        auto* frame = static_cast<EngineFrame*>(movie);
        return scene_method<Function>(frame, engine_, 0x0c, 0x71630)(frame, path, flags);
    }
    // 00742210 calls an external COM interface through +20, self on stack.
    // Its target is supplied by the real interface owner, not a client/DLL RVA.
    std::int32_t submit_settings_control(void* object, frontend::SettingsControlBlock& block) const {
        using Function = std::int32_t (WINAPI *)(void*, frontend::SettingsControlBlock*);
        if (!object) throw std::invalid_argument("Null settings control interface");
        const auto* frame = static_cast<const EngineFrame*>(object);
        return reinterpret_cast<Function>(frame->methods[0x20 / 4])(object, &block);
    }
private:
    void movie_operation(void* movie, std::size_t slot, std::uintptr_t rva) const {
        using Function = std::int32_t (WINAPI *)(EngineFrame*);
        auto* frame = static_cast<EngineFrame*>(movie);
        scene_method<Function>(frame, engine_, slot, rva)(frame);
    }
    using Getter = EngineFrame* (__cdecl *)();
    static Getter getter(HMODULE engine, const char* name, std::uintptr_t rva) {
        const auto entry = engine ? GetProcAddress(engine, name) : nullptr;
        if (!entry || reinterpret_cast<std::uintptr_t>(entry) != reinterpret_cast<std::uintptr_t>(engine) + rva)
            throw std::runtime_error("Unsupported settings singleton export");
        Getter result;
        std::memcpy(&result, &entry, sizeof(result));
        return result;
    }
    HMODULE engine_;
    Getter get_driver_{}, get_graphics_{}, get_sound_{};
};

}
#endif
