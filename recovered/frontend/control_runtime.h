#ifndef HD2_FRONTEND_CONTROL_RUNTIME_H
#define HD2_FRONTEND_CONTROL_RUNTIME_H

#include "frontend/control_state.h"

#include <stdexcept>

namespace hd2::frontend {

// Original 0065cf50 local-target branch dispatches through the EMITTING
// control's +6c. The front-end queue is used only for nonlocal targets.
template<class Control>
class EmittingControlDestination : public ControlEventDestination {
public:
    EmittingControlDestination(Control& control, ControlEventDestination* upstream)
        : control_(control), upstream_(upstream) {}
    void dispatch_local(ControlEvent& event) override { control_.dispatch_control_event(event, *this); }
    void enqueue(const ControlEvent& event) override {
        if (!upstream_) throw std::logic_error("Control event requires the front-end queue");
        upstream_->enqueue(event);
    }
private:
    Control& control_;
    ControlEventDestination* upstream_;
};

// Portable context for the reconstructed shared control messages. Backend owns
// visual/text/child bindings; properties/bindings/destination outlive this call.
// The original control ABI and intrusive ownership are not reproduced here.
template<class Backend>
class ControlStateRuntime {
public:
    ControlStateRuntime(Backend& backend, ControlProperties& properties,
                        ControlEventBindings& bindings, ControlEventDestination& destination)
        : backend_(backend), properties_(properties), bindings_(bindings), destination_(destination) {}

    bool dispatch(ControlEvent& event) { return dispatch_control_state_event(*this, event); }
    bool disabled() const { return properties_.disabled != 0; }
    void set_disabled(std::uint8_t value) { set_text_control_disabled(*this, value); }
    void set_disabled_state(std::uint8_t value) { properties_.disabled = value; }
    bool has_text() const { return backend_.has_text(); }
    void set_text_disabled(std::uint8_t value) { backend_.set_text_disabled(value); }
    void show() { set_control_visibility(*this, 1); }
    void hide() { set_control_visibility(*this, 0); }
    void set_visible_state(std::uint8_t value) { properties_.visible = value; }
    void set_frame_visible(std::uint8_t value) { backend_.set_frame_visible(value); }
    void update_frame() { backend_.update_frame(); }
    void set_focusable(std::uint8_t value) { properties_.focusable = value; }
    void set_word_58(std::uint32_t value) { properties_.word_58 = value; }
    void set_word_5c(std::uint32_t value) { properties_.word_5c = value; }
    std::uint32_t word_58() const { return properties_.word_58; }
    std::uint32_t word_5c() const { return properties_.word_5c; }
    void register_tagged_event(const ControlEvent& event) { register_tagged_control_event(bindings_, event); }
    void register_observer(const ControlEvent& event) { register_control_observer(bindings_, observer_request(event)); }
    void unregister_observer(const ControlEvent& event) { unregister_control_observers(bindings_, observer_request(event)); }
    void emit_event(std::uint32_t tag, std::uint32_t argument) {
        dispatch_tagged_control_events(bindings_, &backend_, tag, argument, destination_);
    }
    std::size_t child_count() const { return backend_.child_count(); }
    void dispatch_child(std::size_t index, ControlEvent& event) { backend_.dispatch_child(index, event); }

private:
    Backend& backend_;
    ControlProperties& properties_;
    ControlEventBindings& bindings_;
    ControlEventDestination& destination_;
};

}

#endif
