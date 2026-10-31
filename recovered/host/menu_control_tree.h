#ifndef HD2_HOST_MENU_CONTROL_TREE_H
#define HD2_HOST_MENU_CONTROL_TREE_H

#include "frontend/defined_control_tree.h"
#include "host/menu_control.h"

namespace hd2::host {

// Frame search is scoped to each group's bound frame. Leaves share the same
// text/button implementation as the live screens. Widgets use the real shared
// context and owned parts; text/font storage is supplied by its owner.
struct MenuControlTreeBackend {
    using Frame = EngineFrame*;
    using Leaf = MenuControl;
    const MenuScene& scene;
    HMODULE module;
    bool confirmation = false;
    std::function<MenuTextObject*(std::uint32_t)> text_lookup{};

    MenuWidgetContext* widgets = nullptr;

    Frame find_frame(Frame root, const char* name, std::uint32_t flags) {
        if (flags != 0x10021) throw std::logic_error("Unexpected recursive control lookup flags");
        // Bounded verified traversal, not complete original FindFrame matching.
        return scene.find_control_frame(root, name, module);
    }
    void report_missing_frame(const frontend::MenuControlDefinition& definition) {
        throw std::runtime_error(std::string("Missing recursive menu frame: ") + definition.frame);
    }
    void report_unknown_kind(std::uint32_t) { throw std::logic_error("Unknown recursive control kind"); }
    std::unique_ptr<Leaf> allocate_leaf(const frontend::MenuControlDefinition& definition, Frame frame, Frame) {
        if (definition.kind != frontend::MenuControlKind::text && definition.kind != frontend::MenuControlKind::button && definition.kind != frontend::MenuControlKind::slider && definition.kind != frontend::MenuControlKind::selector && definition.kind != frontend::MenuControlKind::toggle)
            throw std::logic_error("Recursive widget needs a native constructor backend");
        auto* text = text_lookup ? text_lookup(definition.identifier) : nullptr;
        if (!text) throw std::logic_error("Recursive leaf requires attached text storage");
        return std::make_unique<Leaf>(scene, module, confirmation, definition, frame, *text, true, widgets);
    }
    void set_frame_identifier(Frame frame, std::uint32_t identifier) {
        std::memcpy(reinterpret_cast<unsigned char*>(frame) + 0xec, &identifier, sizeof(identifier));
    }
    void set_frame_visible(Frame frame, std::uint8_t visible) {
        using Visible = void (WINAPI *)(Frame, std::uint8_t);
        scene_method<Visible>(frame, module, 0x30, 0x1cfe0)(frame, visible);
    }
    void update_frame(Frame frame) {
        using Update = void (WINAPI *)(Frame);
        scene_method<Update>(frame, module, 0x24, 0x1e610)(frame);
    }
};

using MenuControlTree = frontend::DefinedControlTree<MenuControlTreeBackend>;

}

#endif
