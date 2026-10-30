#ifndef HD2_FRONTEND_CONTAINER_EVENT_H
#define HD2_FRONTEND_CONTAINER_EVENT_H

#include "frontend/control_events.h"

namespace hd2::frontend {

// 00656e80 (kind five). Unlike screen 006684f0, try the child FIRST,
// including tagged/zero-target messages. A valid child owns the return value;
// an invalid index falls back to base and is always reported handled.
template<class Container>
bool dispatch_container_event(Container& container, ControlEvent& event) {
    const int index = container.control_for_target(event.target);
    if (!container.valid_control(index)) {
        container.dispatch_base_event(event);
        return true;
    }
    return container.dispatch_child_event(container.control_for_target(event.target), event);
}

}

#endif
