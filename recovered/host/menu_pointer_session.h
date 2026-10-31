#ifndef HD2_HOST_MENU_POINTER_SESSION_H
#define HD2_HOST_MENU_POINTER_SESSION_H

#include "host/cursor_geometry.h"
#include "frontend/pointer_update.h"
#include "frontend/pointer_motion.h"
#include "frontend/pointer_dispatch.h"
#include "frontend/common_event_services.h"
#include "frontend/screen_input.h"
#include "frontend/screen_focus.h"
#include <cstdio>
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
    bool inhibit_click = false; // Input +48, set by0066A860.
    bool armed = false;
    bool awaiting_release = false;
    std::uint32_t active_screen = 0;

    void clear() {
        pointer.clear();
        dispatch = {};
        input = {};
        previous_point.reset();
        inhibit_click = false;
        armed = false;
        awaiting_release = false;
        active_screen = 0;
    }
};

// Portable bridge for original pointer sequencing and the explicit legacy
// diagnostic policy. Device snapshots and engine frame movement are services.
template<class Screen>
class MenuFrameInput {
public:
    // Explicit preview constructor retains the previously accepted diagnostic
    // cancellation/rearm policy. The application uses the live-screen overload.
    MenuFrameInput(MenuInputSession& session, hd2::frontend::CommonFrontendState& common,
                   Screen& focus,
                   std::function<std::uint32_t(const CursorPoint&)> repick,
                   std::function<void(float, float)> update_frame,
                   std::function<void()> accepted_click)
        : MenuFrameInput(session, common, [&focus]() -> Screen& { return focus; },
              [repick = std::move(repick)](Screen&, const CursorPoint& point) { return repick(point); },
              std::move(update_frame), std::move(accepted_click), true) {}

    MenuFrameInput(MenuInputSession& session, hd2::frontend::CommonFrontendState& common,
                   std::function<Screen&()> current,
                   std::function<std::uint32_t(Screen&, const CursorPoint&)> repick,
                   std::function<void(float, float)> update_frame,
                   std::function<void()> accepted_click, bool preview_safeguards = false)
        : session_(session), common_(common), current_(std::move(current)), repick_(std::move(repick)),
          update_frame_(std::move(update_frame)), accepted_click_(std::move(accepted_click)),
          preview_safeguards_(preview_safeguards) {}

    // Application input: pick at the PREVIOUS integrated position, then sample
    // and integrate the unmodified relative deltas inside0066A220's gates.
    void tick_relative(float horizontal, float vertical, std::uint32_t buttons,
                       std::uint32_t delta, std::int32_t wheel) {
        if (!session_.armed) throw std::logic_error("Menu cursor has not been initialized");
        pending_ = {buttons, 0, delta, horizontal, vertical, static_cast<float>(wheel)};
        relative_ = true;
        hd2::frontend::dispatch_menu_pointer(session_.dispatch, *this, delta, common_.motion_only);
        relative_ = false;
        session_.previous_point = CursorPoint{session_.pointer.cursor().x(), session_.pointer.cursor().y()};
    }
    void tick(const hd2::host::CursorPoint& point, std::uint32_t target,
              std::uint32_t buttons, std::uint32_t delta, std::int32_t wheel) {
        if (!session_.armed) {
            if (preview_safeguards_ && buttons != 0) return; // Reentry must observe all buttons up first.
            session_.pointer.cursor().initialize(point.x, point.y, 800, 600);
            update_frame_(point.x, point.y);
            session_.previous_point = point;
            session_.armed = true;
        }
        if (preview_safeguards_ && session_.active_screen != current_().identifier()) {
            // Host safety: never carry a press or drag into a different screen.
            // Preserve button history, click timer, cursor and wheel accumulation.
            session_.pointer.cancel_departed_press(0);
            session_.dispatch.dragging = false;
            session_.awaiting_release = buttons != 0;
            session_.active_screen = current_().identifier();
            std::printf("Shared menu input: screen=%s cursor=(%.3f,%.3f) wheel=%.3f idle=%u\n",
                        current_().label(), session_.pointer.cursor().x(), session_.pointer.cursor().y(),
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
        pending_.inhibit_click = session_.inhibit_click;
        if (preview_safeguards_) session_.pointer.cancel_departed_press(target);
        hd2::frontend::dispatch_menu_pointer(session_.dispatch, *this, delta, common_.motion_only);
    }
    std::uint32_t pick_target() const {
        return relative_ ? repick_(current_(), {session_.pointer.cursor().x(), session_.pointer.cursor().y()})
                         : pending_.target;
    }
    bool update_input(std::uint32_t delta, std::uint32_t target) {
        pending_.delta = delta;
        pending_.target = target;
        pending_.inhibit_click = session_.inhibit_click || (preview_safeguards_ && session_.awaiting_release);
        const auto result = session_.pointer.update(pending_);
        if (result.processed) update_frame_(session_.pointer.cursor().x(), session_.pointer.cursor().y());
        const auto& buttons = session_.pointer.buttons();
        session_.input = {buttons.pressed(3), buttons.released(3), buttons.currently_selected(3),
                  result.click.click, !preview_safeguards_ && result.click.double_click,
                  hd2::frontend::pointer_moved(session_.pointer.cursor().horizontal(),
                                             session_.pointer.cursor().vertical()),
                  hd2::frontend::pointer_wheel_moved(session_.pointer.cursor().wheel_delta())};
        return result.processed;
    }
    const hd2::frontend::PointerDispatchInput& input() const { return session_.input; }
    std::optional<std::uint32_t> captured_control_target() const {
        auto owner = common_.capture_owner;
        return owner ? std::optional<std::uint32_t>(owner->identifier()) : std::nullopt;
    }
    int captured_control_event(std::uint32_t target) {
        auto owner = common_.capture_owner;
        if (!owner) throw std::logic_error("Capture disappeared before release");
        return owner->release(target);
    }
    void clear_captured_control() { common_.capture_owner.reset(); }
    int screen_event(hd2::frontend::ScreenPointerCallback callback, std::uint32_t target) {
        auto& focus = current_(); // Every callback resolves the stack afresh.
        using Callback = hd2::frontend::ScreenPointerCallback;
        switch (callback) {
        case Callback::focus: return hd2::frontend::screen_pointer_focus(focus, target);
        case Callback::press: return hd2::frontend::screen_press(focus, target);
        case Callback::release: return hd2::frontend::screen_release(focus, target);
        case Callback::drag: return hd2::frontend::screen_drag(focus, target);
        case Callback::drag_end: return hd2::frontend::screen_drag_end(focus, target);
        case Callback::drag_begin: return hd2::frontend::screen_drag_begin(focus, target);
        case Callback::double_click: return hd2::frontend::screen_double_click(focus, target);
        case Callback::click:
            if (preview_safeguards_ && (target == 0 || focus.control_for_target(target) != focus.focused_control())) {
                return 7;
            }
            std::printf("Engine mouse %s click: target=%08x focus=%d\n",
                        current_().label(), static_cast<unsigned>(target), focus.focused_control());
            return hd2::frontend::screen_click(focus, target);
        }
        throw std::logic_error("Unsupported menu pointer callback");
    }
    void accepted_click() { accepted_click_(); }
    float wheel_delta() const { return session_.pointer.cursor().wheel_delta(); }
    int screen_wheel(std::uint32_t target, std::int32_t delta) {
        auto& focus = current_();
        const int result = hd2::frontend::screen_wheel(focus, target, delta);
        std::printf("Engine mouse wheel: screen=%s target=%08x delta=%d result=%d\n",
                    current_().label(), static_cast<unsigned>(target), static_cast<int>(delta), result);
        return result;
    }
    Screen* current_screen() { return &current_(); }
    std::uint32_t pick_for_screen(Screen* screen) const {
        return repick_(*screen, {session_.pointer.cursor().x(), session_.pointer.cursor().y()});
    }
    void focus_screen(Screen* screen, std::uint32_t target) {
        hd2::frontend::screen_pointer_focus(*screen, target);
    }

private:
    MenuInputSession& session_;
    hd2::frontend::CommonFrontendState& common_;
    std::function<Screen&()> current_;
    std::function<std::uint32_t(Screen&, const CursorPoint&)> repick_;
    std::function<void(float, float)> update_frame_;
    std::function<void()> accepted_click_;
    bool preview_safeguards_;
    bool relative_ = false;
    hd2::frontend::PointerUpdateSample pending_{};
};


} // namespace hd2::host
#endif
