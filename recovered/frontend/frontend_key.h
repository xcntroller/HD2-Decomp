#ifndef HD2_FRONTEND_FRONTEND_KEY_H
#define HD2_FRONTEND_FRONTEND_KEY_H

#include <cstdint>

namespace hd2::frontend {

// Client 0066b3e0 tail, after original device/character preprocessing. Resolve
// the current screen afresh for each callback. An unconsumed Escape (low byte
// one) ends a drag before enqueuing 0200000a with the current screen identifier.
// Original return value means the front end handled the key, not activation.
template<class Frontend>
bool dispatch_frontend_key(Frontend& frontend, std::uint32_t key) {
    if (frontend.key_current_screen(key)) return true;
    if (static_cast<std::uint8_t>(key) != 1) return true;
    if (frontend.dragging()) {
        frontend.end_current_drag(0);
        frontend.clear_dragging();
    }
    frontend.enqueue_back_event(frontend.current_screen_identifier());
    return true;
}

}

#endif
