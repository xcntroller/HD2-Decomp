#ifndef HD2_FRONTEND_FOCUS_REQUESTS_H
#define HD2_FRONTEND_FOCUS_REQUESTS_H
#include "frontend/control_events.h"

namespace hd2::frontend {

// Explicit queued focus, used by006211e0/00637990/00656ed0. Target retains its
// hierarchical identifier. The screen applies focus before child delivery.
template<class Queue>
void request_control_focus(Queue& queue, std::uint32_t target) {
    queue.enqueue({target, 0x02000016, 0, 0, 0, {}});
}

// 00637990: initialize Password input, request named modal slot10, then focus
// its actual editable child. The application's +1c8 mode store is a required
// service; this helper enables no network/password application flow itself.
template<class Services>
void request_password_input(Services& services, ControlEventDestination& queue) {
    services.set_menu_mode(7);
    queue.enqueue({0x28200000, 0x01000015, 14, 2002, 0, {}});
    queue.enqueue({0, 0x02000002, 10, 0, 0, {}});
    request_control_focus(queue, 0x28200000);
}

}
#endif
