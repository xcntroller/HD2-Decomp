#include "frontend/control_events.h"

#include <stdexcept>

namespace hd2::frontend {

bool notify_control_observers(ControlEventBindings& bindings, void* sender,
                              std::uint32_t tag, std::uint32_t argument) {
    if (bindings.override_tag == 0x04021000) {
        const auto callback = bindings.override_callback;
        if (!callback) throw std::logic_error("Missing control override callback");
        return callback(bindings.override_receiver, nullptr, sender, tag, argument);
    }

    for (std::size_t i = 0; i < bindings.observers.size(); ++i) {
        if (bindings.observers[i].tag == tag && bindings.observers[i].callback) {
            const auto context = bindings.observers[i].context;
            const auto callback = bindings.observers[i].callback;
            return callback(context, context, sender, tag, argument);
        }
    }
    return true;
}

void register_tagged_control_event(ControlEventBindings& bindings, const ControlEvent& event) {
    if ((event.code & 0x040ff000) == 0) {
        throw std::invalid_argument("Control event has no tag selector");
    }
    bindings.events.push_back(event);
}

void dispatch_tagged_control_events(ControlEventBindings& bindings, void* sender,
                                    std::uint32_t tag, std::uint32_t argument,
                                    ControlEventDestination& destination) {
    notify_control_observers(bindings, sender, tag, argument);
    ControlEvent event{};
    for (std::size_t i = 0; i < bindings.events.size(); ++i) {
        if ((bindings.events[i].code & 0x040ff000) != tag) continue;
        event = bindings.events[i];
        event.code &= 0xfbf00fff;
        if (event.target == 0xffffffff) {
            destination.dispatch_local(event);
        } else {
            destination.enqueue(event);
        }
    }
}

}
