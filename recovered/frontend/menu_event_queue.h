#ifndef HD2_FRONTEND_MENU_EVENT_QUEUE_H
#define HD2_FRONTEND_MENU_EVENT_QUEUE_H

#include "frontend/control_events.h"
#include "frontend/screen_stack.h"
#include "frontend/application_events.h"

#include <deque>
#include <utility>

namespace hd2::frontend {

class MenuEventActions {
public:
    virtual ~MenuEventActions() = default;
    virtual void quit() = 0;
    // Host runtime bindings remain limited to implemented screens.
    // New screens supply these same lifecycle hooks instead of queue branches.
    virtual bool screen_available(std::uint32_t index) const = 0;
    virtual void activate_screen(const ScreenRegistration& screen, std::uint32_t focus) = 0;
    virtual void deactivate_screen(const ScreenRegistration& screen) = 0;
    virtual bool dispatch_screen_event(std::uint32_t index, ControlEvent& event) = 0;
    virtual bool dispatch_menu_handler(MenuApplicationHandler, ControlEvent&) { return false; }
    virtual void profile_back_cleanup();
    virtual void select_application_state(std::uint32_t state);
    virtual void resume_last_save();
};

class MenuEventQueue : public ControlEventDestination {
public:
    explicit MenuEventQueue(MenuEventActions& actions, bool seed_main = true)
        : actions_(actions), screens_(actions) { if (seed_main) screens_.seed(1); }

    void dispatch_local(ControlEvent& event) override;
    void enqueue(const ControlEvent& event) override;
    void pump();
    // Factory's final 02000001 is direct; it must precede later queued work.
    void dispatch_immediate(ControlEvent event) { route(std::move(event)); }
    void cancel_confirmation();
    const ScreenStack<MenuEventActions>& screens() const { return screens_; }

private:
    void route(ControlEvent event);
    void route_application(ControlEvent& event);

    MenuEventActions& actions_;
    std::deque<ControlEvent> pending_;
    std::deque<ControlEvent> deferred_;
    ScreenStack<MenuEventActions> screens_;
};

}

#endif
