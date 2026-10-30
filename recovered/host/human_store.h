#ifndef HD2_HOST_HUMAN_STORE_H
#define HD2_HOST_HUMAN_STORE_H

#include "frontend/human_store.h"
#include "host/menu_scene.h"

namespace hd2::host {

// Original 006A0EA0 portrait boundary. Borrow LS3DF; resolve GetIGraph afresh
// for each load. Frame/texture ownership stays with frontend::HumanStore.
class HumanPortraits {
public:
    explicit HumanPortraits(HMODULE engine) : engine_(engine) {
        static_assert(sizeof(void*) == 4, "Original HumanStore ABI requires i386");
        const auto entry = engine ? GetProcAddress(engine, "GetIGraph") : nullptr;
        if (!entry || reinterpret_cast<std::uintptr_t>(entry) !=
                reinterpret_cast<std::uintptr_t>(engine) + 0x68f90)
            throw std::runtime_error("Unsupported HumanStore GetIGraph export");
        static_assert(sizeof(get_graphics_) == sizeof(entry));
        std::memcpy(&get_graphics_, &entry, sizeof(entry));
    }
    std::int32_t load_portrait(const char* face, const char* opacity,
                               std::uint32_t flags, void** output) const {
        using Load = std::int32_t (WINAPI *)(EngineFrame*, const char*, const char*, std::uint32_t, void**);
        auto* graphics = get_graphics_();
        return scene_method<Load>(graphics, engine_, 0x3c, 0x69f70)(graphics, face, opacity, flags, output);
    }
    void release_portrait(void* portrait) const {
        using Release = std::uint32_t (WINAPI *)(EngineFrame*);
        auto* texture = static_cast<EngineFrame*>(portrait);
        scene_method<Release>(texture, engine_, 0, 0x73760)(texture);
    }
private:
    HMODULE engine_;
    EngineFrame* (__cdecl *get_graphics_)() = nullptr;
};

}
#endif
