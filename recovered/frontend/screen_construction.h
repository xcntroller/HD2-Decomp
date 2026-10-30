#ifndef HD2_FRONTEND_SCREEN_CONSTRUCTION_H
#define HD2_FRONTEND_SCREEN_CONSTRUCTION_H

#include "frontend/control_events.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <new>
#include <utility>

namespace hd2::frontend {

// Typed destinations for 00627060's identifier-specific definition builders.
// These are service selectors, never executable addresses from the old client.
enum class ScreenBuilder {
    profile, single_mission, options, cinematics, multiplayer, create_server,
    password, filters, game_info, connect_console, add_server
};

struct ScreenBuilderBinding {
    std::uint32_t identifier;
    ScreenBuilder builder;
};

inline constexpr std::array<ScreenBuilderBinding, 10> remaining_screen_builders{{
    {0x14000000, ScreenBuilder::single_mission},
    {0x18000000, ScreenBuilder::options},
    {0x1c000000, ScreenBuilder::cinematics},
    {0x20000000, ScreenBuilder::multiplayer},
    {0x24000000, ScreenBuilder::create_server},
    {0x28000000, ScreenBuilder::password},
    {0x2c000000, ScreenBuilder::filters},
    {0x30000000, ScreenBuilder::game_info},
    {0x34000000, ScreenBuilder::connect_console},
    {0x38000000, ScreenBuilder::add_server}
}};

// 00668470, screen +8c. Create children using the model as BOTH frame roots;
// ignore individual factory results and reread the mutable child list. Copy
// the model key and identifier only AFTER all children have been attempted.
// Backend owns successfully created children and retains its own model name.
template<class Screen, class Definition>
bool initialize_screen_definition(Screen& screen, Definition& definition) {
    for (std::size_t i = 0; i < definition.controls.size(); ++i) {
        const auto model = screen.model_frame();
        screen.create_child(model, model, definition.controls[i]);
    }
    screen.retain_model_name(definition.model);
    screen.set_identifier(definition.identifier);
    return true;
}

// 00627060, registered-screen construction. Factory supplies model/resource
// loading, the screen-specific builders above and an owning screen product.
// Reacquire definitions after callbacks: the registry may grow/reallocate and
// builders may change an identifier. Independent tests preserve that behavior.
// append_screen takes ownership; definitions can be destroyed before dispatch.
// Products retain their own model/metadata/children. Observer contexts still
// borrow application services, whose lifetime must extend beyond the products.
// Native RAII destroys a rejected product instead of reproducing the original
// failed-+8c allocation leak. No screen is inserted on load/bind failure.
template<class Factory>
bool construct_registered_screens(Factory& factory) {
    factory.create_definitions();
    for (std::size_t i = 0; i < factory.definition_count(); ++i) {
        auto model = factory.load_model(factory.definition(i));
        if (!model) continue;
        if ((factory.definition(i).identifier & 0xfc000000u) == 0x08000000) {
            factory.augment_screen(ScreenBuilder::profile, factory.definition(i), model);
        }
        if ((factory.definition(i).identifier & 0xfc000000u) == 0x10000000) {
            factory.position_model(model, 0.5f, -0.375f, -0.5f);
        }
        for (const auto& binding : remaining_screen_builders) {
            if ((factory.definition(i).identifier & 0xfc000000u) == binding.identifier) {
                factory.augment_screen(binding.builder, factory.definition(i), model);
            }
        }
        auto screen = factory.allocate_screen(model);
        if (!screen) throw std::bad_alloc();
        if (!screen->bind_definition(factory.definition(i))) continue; // Virtual +8c.
        screen->initialize_records(factory.definition(i).initialization); // Virtual +70.
        factory.append_screen(std::move(screen));
    }
    factory.destroy_definitions();
    ControlEvent initial{0, 0x02000001, 1, 0, 0, {}};
    factory.dispatch_initial_event(initial); // Direct 0066c950, not queue submission.
    return true;
}

// 0066c1d0 constructs slot zero separately, before the named screen loop.
// Factory owns model loading and definition/product allocations. install keeps
// a stable owner (original front-end +100) BEFORE virtual +8c; a failed binding
// leaves that installed product owned but does not insert a registry entry.
// The temporary definition dies on either path. No root +70 call is made here.
template<class Factory>
bool construct_runtime_confirmation(Factory& factory) {
    auto model = factory.load_runtime_model();
    if (!model) return false;
    factory.position_model(model, 0.5f, -0.375f, -0.5f);
    auto definition = factory.create_runtime_definition();
    if (!definition) throw std::bad_alloc();
    auto screen = factory.allocate_runtime_confirmation(model);
    if (!screen) throw std::bad_alloc();
    auto& installed = factory.install_runtime_confirmation(std::move(screen));
    if (!installed.bind_definition(*definition)) return false;
    factory.append_runtime_confirmation(installed);
    return true;
}

}

#endif
