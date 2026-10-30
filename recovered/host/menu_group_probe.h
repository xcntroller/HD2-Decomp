#ifndef HD2_HOST_MENU_GROUP_PROBE_H
#define HD2_HOST_MENU_GROUP_PROBE_H

#include "frontend/control_definition_ids.h"
#include "host/menu_control_tree.h"

namespace hd2::host {

// Opt-in diagnostic: construct a three-level definition tree over real linked
// dummy frames. Duplicate leaf names in separate branches require scoped lookup.
inline void probe_menu_group_frames(MenuScene& scene, HMODULE module) {
    struct Queue final : frontend::ControlEventDestination {
        void dispatch_local(frontend::ControlEvent&) override { throw std::logic_error("Unexpected probe local event"); }
        void enqueue(const frontend::ControlEvent&) override { throw std::logic_error("Unexpected probe queued event"); }
    } queue;
    const std::array<float, 3> origin{0, 0, 0};
    auto* root = scene.create_group_frame(module, scene.main_model_frame(), "codex_group_probe", origin);
    auto* child = scene.create_group_frame(module, root, "codex_branch_a", origin);
    auto* other = scene.create_group_frame(module, root, "codex_branch_b", origin);
    auto* leaf = scene.create_group_frame(module, child, "codex_shared_leaf", origin);
    auto* other_leaf = scene.create_group_frame(module, other, "codex_shared_leaf", origin);
    if (scene.control_frame("codex_group_probe", module) != root) {
        throw std::runtime_error("Dummy group hierarchy lookup failed");
    }
    using Count = std::uint32_t (WINAPI *)(EngineFrame*);
    using Child = EngineFrame* (WINAPI *)(EngineFrame*, std::uint32_t);
    if (scene_method<Count>(root, module, 0x3c, 0x1db80)(root) != 2 ||
        (scene_method<Child>(root, module, 0x40, 0x1dba0)(root, 0) != child &&
         scene_method<Child>(root, module, 0x40, 0x1dba0)(root, 1) != child)) {
        throw std::runtime_error("Dummy nested group link failed");
    }

    constexpr std::uint32_t parent_id = 0x07e00000;
    const auto child_id = frontend::nested_child_identifier(parent_id, 0);
    const auto other_id = frontend::nested_child_identifier(parent_id, 1);
    const auto leaf_id = frontend::nested_child_identifier(child_id, 0);
    const auto other_leaf_id = frontend::nested_child_identifier(other_id, 0);
    using Kind = frontend::MenuControlKind;
    using Role = frontend::MenuControlRole;
    const frontend::MenuControlDefinition definition{
        parent_id, "codex_group_probe", Kind::container, nullptr, nullptr, Role::none, {}, {
            {child_id, "codex_branch_a", Kind::container, nullptr, nullptr, Role::none, {}, {
                {leaf_id, "codex_shared_leaf", Kind::container, nullptr, nullptr, Role::none, {}}
            }},
            {other_id, "codex_branch_b", Kind::container, nullptr, nullptr, Role::none, {}, {
                {other_leaf_id, "codex_shared_leaf", Kind::container, nullptr, nullptr, Role::none,
                    {{0, 0x01000004, 0, 0, 0, {}}}}
            }}
        }};
    MenuControlTreeBackend backend{scene, module};
    auto result = MenuControlTree::construct(backend, definition, scene.main_model_frame(), root, queue);
    if (!result.handled || !result.control) throw std::logic_error("Recursive group factory failed");
    auto& parent = *result.control;
    if (parent.child_count() != 2 || parent.child_at(0).frame() != child ||
        parent.child_at(1).frame() != other || parent.child_at(0).child_at(0).frame() != leaf ||
        parent.child_at(1).child_at(0).frame() != other_leaf ||
        parent.child_at(0).child_at(0).layout_root() != root) {
        throw std::logic_error("Recursive factory scope or layout root failed");
    }
    std::uint32_t actual_root = 0, actual_child = 0;
    std::memcpy(&actual_root, reinterpret_cast<const unsigned char*>(root) + 0xec, sizeof(actual_root));
    std::memcpy(&actual_child, reinterpret_cast<const unsigned char*>(child) + 0xec, sizeof(actual_child));
    if (actual_root != parent_id || actual_child != child_id)
        throw std::logic_error("Dummy group pick identifiers failed verification");

    frontend::ControlEvent hide{leaf_id, 0x01000004, 0, 0, 0, {}};
    frontend::ControlEvent show{leaf_id, 0x01000003, 0, 0, 0, {}};
    frontend::ControlEvent set{leaf_id, 0x01000009, 0x12345678, 0, 0, {}};
    frontend::ControlEvent get{leaf_id, 0x0100000a, 0, 0, 0, {}};
    frontend::ControlEvent other_get{other_leaf_id, 0x0100000a, 0, 0, 0, {}};
    if (!parent.receive_event(hide, queue) || !parent.receive_event(show, queue) ||
        !parent.receive_event(set, queue) || !parent.receive_event(get, queue) || get.argument != 0x12345678 ||
        !parent.receive_event(other_get, queue) || other_get.argument != 0xffffffffu) {
        throw std::runtime_error("Nested dummy group event probe failed");
    }
    if (parent.enter_focus(leaf_id) != 0 || parent.focused_control() != 0 ||
        parent.child_at(0).focused_control() != 0 || parent.key(0x1c) || parent.enter_focus(15) != 0 ||
        parent.pointer(frontend::ScreenPointerCallback::press, leaf_id) != 0 ||
        parent.click(leaf_id) != 7 || parent.leave_focus(0) != 1 || parent.focused_control() != -1) {
        throw std::logic_error("Nested group input forwarding failed");
    }
    std::puts("Original recursive group factory: five controls, scoped duplicate names, three-level state and input passed");
}

}

#endif
