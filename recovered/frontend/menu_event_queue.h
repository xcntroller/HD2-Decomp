#ifndef HD2_FRONTEND_MENU_EVENT_QUEUE_H
#define HD2_FRONTEND_MENU_EVENT_QUEUE_H

#include "frontend/control_events.h"

#include <deque>

namespace hd2::frontend {

class MenuEventActions {
public:
    virtual ~MenuEventActions() = default;
    virtual void show_confirmation() = 0;
    virtual void hide_confirmation() = 0;
    virtual void quit() = 0;
};

class MenuEventQueue : public ControlEventDestination {
public:
    explicit MenuEventQueue(MenuEventActions& actions) : actions_(actions) {}

    void dispatch_local(ControlEvent& event) override;
    void enqueue(const ControlEvent& event) override;
    void pump();
    void cancel_confirmation();

private:
    void route(ControlEvent event);

    MenuEventActions& actions_;
    std::deque<ControlEvent> pending_;
    std::deque<ControlEvent> deferred_;
    unsigned screen_depth_ = 1;
};

}

#endif
