#ifndef HD2_HOST_MENU_INPUT_SESSION_H
#define HD2_HOST_MENU_INPUT_SESSION_H

#include "host/menu_screen_product.h"
#include "frontend/pointer_update.h"
#include "frontend/pointer_motion.h"
#include "frontend/pointer_dispatch.h"
#include "frontend/menu_event_queue.h"
#include "frontend/screen_input.h"
#include "frontend/screen_key.h"
#include "frontend/screen_focus.h"
#include <functional>

namespace hd2::host {

// One host-owned session mirrors the shared input acquired by 0066a6e0 and
// retained at front-end +f4 (0066ad10). Dispatch state belongs to that same
// front end, not individual screens. Original singleton/refcount ABI is absent.
struct MenuInputSession {
    hd2::frontend::PointerUpdate pointer;
    hd2::frontend::PointerDispatchState dispatch;
    hd2::frontend::PointerDispatchInput input;
    std::optional<hd2::host::CursorPoint> previous_point;
    bool armed = false;
    bool awaiting_release = false;
    std::uint32_t active_screen = 0;

    void clear() {
        pointer.clear();
        dispatch = {};
        input = {};
        previous_point.reset();
        armed = false;
        awaiting_release = false;
        active_screen = 0;
    }
};

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
        return confirming_ ? hd2::frontend::screen_key(confirmation_, key)
                           : hd2::frontend::screen_key(main_, key);
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

// Functional opt-in bridge. Positions/focus and departure cancellation remain
// host-owned; button snapshots come from the rebuilt LS3DF polling subset.
template<class Screen>
class MenuFrameInput {
public:
    MenuFrameInput(MenuInputSession& session, Screen& focus,
                   std::function<std::uint32_t(const hd2::host::CursorPoint&)> repick,
                   std::function<void(float, float)> update_frame,
                   std::function<void()> accepted_click)
        : session_(session), focus_(focus), repick_(std::move(repick)), update_frame_(std::move(update_frame)),
          accepted_click_(std::move(accepted_click)) {}
    void tick(const hd2::host::CursorPoint& point, std::uint32_t target,
              std::uint32_t buttons, std::uint32_t delta, std::int32_t wheel) {
        if (!session_.armed) {
            if (buttons != 0) return; // Reentry must observe all buttons up first.
            session_.pointer.cursor().initialize(point.x, point.y, 800, 600);
            update_frame_(point.x, point.y);
            session_.previous_point = point;
            session_.armed = true;
        }
        if (session_.active_screen != focus_.identifier()) {
            // Host safety: never carry a press or drag into a different screen.
            // Preserve button history, click timer, cursor and wheel accumulation.
            session_.pointer.cancel_departed_press(0);
            session_.dispatch.dragging = false;
            session_.awaiting_release = buttons != 0;
            session_.active_screen = focus_.identifier();
            std::printf("Shared menu input: screen=%s cursor=(%.3f,%.3f) wheel=%.3f idle=%u\n",
                        focus_.label(), session_.pointer.cursor().x(), session_.pointer.cursor().y(),
                        session_.pointer.cursor().wheel(), static_cast<unsigned>(session_.pointer.idle()));
        }
        pending_ = {buttons, target, delta, point.x - session_.previous_point->x,
                    point.y - session_.previous_point->y, static_cast<float>(wheel)};
        session_.previous_point = point;
        if (session_.awaiting_release) {
            // Keep shared sampling/cursor updates, but absorb a held transition
            // and its release. A new screen must observe all buttons up first.
            pending_.inhibit_click = true;
            update_input(delta, target);
            session_.dispatch.previous_target = target;
            if (buttons == 0) session_.awaiting_release = false;
            return;
        }
        session_.pointer.cancel_departed_press(target); // Retain established host safety.
        hd2::frontend::dispatch_menu_pointer(session_.dispatch, *this, delta);
    }
    std::uint32_t pick_target() const { return pending_.target; }
    bool update_input(std::uint32_t delta, std::uint32_t target) {
        pending_.delta = delta;
        pending_.target = target;
        const auto result = session_.pointer.update(pending_);
        if (result.processed) update_frame_(session_.pointer.cursor().x(), session_.pointer.cursor().y());
        const auto& buttons = session_.pointer.buttons();
        session_.input = {buttons.pressed(3), buttons.released(3), buttons.currently_selected(3),
                  result.click.click, false, // Double-click remains observational only.
                  hd2::frontend::pointer_moved(session_.pointer.cursor().horizontal(),
                                             session_.pointer.cursor().vertical()),
                  hd2::frontend::pointer_wheel_moved(session_.pointer.cursor().wheel_delta())};
        return result.processed;
    }
    const hd2::frontend::PointerDispatchInput& input() const { return session_.input; }
    std::optional<std::uint32_t> captured_control_target() const { return std::nullopt; }
    int captured_control_event(std::uint32_t) {
        throw std::logic_error("No captured screen control object");
    }
    void clear_captured_control() {
        throw std::logic_error("No captured screen control object");
    }
    int screen_event(hd2::frontend::ScreenPointerCallback callback, std::uint32_t target) {
        using Callback = hd2::frontend::ScreenPointerCallback;
        switch (callback) {
        case Callback::focus: return hd2::frontend::screen_pointer_focus(focus_, target);
        case Callback::press: return hd2::frontend::screen_press(focus_, target);
        case Callback::release: return hd2::frontend::screen_release(focus_, target);
        case Callback::drag: return hd2::frontend::screen_drag(focus_, target);
        case Callback::drag_end: return hd2::frontend::screen_drag_end(focus_, target);
        case Callback::drag_begin: return hd2::frontend::screen_drag_begin(focus_, target);
        case Callback::double_click: return 7;
        case Callback::click:
            if (target == 0 || focus_.control_for_target(target) != focus_.focused_control()) {
                return 7;
            }
            std::printf("Engine mouse %s click: target=%08x focus=%d\n",
                        focus_.label(), static_cast<unsigned>(target), focus_.focused_control());
            return hd2::frontend::screen_click(focus_, target);
        }
        throw std::logic_error("Unsupported menu pointer callback");
    }
    void accepted_click() { accepted_click_(); }
    float wheel_delta() const { return session_.pointer.cursor().wheel_delta(); }
    int screen_wheel(std::uint32_t target, std::int32_t delta) {
        const int result = hd2::frontend::screen_wheel(focus_, target, delta);
        std::printf("Engine mouse wheel: screen=%s target=%08x delta=%d result=%d\n",
                    focus_.label(), static_cast<unsigned>(target), static_cast<int>(delta), result);
        return result;
    }
    Screen* current_screen() { return &focus_; }
    std::uint32_t pick_for_screen(Screen*) const { return repick_(*session_.previous_point); }
    void focus_screen(Screen* screen, std::uint32_t target) {
        hd2::frontend::screen_pointer_focus(*screen, target);
    }

private:
    MenuInputSession& session_;
    Screen& focus_;
    std::function<std::uint32_t(const hd2::host::CursorPoint&)> repick_;
    std::function<void(float, float)> update_frame_;
    std::function<void()> accepted_click_;
    hd2::frontend::PointerUpdateSample pending_{};
};


} // namespace hd2::host
#endif
