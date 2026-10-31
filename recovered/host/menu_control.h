#ifndef HD2_HOST_MENU_CONTROL_H
#define HD2_HOST_MENU_CONTROL_H

#include "host/menu_button_visuals.h"
#include "host/menu_options_widget.h"

namespace hd2::host {

// Native text/button/slider/selector/toggle factory product. Attached text stays
// owned by the supplied text store; visual/internal-part/runtime state lives
// here. The shared widget context supplies actual input/settings services.
class MenuControl {
public:
    using targeted_menu_input = void;
    MenuControl(const MenuScene& scene, HMODULE module, bool confirmation,
                const frontend::MenuControlDefinition& definition, EngineFrame* frame,
                MenuTextObject& text, bool available, MenuWidgetContext* widgets = nullptr)
        : scene_(scene), module_(module), confirmation_(confirmation), definition_(definition),
          frame_(frame), text_(text), available_(available), widgets_(widgets) { text_.bind_frame(frame_); }
    bool initialize_visuals() {
        if (definition_.kind == frontend::MenuControlKind::button) {
            button_ = std::make_unique<ButtonVisualPair>(scene_, module_, definition_.frame,
                definition_.normal, definition_.active, confirmation_, &text_, available_, frame_);
            button_->bind_pick_target(definition_.identifier);
        } else if (definition_.kind == frontend::MenuControlKind::text) {
            // 006618a0 writes control +60 to its bound frame's pick word.
            std::memcpy(reinterpret_cast<unsigned char*>(frame_) + 0xec,
                        &definition_.identifier, sizeof(definition_.identifier));
        } else if (definition_.kind == frontend::MenuControlKind::slider || definition_.kind == frontend::MenuControlKind::selector || definition_.kind == frontend::MenuControlKind::toggle) {
            if (!widgets_) throw std::logic_error("Widget construction requires shared input/settings services");
            widget_visuals_ = std::make_unique<MenuWidgetVisuals>(module_,frame_,text_,*widgets_,[this] { return parent_disabled_ && parent_disabled_(); });
            widget_ = std::make_unique<MenuOptionsWidget>(definition_.kind,definition_.identifier,*widget_visuals_,widgets_->queue);
            if (!widget_->initialize_visuals()) return false;
            text_.attach_control(widget_->properties(),true);
        } else {
            throw std::logic_error("Original widget kind needs a native constructor backend");
        }
        return true;
    }
    void initialize_definition_data() {} // All five implemented kinds use +78 /0065CD30.
    void initialize_events(const std::vector<frontend::ControlEvent>& events) {
        if (initialized_) throw std::logic_error("Control initialized twice");
        if (widget_) { widget_->initialize(events); widgets_->connect_options(*widget_); }
        else if (button_) button_->initialize_events(events);
        else {
            text_.initialize_events(events);
            std::printf("Original static initialization: frame=%s records=%u\n",
                        definition_.frame, static_cast<unsigned>(events.size()));
        }
        initialized_ = true;
    }
    bool receive_event(frontend::ControlEvent& event, frontend::ControlEventDestination& upstream) {
        if (widget_) return widget_->receive_event(event, upstream);
        if (button_) return button_->receive_event(event, upstream);
        frontend::EmittingControlDestination<MenuTextObject> destination{text_, &upstream};
        return text_.dispatch_control_event(event, destination);
    }
    bool eligible() const { return widget_ ? widget_->eligible() : button_ ? button_->eligible() : text_.eligible(); }
    bool disabled() const { return widget_ ? widget_->disabled() : button_ ? button_->disabled() : text_.disabled(); }
    bool key(std::uint32_t key) { return widget_ ? false : button().key(key); }
    // Text table00813CC0 and button table00814760 +1c both point to the
    // verified RET4 body0064DA20. No text refresh or fabricated tick service.
    void update(std::uint32_t delta) { if (widget_) widget_->update(delta); }
    bool preserves_part_targets() const { return widget_ != nullptr; }
    void set_parent_disabled_query(std::function<bool()> query) { parent_disabled_ = std::move(query); }
    int enter_focus(std::uint32_t target) { return widget_ ? widget_->enter_focus(target) : button().apply(true); }
    int leave_focus(std::uint32_t target) { return widget_ ? widget_->leave_focus(target) : button().apply(false); }
    int pointer_focus(std::uint32_t target) { return widget_ ? widget_->pointer_focus(target) : button().pointer_move(); }
    int pointer_target(frontend::ScreenPointerCallback callback,std::uint32_t target,frontend::ControlEventDestination& events) { return widget_ ? widget_->pointer_event(callback,target) : button().pointer_event(callback,events); }
    int wheel_target(std::int32_t delta,frontend::ControlEventDestination& events) { return widget_ ? widget_->wheel(delta) : button().wheel_event(events); }
    int apply(bool hover) { return widget_ ? (hover ? widget_->enter_focus(definition_.identifier) : widget_->leave_focus(definition_.identifier)) : button().apply(hover); }
    int pointer_move() { return widget_ ? widget_->pointer_focus(definition_.identifier) : button().pointer_move(); }
    int pointer_event(frontend::ScreenPointerCallback callback, frontend::ControlEventDestination& events) {
        return widget_ ? widget_->pointer_event(callback,definition_.identifier) : button().pointer_event(callback, events);
    }
    int wheel_event(frontend::ControlEventDestination& events) { return widget_ ? widget_->wheel(0) : button().wheel_event(events); }
    int activate(frontend::ControlEventDestination& events) { return widget_ ? widget_->activate() : button().activate(events); }
    int deactivate(frontend::ControlEventDestination& events) { return widget_ ? widget_->deactivate() : button().deactivate(events); }
    int click(frontend::ControlEventDestination& events) { return widget_ ? widget_->click() : button().click(events); }
    void bind_pick_target(std::uint32_t id) { if (widget_) { if (id != definition_.identifier) throw std::logic_error("Widget ID changed after construction"); } else button().bind_pick_target(id); }
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
    MenuWidgetContext* widgets_;
    std::function<bool()> parent_disabled_;
    std::unique_ptr<MenuWidgetVisuals> widget_visuals_;
    std::unique_ptr<MenuOptionsWidget> widget_;
};

using MenuFocus = frontend::DefinedScreen<MenuControl>;
using MenuScreenControls = frontend::DefinedControls<MenuControl>;

struct MenuControlCreator {
    const MenuScene& scene;
    HMODULE module;
    bool confirmation;
    const std::vector<frontend::MenuControlRole>& available;
    const std::function<MenuTextObject*(std::uint32_t)>& text_lookup;
    EngineFrame* model_root = nullptr;
    EngineFrame* frame = nullptr;
    MenuWidgetContext* widgets = nullptr;

    bool find_frame(const frontend::MenuControlDefinition& definition, std::uint32_t flags) {
        if (flags != 0x10021) throw std::logic_error("Unexpected original control lookup flags");
        // Existing verified traversal bridges FindFrame. Broader flag matching
        // and original error UI are not reconstructed by this native adapter.
        frame = scene.control_frame(definition.frame, module, confirmation, model_root);
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
        return std::make_unique<MenuControl>(scene, module, confirmation, definition, frame, *text, enabled, widgets);
    }
};

inline std::unique_ptr<MenuScreenControls> make_menu_controls(
        const MenuScene& scene, HMODULE module, const frontend::MenuScreenDefinition& definition,
        bool confirmation, const std::vector<frontend::MenuControlRole>& available,
        const std::function<MenuTextObject*(std::uint32_t)>& text_lookup,
        EngineFrame* model_root = nullptr, MenuWidgetContext* widgets = nullptr) {
    return std::make_unique<MenuScreenControls>(definition, available,
        MenuControlCreator{scene, module, confirmation, available, text_lookup, model_root, nullptr, widgets});
}

}

#endif
