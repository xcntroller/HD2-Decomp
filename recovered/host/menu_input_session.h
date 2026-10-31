#ifndef HD2_HOST_MENU_INPUT_SESSION_H
#define HD2_HOST_MENU_INPUT_SESSION_H

#include "host/menu_screen_product.h"
#include "host/menu_pointer_session.h"
#include "frontend/menu_event_queue.h"
#include "frontend/screen_key.h"

namespace hd2::host {

// Portable keyboard context for the two enabled screens; callbacks always
// resolve the current screen instead of keeping one across focus/drag changes.
class MenuKeyboardInput {
public:
    MenuKeyboardInput(MenuInputSession& session, hd2::host::MenuFocus& main,
                      hd2::host::MenuFocus& confirmation, hd2::frontend::MenuEventQueue& events,
                      bool& confirming)
        : session_(session), main_(main), confirmation_(confirmation), events_(events),
          confirming_(confirming) {}
    bool key_current_screen(std::uint32_t key) {
        return confirming_ ? confirmation_.key(key) : main_.key(key);
    }
    bool dragging() const { return session_.dispatch.dragging; }
    void end_current_drag(std::uint32_t target) {
        if (confirming_) hd2::frontend::screen_drag_end(confirmation_, target);
        else hd2::frontend::screen_drag_end(main_, target);
    }
    void clear_dragging() { session_.dispatch.dragging = false; }
    std::uint32_t current_screen_identifier() const {
        return confirming_ ? confirmation_.identifier() : main_.identifier();
    }
    void enqueue_back_event(std::uint32_t screen) {
        events_.enqueue({0, 0x0200000a, screen, 0, 0, {}});
    }
private:
    MenuInputSession& session_;
    hd2::host::MenuFocus& main_;
    hd2::host::MenuFocus& confirmation_;
    hd2::frontend::MenuEventQueue& events_;
    bool& confirming_;
};


} // namespace hd2::host
#endif
