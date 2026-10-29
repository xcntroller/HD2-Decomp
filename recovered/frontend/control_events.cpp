#include "frontend/control_events.h"

#include <stdexcept>

namespace hd2::frontend {

ControlEvent observer_control_event(std::uint32_t target, std::uint32_t tag,
                                   void* context, ObserverCallback callback, bool remove) {
    return {target, remove ? 0x01000008u : 0x01000007u, tag, 0, 0,
            {}, std::make_shared<ControlObserverRequest>(ControlObserverRequest{context, callback})};
}

ControlObserver observer_request(const ControlEvent& event) {
    if (event.word_0c != 0 || event.word_10 != 0) {
        throw std::invalid_argument("Original observer addresses require a typed portable payload");
    }
    if (!event.observer) return {event.argument, nullptr, nullptr};
    return {event.argument, event.observer->context, event.observer->callback};
}

// Client 0065cac0. Duplicate ordinary tags append; lookup calls the first match.
bool register_control_observer(ControlEventBindings& bindings, const ControlObserver& observer) {
    if (observer.tag == 0 || !observer.callback) return false;
    if (observer.tag == 0x04021000) {
        bindings.override_tag = observer.tag;
        bindings.override_receiver = observer.context;
        bindings.override_callback = observer.callback;
    } else {
        bindings.observers.push_back(observer);
    }
    return true;
}

// Client 0065c9a0. Each nonzero criterion OVERWRITES the previous comparison:
// context takes precedence over tag, which takes precedence over callback.
// This is deliberately not conjunction/wildcard filtering. Walk backward so
// erasing preserves the relative order of all retained observers.
bool unregister_control_observers(ControlEventBindings& bindings, const ControlObserver& filter) {
    if (filter.tag == 0x04021000) {
        bindings.override_tag = 0;
        bindings.override_receiver = nullptr;
        bindings.override_callback = nullptr;
    }
    for (std::size_t i = bindings.observers.size(); i != 0;) {
        --i;
        bool match = false;
        if (filter.callback) match = bindings.observers[i].callback == filter.callback;
        if (filter.tag) match = bindings.observers[i].tag == filter.tag;
        if (filter.context) match = bindings.observers[i].context == filter.context;
        if (match) bindings.observers.erase(bindings.observers.begin() + static_cast<std::ptrdiff_t>(i));
    }
    return true;
}

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
