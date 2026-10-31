#ifndef HD2_FRONTEND_FRONTEND_UPDATE_H
#define HD2_FRONTEND_FRONTEND_UPDATE_H

#include <cstdint>

namespace hd2::frontend {

struct FrontendUpdateState {
    std::uint32_t mode = 10;          // Front end +20.
    std::uint8_t sound_gate = 7;      // Global00982100 low three bits.
    bool timer_enabled = false;      // +14.
    std::uint32_t timer = 0;         // +18; wrapping arithmetic.
};

// Client0066BEA0. There is ONE drain, before pointer/capture/screen updates.
// Services resolve live owners at each phase. Events enqueued after the drain
// remain pending until the next update, even if they change the current screen.
template<class Services>
void update_frontend(FrontendUpdateState& state, Services& services, std::uint32_t milliseconds) {
    services.update_menu(milliseconds); // Virtual +48, twice, unconditionally.
    if (state.mode != 11 || (state.sound_gate & 7) == 7) services.update_sound(milliseconds);
    services.update_menu(milliseconds);
    services.update_sound(milliseconds);
    if (state.timer_enabled) state.timer += milliseconds;
    services.pump_events();
    if (!services.screens_available()) return;
    services.update_pointer(milliseconds);
    services.update_capture(milliseconds);
    services.update_current_screen(milliseconds);
    services.update_overlay(milliseconds);
    // Snapshot the count, but resolve the list and its entry again on EVERY
    // iteration. Appends cannot extend this pass; callback mutations are live.
    auto count = services.listener_count();
    while (count != 0) services.update_listener(--count, milliseconds);
}

// Client00656AF0 (+1c group), also the tail of00668750 (screen).
template<class Container>
void update_focused_control(Container& container, std::uint32_t milliseconds) {
    if (container.valid_control(container.focused_control()))
        container.update_control(container.focused_control(), milliseconds);
}

}
#endif
