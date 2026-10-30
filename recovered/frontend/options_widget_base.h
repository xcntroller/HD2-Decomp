#ifndef HD2_FRONTEND_OPTIONS_WIDGET_BASE_H
#define HD2_FRONTEND_OPTIONS_WIDGET_BASE_H

#include "frontend/control_initialization.h"
#include "frontend/control_runtime.h"
#include "frontend/text_control_runtime.h"

#include <cstring>

namespace hd2::frontend {

inline std::int32_t widget_signed_word(std::uint32_t value) {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
inline float widget_float_word(std::uint32_t value) {
    float result;
    static_assert(sizeof(result) == sizeof(value));
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
inline std::uint32_t widget_float_bits(float value) {
    std::uint32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}

// Portable state/observer composition, not an additional original function.
// Backend supplies frames, attached text and already-constructed internal parts.
// Those objects and the queue outlive this runtime. No engine objects are built
// here, and no original raw callback address is transported into native code.
template<class Derived, class Backend>
class OptionsWidgetBase {
public:
    OptionsWidgetBase(std::uint32_t identifier, Backend& backend, ControlEventDestination& queue)
        : backend_(backend), queue_(queue), identifier_(identifier) {}
    OptionsWidgetBase(const OptionsWidgetBase&) = delete;
    OptionsWidgetBase& operator=(const OptionsWidgetBase&) = delete;
    bool eligible() const { return properties_.eligible(); }
    std::uint32_t identifier() const { return identifier_; }
    bool disabled() const { return properties_.disabled != 0; }
    std::uint8_t disabled_byte() const { return properties_.disabled; }
    bool key(std::uint32_t) const { return false; } // All three use 0064da30.
    bool receive_event(ControlEvent& event, ControlEventDestination& upstream) {
        EmittingControlDestination<Derived> destination{derived(), &upstream};
        return derived().dispatch_control_event(event, destination);
    }
    bool dispatch(ControlEvent& event) { return receive_event(event, queue_); }
    void initialize(const std::vector<ControlEvent>& events) {
        auto records = events;
        initialize_control_events(*this, records);
    }
    bool dispatch_initialization_event(ControlEvent& event) { return dispatch(event); }

    void set_disabled_state(std::uint8_t value) { properties_.disabled = value; }
    bool has_text() const { return backend_.has_text(); }
    void set_text_disabled(std::uint8_t value) { backend_.object().disabled = value; }
    void set_frame_visible(std::uint8_t value) { backend_.set_frame_visible(value); }
    void update_frame() { backend_.update_frame(); }
    std::size_t child_count() const { return backend_.child_count(); }
    void dispatch_child(std::size_t index, ControlEvent& event) {
        dispatch_copied_control_event(event, [&](ControlEvent& copy) { return backend_.receive_child(index, copy, queue_); });
    }
protected:
    Derived& derived() { return static_cast<Derived&>(*this); }
    void emit(std::uint32_t tag, std::uint32_t argument, ControlEventDestination& destination) {
        dispatch_tagged_control_events(bindings_, &derived(), tag, argument, destination);
    }
    void emit(std::uint32_t tag, std::uint32_t argument) {
        EmittingControlDestination<Derived> destination{derived(), &queue_};
        emit(tag, argument, destination);
    }
    // Derived +7c policies must also apply when the base message handler
    // receives disable/enable. A plain base runtime would bypass that override.
    class State : public ControlStateRuntime<OptionsWidgetBase> {
    public:
        State(OptionsWidgetBase& owner, ControlEventDestination& destination)
            : ControlStateRuntime<OptionsWidgetBase>(owner, owner.properties_, owner.bindings_, destination),
              owner_(owner), destination_(destination) {}
        bool dispatch(ControlEvent& event) { return dispatch_control_state_event(*this, event); }
        void set_disabled(std::uint8_t value) { owner_.derived().set_disabled(value); }
        void show() { set_control_visibility(*this, 1); }
        void hide() { set_control_visibility(*this, 0); }
        void emit_event(std::uint32_t tag, std::uint32_t argument) { owner_.emit(tag, argument, destination_); }
    private:
        OptionsWidgetBase& owner_;
        ControlEventDestination& destination_;
    };
    bool dispatch_base(ControlEvent& event, ControlEventDestination& destination) {
        State state{*this, destination};
        return state.dispatch(event);
    }
    bool dispatch_text(ControlEvent& event, ControlEventDestination& destination) {
        State state{*this, destination};
        TextControlRuntime<Backend, State> text{backend_, state};
        return text.dispatch(event);
    }
    Backend& backend_;
    ControlEventDestination& queue_;
    ControlProperties properties_;
    ControlEventBindings bindings_;
private:
    std::uint32_t identifier_;
};

}

#endif
