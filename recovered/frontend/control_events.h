#ifndef HD2_FRONTEND_CONTROL_EVENTS_H
#define HD2_FRONTEND_CONTROL_EVENTS_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace hd2::frontend {

struct ControlEvent {
    std::uint32_t target;
    std::uint32_t code;
    std::uint32_t argument;
    std::uint32_t word_0c;
    std::uint32_t word_10;
    std::shared_ptr<void> payload;
};

using ObserverCallback = bool (*)(void* receiver, void* context, void* sender,
                                  std::uint32_t tag, std::uint32_t argument);

struct ControlObserver {
    std::uint32_t tag;
    void* context;
    ObserverCallback callback;
};

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

bool notify_control_observers(ControlEventBindings& bindings, void* sender,
                              std::uint32_t tag, std::uint32_t argument);
void register_tagged_control_event(ControlEventBindings& bindings, const ControlEvent& event);
void dispatch_tagged_control_events(ControlEventBindings& bindings, void* sender,
                                    std::uint32_t tag, std::uint32_t argument,
                                    ControlEventDestination& destination);

}

#endif
