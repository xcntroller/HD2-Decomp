#ifndef HD2_FRONTEND_DEFINED_CONTAINER_EVENTS_H
#define HD2_FRONTEND_DEFINED_CONTAINER_EVENTS_H

#include "frontend/container_event.h"
#include "frontend/control_runtime.h"
#include "frontend/control_initialization.h"
#include "frontend/screen_target.h"

#include <functional>

namespace hd2::frontend {

// Native kind-five message/state subset. Children append only after successful
// construction, preserving sparse definition IDs and actual insertion order.
// Backend owns frames/children; callable bindings do not own those objects.
template<class Backend>
class DefinedContainerEvents {
public:
    using Child = std::function<bool(ControlEvent&, ControlEventDestination&)>;
    DefinedContainerEvents(std::uint32_t identifier, Backend& backend, ControlEventDestination& queue)
        : identifier_(identifier), backend_(backend), queue_(queue) {}
    DefinedContainerEvents(const DefinedContainerEvents&) = delete;
    DefinedContainerEvents& operator=(const DefinedContainerEvents&) = delete;

    bool eligible() const { return properties_.eligible(); }
    bool visible() const { return properties_.visible != 0; }
    bool disabled() const { return properties_.disabled != 0; }

    void append_child(std::uint32_t identifier, Child child) {
        if (!child) throw std::logic_error("Container child has no message endpoint");
        // Keep parallel vectors consistent if allocation fails.
        children_.push_back(std::move(child));
        try { identifiers_.push_back(identifier); }
        catch (...) { children_.pop_back(); throw; }
    }
    bool dispatch(ControlEvent& event) {
        EmittingControlDestination<DefinedContainerEvents> destination{*this, &queue_};
        return dispatch_control_event(event, destination);
    }
    bool dispatch_control_event(ControlEvent& event, ControlEventDestination& destination) {
        Messages context{*this, destination};
        return dispatch_container_event(context, event);
    }
    void initialize(const std::vector<ControlEvent>& events) {
        auto records = events;
        initialize_control_events(*this, records);
    }
    bool dispatch_initialization_event(ControlEvent& event) { return dispatch(event); }
    bool has_text() const { return false; }
    void set_text_disabled(std::uint8_t) { throw std::logic_error("Container has no attached text"); }
    void set_frame_visible(std::uint8_t visible) { backend_.set_frame_visible(visible); }
    void update_frame() { backend_.update_frame(); }
    std::size_t child_count() const { return children_.size(); }
    void dispatch_child(std::size_t index, ControlEvent& event) {
        dispatch_copied_control_event(event, [&](ControlEvent& copy) {
            return deliver(index, copy, queue_);
        });
    }
private:
    struct Messages {
        DefinedContainerEvents& owner;
        ControlEventDestination& destination;
        int control_for_target(std::uint32_t target) const {
            return find_screen_control(owner.identifier_, owner.identifiers_, target);
        }
        bool valid_control(int index) const {
            return index >= 0 && static_cast<std::size_t>(index) < owner.children_.size();
        }
        bool dispatch_base_event(ControlEvent& event) {
            ControlStateRuntime<DefinedContainerEvents> state{owner, owner.properties_, owner.bindings_, destination};
            return state.dispatch(event);
        }
        bool dispatch_child_event(int index, ControlEvent& event) {
            if (index < 0) throw std::logic_error("Container lookup changed to invalid index");
            return owner.deliver(static_cast<std::size_t>(index), event, destination);
        }
    };
    bool deliver(std::size_t index, ControlEvent& event, ControlEventDestination& destination) {
        if (index >= children_.size()) throw std::logic_error("Missing container child");
        auto child = children_[index];
        return child(event, destination);
    }
    std::uint32_t identifier_;
    Backend& backend_;
    ControlEventDestination& queue_;
    ControlProperties properties_;
    ControlEventBindings bindings_;
    std::vector<Child> children_;
    std::vector<std::uint32_t> identifiers_;
};

}

#endif
