#ifndef HD2_FRONTEND_MENU_EVENT_QUEUE_H
#define HD2_FRONTEND_MENU_EVENT_QUEUE_H

#include "frontend/control_events.h"
#include "frontend/screen_stack.h"

#include <deque>

namespace hd2::frontend {

class MenuEventActions {
public:
    virtual ~MenuEventActions() = default;
    virtual void quit() = 0;
    // Host runtime bindings remain limited to the two implemented screens.
    // New screens supply these same lifecycle hooks instead of queue branches.
    virtual bool screen_available(std::uint32_t index) const = 0;
    virtual void activate_screen(const ScreenRegistration& screen, std::uint32_t focus) = 0;
    virtual void deactivate_screen(const ScreenRegistration& screen) = 0;
};

class MenuEventQueue : public ControlEventDestination {
public:
    explicit MenuEventQueue(MenuEventActions& actions) : actions_(actions), screens_(actions) { screens_.seed(1); }

    void dispatch_local(ControlEvent& event) override;
    void enqueue(const ControlEvent& event) override;
    void pump();
    void cancel_confirmation();
    const ScreenStack<MenuEventActions>& screens() const { return screens_; }

private:
    void route(ControlEvent event);

    MenuEventActions& actions_;
    std::deque<ControlEvent> pending_;
    std::deque<ControlEvent> deferred_;
    ScreenStack<MenuEventActions> screens_;
};

}

#endif
