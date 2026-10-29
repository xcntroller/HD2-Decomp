#ifndef HD2_FRONTEND_DEFINED_SCREEN_EVENTS_H
#define HD2_FRONTEND_DEFINED_SCREEN_EVENTS_H

#include "frontend/control_runtime.h"
#include "frontend/control_initialization.h"
#include "frontend/menu_definition.h"
#include "frontend/screen_event.h"
#include "frontend/screen_lifecycle.h"
#include "frontend/screen_target.h"

#include <functional>

namespace hd2::frontend {

// Borrowed native message endpoints keep heterogeneous text/button children
// in their original definition slots, independently of input availability.
// Backend supplies model/focus hooks; it and the queue outlive this object.
template<class Backend>
class DefinedScreenEvents {
public:
    using Child = std::function<bool(ControlEvent&, ControlEventDestination&)>;
    DefinedScreenEvents(const MenuScreenDefinition& definition, Backend& backend,
                        ControlEventDestination& queue)
        : definition_(definition), backend_(backend), queue_(queue), children_(definition.controls.size()) {
        for (const auto& child : definition.controls) ids_.push_back(child.identifier);
    }
    DefinedScreenEvents(const DefinedScreenEvents&) = delete;
    DefinedScreenEvents& operator=(const DefinedScreenEvents&) = delete;

    void bind_child(std::size_t index, Child child) {
        if (index >= children_.size() || children_[index] || !child) {
            throw std::logic_error("Invalid or duplicate screen message binding");
        }
        children_[index] = std::move(child);
    }
    bool dispatch(ControlEvent& event) {
        EmittingControlDestination<DefinedScreenEvents> destination{*this, &queue_};
        return dispatch_control_event(event, destination);
    }
    bool dispatch_control_event(ControlEvent& event, ControlEventDestination& destination) {
        Messages messages{*this, destination};
        return dispatch_screen_event(messages, event);
    }
    // Native typed initialization records only. Original callback addresses
    // must be reconstructed before they can be installed as observers.
    void initialize(const std::vector<ControlEvent>& events) {
        auto records = events;
        initialize_control_events(*this, records);
    }
    bool dispatch_initialization_event(ControlEvent& event) { return dispatch(event); }
    void activate(std::uint32_t focus) { activate_screen(*this, focus); }
    void deactivate() { deactivate_screen(*this); }
    bool frame_visible() const { return backend_.frame_visible(); }
    int focused_control() const { return backend_.focused_control(); }
    int enter_focus(std::uint32_t target) { return backend_.enter_focus(target); }
    int leave_focus(std::uint32_t target) { return backend_.leave_focus(target); }
    void set_frame_visible(std::uint8_t visible) { backend_.set_frame_visible(visible); }
    void update_frame() { backend_.update_frame(); }
    void emit(std::uint32_t tag, std::uint32_t argument) {
        EmittingControlDestination<DefinedScreenEvents> destination{*this, &queue_};
        dispatch_tagged_control_events(bindings_, this, tag, argument, destination);
        backend_.event_emitted(tag, argument);
    }
    bool has_text() const { return false; }
    void set_text_disabled(std::uint8_t) { throw std::logic_error("Screen has no attached text"); }
    std::size_t child_count() const { return children_.size(); }
    void dispatch_child(std::size_t index, ControlEvent& event) {
        // Child +74 layout propagation uses a fresh local event supplied by
        // the base dispatcher; these bounded text/button endpoints share it.
        dispatch_copied_control_event(event, [&](ControlEvent& copy) {
            return dispatch_child_event(static_cast<int>(index), copy, queue_);
        });
    }
private:
    // Screen +14 overrides base hide with 00668440. Base +10 show retains
    // 0065c760's unconditional visible-byte/frame/update/notification sequence.
    class State : public ControlStateRuntime<DefinedScreenEvents> {
    public:
        State(DefinedScreenEvents& screen, ControlEventDestination& destination)
            : ControlStateRuntime<DefinedScreenEvents>(screen, screen.properties_, screen.bindings_, destination),
              screen_(screen) {}
        bool dispatch(ControlEvent& event) { return dispatch_control_state_event(*this, event); }
        void hide() { screen_.deactivate(); }
    private:
        DefinedScreenEvents& screen_;
    };
    struct Messages {
        DefinedScreenEvents& screen;
        ControlEventDestination& destination;
        bool dispatch_base_event(ControlEvent& event) { State base{screen, destination}; return base.dispatch(event); }
        int focused_control() const { return screen.focused_control(); }
        int leave_focus(std::uint32_t target) { return screen.leave_focus(target); }
        int enter_focus(std::uint32_t target) { return screen.enter_focus(target); }
        int control_for_target(std::uint32_t target) const {
            return find_screen_control(screen.definition_.identifier, screen.ids_, target);
        }
        void dispatch_child_event(int index, ControlEvent& event) {
            screen.dispatch_child_event(index, event, destination);
        }
    };
    bool dispatch_child_event(int index, ControlEvent& event, ControlEventDestination& destination) {
        if (index < 0 || static_cast<std::size_t>(index) >= children_.size() || !children_[index]) {
            throw std::logic_error("Screen child has no message endpoint");
        }
        // Copy the endpoint before calling it so a callback may append/rebind
        // other native state without invalidating the callable in progress.
        auto child = children_[index];
        return child(event, destination);
    }
    friend class ControlStateRuntime<DefinedScreenEvents>;
    const MenuScreenDefinition& definition_;
    Backend& backend_;
    ControlEventDestination& queue_;
    std::vector<Child> children_;
    std::vector<std::uint32_t> ids_;
    ControlProperties properties_;
    ControlEventBindings bindings_;
};

}

#endif
