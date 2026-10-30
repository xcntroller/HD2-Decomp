#ifndef HD2_HOST_SOUND_SERVICES_H
#define HD2_HOST_SOUND_SERVICES_H

#include "host/settings_services.h"
#include "frontend/sound_runtime.h"
#include <cstddef>
#include <exception>
#include <type_traits>

namespace hd2::host {

static_assert(sizeof(frontend::SoundSectorData) == 12 &&
              offsetof(frontend::SoundSectorData, scale) == 8,
              "Original sector userdata requires the i386 metadata layout");

// Real DLL boundary, borrowed application view and engine-owned frames. No
// original client addresses or permanent empty world are used as services.
// The rebuilt application must supply its live scene/camera-sector view;
// reconstructing that application/camera owner remains a separate prerequisite.
class SoundServices : public SettingsServices {
public:
    SoundServices(HMODULE engine, frontend::SoundWorldView*& world)
        : SettingsServices(engine), engine_(engine), world_(world) {
        const auto entry = GetProcAddress(engine, "?UpdateWMatrixProc@I3D_frame@@AAEXXZ");
        if (reinterpret_cast<std::uintptr_t>(entry) != reinterpret_cast<std::uintptr_t>(engine) + 0x1e390)
            throw std::runtime_error("Unsupported original sound frame transform export");
        std::memcpy(&world_matrix_, &entry, sizeof(entry));
    }
    frontend::SoundWorldView* sound_world() const { return world_; }
    void enable_sound(void* sound, std::uint32_t enabled) const {
        using Function = void (WINAPI *)(EngineFrame*, std::uint32_t);
        auto* frame = static_cast<EngineFrame*>(sound);
        scene_method<Function>(frame, engine_, 0x30, 0x1bb10)(frame, enabled);
    }
    bool sound_active(void* sound, std::uint32_t mode) const {
        using Function = std::uint8_t (WINAPI *)(EngineFrame*, std::uint32_t);
        auto* frame = static_cast<EngineFrame*>(sound);
        return scene_method<Function>(frame, engine_, 0x78, 0x548d0)(frame, mode) != 0;
    }
    void sound_volume(void* sound, float value) const {
        using Function = void (WINAPI *)(EngineFrame*, float);
        auto* frame = static_cast<EngineFrame*>(sound);
        scene_method<Function>(frame, engine_, 0x8c, 0x54c40)(frame, value);
    }
    float sound_volume(void* sound) const { return field<float>(sound, 0x16c); }
    void sound_mode(void* sound, std::uint32_t first, std::uint32_t second) const {
        using Function = void (WINAPI *)(EngineFrame*, std::uint32_t, std::uint32_t);
        auto* frame = static_cast<EngineFrame*>(sound);
        scene_method<Function>(frame, engine_, 0xac, 0x54f30)(frame, first, second);
    }
    void update_sound(void* sound) const {
        using Function = void (WINAPI *)(EngineFrame*);
        auto* frame = static_cast<EngineFrame*>(sound);
        scene_method<Function>(frame, engine_, 0x24, 0x1e610)(frame);
    }
    void release_catalog_sound(void* sound) const {
        using Function = std::int32_t (WINAPI *)(EngineFrame*);
        auto* frame = static_cast<EngineFrame*>(sound);
        scene_method<Function>(frame, engine_, 0, 0x1c370)(frame);
    }
    void retain_tracked_sound(void* sound) const {
        const auto count = static_cast<std::uint16_t>(field<std::uint16_t>(sound, 0xe4) + 1);
        field(sound, 0xe4, count); // Original direct wrapping WORD increment.
    }
    frontend::SoundPosition sound_position(void* sound) const {
        if (!(field<std::uint8_t>(sound, 0xe0) & 0x20))
            world_matrix_(static_cast<EngineFrame*>(sound));
        return field<frontend::SoundPosition>(sound, 0x70);
    }
    void* sound_sector(void* scene, const frontend::SoundPosition& position) const {
        using Function = EngineFrame* (WINAPI *)(EngineFrame*, const float*);
        auto* frame = static_cast<EngineFrame*>(scene);
        return scene_method<Function>(frame, engine_, 0xe0, 0x47a90)(frame, position.data());
    }
    void* primary_sound_sector(void* scene) const { return field<void*>(scene, 0x1f8); }
    void* scene_sound_listener(void* scene) const { return field<void*>(scene, 0x15c); }
    frontend::SoundSectorData* sector_sound_data(void* sector) const {
        auto* result = field<frontend::SoundSectorData*>(sector, 0xec);
        if (!result) throw std::runtime_error("Missing original sound sector metadata");
        return result;
    }
    void sector_sound_data(void* sector, frontend::SoundSectorData* data) const { field(sector, 0xec, data); }
    std::uint32_t sound_portal_count(void* sector, unsigned side) const {
        return field<std::uint32_t>(sector, side ? 0x264 : 0x254);
    }
    void* sound_portal(void* sector, unsigned side, std::uint32_t index) const {
        if (index >= sound_portal_count(sector, side)) throw std::out_of_range("Invalid original sound portal index");
        auto* list = field<void**>(sector, side ? 0x25c : 0x24c);
        if (!list) throw std::runtime_error("Missing original sound portal storage");
        return list[index];
    }
    void* sound_portal_sector(void* portal, unsigned side) const {
        return field<void*>(portal, side ? 0x194 : 0x198);
    }
    float sound_portal_radius(void* portal) const { return field<float>(portal, 0x190); }
    bool sound_portal_opened(void* portal) const {
        // Portal type-specific +04 is supplied by the real engine object.
        // No guessed method target: the exact caller ABI is self on stack.
        using Function = std::uint8_t (WINAPI *)(EngineFrame*);
        auto* frame = static_cast<EngineFrame*>(portal);
        if (!frame || !frame->methods[1]) throw std::runtime_error("Invalid original sound portal");
        return reinterpret_cast<Function>(frame->methods[1])(frame) != 0;
    }
    template<class Visitor>
    void enumerate_sound_sectors(void* root, Visitor&& visitor) const {
        struct Context {
            std::remove_reference_t<Visitor>* visitor;
            std::exception_ptr error;
            static std::int32_t WINAPI invoke(EngineFrame* frame, void* opaque) {
                auto& state = *static_cast<Context*>(opaque);
                if (state.error) return 1;
                try { return (*state.visitor)(frame); }
                catch (...) { state.error = std::current_exception(); return 1; }
            }
        };
        Context context{&visitor, {}};
        using Callback = std::int32_t (WINAPI *)(EngineFrame*, void*);
        const Callback callback = &Context::invoke;
        using Function = std::int32_t (WINAPI *)(EngineFrame*, Callback, void*, std::uint32_t, void*);
        auto* frame = static_cast<EngineFrame*>(root);
        scene_method<Function>(frame, engine_, 0x44, 0x1de40)(frame, callback, &context, 0x10, nullptr);
        if (context.error) std::rethrow_exception(context.error);
    }
private:
    template<class T> static T field(void* frame, std::size_t offset) {
        if (!frame) throw std::invalid_argument("Null original sound frame");
        T value;
        std::memcpy(&value, static_cast<const unsigned char*>(frame) + offset, sizeof(value));
        return value;
    }
    template<class T> static void field(void* frame, std::size_t offset, const T& value) {
        if (!frame) throw std::invalid_argument("Null original sound frame");
        std::memcpy(static_cast<unsigned char*>(frame) + offset, &value, sizeof(value));
    }
    using WorldMatrix = void (__thiscall *)(EngineFrame*);
    HMODULE engine_;
    frontend::SoundWorldView*& world_;
    WorldMatrix world_matrix_{};
};

}
#endif
