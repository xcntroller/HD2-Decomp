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
    if (screen_depth_ != 2) throw std::logic_error("Confirmation is not on the screen stack");
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
    if (event.target != 0) throw std::logic_error("Targeted menu events are not implemented");

    switch (event.code & 0x03000fff) {
    case 0x02000003:
        if (screen_depth_ == 1) {
            enqueue({0, 0x02000002, 4, 0, 0, {}});
            enqueue({0, 0x22000004, 0, 0, 0, {}});
        } else if (screen_depth_ == 2 && (event.argument & 0xfc000000) == 0x10000000) {
            cancel_confirmation();
        } else {
            throw std::logic_error("Unsupported menu back event");
        }
        break;
    case 0x02000002:
        if (screen_depth_ != 1 || event.argument != 4) {
            throw std::logic_error("Unsupported menu screen push");
        }
        ++screen_depth_;
        actions_.show_confirmation();
        break;
    case 0x02000005:
        if (screen_depth_ != 2) throw std::logic_error("Unsupported menu screen pop");
        --screen_depth_;
        actions_.hide_confirmation();
        break;
    case 0x02000004:
        actions_.quit();
        break;
    default:
        throw std::logic_error("Unsupported front-end event");
    }
}

}
