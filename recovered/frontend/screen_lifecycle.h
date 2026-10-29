#ifndef HD2_FRONTEND_SCREEN_LIFECYCLE_H
#define HD2_FRONTEND_SCREEN_LIFECYCLE_H

#include <cstdint>

namespace hd2::frontend {

// 00668400, screen +84. The model's enabled bit gates the whole operation;
// entering focus may fail, but that does not veto showing/emitting the event.
template<class Screen>
void activate_screen(Screen& screen, std::uint32_t focus) {
    if (screen.frame_visible()) return;
    screen.enter_focus(focus);
    screen.set_frame_visible(true);
    screen.emit(0x0400d000, 0);
}

// 00668440, screen +14. Hide first, then release focus, then notify.
template<class Screen>
void deactivate_screen(Screen& screen) {
    if (!screen.frame_visible()) return;
    screen.set_frame_visible(false);
    screen.leave_focus(0);
    screen.emit(0x0400c000, 0);
}

}

#endif
