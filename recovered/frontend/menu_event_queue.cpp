#include "frontend/menu_event_queue.h"

#include <cstdio>
#include <stdexcept>
#include <utility>

namespace hd2::frontend {

void MenuEventActions::profile_back_cleanup() {
    throw std::logic_error("Profile back service is not reconstructed");
}
void MenuEventActions::select_application_state(std::uint32_t) {
    throw std::logic_error("Application state service is not reconstructed");
}
void MenuEventActions::resume_last_save() {
    throw std::logic_error("Resume/save service is not reconstructed");
}
void MenuEventActions::reset_resources() { throw std::logic_error("Resource reset service is not reconstructed"); }
void MenuEventActions::release_resource(std::uint32_t) {
    throw std::logic_error("Frame resource release service is not connected");
}
void MenuEventActions::set_application_input_enabled(bool) {
    throw std::logic_error("Application input mode service is not connected");
}
void MenuEventActions::set_input_motion_only(bool) { throw std::logic_error("Input mode backend is not connected"); }
std::uintptr_t MenuEventActions::screen_model(std::uint32_t) {
    throw std::logic_error("Screen model notification service is not connected");
}
std::uint32_t MenuEventActions::pick_screen_model(std::uintptr_t) {
    throw std::logic_error("Notification picker service is not connected");
}
void MenuEventActions::focus_screen(std::uint32_t, std::uint32_t) {
    throw std::logic_error("Notification focus service is not connected");
}
void MenuEventActions::leave_screen(std::uint32_t, std::uint32_t) {
    throw std::logic_error("Notification leave service is not connected");
}

void MenuEventQueue::dispatch_local(ControlEvent&) {
    throw std::logic_error("Local control event dispatch is not implemented");
}

void MenuEventQueue::enqueue(const ControlEvent& event) {
    pending_.push_back(event);
}

void MenuEventQueue::pump() {
    while (!pending_.empty()) {
        ControlEvent event = std::move(pending_.front());
        pending_.pop_front();
        route(std::move(event));
    }
}

void MenuEventQueue::cancel_confirmation() {
    if (!screens_.current() || screens_.current()->identifier != 0x10000000) {
        throw std::logic_error("Confirmation is not on the screen stack");
    }
    enqueue({0, 0x14001200, 1, 0, 0, {}});
    enqueue({0, 0x06001005, 0, 0, 0, {}});
}

void MenuEventQueue::route(ControlEvent event) {
    std::printf("Menu event: target=%08x code=%08x argument=%08x\n",
                static_cast<unsigned>(event.target), static_cast<unsigned>(event.code),
                static_cast<unsigned>(event.argument));
    if ((event.code & 0x20000000) != 0) {
        deferred_.push_back(std::move(event));
        return;
    }
    if ((event.code & 0x10000000) != 0) {
        if ((event.code & 0x100) != 0) {
            while (!deferred_.empty()) {
                ControlEvent released = std::move(deferred_.front());
                deferred_.pop_front();
                released.code &= 0xdfffffff;
                enqueue(released);
            }
        }
        if ((event.code & 0x200) != 0) deferred_.clear();
        return;
    }
    if (event.target != 0) {
        // 0066ca43: resolve registered screen from the target's high six bits,
        // independently of which screen is currently on top of the stack.
        const auto index = event.target >> 26;
        if (!actions_.screen_available(index)) {
            throw std::logic_error("Targeted menu screen has no runtime binding");
        }
        actions_.dispatch_screen_event(index, event); // Original ignores result.
        return;
    }

    switch (event.code & 0x03000fff) {
    case 0x02000003:
        if (screens_.size() == 1) {
            enqueue({0, 0x02000002, 4, 0, 0, {}});
            enqueue({0, 0x22000004, 0, 0, 0, {}});
        } else if ((event.argument & 0xfc000000) == 0x10000000) {
            cancel_confirmation();
        } else {
            screens_.pop(false);
        }
        break;
    case 0x02000001:
        screens_.push(event.argument, false);
        break;
    case 0x02000002:
        screens_.push(event.argument, true);
        break;
    case 0x02000005:
        screens_.pop(true);
        break;
    case 0x02000096:
        screens_.push_runtime_confirmation();
        break;
    case 0x02000097:
        screens_.dismiss_runtime_confirmation();
        break;
    case 0x02000004:
        actions_.quit();
        break;
    default:
        if (!route_common_service(event)) route_application(event);
    }
}

bool MenuEventQueue::route_common_service(ControlEvent& event) {
    struct Services {
        MenuEventActions& actions;
        MenuEventQueue& queue;
        ScreenStack<MenuEventActions>& stack;
        bool has_current_screen() const { return stack.current() != nullptr; }
        std::uint32_t current_screen() const { return stack.current_index(); }
        std::uintptr_t current_model() { return actions.screen_model(current_screen()); }
        std::uint32_t pick_model(std::uintptr_t model) { return actions.pick_screen_model(model); }
        void focus_screen(std::uint32_t index, std::uint32_t target) { actions.focus_screen(index, target); }
        void leave_screen(std::uint32_t index, std::uint32_t target) { actions.leave_screen(index, target); }
        void clear_screens() { stack.clear(); }
        void select_application_state(std::uint32_t state) { actions.select_application_state(state); }
        void reset_resources() { actions.reset_resources(); }
        void release_resource(std::uint32_t token) { actions.release_resource(token); }
        void set_application_input_enabled(bool enabled) { actions.set_application_input_enabled(enabled); }
        void set_input_motion_only(bool enabled) { actions.set_input_motion_only(enabled); }
        void dispatch_immediate(ControlEvent value) { queue.dispatch_immediate(std::move(value)); }
    } services{actions_, *this, screens_};
    return dispatch_common_frontend_service(common_, services, event);
}

void MenuEventQueue::route_application(ControlEvent& event) {
    struct Services {
        MenuEventActions& actions;
        MenuEventQueue& queue;
        bool dispatch_handler(MenuApplicationHandler handler, ControlEvent& value) {
            return actions.dispatch_menu_handler(handler, value);
        }
        std::optional<std::uint32_t> current_identifier() const {
            const auto* screen = queue.screens().current();
            if (!screen) return std::nullopt;
            return screen->identifier;
        }
        void profile_back_cleanup() { actions.profile_back_cleanup(); }
        void select_application_state(std::uint32_t state) { actions.select_application_state(state); }
        void resume_last_save() { actions.resume_last_save(); }
        void enqueue(const ControlEvent& value) { queue.enqueue(value); }
    } services{actions_, *this};
    if (!dispatch_menu_application_event(services, event)) {
        throw std::logic_error("Unsupported front-end event");
    }
}

}
