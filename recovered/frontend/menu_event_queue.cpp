#include "frontend/menu_event_queue.h"

#include <cstdio>
#include <stdexcept>
#include <utility>

namespace hd2::frontend {

void MenuEventQueue::dispatch_local(ControlEvent&) {
    throw std::logic_error("Local control event dispatch is not implemented");
}

void MenuEventQueue::enqueue(const ControlEvent& event) {
    pending_.push_back(event);
}

void MenuEventQueue::pump() {
    while (!pending_.empty()) {
        ControlEvent event = std::move(pending_.front());
        pending_.pop_front();
        route(std::move(event));
    }
}

void MenuEventQueue::cancel_confirmation() {
    if (!screens_.current() || screens_.current()->identifier != 0x10000000) {
        throw std::logic_error("Confirmation is not on the screen stack");
    }
    enqueue({0, 0x14001200, 1, 0, 0, {}});
    enqueue({0, 0x06001005, 0, 0, 0, {}});
}

void MenuEventQueue::route(ControlEvent event) {
    std::printf("Menu event: target=%08x code=%08x argument=%08x\n",
                static_cast<unsigned>(event.target), static_cast<unsigned>(event.code),
                static_cast<unsigned>(event.argument));
    if ((event.code & 0x20000000) != 0) {
        deferred_.push_back(std::move(event));
        return;
    }
    if ((event.code & 0x10000000) != 0) {
        if ((event.code & 0x100) != 0) {
            while (!deferred_.empty()) {
                ControlEvent released = std::move(deferred_.front());
                deferred_.pop_front();
                released.code &= 0xdfffffff;
                enqueue(released);
            }
        }
        if ((event.code & 0x200) != 0) deferred_.clear();
        return;
    }
    if (event.target != 0) {
        // 0066ca43: resolve registered screen from the target's high six bits,
        // independently of which screen is currently on top of the stack.
        const auto index = event.target >> 26;
        if (!actions_.screen_available(index)) {
            throw std::logic_error("Targeted menu screen has no runtime binding");
        }
        actions_.dispatch_screen_event(index, event); // Original ignores result.
        return;
    }

    switch (event.code & 0x03000fff) {
    case 0x0200000a:
        // Derived menu callback 00626b20 uses the CURRENT stack, not the
        // identifier captured in the queued keyboard event. Bounded to the
        // main menu / confirmation; original screen 2/14/10 cases are absent.
        if (!screens_.current()) break;
        if (screens_.current_index() == 4) cancel_confirmation();
        else if (screens_.current_index() == 1) enqueue({0, 0x02000003, 0, 0, 0, {}});
        else throw std::logic_error("Unsupported keyboard back screen");
        break;
    case 0x02000003:
        if (screens_.size() == 1) {
            enqueue({0, 0x02000002, 4, 0, 0, {}});
            enqueue({0, 0x22000004, 0, 0, 0, {}});
        } else if ((event.argument & 0xfc000000) == 0x10000000) {
            cancel_confirmation();
        } else {
            screens_.pop(false);
        }
        break;
    case 0x02000001:
        screens_.push(event.argument, false);
        break;
    case 0x02000002:
        screens_.push(event.argument, true);
        break;
    case 0x02000005:
        screens_.pop(true);
        break;
    case 0x02000004:
        actions_.quit();
        break;
    default:
        throw std::logic_error("Unsupported front-end event");
    }
}

}
