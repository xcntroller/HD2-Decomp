#ifndef HD2_HOST_MENU_BUTTON_VISUALS_H
#define HD2_HOST_MENU_BUTTON_VISUALS_H

#include "frontend/button_focus.h"
#include "frontend/button_input.h"
#include "frontend/control_events.h"
#include "frontend/control_state.h"
#include "frontend/control_initialization.h"
#include "host/menu_scene.h"
#include "host/menu_text_geometry.h"

#include <initializer_list>

namespace hd2::host {

class ButtonVisualPair {
public:
    ButtonVisualPair(const MenuScene& scene, HMODULE module, const char* frame,
                     const char* normal, const char* active, bool confirmation = false)
        : normal_(scene.label_visual(frame, normal, module, confirmation)),
          active_(scene.label_visual(frame, active, module, confirmation)) {
        visibility_ = scene_method<Visibility>(normal_, module, 0x30, 0x1cfe0);
        scene_method<Visibility>(active_, module, 0x30, 0x1cfe0);
        update_ = scene_method<Update>(normal_, module, 0x24, 0x1e610);
        scene_method<Update>(active_, module, 0x24, 0x1e610);
        auto* active_bytes = reinterpret_cast<unsigned char*>(active_);
        const auto* normal_bytes = reinterpret_cast<const unsigned char*>(normal_);
        std::uint32_t flags = 0;
        std::memcpy(&flags, active_bytes + 0xe0, sizeof(flags));
        if ((flags & 0x80) == 0) {
            std::memcpy(active_bytes + 0xb0, normal_bytes + 0xb0, 3 * sizeof(float));
            flags = (flags & 0xfffffedf) | 0x40000000;
            std::memcpy(active_bytes + 0xe0, &flags, sizeof(flags));
        }
        visibility_(normal_, 1);
        visibility_(active_, 0);
        update_(normal_);
        update_(active_);
    }

    // The host owns these portable definition records. Original factory
    // 0065dfbb/+78 precedes +70/0065cf00; button +78 is the no-op 0065cd30.
    void initialize_events(std::initializer_list<frontend::ControlEvent> events) {
        if (events_bound_) throw std::logic_error("Button events already initialized");
        initialization_events_.assign(events);
        InitializationAdapter adapter{*this};
        frontend::initialize_control_events(adapter, initialization_events_);
        events_bound_ = true;
    }

    bool eligible() const { return properties_.eligible(); }
    bool disabled() const { return properties_.disabled != 0; }

    void bind_pick_target(std::uint32_t target) const {
        std::memcpy(reinterpret_cast<unsigned char*>(normal_) + 0xec, &target, sizeof(target));
        std::memcpy(reinterpret_cast<unsigned char*>(active_) + 0xec, &target, sizeof(target));
    }

    bool apply(bool hover) {
        if (!events_bound_) throw std::logic_error("Button events are not bound");
        FocusDestination destination;
        FocusAdapter adapter{*this, destination};
        if (hover) {
            return frontend::button_focus_enter(adapter);
        }
        return frontend::button_focus_leave(adapter);
    }

    int pointer_move() {
        if (!events_bound_) throw std::logic_error("Button events are not bound");
        FocusDestination destination;
        FocusAdapter adapter{*this, destination};
        adapter.emit_event(0x04005000, 0, !pointer_move_logged_);
        pointer_move_logged_ = true;
        return 7;
    }

    int pointer_event(frontend::ScreenPointerCallback callback,
                      frontend::ControlEventDestination& destination) {
        if (!events_bound_) throw std::logic_error("Button events are not bound");
        const auto bit = std::uint32_t{1} << (static_cast<unsigned>(callback) / 4);
        FocusAdapter adapter{*this, destination, (pointer_events_logged_ & bit) == 0};
        pointer_events_logged_ |= bit;
        return frontend::button_pointer_event(adapter, callback);
    }

    int wheel_event(frontend::ControlEventDestination& destination) {
        if (!events_bound_) throw std::logic_error("Button events are not bound");
        constexpr auto bit = std::uint32_t{1} << (0x4c / 4);
        FocusAdapter adapter{*this, destination, (pointer_events_logged_ & bit) == 0};
        pointer_events_logged_ |= bit;
        return frontend::button_wheel(adapter);
    }

    bool activate(frontend::ControlEventDestination& destination) {
        if (!events_bound_) throw std::logic_error("Button events are not bound");
        FocusAdapter adapter{*this, destination};
        return frontend::button_activate(adapter);
    }

    bool click(frontend::ControlEventDestination& destination) {
        if (!events_bound_) throw std::logic_error("Button events are not bound");
        FocusAdapter adapter{*this, destination};
        return frontend::button_click(adapter);
    }

private:
    struct FocusAdapter {
        ButtonVisualPair& pair;
        frontend::ControlEventDestination& destination;
        bool logging = true;
        bool focus_blocked() const { return pair.disabled(); }
        void set_active_visible(bool visible) const { pair.visibility_(pair.active_, visible); }
        void set_normal_visible(bool visible) const { pair.visibility_(pair.normal_, visible); }
        void update_active() const { pair.update_(pair.active_); }
        void update_normal() const { pair.update_(pair.normal_); }
        bool activate_control() const { return frontend::button_activate(*this); }
        void emit_event(std::uint32_t event, std::uint32_t argument, bool log = true) const {
            frontend::dispatch_tagged_control_events(pair.bindings_, &pair, event,
                                                     argument, destination);
            if (log && logging) {
                std::printf("Original button event: %08x argument=%08x\n",
                            static_cast<unsigned>(event), static_cast<unsigned>(argument));
            }
        }
    };

    struct InitializationAdapter {
        ButtonVisualPair& pair;
        bool dispatch_initialization_event(frontend::ControlEvent& event) {
            // Current Exit/Yes/No lists contain only tagged activation bindings.
            // This is the shared tagged branch of button 006551d0 -> text
            // 00661940 -> base 0065cd40. Untagged derived/text initialization
            // still needs the original layout/font/observer contracts.
            frontend::register_tagged_control_event(pair.bindings_, event);
            return true;
        }
    };

    class FocusDestination : public frontend::ControlEventDestination {
    public:
        void dispatch_local(frontend::ControlEvent&) override {
            throw std::logic_error("Local button event dispatch is not implemented");
        }
        void enqueue(const frontend::ControlEvent&) override {
            throw std::logic_error("Button event queue is not implemented");
        }
    };

    using Visibility = void (WINAPI *)(EngineFrame*, std::uint8_t);
    using Update = void (WINAPI *)(EngineFrame*);
    EngineFrame* normal_;
    EngineFrame* active_;
    Visibility visibility_;
    Update update_;
    frontend::ControlProperties properties_;
    frontend::ControlEventBindings bindings_;
    std::vector<frontend::ControlEvent> initialization_events_;
    bool events_bound_ = false;
    bool pointer_move_logged_ = false;
    std::uint32_t pointer_events_logged_ = 0;
};

class MenuButtonVisuals {
public:
    MenuButtonVisuals(const MenuScene& scene, HMODULE module) {
        std::size_t count = 0;
        for (const auto& binding : menu_label_bindings) {
            if (!binding.button) continue;
            ButtonVisualPair pair(scene, module, binding.frame, binding.visual,
                                  binding.active_visual);
            if (binding.id == 2112) {
                pair.initialize_events({{0, 0x06001003, 0x04000000, 0, 0, {}}});
                exit_ = pair;
            }
            ++count;
        }
        if (!exit_) {
            throw std::runtime_error("Missing original Exit visual pair");
        }
        std::printf("Original button visuals ready: %u pairs; Exit hover only\n",
                    static_cast<unsigned>(count));
    }

    void set_exit_hover(bool hover) {
        if (hover == exit_hover_) return;
        exit_->apply(hover);
        exit_hover_ = hover;
        std::printf("Exit visual state: %s\n", hover ? "focused" : "normal");
    }

    ButtonVisualPair& exit_control() { return *exit_; }

    bool click_exit(frontend::ControlEventDestination& destination) {
        return exit_->click(destination);
    }

private:
    std::optional<ButtonVisualPair> exit_;
    bool exit_hover_ = false;
};

}

#endif
