#ifndef HD2_FRONTEND_STARTUP_MODULES_H
#define HD2_FRONTEND_STARTUP_MODULES_H

#include "frontend/module_registration.h"
#include "frontend/vertex_animations.h"
#include "frontend/cleanup.h"
#include <memory>

namespace hd2::frontend {

// Native normal-startup module views. These registrations do not implement the
// modules' later gameplay/event methods. Sound's actual data owner registers6.
// Original slot04 methods are verified RETs except Items3 and Vertex anim9.
// Items starts with an empty request deque; its cold initialization writes-1.
// A dirty Items restart is explicitly unsupported, never a successful stub.
class StartupModules {
public:
    explicit StartupModules(ClientModuleRegistry& registry) : registry_(registry), sound_(registry[6]) {
        if (!sound_) throw std::logic_error("Startup requires the real sound data registration");
        const std::array<const char*, 18> names{{"Track module", "GEffects", "Physics", "Items",
            "Script", "Sight", "Sound", "Versions", "LMapsGrp", "Vertex anim", "TShift",
            "TeamMgr", "Temp. Actors", "Actors", "ShotMgr", "CameraModule", "HeavyFireSupportModule", "Particles"}};
        for (std::uint32_t kind = 0; kind != names.size(); ++kind) {
            if (kind == 6) continue;
            if (registry[kind]) throw std::logic_error("Original startup module already registered");
            modules_[kind] = std::make_unique<ClientModuleRegistration>(registry, kind, names[kind]);
        }
    }
    // 0048DA60: fresh slot on every ascending iteration, skip missing entries.
    template<class Files>
    void initialize(const std::uint32_t& gate, Files& files) {
        for (std::uint32_t kind = 0; kind != registry_.size(); ++kind) {
            if (!registry_[kind]) continue;
            if (kind >= 18 || registry_[kind] != (kind == 6 ? sound_ : modules_[kind].get()))
                throw std::logic_error("Unsupported replacement startup module");
            if (kind == 3) {
                if (items_pending != 0) throw std::logic_error("Items restart queue is not reconstructed");
                items_current = -1; // 007DCD20 cold-start branch; no device work.
            } else if (kind == 9) vertices.startup(gate, files);
            // Other registered slot04 bodies are original empty RETs, not
            // stand-ins for unrecovered module work. Evidence lists each table.
        }
    }
    //0048DA80: ascending FRESH registrations, including null slot18. The sixteen
    // other normal targets are verified RETs. No gameplay shutdown is invented.
    void shutdown(const std::uint32_t& gate) {
        CleanupErrors errors;
        for (std::uint32_t kind = 0; kind != registry_.size(); ++kind) {
            errors.attempt([&] {
                if (!registry_[kind]) return;
                if (kind >= 18 || registry_[kind] != (kind == 6 ? sound_ : modules_[kind].get()))
                    throw std::logic_error("Unsupported replacement shutdown module");
                if (shutdown_[kind]) return;
                if (kind == 3) {
                    //007DCA30 zeros500 slots; the standalone host has no item
                    // or0078A980 list producer. Do not accept populated state.
                    for (auto* object : items_objects)
                        if (object) throw std::logic_error("Live Items shutdown is not reconstructed");
                    if (items_pending || items_list_first || items_list_second)
                        throw std::logic_error("Live Items queues are not reconstructed");
                    if (gate & 2u) items_list_count = 0; //0078A980 cold side effect.
                } else if (kind == 9 && (gate & 2u)) vertices.clear(); //00474A20.
                shutdown_[kind] = true;
            });
        }
        errors.rethrow();
    }
    VertexAnimations vertices;
    std::int32_t items_current = 0;
    std::uint32_t items_pending = 0;
    std::array<void*, 500> items_objects{};
    void* items_list_first = nullptr;
    void* items_list_second = nullptr;
    std::uint32_t items_list_count = 0;
private:
    ClientModuleRegistry& registry_;
    ClientModuleRegistration* sound_;
    std::array<std::unique_ptr<ClientModuleRegistration>, 18> modules_;
    std::array<bool, 19> shutdown_{};
};

}
#endif
