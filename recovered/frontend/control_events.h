#ifndef HD2_FRONTEND_CONTROL_EVENTS_H
#define HD2_FRONTEND_CONTROL_EVENTS_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace hd2::frontend {

using ObserverCallback = bool (*)(void* receiver, void* context, void* sender,
                                  std::uint32_t tag, std::uint32_t argument);

// Portable observer-definition payload. Original uses 32-bit words +0c/+10;
// typed payloads retain native pointer width without treating process addresses
// from the original client as callable pointers. Context remains borrowed.
struct ControlObserverRequest {
    void* context;
    ObserverCallback callback;
};

struct ControlEvent {
    std::uint32_t target;
    std::uint32_t code;
    std::uint32_t argument;
    std::uint32_t word_0c;
    std::uint32_t word_10;
    std::shared_ptr<void> payload;
    std::shared_ptr<ControlObserverRequest> observer{};
};

struct ControlObserver {
    std::uint32_t tag;
    void* context;
    ObserverCallback callback;
};

ControlEvent observer_control_event(std::uint32_t target, std::uint32_t tag,
                                   void* context, ObserverCallback callback,
                                   bool remove = false);
ControlObserver observer_request(const ControlEvent& event);

struct ControlEventBindings {
    std::uint32_t override_tag = 0;
    void* override_receiver = nullptr;
    ObserverCallback override_callback = nullptr;
    std::vector<ControlObserver> observers;
    std::vector<ControlEvent> events;
};

class ControlEventDestination {
public:
    virtual ~ControlEventDestination() = default;
    virtual void dispatch_local(ControlEvent& event) = 0;
    virtual void enqueue(const ControlEvent& event) = 0;
};

bool register_control_observer(ControlEventBindings& bindings, const ControlObserver& observer);
bool unregister_control_observers(ControlEventBindings& bindings, const ControlObserver& filter);

bool notify_control_observers(ControlEventBindings& bindings, void* sender,
                              std::uint32_t tag, std::uint32_t argument);
void register_tagged_control_event(ControlEventBindings& bindings, const ControlEvent& event);
void dispatch_tagged_control_events(ControlEventBindings& bindings, void* sender,
                                    std::uint32_t tag, std::uint32_t argument,
                                    ControlEventDestination& destination);

}

#endif
