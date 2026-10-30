#ifndef HD2_HOST_MENU_TEXT_OBJECT_H
#define HD2_HOST_MENU_TEXT_OBJECT_H

#include "frontend/control_runtime.h"
#include "frontend/control_initialization.h"
#include "frontend/text_control_runtime.h"
#include "frontend/menu_definition.h"
#include "host/menu_scene.h"
#include "host/menu_text_geometry.h"

#include <optional>

namespace hd2::host {

// Stable attached text state shared by definition initialization and glyph
// submission. Geometry still uses the verified host visual-bounds bridge;
// original text-object/class ABI and general paragraph layout remain absent.
class MenuTextObject {
public:
    MenuTextObject(const MenuScene& scene, HMODULE module,
                   const frontend::MenuControlDefinition& definition, bool confirmation,
                   frontend::TextObject::Localizer localize)
        : scene_(scene), module_(module), definition_(definition), confirmation_(confirmation),
          localize_(std::move(localize)) {
        text_.inset = definition.kind == frontend::MenuControlKind::button ? 10 : 1;
        properties_.focusable = 0; // Original static-text control constructor.
    }
    MenuTextObject(const MenuTextObject&) = delete;
    MenuTextObject& operator=(const MenuTextObject&) = delete;
    frontend::TextObject& object() { return text_; }
    const frontend::TextObject& object() const { return text_; }
    const frontend::MenuControlDefinition& definition() const { return definition_; }
    void attach_control(const frontend::ControlProperties& properties, bool available) {
        owner_ = &properties;
        available_ = available;
    }
    bool visible() const { return (owner_ ? owner_->visible : properties_.visible) != 0; }
    bool available() const { return available_; }
    bool eligible() const { return properties_.eligible(); }
    bool disabled() const { return properties_.disabled != 0; }
    bool set_localized(std::uint32_t id, std::uint32_t style, std::int32_t index) {
        if (style == 40) {
            std::printf("Deferred original version formatter: frame=%s localization=%u\n", definition_.frame, id);
            return false;
        }
        return text_.set_localized(id, style, index, localize_);
    }
    bool append_localized(std::uint32_t id, std::uint32_t style) {
        return text_.append_localized(id, style, localize_);
    }
    void layout_text_on_frame() { layout_text_on_normal(); }
    void layout_text_on_normal() {
        bounds_ = scene_.label_bounds(definition_.frame,
            definition_.normal ? definition_.normal : definition_.frame, module_, confirmation_);
        text_.dirty = true;
    }
    void set_frame_direction(float x, float y, float z) {
        auto* frame = scene_.label_visual(definition_.frame,
            definition_.normal ? definition_.normal : definition_.frame, module_, confirmation_);
        using Direction = void (WINAPI *)(EngineFrame*, const float*, const float*);
        const float direction[]{x,y,z};
        scene_method<Direction>(frame, module_, 0x0c, 0x1c8d0)(frame, direction, nullptr);
    }
    const std::array<float,8>& bounds() {
        if (!bounds_) layout_text_on_normal();
        return *bounds_;
    }
    MenuRectangle rectangle(bool inset) {
        return menu_rectangle(bounds(), 800, 600, inset ? text_.inset : 0);
    }
    // Both static labels and buttons now initialize through the shared factory.
    void initialize_events(const std::vector<frontend::ControlEvent>& initialization) {
        auto events = initialization;
        frontend::initialize_control_events(*this, events);
    }
    bool dispatch_initialization_event(frontend::ControlEvent& event) {
        frontend::EmittingControlDestination<MenuTextObject> destination{*this, nullptr};
        return dispatch_control_event(event, destination);
    }
    bool dispatch_control_event(frontend::ControlEvent& event, frontend::ControlEventDestination& destination) {
        frontend::ControlStateRuntime<MenuTextObject> base{*this, properties_, bindings_, destination};
        frontend::TextControlRuntime<MenuTextObject, decltype(base)> text{*this, base};
        return text.dispatch(event);
    }
    bool has_text() const { return true; }
    void set_text_disabled(std::uint8_t value) { text_.disabled = value; }
    void set_frame_visible(std::uint8_t value) {
        auto* frame = scene_.control_frame(definition_.frame, module_, confirmation_);
        using Visibility = void (WINAPI *)(EngineFrame*, std::uint8_t);
        scene_method<Visibility>(frame, module_, 0x30, 0x1cfe0)(frame, value);
    }
    void update_frame() {
        auto* frame = scene_.control_frame(definition_.frame, module_, confirmation_);
        using Update = void (WINAPI *)(EngineFrame*);
        scene_method<Update>(frame, module_, 0x24, 0x1e610)(frame);
    }
    std::size_t child_count() const { return 0; }
    void dispatch_child(std::size_t, frontend::ControlEvent&) {
        throw std::logic_error("Static text has no reconstructed child controls");
    }
private:
    const MenuScene& scene_;
    HMODULE module_;
    const frontend::MenuControlDefinition& definition_;
    bool confirmation_;
    frontend::TextObject::Localizer localize_;
    frontend::TextObject text_;
    frontend::ControlProperties properties_;
    frontend::ControlEventBindings bindings_;
    const frontend::ControlProperties* owner_ = nullptr;
    bool available_ = true;
    std::optional<std::array<float,8>> bounds_;
};

}

#endif
