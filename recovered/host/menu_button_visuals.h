#ifndef HD2_HOST_MENU_BUTTON_VISUALS_H
#define HD2_HOST_MENU_BUTTON_VISUALS_H

#include "frontend/button_focus.h"
#include "frontend/button_input.h"
#include "frontend/control_events.h"
#include "frontend/control_state.h"
#include "frontend/control_runtime.h"
#include "frontend/text_control_event.h"
#include "frontend/control_initialization.h"
#include "frontend/defined_controls.h"
#include "host/menu_scene.h"
#include "host/menu_text_geometry.h"
#include "host/menu_text_object.h"

#include <initializer_list>

namespace hd2::host {

class ButtonVisualPair {
    using EventDestination = frontend::EmittingControlDestination<ButtonVisualPair>;
public:
    ButtonVisualPair(const MenuScene& scene, HMODULE module, const char* frame,
                     const char* normal, const char* active, bool confirmation = false,
                     MenuTextObject* text = nullptr, bool available = true)
        : module_(module), frame_name_(frame), text_(text),
          frame_(scene.control_frame(frame, module, confirmation)),
          normal_(scene.label_visual(frame, normal, module, confirmation)),
          active_(scene.label_visual(frame, active, module, confirmation)) {
        if (text_) text_->attach_control(properties_, available);
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
    void initialize_events(const std::vector<frontend::ControlEvent>& events) {
        if (events_bound_) throw std::logic_error("Button events already initialized");
        initialization_events_ = events;
        EventDestination destination{*this, nullptr};
        InitializationAdapter adapter{*this, destination};
        frontend::initialize_control_events(adapter, initialization_events_);
        events_bound_ = true;
        std::printf("Original control initialization: frame=%s records=%u visible=%u disabled=%u\n",
                    frame_name_, static_cast<unsigned>(initialization_events_.size()),
                    static_cast<unsigned>(properties_.visible), static_cast<unsigned>(properties_.disabled));
    }

    bool eligible() const { return properties_.eligible(); }
    bool key(std::uint32_t) const { return false; } // Original button +20 / 0064da30.
    bool disabled() const { return properties_.disabled != 0; }

    bool receive_event(frontend::ControlEvent& event, frontend::ControlEventDestination& upstream) {
        EventDestination destination{*this, &upstream};
        return dispatch_control_event(event, destination);
    }

    void bind_pick_target(std::uint32_t target) const {
        std::memcpy(reinterpret_cast<unsigned char*>(normal_) + 0xec, &target, sizeof(target));
        std::memcpy(reinterpret_cast<unsigned char*>(active_) + 0xec, &target, sizeof(target));
    }

    bool apply(bool hover) {
        if (!events_bound_) throw std::logic_error("Button events are not bound");
        EventDestination destination{*this, nullptr};
        FocusAdapter adapter{*this, destination};
        if (hover) {
            return frontend::button_focus_enter(adapter);
        }
        return frontend::button_focus_leave(adapter);
    }

    int pointer_move() {
        if (!events_bound_) throw std::logic_error("Button events are not bound");
        EventDestination destination{*this, nullptr};
        FocusAdapter adapter{*this, destination};
        adapter.emit_event(0x04005000, 0, !pointer_move_logged_);
        pointer_move_logged_ = true;
        return 7;
    }

    int pointer_event(frontend::ScreenPointerCallback callback,
                      frontend::ControlEventDestination& upstream) {
        EventDestination destination{*this, &upstream};
        if (!events_bound_) throw std::logic_error("Button events are not bound");
        const auto bit = std::uint32_t{1} << (static_cast<unsigned>(callback) / 4);
        FocusAdapter adapter{*this, destination, (pointer_events_logged_ & bit) == 0};
        pointer_events_logged_ |= bit;
        return frontend::button_pointer_event(adapter, callback);
    }

    int wheel_event(frontend::ControlEventDestination& upstream) {
        EventDestination destination{*this, &upstream};
        if (!events_bound_) throw std::logic_error("Button events are not bound");
        constexpr auto bit = std::uint32_t{1} << (0x4c / 4);
        FocusAdapter adapter{*this, destination, (pointer_events_logged_ & bit) == 0};
        pointer_events_logged_ |= bit;
        return frontend::button_wheel(adapter);
    }

    bool activate(frontend::ControlEventDestination& upstream) {
        EventDestination destination{*this, &upstream};
        if (!events_bound_) throw std::logic_error("Button events are not bound");
        FocusAdapter adapter{*this, destination};
        return frontend::button_activate(adapter);
    }

    bool click(frontend::ControlEventDestination& upstream) {
        EventDestination destination{*this, &upstream};
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

    // Attached text is owned by MenuText and outlives all button controls.
    bool has_text() const { return text_ != nullptr; }
    void set_text_disabled(std::uint8_t value) { text_->object().disabled = value; }
    void set_frame_visible(std::uint8_t value) {
        scene_method<Visibility>(frame_, module_, 0x30, 0x1cfe0)(frame_, value);
    }
    void update_frame() { scene_method<Update>(frame_, module_, 0x24, 0x1e610)(frame_); }
    std::size_t child_count() const { return 0; } // No reconstructed UI child objects on these buttons.
    void dispatch_child(std::size_t, frontend::ControlEvent&) {
        throw std::logic_error("Button UI child dispatch is not reconstructed");
    }

    struct ButtonMessages {
        ButtonVisualPair& pair;
        frontend::ControlStateRuntime<ButtonVisualPair>& state;
        bool has_text() const { return pair.has_text(); }
        void layout_text_on_normal() { pair.text_->layout_text_on_normal(); }
        void set_text_word_94(std::uint32_t value) { pair.text_->object().word_94 = value; }
        bool dispatch_text_event(frontend::ControlEvent& event) {
            if (!pair.text_) {
                if ((event.code & 0x040ff000u) == 0 &&
                    (event.code & 0x03000fffu) == 0x01000015) {
                    throw std::logic_error("Caption initialization requires attached text");
                }
                return state.dispatch(event);
            }
            frontend::TextControlRuntime<MenuTextObject, decltype(state)> text{*pair.text_, state};
            return text.dispatch(event);
        }
    };

    bool dispatch_control_event(frontend::ControlEvent& event, frontend::ControlEventDestination& destination) {
        frontend::ControlStateRuntime<ButtonVisualPair> state{*this, properties_, bindings_, destination};
        ButtonMessages messages{*this, state};
        return frontend::dispatch_button_control_event(messages, event);
    }

    struct InitializationAdapter {
        ButtonVisualPair& pair;
        frontend::ControlEventDestination& destination;
        bool dispatch_initialization_event(frontend::ControlEvent& event) {
            return pair.dispatch_control_event(event, destination);
        }
    };

    friend class frontend::ControlStateRuntime<ButtonVisualPair>;
    friend class frontend::EmittingControlDestination<ButtonVisualPair>;

    using Visibility = void (WINAPI *)(EngineFrame*, std::uint8_t);
    using Update = void (WINAPI *)(EngineFrame*);
    HMODULE module_;
    const char* frame_name_;
    MenuTextObject* text_;
    EngineFrame* frame_;
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

}

#endif
