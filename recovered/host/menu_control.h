#ifndef HD2_HOST_MENU_CONTROL_H
#define HD2_HOST_MENU_CONTROL_H

#include "host/menu_button_visuals.h"

namespace hd2::host {

// One native factory product for the two implemented leaf kinds. Text state
// stays owned by MenuText; visual/button state is owned here. Containers and
// other original widget constructors still require engine/input backends.
class MenuControl {
public:
    MenuControl(const MenuScene& scene, HMODULE module, bool confirmation,
                const frontend::MenuControlDefinition& definition, EngineFrame* frame,
                MenuTextObject& text, bool available)
        : scene_(scene), module_(module), confirmation_(confirmation), definition_(definition),
          frame_(frame), text_(text), available_(available) { text_.bind_frame(frame_); }
    bool initialize_visuals() {
        if (definition_.kind == frontend::MenuControlKind::button) {
            button_ = std::make_unique<ButtonVisualPair>(scene_, module_, definition_.frame,
                definition_.normal, definition_.active, confirmation_, &text_, available_, frame_);
            button_->bind_pick_target(definition_.identifier);
        } else if (definition_.kind == frontend::MenuControlKind::text) {
            // 006618a0 writes control +60 to its bound frame's pick word.
            std::memcpy(reinterpret_cast<unsigned char*>(frame_) + 0xec,
                        &definition_.identifier, sizeof(definition_.identifier));
        } else {
            throw std::logic_error("Original widget kind needs a native constructor backend");
        }
        return true;
    }
    void initialize_definition_data() {} // Both leaf kinds use +78 / 0065cd30.
    void initialize_events(const std::vector<frontend::ControlEvent>& events) {
        if (initialized_) throw std::logic_error("Control initialized twice");
        if (button_) button_->initialize_events(events);
        else {
            text_.initialize_events(events);
            std::printf("Original static initialization: frame=%s records=%u\n",
                        definition_.frame, static_cast<unsigned>(events.size()));
        }
        initialized_ = true;
    }
    bool receive_event(frontend::ControlEvent& event, frontend::ControlEventDestination& upstream) {
        if (button_) return button_->receive_event(event, upstream);
        frontend::EmittingControlDestination<MenuTextObject> destination{text_, &upstream};
        return text_.dispatch_control_event(event, destination);
    }
    bool eligible() const { return button_ ? button_->eligible() : text_.eligible(); }
    bool disabled() const { return button_ ? button_->disabled() : text_.disabled(); }
    bool key(std::uint32_t key) { return button().key(key); }
    int apply(bool hover) { return button().apply(hover); }
    int pointer_move() { return button().pointer_move(); }
    int pointer_event(frontend::ScreenPointerCallback callback, frontend::ControlEventDestination& events) {
        return button().pointer_event(callback, events);
    }
    int wheel_event(frontend::ControlEventDestination& events) { return button().wheel_event(events); }
    bool activate(frontend::ControlEventDestination& events) { return button().activate(events); }
    bool click(frontend::ControlEventDestination& events) { return button().click(events); }
    void bind_pick_target(std::uint32_t id) { button().bind_pick_target(id); }
private:
    ButtonVisualPair& button() const {
        if (!button_) throw std::logic_error("Static text cannot receive a button callback");
        return *button_;
    }
    const MenuScene& scene_;
    HMODULE module_;
    bool confirmation_;
    const frontend::MenuControlDefinition& definition_;
    EngineFrame* frame_;
    MenuTextObject& text_;
    bool available_;
    std::unique_ptr<ButtonVisualPair> button_;
    bool initialized_ = false;
};

using MenuFocus = frontend::DefinedScreen<MenuControl>;
using MenuScreenControls = frontend::DefinedControls<MenuControl>;

struct MenuControlCreator {
    const MenuScene& scene;
    HMODULE module;
    bool confirmation;
    std::initializer_list<frontend::MenuControlRole> available;
    const std::function<MenuTextObject*(std::uint32_t)>& text_lookup;
    EngineFrame* frame = nullptr;

    bool find_frame(const frontend::MenuControlDefinition& definition, std::uint32_t flags) {
        if (flags != 0x10021) throw std::logic_error("Unexpected original control lookup flags");
        // Existing verified traversal bridges FindFrame. Broader flag matching
        // and original error UI are not reconstructed by this native adapter.
        frame = scene.control_frame(definition.frame, module, confirmation);
        return frame != nullptr;
    }
    void report_missing_frame(const frontend::MenuControlDefinition& definition) {
        throw std::runtime_error(std::string("Missing original control frame: ") + definition.frame);
    }
    void report_unknown_kind(std::uint32_t) { throw std::logic_error("Unknown original control kind"); }
    std::unique_ptr<MenuControl> operator()(const frontend::MenuControlDefinition& definition) {
        bool enabled = false;
        for (const auto role : available) if (role == definition.role && role != frontend::MenuControlRole::none) enabled = true;
        auto* text = text_lookup(definition.identifier);
        if (!text) throw std::logic_error("Control requires attached text storage");
        return std::make_unique<MenuControl>(scene, module, confirmation, definition, frame, *text, enabled);
    }
};

inline std::unique_ptr<MenuScreenControls> make_menu_controls(
        const MenuScene& scene, HMODULE module, const frontend::MenuScreenDefinition& definition,
        bool confirmation, std::initializer_list<frontend::MenuControlRole> available,
        const std::function<MenuTextObject*(std::uint32_t)>& text_lookup) {
    return std::make_unique<MenuScreenControls>(definition, available,
        MenuControlCreator{scene, module, confirmation, available, text_lookup});
}

}

#endif
