#ifndef HD2_FRONTEND_DEFINED_SCREEN_H
#define HD2_FRONTEND_DEFINED_SCREEN_H

#include "frontend/menu_definition.h"
#include "frontend/screen_target.h"
#include "frontend/screen_key.h"
#include "frontend/screen_focus.h"
#include "frontend/pointer_dispatch.h"

#include <limits>
#include <stdexcept>

namespace hd2::frontend {

// Portable definition-backed context for the reconstructed screen helpers.
// Controls/definition/destination outlive this screen. Storage is fixed after
// construction and bound control addresses must remain stable. Original object
// ABI/ownership and heterogeneous widgets are not reconstructed by this adapter.
template<class Control>
class DefinedScreen {
public:
    DefinedScreen(const MenuScreenDefinition& definition, ControlEventDestination& events)
        : definition_(definition), events_(events), controls_(definition.controls.size(), nullptr) {
        if (definition.controls.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            throw std::length_error("Too many screen controls");
        }
        for (const auto& control : definition.controls) ids_.push_back(control.identifier);
    }
    DefinedScreen(const DefinedScreen&) = delete;
    DefinedScreen& operator=(const DefinedScreen&) = delete;

    // Binding is explicit host availability policy. Unbound original children
    // retain their indices/IDs but cannot receive focus, keys or activation.
    void bind_control(int index, Control& control) {
        if (!valid_control(index) || controls_[static_cast<std::size_t>(index)] != nullptr) {
            throw std::logic_error("Invalid or duplicate screen control binding");
        }
        controls_[static_cast<std::size_t>(index)] = &control;
    }
    const MenuScreenDefinition& definition() const { return definition_; }
    const char* label() const { return definition_.label; }
    std::uint32_t identifier() const { return definition_.identifier; }
    int control_count() const { return static_cast<int>(controls_.size()); }
    bool valid_control(int index) const { return index >= 0 && index < control_count(); }
    int control_for_target(std::uint32_t target) const {
        return find_screen_control(identifier(), ids_, target);
    }
    bool control_eligible(int index) const {
        return valid_control(index) && controls_[static_cast<std::size_t>(index)] &&
               controls_[static_cast<std::size_t>(index)]->eligible();
    }
    bool control_disabled(int index) const {
        return !valid_control(index) || !controls_[static_cast<std::size_t>(index)] ||
               controls_[static_cast<std::size_t>(index)]->disabled();
    }
    std::uint32_t control_argument(int index) const { return ids_.at(static_cast<std::size_t>(index)); }
    int enter_focus(std::uint32_t target) { return screen_focus_enter(*this, target); }
    int leave_focus(std::uint32_t target) { return screen_focus_leave(*this, target); }
    bool key(std::uint32_t word) { return screen_key(*this, word); }
    bool key_control(int index, std::uint32_t key) { return bound(index).key(key); }
    bool activate_control(int index, std::uint32_t) {
        return !control_disabled(index) && bound(index).activate(events_);
    }
    int focused_control() const { return focused_; }
    void set_focused_control(int index) { focused_ = index; }
    int leave_control(int index, std::uint32_t) { return bound(index).apply(false); }
    int enter_control(int index, std::uint32_t) { return bound(index).apply(true); }
    int move_control(int index, std::uint32_t) { return bound(index).pointer_move(); }
    int click_control(int index, std::uint32_t) { return bound(index).click(events_); }
    int wheel_control(int index, std::uint32_t, std::int32_t) { return bound(index).wheel_event(events_); }
    int pointer_control(int index, ScreenPointerCallback callback, std::uint32_t) {
        return bound(index).pointer_event(callback, events_);
    }

    // Host picker/rectangle compatibility; original hierarchical target lookup
    // remains control_for_target. Canonicalize only available, eligible targets.
    std::uint32_t target_for_control(int index) const {
        return control_eligible(index) ? control_argument(index) : 0;
    }
    int interactive_control_for_target(std::uint32_t target) const {
        const int index = control_for_target(target);
        return control_eligible(index) ? index : -1;
    }
    std::uint32_t interactive_target(std::uint32_t target) const {
        return target_for_control(interactive_control_for_target(target));
    }

private:
    Control& bound(int index) const {
        if (!valid_control(index) || !controls_[static_cast<std::size_t>(index)]) {
            throw std::logic_error("Unbound screen control callback");
        }
        return *controls_[static_cast<std::size_t>(index)];
    }
    const MenuScreenDefinition& definition_;
    ControlEventDestination& events_;
    std::vector<Control*> controls_;
    std::vector<std::uint32_t> ids_;
    int focused_ = -1;
};

}

#endif
