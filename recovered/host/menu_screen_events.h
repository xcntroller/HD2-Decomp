#ifndef HD2_HOST_MENU_SCREEN_EVENTS_H
#define HD2_HOST_MENU_SCREEN_EVENTS_H

#include "frontend/defined_screen_events.h"
#include "frontend/screen_focus.h"
#include "host/menu_control.h"

namespace hd2::host {

// Borrowed scene/focus bridge. Screen event/observer/child state is owned by
// DefinedScreenEvents; no original executable callback pointer is called here.
class MenuScreenFrame {
public:
    MenuScreenFrame(const MenuScene& scene, HMODULE module, bool confirmation, MenuFocus& focus)
        : scene_(scene), module_(module), confirmation_(confirmation), focus_(focus) {}
    bool frame_visible() const { return scene_.screen_visible(confirmation_); }
    int focused_control() const { return focus_.focused_control(); }
    int enter_focus(std::uint32_t target) { return frontend::screen_focus_enter(focus_, target); }
    int leave_focus(std::uint32_t target) { return frontend::screen_focus_leave(focus_, target); }
    void set_frame_visible(std::uint8_t visible) { scene_.set_screen_enabled(module_, confirmation_, visible != 0); }
    void update_frame() { scene_.update_screen_frame(module_, confirmation_); }
    void event_emitted(std::uint32_t tag, std::uint32_t argument) {
        std::printf("Original screen lifecycle: screen=%s tag=%08x argument=%08x\n",
                    confirmation_ ? "confirmation" : "main-menu", tag, argument);
    }
private:
    const MenuScene& scene_;
    HMODULE module_;
    bool confirmation_;
    MenuFocus& focus_;
};

using MenuScreenEvents = frontend::DefinedScreenEvents<MenuScreenFrame>;

inline void bind_screen_messages(MenuScreenEvents& screen,
        const frontend::MenuScreenDefinition& definition, MenuScreenControls& controls) {
    for (std::size_t i = 0; i < definition.controls.size(); ++i) {
        auto* control = &controls.message_control_at(i);
        screen.bind_child(i, [control](frontend::ControlEvent& event, frontend::ControlEventDestination& upstream) {
            return control->receive_event(event, upstream);
        });
    }
    // 00627060 runs screen +70 only after +8c has bound every child. Typed
    // definition records can now install root observers/tagged events here.
    screen.initialize(definition.initialization);
}

}

#endif
