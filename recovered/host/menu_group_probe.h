#ifndef HD2_HOST_MENU_GROUP_PROBE_H
#define HD2_HOST_MENU_GROUP_PROBE_H

#include "frontend/control_definition_ids.h"
#include "frontend/defined_container_events.h"
#include "frontend/menu_definition.h"
#include "host/menu_scene.h"

namespace hd2::host {

// Real dummy-frame backend for the recovered kind-five state/event subset.
// Frame ownership remains with MenuScene; this object only borrows the frame.
struct MenuGroupBackend {
    HMODULE module;
    EngineFrame* frame;
    void set_frame_visible(std::uint8_t visible) {
        using Visible = void (WINAPI *)(EngineFrame*, std::uint8_t);
        scene_method<Visible>(frame, module, 0x30, 0x1cfe0)(frame, visible);
    }
    void update_frame() {
        using Update = void (WINAPI *)(EngineFrame*);
        scene_method<Update>(frame, module, 0x24, 0x1e610)(frame);
    }
};

// Opt-in diagnostic: exercise two real linked frames through the recovered
// container routing and state runtime without modifying actual menu controls.
inline void probe_menu_group_frames(MenuScene& scene, HMODULE module) {
    struct Queue final : frontend::ControlEventDestination {
        void dispatch_local(frontend::ControlEvent&) override { throw std::logic_error("Unexpected probe local event"); }
        void enqueue(const frontend::ControlEvent&) override { throw std::logic_error("Unexpected probe queued event"); }
    } queue;
    const std::array<float, 3> origin{0, 0, 0};
    auto* root = scene.create_group_frame(module, scene.main_model_frame(), "codex_group_probe", origin);
    auto* child = scene.create_group_frame(module, root, "codex_nested_probe", origin);
    if (scene.control_frame("codex_group_probe", module) != root ||
        scene.control_frame("codex_nested_probe", module) != child) {
        throw std::runtime_error("Dummy group hierarchy lookup failed");
    }
    using Count = std::uint32_t (WINAPI *)(EngineFrame*);
    using Child = EngineFrame* (WINAPI *)(EngineFrame*, std::uint32_t);
    if (scene_method<Count>(root, module, 0x3c, 0x1db80)(root) != 1 ||
        scene_method<Child>(root, module, 0x40, 0x1dba0)(root, 0) != child) {
        throw std::runtime_error("Dummy nested group link failed");
    }

    constexpr std::uint32_t parent_id = 0x07e00000;
    const auto child_id = frontend::nested_child_identifier(parent_id, 0);
    struct Binding {
        std::uint32_t id;
        EngineFrame* bound = nullptr;
        std::uint32_t identifier() const { return id; }
        void bind_frame(EngineFrame* value) { bound = value; }
        void set_frame_identifier(EngineFrame* value, std::uint32_t identifier) {
            std::memcpy(reinterpret_cast<unsigned char*>(value) + 0xec, &identifier, sizeof(identifier));
        }
        void set_definition_kind(frontend::MenuControlKind) { throw std::logic_error("Unexpected probe definition"); }
        bool create_child(EngineFrame*, EngineFrame*, const frontend::MenuControlDefinition&) {
            throw std::logic_error("Unexpected probe child definition");
        }
    } root_binding{parent_id}, child_binding{child_id};
    frontend::initialize_container_visuals(root_binding, root, root,
                                          static_cast<frontend::MenuControlDefinition*>(nullptr));
    frontend::initialize_container_visuals(child_binding, child, root,
                                          static_cast<frontend::MenuControlDefinition*>(nullptr));
    if (root_binding.bound != root || child_binding.bound != child)
        throw std::logic_error("Dummy group frame binding failed");
    std::uint32_t actual_root = 0, actual_child = 0;
    std::memcpy(&actual_root, reinterpret_cast<const unsigned char*>(root) + 0xec, sizeof(actual_root));
    std::memcpy(&actual_child, reinterpret_cast<const unsigned char*>(child) + 0xec, sizeof(actual_child));
    if (actual_root != parent_id || actual_child != child_id)
        throw std::logic_error("Dummy group pick identifiers failed verification");

    MenuGroupBackend root_backend{module, root}, child_backend{module, child};
    frontend::DefinedContainerEvents<MenuGroupBackend> parent(parent_id, root_backend, queue);
    frontend::DefinedContainerEvents<MenuGroupBackend> nested(child_id, child_backend, queue);
    parent.append_child(child_id, [&](frontend::ControlEvent& event, frontend::ControlEventDestination& destination) {
        return nested.dispatch_control_event(event, destination);
    });
    frontend::ControlEvent hide{child_id, 0x01000004, 0, 0, 0, {}};
    frontend::ControlEvent show{child_id, 0x01000003, 0, 0, 0, {}};
    frontend::ControlEvent set{child_id, 0x01000009, 0x12345678, 0, 0, {}};
    frontend::ControlEvent get{child_id, 0x0100000a, 0, 0, 0, {}};
    if (!parent.dispatch(hide) || !parent.dispatch(show) || !parent.dispatch(set) ||
        !parent.dispatch(get) || get.argument != 0x12345678) {
        throw std::runtime_error("Nested dummy group event probe failed");
    }
    std::puts("Original dummy group probe: linked two frames, bound IDs and routed nested state/events");
}

}

#endif
