#ifndef HD2_HOST_MAIN_MENU_FOCUS_H
#define HD2_HOST_MAIN_MENU_FOCUS_H

#include "frontend/screen_target.h"
#include "host/menu_button_visuals.h"

namespace hd2::host {

// Bounded screen-one adapter. Original definition order has seven static
// labels, nine buttons, then version: Exit is child sixteen / vector index 15.
// Only Exit is eligible; other original controls remain unsupported/disabled.
class MainMenuFocus {
public:
    static constexpr std::uint32_t exit_target = 0x05000000;
    MainMenuFocus(MenuButtonVisuals& buttons, frontend::ControlEventDestination& events)
        : buttons_(buttons), events_(events) {}
    const char* label() const { return "main-menu"; }
    std::uint32_t identifier() const { return 0x04000000; }
    static std::uint32_t target_for_pick(std::uint32_t picked) {
        return picked == exit_target ? exit_target : 0;
    }
    int control_for_target(std::uint32_t target) const {
        return frontend::find_screen_control(0x04000000, control_ids_, target);
    }
    bool valid_control(int index) const {
        return index >= 0 && static_cast<std::size_t>(index) < control_ids_.size();
    }
    bool control_eligible(int index) const { return index == 15; }
    int focused_control() const { return focused_; }
    void set_focused_control(int index) { focused_ = index; }
    int leave_control(int index, std::uint32_t) {
        require_exit(index);
        buttons_.set_exit_hover(false);
        return 1;
    }
    int enter_control(int index, std::uint32_t) {
        require_exit(index);
        buttons_.set_exit_hover(true);
        return 1;
    }
    int move_control(int index, std::uint32_t) {
        require_exit(index);
        return buttons_.exit_control().pointer_move();
    }
    int click_control(int index, std::uint32_t) {
        require_exit(index);
        return buttons_.click_exit(events_);
    }
    int pointer_control(int index, frontend::ScreenPointerCallback callback, std::uint32_t) {
        require_exit(index);
        return buttons_.exit_control().pointer_event(callback, events_);
    }
    int wheel_control(int index, std::uint32_t, std::int32_t) {
        require_exit(index);
        return buttons_.exit_control().wheel_event(events_);
    }

private:
    static void require_exit(int index) {
        if (index != 15) throw std::logic_error("Unsupported main-menu control");
    }
    static constexpr std::array<std::uint32_t, 17> control_ids_{{
        0x04100000, 0x04200000, 0x04300000, 0x04400000, 0x04500000,
        0x04600000, 0x04700000, 0x04800000, 0x04900000, 0x04a00000,
        0x04b00000, 0x04c00000, 0x04d00000, 0x04e00000, 0x04f00000,
        exit_target, 0x05100000
    }};
    MenuButtonVisuals& buttons_;
    frontend::ControlEventDestination& events_;
    int focused_ = -1;
};

}

#endif
