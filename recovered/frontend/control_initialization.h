#ifndef HD2_FRONTEND_CONTROL_INITIALIZATION_H
#define HD2_FRONTEND_CONTROL_INITIALIZATION_H

#include <cstddef>

namespace hd2::frontend {

// Client 0065cf00, control +70. List size/storage are resolved each iteration;
// dispatch receives the actual mutable definition record, without a copy.
// A handler's return value does not stop initialization. The original list is
// a vector of 24-byte records; List supplies portable storage/ownership here.
template<class Control, class List>
bool initialize_control_events(Control& control, List& events) {
    for (std::size_t i = 0; i < events.size(); ++i) {
        control.dispatch_initialization_event(events[i]); // Virtual +6c.
    }
    return true;
}

// Successful-allocation tail of the original control factory, 0065df91..dff6.
// The original +2c visual initialization gate runs before definition +20/+10
// lists. Failed initialization destroys the control but reports handled=true;
// it is NOT inserted. List-handler return values do not affect insertion.
// Allocation, kind selection, errors and original collection ABI are excluded.
template<class Factory>
bool initialize_created_control(Factory& factory) {
    if (!factory.initialize_visuals()) {
        factory.destroy_control(); // Virtual +80(1).
        return true;
    }
    factory.initialize_definition_data();   // Virtual +78, definition +20.
    factory.initialize_definition_events(); // Virtual +70, definition +10.
    factory.append_control();
    return true;
}

// 006618a0, called by kind-five +2c/00656e60. Assign this control's frame
// and frame pick identifier before definition-dependent recursion. Child
// creation results are ignored; reread the list on every iteration.
template<class Container, class Frame, class Definition>
bool initialize_container_visuals(Container& container, Frame frame, Frame layout_root, Definition* definition) {
    container.bind_frame(frame);
    container.set_frame_identifier(frame, container.identifier());
    if (definition) {
        container.set_definition_kind(definition->kind);
        for (std::size_t i = 0; i < definition->children.size(); ++i) {
            container.create_child(frame, layout_root, definition->children[i]);
        }
    }
    return true;
}

}

#endif
