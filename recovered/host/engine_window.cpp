#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "host/graphics_settings.h"
#include "host/menu_scene.h"
#include "host/menu_group_probe.h"
#include "host/menu_cursor.h"
#include "host/menu_text.h"
#include "host/menu_button_visuals.h"
#include "host/menu_screen_events.h"
#include "host/menu_screen_product.h"
#include "host/menu_screen_factory.h"
#include "frontend/runtime_confirmation.h"
#include "frontend/menu_event_queue.h"
#include "frontend/screen_lifecycle.h"
#include "frontend/pointer_click.h"
#include "host/pointer_dispatch_probe.h"
#include "host/engine_mouse.h"
#include "host/engine_sound.h"
#include "frontend/pointer_motion.h"
#include "frontend/screen_focus.h"
#include "frontend/screen_input.h"
#include "frontend/screen_key.h"
#include "frontend/frontend_key.h"
#include "frontend/screen_target.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <exception>
#include <functional>
#include <io.h>

namespace {

using MenuRole = hd2::frontend::MenuControlRole;

struct Graphics;
using Method = void (WINAPI *)(Graphics*);
using Settings = hd2::host::GraphicsSettings32;

struct GraphicsVtable {
    std::int32_t (WINAPI *initialize)(Graphics*, const Settings*);
    const Settings* (WINAPI *settings)(Graphics*);
    std::int32_t (WINAPI *set_flags)(Graphics*, std::uint32_t);
    std::uint8_t (WINAPI *initialized)(Graphics*);
    Method close;
    Method pump;
    std::int32_t (WINAPI *width)(Graphics*);
    std::int32_t (WINAPI *height)(Graphics*);
    Method reserved_20[7];
    std::int32_t (WINAPI *load_texture)(Graphics*, const char*, const char*, std::uint32_t,
                                       hd2::host::EngineFrame**);
    Method reserved_40;
    std::uint8_t (WINAPI *bind_texture)(Graphics*, hd2::host::EngineFrame*, std::uint32_t);
    HWND (WINAPI *window)(Graphics*);
    HWND (WINAPI *child_window)(Graphics*);
    Method reserved_50[5];
    void (WINAPI *clear)(Graphics*, std::uint32_t, float, std::uint32_t);
    Method begin_scene;
    Method end_scene;
    Method present;
    Method reserved_74[2];
    std::int32_t (WINAPI *set_viewport)(Graphics*, const hd2::host::CursorViewport*);
    const hd2::host::CursorViewport* (WINAPI *viewport)(Graphics*);
    Method reserved_84;
    std::int32_t (WINAPI *draw_primitive)(Graphics*, std::uint32_t, std::uint32_t,
                                         const hd2::host::CursorVertex*, std::uint32_t);
};

struct Graphics {
    const GraphicsVtable* methods;
};

static_assert(sizeof(void*) == 4, "The original engine requires an i386 executable");
static_assert(offsetof(GraphicsVtable, close) == 0x10);
static_assert(offsetof(GraphicsVtable, window) == 0x48);
static_assert(offsetof(GraphicsVtable, clear) == 0x64);
static_assert(offsetof(GraphicsVtable, present) == 0x70);
static_assert(offsetof(GraphicsVtable, load_texture) == 0x3c);
static_assert(offsetof(GraphicsVtable, bind_texture) == 0x44);
static_assert(offsetof(GraphicsVtable, set_viewport) == 0x7c);
static_assert(offsetof(GraphicsVtable, viewport) == 0x80);
static_assert(offsetof(GraphicsVtable, draw_primitive) == 0x88);

bool running = true;
bool custom_cursor_active = false;
WNDPROC previous_window_proc = nullptr;

bool mouse_button_state_message(UINT message) {
    switch (message) {
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN: case WM_XBUTTONUP: case WM_XBUTTONDBLCLK:
        return true;
    default:
        return false;
    }
}

std::uint32_t mouse_buttons_from_message(WPARAM keys) {
    std::uint32_t buttons = 0;
    if (keys & MK_LBUTTON) buttons |= 0x01;
    if (keys & MK_RBUTTON) buttons |= 0x02;
    if (keys & MK_MBUTTON) buttons |= 0x04;
    if (keys & MK_XBUTTON1) buttons |= 0x08;
    if (keys & MK_XBUTTON2) buttons |= 0x10;
    return buttons;
}

class HostMenuEventActions : public hd2::frontend::MenuEventActions {
public:
    HostMenuEventActions(bool& confirming, bool& runtime_active)
        : confirming_(confirming), runtime_active_(runtime_active) {}

    void bind_screen(std::uint32_t index, hd2::host::MenuScreenEvents& screen) {
        if (index >= screens_.size() || screens_[index])
            throw std::logic_error("Invalid or duplicate registered screen binding");
        if ((screen.identifier() >> 26) != index)
            throw std::logic_error("Registered screen family mismatch");
        screens_[index] = &screen;
    }
    bool screen_available(std::uint32_t index) const override {
        return index < screens_.size() && screens_[index] != nullptr;
    }
    void activate_screen(const hd2::frontend::ScreenRegistration& screen, std::uint32_t focus) override {
        runtime(screen.identifier >> 26).activate(focus);
        confirming_ = screen.identifier == 0x10000000;
        runtime_active_ = screen.identifier == 0;
        if (runtime_active_) confirming_ = true;
        if (confirming_) std::puts("Original event flow: confirmation opened");
    }
    void deactivate_screen(const hd2::frontend::ScreenRegistration& screen) override {
        runtime(screen.identifier >> 26).deactivate();
        if (screen.identifier == 0x10000000) {
            confirming_ = false;
            std::puts("Original event flow: confirmation dismissed");
        }
        if (screen.identifier == 0) {
            runtime_active_ = false;
            confirming_ = false;
            std::puts("Original event flow: runtime confirmation dismissed");
        }
    }
    bool dispatch_screen_event(std::uint32_t index, hd2::frontend::ControlEvent& event) override {
        return runtime(index).dispatch(event);
    }
    void quit() override {
        std::puts("Original event flow: closing reconstruction host");
        running = false;
    }
private:
    hd2::host::MenuScreenEvents& runtime(std::uint32_t index) const {
        auto* screen = index < screens_.size() ? screens_[index] : nullptr;
        if (!screen) throw std::logic_error("Screen event runtime is not bound");
        return *screen;
    }
    bool& confirming_;
    bool& runtime_active_;
    std::array<hd2::host::MenuScreenEvents*, 64> screens_{};
};

struct HostRuntimeConfirmationServices {
    hd2::host::MenuFocus* focus = nullptr;
    bool screen_key(std::uint32_t word) {
        if (!focus) throw std::logic_error("Runtime confirmation focus is unbound");
        return hd2::frontend::screen_key(*focus, word);
    }
    std::uint32_t sender_identifier(void* sender) const {
        return static_cast<hd2::host::ButtonVisualPair*>(sender)->identifier();
    }
};

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

void windows_error(const char* operation) {
    std::fprintf(stderr, "%s failed: Win32 error %lu\n", operation, GetLastError());
}

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_CLOSE) {
        std::puts("WM_CLOSE received");
        running = false;
        return 0;
    }
    if (message == WM_SETCURSOR && custom_cursor_active && LOWORD(lparam) == HTCLIENT) {
        SetCursor(nullptr);
        return TRUE;
    }
    return CallWindowProcW(previous_window_proc, window, message, wparam, lparam);
}

template<class Function>
bool expected_method(Function function, HMODULE module, std::uintptr_t rva) {
    return reinterpret_cast<std::uintptr_t>(function) ==
           reinterpret_cast<std::uintptr_t>(module) + rva;
}

bool verify_methods(Graphics* graphics, HMODULE module) {
    const auto* methods = graphics->methods;
    return expected_method(methods->initialize, module, 0x6c2d0) &&
           expected_method(methods->initialized, module, 0x6da20) &&
           expected_method(methods->close, module, 0x6da40) &&
           expected_method(methods->window, module, 0x6a240) &&
           expected_method(methods->clear, module, 0x6a6c0) &&
           expected_method(methods->begin_scene, module, 0x6b380) &&
           expected_method(methods->end_scene, module, 0x6b3b0) &&
           expected_method(methods->present, module, 0x6b3e0);
}

bool verify_cursor_methods(Graphics* graphics, HMODULE module) {
    const auto* methods = graphics->methods;
    return expected_method(methods->load_texture, module, 0x69f70) &&
           expected_method(methods->bind_texture, module, 0x6a1f0) &&
           expected_method(methods->child_window, module, 0x6a250) &&
           expected_method(methods->width, module, 0x6de50) &&
           expected_method(methods->height, module, 0x6de70) &&
           expected_method(methods->set_viewport, module, 0x6c270) &&
           expected_method(methods->draw_primitive, module, 0x6b5a0);
}

int finish(Graphics* graphics, int result) {
    graphics->methods->close(graphics);
    if (graphics->methods->initialized(graphics) != 0) {
        std::fputs("Engine cleanup did not clear the initialized flag\n", stderr);
        return 1;
    }
    std::puts("Graphics closed");
    return result;
}

int run_window(Graphics* graphics, HMODULE module, std::uint32_t frame_limit, bool menu_model,
               bool menu_ui, bool confirmation_preview, bool confirmation_enabled,
               bool confirmation_pick_probe, bool confirmation_pick_active, bool use_engine_mouse,
               bool group_probe, bool runtime_probe) {
    const auto settings = hd2::host::windowed_settings(
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))));
    const auto* methods = graphics->methods;
    const auto status = methods->initialize(graphics, &settings);
    if (status < 0) {
        std::fprintf(stderr, "IGraph initialization failed: 0x%08x\n",
                     static_cast<unsigned>(status));
        return finish(graphics, 1);
    }
    HWND window = methods->window(graphics);
    if (methods->initialized(graphics) == 0 || window == nullptr || !IsWindow(window)) {
        std::fputs("Engine initialization returned without a valid initialized window\n", stderr);
        return finish(graphics, 1);
    }
    if (!SetWindowTextW(window, confirmation_preview ?
            L"HD2 reconstruction - confirmation artwork diagnostic" : menu_ui ?
            L"HD2 reconstruction - static menu text / Exit only" : menu_model ?
            L"HD2 reconstruction - original menu model (not interactive)" :
            L"HD2 reconstruction - engine bootstrap (not menu)")) {
        windows_error("SetWindowTextW");
        return finish(graphics, 1);
    }
    SetLastError(0);
    const auto previous = SetWindowLongPtrW(window, GWLP_WNDPROC,
                                            reinterpret_cast<LONG_PTR>(&window_proc));
    if (previous == 0) {
        windows_error("SetWindowLongPtrW");
        return finish(graphics, 1);
    }
    previous_window_proc = reinterpret_cast<WNDPROC>(previous);
    std::puts("Graphics initialized; original LS3DF.dll owns the window and D3D8 device");
    std::fflush(stdout);
    std::uint32_t frames = 0;
    int result = 0;
    try {
        // Declaration order keeps audio alive through all scene/frame teardown.
        std::unique_ptr<hd2::host::EngineSound> sound;
        if (use_engine_mouse) sound = std::make_unique<hd2::host::EngineSound>(module, window);
        std::unique_ptr<hd2::host::MenuScene> menu;
        if (menu_model) {
            menu = std::make_unique<hd2::host::MenuScene>(
                module, confirmation_preview, confirmation_enabled, use_engine_mouse, runtime_probe);
            if (group_probe) hd2::host::probe_menu_group_frames(*menu, module);
            if (sound && sound->initialized()) menu->load_click_sound(module);
        }
        std::unique_ptr<hd2::host::MenuCursor<Graphics>> cursor;
        if (menu_model) {
            if (!verify_cursor_methods(graphics, module)) {
                throw std::runtime_error("Unsupported original-cursor graphics ABI");
            }
            cursor = std::make_unique<hd2::host::MenuCursor<Graphics>>(graphics, module);
        }
        bool cursor_drawn = false;
        bool text_drawn = false;
        bool modal_drawn = false;
        std::optional<hd2::frontend::MenuScreenDefinition> runtime_definition;
        std::unique_ptr<hd2::host::MenuText<Graphics>> text;
        std::unique_ptr<hd2::host::MenuScreenControls> preview_buttons;
        hd2::host::MenuScreenControls* buttons = nullptr;
        hd2::host::ExitButton exit_button;
        hd2::frontend::PointerClick confirmation_exit_click;
        hd2::frontend::PointerClick confirmation_click;
        hd2::host::PointerDispatchProbe confirmation_probe_input;
        std::optional<hd2::host::CursorPoint> confirmation_probe_point;
        std::optional<DWORD> confirmation_probe_time;
        hd2::host::MenuScreenControls* confirmation_controls = nullptr;
        bool confirming = false;
        bool runtime_active = false;
        int previous_confirm_hover = 0;
        std::optional<hd2::host::CursorPoint> previous_confirmation_point;
        MenuInputSession input_session;
        hd2::host::MenuFocus* confirmation_focus = nullptr;
        std::optional<MenuFrameInput<hd2::host::MenuFocus>> frame_input;
        hd2::host::MenuFocus* main_focus = nullptr;
        std::optional<MenuFrameInput<hd2::host::MenuFocus>> main_frame_input;
        std::unique_ptr<hd2::host::EngineMouse> engine_mouse;
        std::optional<std::uint32_t> previous_mouse_status;
        std::optional<std::uint32_t> previous_main_pick;
        std::optional<std::array<std::uint32_t, 2>> previous_pick_probe;
        std::optional<HostMenuEventActions> menu_actions;
        std::optional<hd2::frontend::MenuEventQueue> menu_events;
        HostRuntimeConfirmationServices runtime_services;
        std::unique_ptr<hd2::frontend::RuntimeConfirmation<HostRuntimeConfirmationServices>> runtime_confirmation;
        hd2::frontend::OwnedScreens<hd2::host::MenuScreenProduct> named_products;
        std::unique_ptr<hd2::host::MenuScreenProduct> runtime_product;
        std::optional<MenuKeyboardInput> keyboard_input;
        const auto confirmation_hit_for_target = [&](std::uint32_t target) {
            if (!confirmation_focus) throw std::logic_error("Confirmation screen is not constructed");
            return std::max(0, confirmation_focus->interactive_control_for_target(target));
        };
        const auto active_confirmation_focus = [&]() -> hd2::host::MenuFocus& {
            return runtime_active ? runtime_product->focus() : *confirmation_focus;
        };
        const auto active_confirmation_controls = [&]() -> hd2::host::MenuScreenControls& {
            return runtime_active ? runtime_product->controls() : *confirmation_controls;
        };
        if (menu_ui) {
            if (!expected_method(methods->viewport, module, 0x6c2b0)) {
                throw std::runtime_error("Unsupported original viewport getter ABI");
            }
            if (confirmation_enabled) menu->initialize_confirmation_text_frame(module);
            if (runtime_probe) menu->initialize_confirmation_text_frame(module, menu->runtime_confirmation_frame());
            text = std::make_unique<hd2::host::MenuText<Graphics>>(
                graphics, module, *menu, confirmation_enabled, runtime_probe);
            if (confirmation_enabled) {
                menu_actions.emplace(confirming, runtime_active);
                menu_events.emplace(*menu_actions, false);
                if (runtime_probe) {
                    runtime_confirmation = std::make_unique<hd2::frontend::RuntimeConfirmation<HostRuntimeConfirmationServices>>(
                        runtime_services, *menu_events);
                    runtime_definition.emplace(runtime_confirmation->definition());
                    text->attach_runtime_definition(*menu, module, *runtime_definition);
                }
                if (runtime_probe) {
                    hd2::host::RuntimeMenuScreenFactory<HostMenuEventActions> runtime_factory(
                        *menu, module, *runtime_definition,
                        [&](std::uint32_t id) { return text->object_for(id); },
                        *menu_events, runtime_product, *menu_actions);
                    if (!hd2::frontend::construct_runtime_confirmation(runtime_factory))
                        throw std::runtime_error("Runtime confirmation construction failed");
                    runtime_services.focus = &runtime_product->focus();
                }
                hd2::host::MenuScreenFactory<HostMenuEventActions> factory(*menu, module,
                    [&](std::uint32_t id) { return text->object_for(id); },
                    *menu_events, named_products, *menu_actions);
                hd2::frontend::construct_registered_screens(factory);
                buttons = &named_products.screen(0).controls();
                confirmation_controls = &named_products.screen(1).controls();
                main_focus = &named_products.screen(0).focus();
                confirmation_focus = &named_products.screen(1).focus();
                if (confirmation_pick_probe || confirmation_pick_active) {
                    confirmation_controls->bind_pick_targets();
                }
                if (confirmation_pick_probe) {
                    std::puts("Confirmation engine-picking comparison enabled");
                    std::puts("Confirmation input transition probe enabled (Win32 message snapshots)");
                }
                if (confirmation_pick_active) {
                    std::puts("Confirmation engine picking controls hover and activation");
                }
                std::puts("Registered screen event trees ready: 17 main / 4 confirmation children");
                keyboard_input.emplace(input_session, *main_focus, *confirmation_focus,
                                       *menu_events, confirming);
                if (use_engine_mouse) {
                    buttons->bind_pick_targets();
                    main_frame_input.emplace(input_session, *main_focus, [&](const hd2::host::CursorPoint& point) {
                        return main_focus->interactive_target(menu->pick_main_menu(
                            module, point.x, point.y, static_cast<float>(methods->width(graphics)),
                            static_cast<float>(methods->height(graphics))));
                    }, [&](float x, float y) {
                        menu->update_input_cursor(x, y, static_cast<float>(methods->width(graphics)),
                                                  static_cast<float>(methods->height(graphics)));
                    }, [&] { menu->request_click_sound(); });
                    std::puts("Rebuilt engine input controls main-menu Exit and confirmation");
                    frame_input.emplace(input_session, *confirmation_focus, [&](const hd2::host::CursorPoint& point) {
                        const auto picked = menu->pick_confirmation(
                            module, point.x, point.y,
                            static_cast<float>(methods->width(graphics)),
                            static_cast<float>(methods->height(graphics)));
                        return confirmation_focus->target_for_control(
                            confirmation_hit_for_target(picked));
                    }, [&](float x, float y) {
                        menu->update_input_cursor(x, y, static_cast<float>(methods->width(graphics)),
                                                  static_cast<float>(methods->height(graphics)));
                    }, [&] { menu->request_click_sound(); });
                    engine_mouse = std::make_unique<hd2::host::EngineMouse>(
                        reinterpret_cast<hd2::host::EngineFrame*>(graphics), module);
                }
            } else {
                preview_buttons = hd2::host::make_menu_controls(
                    *menu, module, hd2::frontend::main_menu_definition(), false,
                    std::initializer_list<MenuRole>{MenuRole::exit},
                    [&](std::uint32_t id) { return text->object_for(id); });
                buttons = preview_buttons.get();
            }
        }
        if (confirmation_preview) {
            const auto& definition = hd2::frontend::confirmation_definition();
            std::vector<std::unique_ptr<hd2::host::ButtonVisualPair>> preview_controls;
            for (const auto& entry : definition.controls) {
                if (entry.kind == hd2::frontend::MenuControlKind::button) {
                    preview_controls.push_back(std::make_unique<hd2::host::ButtonVisualPair>(
                        *menu, module, entry.frame, entry.normal, entry.active));
                }
            }
            for (const auto& entry : definition.controls) {
                if (entry.role == MenuRole::ok) continue;
                const auto* visual = entry.kind == hd2::frontend::MenuControlKind::text
                    ? entry.frame : entry.normal;
                const auto bounds = menu->label_bounds(entry.frame, visual, module);
                const auto rectangle = hd2::host::menu_rectangle(bounds, 800, 600);
                std::printf("Confirmation diagnostic frame=%s bounds=(%.1f,%.1f)-(%.1f,%.1f)\n",
                            entry.frame, rectangle.left, rectangle.top,
                            rectangle.right, rectangle.bottom);
            }
            std::puts("Confirmation visual pairs initialized; diagnostic remains non-interactive");
        }
        const auto pick_confirmation = [&](const std::optional<hd2::host::CursorPoint>& point) {
            return point
                ? menu->pick_confirmation(module, point->x, point->y,
                      static_cast<float>(methods->width(graphics)),
                      static_cast<float>(methods->height(graphics)))
                : std::uint32_t{0};
        };
        const auto confirmation_hit = [&](const std::optional<hd2::host::CursorPoint>& point) {
            if (runtime_active) return text->confirmation_hit(*runtime_definition, point);
            return confirmation_pick_active
                ? confirmation_hit_for_target(pick_confirmation(point))
                : text->confirmation_hit(point);
        };
        while (running) {
            MSG message{};
            while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
                if (message.message == WM_QUIT) {
                    running = false;
                    break;
                }
                if (confirmation_pick_probe) {
                    if (!confirming ||
                        (message.message == WM_ACTIVATEAPP && message.wParam == 0) ||
                        message.message == WM_CANCELMODE) {
                        confirmation_probe_input.clear();
                        confirmation_probe_point.reset();
                        confirmation_probe_time.reset();
                    } else if (mouse_button_state_message(message.message)) {
                        const auto delta = confirmation_probe_time
                            ? message.time - *confirmation_probe_time : DWORD{0};
                        confirmation_probe_time = message.time;
                        const auto point = cursor->position(window, &message.pt);
                        const auto target = pick_confirmation(point);
                        if (point && !confirmation_probe_point) {
                            confirmation_probe_input.initialize_cursor(
                                point->x, point->y, static_cast<float>(methods->width(graphics)),
                                static_cast<float>(methods->height(graphics)));
                        }
                        // Observational absolute-position differences approximate relative
                        // input. Original engine polling and clamping are still absent.
                        const float dx = point && confirmation_probe_point
                            ? point->x - confirmation_probe_point->x : 0.0f;
                        const float dy = point && confirmation_probe_point
                            ? point->y - confirmation_probe_point->y : 0.0f;
                        confirmation_probe_point = point;
                        const auto update = confirmation_probe_input.observe({
                            mouse_buttons_from_message(message.wParam), target, delta, dx, dy});
                        const auto& confirmation_probe_buttons = confirmation_probe_input.buttons();
                        if (confirmation_probe_buttons.previous() != confirmation_probe_buttons.current()) {
                            const bool pressed = confirmation_probe_buttons.pressed(0);
                            const bool released = confirmation_probe_buttons.released(0);
                            const auto captured = update.captured;
                            const auto result = update.click;
                            std::printf(
                                "Confirmation input probe: buttons=%02x->%02x any=%s "
                                "mask3=%s target=%08x captured=%08x click=%u double=%u "
                                "delta_ms=%lu processed=%u\n",
                                static_cast<unsigned>(confirmation_probe_buttons.previous()),
                                static_cast<unsigned>(confirmation_probe_buttons.current()),
                                pressed ? "press" : released ? "release" : "unchanged",
                                confirmation_probe_buttons.pressed(3) ? "press" :
                                    confirmation_probe_buttons.released(3) ? "release" : "unchanged",
                                static_cast<unsigned>(target), static_cast<unsigned>(captured),
                                static_cast<unsigned>(result.click),
                                static_cast<unsigned>(result.double_click),
                                static_cast<unsigned long>(delta),
                                static_cast<unsigned>(update.processed));
                        }
                    }
                }
                if (runtime_probe && !confirming && message.message == WM_KEYDOWN &&
                    message.wParam == VK_F9 &&
                    (static_cast<std::uintptr_t>(message.lParam) & (1u << 30)) == 0 &&
                    GetForegroundWindow() == window) {
                    previous_confirm_hover = 0;
                    confirmation_click.clear();
                    runtime_confirmation->open_localized(1, 2004, 0x02000004, 0);
                    menu_events->pump();
                    std::puts("Runtime confirmation diagnostic opened by F9");
                    continue;
                }
                if (runtime_active && message.message == WM_KEYDOWN &&
                    (message.wParam == VK_RETURN || message.wParam == VK_SPACE ||
                     message.wParam == VK_ESCAPE)) {
                    if ((static_cast<std::uintptr_t>(message.lParam) & (1u << 30)) == 0 &&
                        GetForegroundWindow() == window) {
                        const auto scan = message.wParam == VK_ESCAPE ? 0x01u :
                            message.wParam == VK_SPACE ? 0x39u :
                            ((static_cast<std::uintptr_t>(message.lParam) & (1u << 24)) != 0
                                ? 0x9cu : 0x1cu);
                        const bool handled = runtime_confirmation->key(scan);
                        std::printf("Menu keyboard: screen=runtime-confirmation scan=%02x handled=%u\n",
                                    scan, static_cast<unsigned>(handled));
                        menu_events->pump();
                        if (!running) break;
                    }
                    continue;
                }
                if (keyboard_input && message.message == WM_KEYDOWN &&
                    (message.wParam == VK_RETURN || message.wParam == VK_SPACE ||
                     message.wParam == VK_ESCAPE)) {
                    // Host transport only: map Win32 make/extended bits to the
                    // original scan-code byte. Ignore auto-repeat to avoid a
                    // held key activating a second screen after a push/pop.
                    if ((static_cast<std::uintptr_t>(message.lParam) & (1u << 30)) == 0 &&
                        GetForegroundWindow() == window) {
                        const auto scan = message.wParam == VK_ESCAPE ? 0x01u :
                            message.wParam == VK_SPACE ? 0x39u :
                            ((static_cast<std::uintptr_t>(message.lParam) & (1u << 24)) != 0
                                ? 0x9cu : 0x1cu);
                        const bool handled = hd2::frontend::dispatch_frontend_key(*keyboard_input, scan);
                        std::printf("Menu keyboard: screen=%s scan=%02x handled=%u\n",
                                    confirming ? "confirmation" : "main-menu", scan,
                                    static_cast<unsigned>(handled));
                        menu_events->pump();
                        if (!running) break;
                    }
                    continue;
                }
                if (message.message == WM_KEYDOWN && message.wParam == VK_ESCAPE) {
                    if (confirming) {
                        confirmation_click.clear();
                        menu_events->cancel_confirmation();
                        menu_events->pump();
                        std::puts("Confirmation cancelled by Escape");
                        continue;
                    }
                    running = false;
                    break;
                }
                if (text && !(use_engine_mouse && mouse_button_state_message(message.message))) {
                    const bool inside = text->over_exit(cursor->position(window, &message.pt));
                    if (confirming && message.message == WM_LBUTTONDOWN) {
                        confirmation_click.press(confirmation_hit(cursor->position(window, &message.pt)));
                    } else if (confirming && message.message == WM_LBUTTONUP) {
                        const int hit = confirmation_hit(cursor->position(window, &message.pt));
                        const bool matched = confirmation_click.release(hit);
                        if (hit != 0 && matched) {
                            int result;
                            auto& active_focus = active_confirmation_focus();
                            if (active_focus.focused_control() == hit) {
                                std::printf("Confirmation click: routed through focused control %d\n",
                                            hit);
                                result = hd2::frontend::screen_click(
                                    active_focus, active_focus.target_for_control(hit));
                            } else {
                                std::printf("Confirmation click: sampled focus=%d target=%d; "
                                            "using host target selection\n",
                                            active_focus.focused_control(), hit);
                                auto& button = active_confirmation_controls().control_at(hit);
                                result = button.click(*menu_events);
                            }
                            if (result != 1) {
                                throw std::logic_error("Confirmation activation was blocked");
                            }
                            menu_events->pump();
                            if (!running) break;
                        }
                    } else if (!confirming && message.message == WM_LBUTTONDOWN) {
                        if (confirmation_enabled) confirmation_exit_click.press(inside ? 1 : 0);
                        else exit_button.press(inside);
                    } else if (!confirming && message.message == WM_LBUTTONUP) {
                        const bool activated = confirmation_enabled
                            ? (confirmation_exit_click.release(inside ? 1 : 0) && inside)
                            : exit_button.release(inside);
                        if (activated) {
                            if (confirmation_enabled) {
                                if (!buttons->control_for_role(MenuRole::exit).click(*menu_events)) {
                                    throw std::logic_error("Exit activation was blocked");
                                }
                                menu_events->pump();
                            } else {
                                std::puts("Exit control activated: closing reconstruction host");
                                running = false;
                                break;
                            }
                        }
                    } else if ((message.message == WM_ACTIVATEAPP && message.wParam == 0) ||
                                message.message == WM_CANCELMODE) {
                        exit_button.cancel();
                        confirmation_exit_click.clear();
                        confirmation_click.clear();
                        input_session.clear();
                    }
                }
                TranslateMessage(&message);
                DispatchMessageW(&message);
            }
            if (!running) {
                break;
            }
            std::optional<hd2::host::EngineMouse::Snapshot> mouse_snapshot;
            if (engine_mouse) {
                mouse_snapshot = engine_mouse->poll();
                if (!previous_mouse_status || *previous_mouse_status != mouse_snapshot->status) {
                    std::printf("Rebuilt engine mouse poll: status=%08x\n",
                                static_cast<unsigned>(mouse_snapshot->status));
                    previous_mouse_status = mouse_snapshot->status;
                }
            }
            std::optional<hd2::host::CursorPoint> frame_point;
            std::uint32_t frame_target = 0;
            bool exit_hover = false;
            if (text) {
                const auto point = cursor->position(window);
                frame_point = point;
                const auto main_picked = use_engine_mouse && !confirming && point
                    ? menu->pick_main_menu(module, point->x, point->y,
                                          static_cast<float>(methods->width(graphics)),
                                          static_cast<float>(methods->height(graphics))) : 0u;
                if (use_engine_mouse && !confirming && point) {
                    if (!previous_main_pick || *previous_main_pick != main_picked) {
                        std::printf("Main-menu engine picking: target=%08x cursor=(%.1f,%.1f)\n",
                                    static_cast<unsigned>(main_picked), point->x, point->y);
                        previous_main_pick = main_picked;
                    }
                } else {
                    previous_main_pick.reset();
                }
                exit_hover = !confirming && (use_engine_mouse
                    ? main_focus->interactive_target(main_picked) != 0
                    : text->over_exit(point));
                if (!exit_hover) {
                    exit_button.cancel();
                    confirmation_exit_click.clear();
                }
                if (main_focus) {
                    const auto target = exit_hover ? buttons->target_for_role(MenuRole::exit) : 0u;
                    if (main_focus->control_for_target(target) != main_focus->focused_control()) {
                        if (hd2::frontend::screen_pointer_focus(*main_focus, target) == 0) {
                            throw std::logic_error("Main-menu focus transition failed");
                        }
                    }
                } else {
                    buttons->set_hover(MenuRole::exit, exit_hover);
                }
                const std::uint32_t picked =
                    confirming && (confirmation_pick_active || confirmation_pick_probe)
                        ? pick_confirmation(point) : 0;
                const int rectangle_hit = confirming ? text->confirmation_hit(point) : 0;
                const int hover = confirmation_pick_active
                    ? confirmation_hit_for_target(picked) : rectangle_hit;
                frame_target = confirming ? active_confirmation_focus().target_for_control(hover)
                    : exit_hover ? buttons->target_for_role(MenuRole::exit) : 0u;
                const bool moved = confirmation_pick_active && confirming && point &&
                    previous_confirmation_point &&
                    hd2::frontend::pointer_moved(
                        point->x - previous_confirmation_point->x,
                        point->y - previous_confirmation_point->y);
                if (confirmation_pick_active && confirming) previous_confirmation_point = point;
                else previous_confirmation_point.reset();
                if (confirmation_click.pressed() != 0 &&
                    confirmation_click.pressed() != static_cast<std::uint32_t>(hover)) {
                    confirmation_click.clear();
                }
                if ((hover != previous_confirm_hover || (moved && !use_engine_mouse)) && confirmation_enabled) {
                    if (hd2::frontend::screen_pointer_focus(
                            active_confirmation_focus(), active_confirmation_focus().target_for_control(hover)) == 0) {
                        throw std::logic_error("Confirmation focus transition failed");
                    }
                    previous_confirm_hover = hover;
                }
                if (confirmation_pick_probe && confirming) {
                    const std::array<std::uint32_t, 2> comparison{{
                        confirmation_focus->target_for_control(rectangle_hit), picked
                    }};
                    if (!previous_pick_probe || *previous_pick_probe != comparison) {
                        std::printf("Confirmation picking probe: rectangle=%08x engine=%08x\n",
                                    static_cast<unsigned>(comparison[0]),
                                    static_cast<unsigned>(comparison[1]));
                        previous_pick_probe = comparison;
                    }
                } else {
                    previous_pick_probe.reset();
                }
            }
            if (frame_input) {
                if (frame_point && mouse_snapshot->status == 0) {
                    if (confirming) {
                        frame_input->tick(*frame_point, frame_target, mouse_snapshot->buttons,
                                          mouse_snapshot->delta, mouse_snapshot->state.z);
                    } else {
                        main_frame_input->tick(*frame_point, frame_target, mouse_snapshot->buttons,
                                               mouse_snapshot->delta, mouse_snapshot->state.z);
                    }
                } else {
                    input_session.clear();
                }
                menu_events->pump();
                if (!running) break;
            }
            if (menu) {
                if (confirming) menu->set_confirmation_visible(module, false);
                if (runtime_active) menu->set_screen_visible(module, menu->runtime_confirmation_frame(), false);
                const auto render_status = menu->render();
                if (render_status < 0) {
                    std::fprintf(stderr, "Menu scene render failed: 0x%08x\n",
                                 static_cast<unsigned>(render_status));
                    result = 1;
                    break;
                }
            } else {
                methods->clear(graphics, 0xff203050, 1.0f, 3);
                methods->begin_scene(graphics);
                methods->end_scene(graphics);
            }
            if (text) {
                text->draw(exit_hover);
                if (confirming) {
                    hd2::host::MenuScene::ModalPass modal(*menu, module,
                        runtime_active ? menu->runtime_confirmation_frame() : nullptr);
                    const auto status = menu->render();
                    if (status < 0) {
                        std::fprintf(stderr, "Confirmation scene render failed: 0x%08x\n",
                                     static_cast<unsigned>(status));
                        result = 1;
                        break;
                    }
                    if (runtime_active) text->draw_runtime_confirmation(*runtime_definition, previous_confirm_hover);
                    else text->draw(false, true, previous_confirm_hover);
                    if (!modal.restore()) {
                        throw std::runtime_error("Cannot restore original modal render state");
                    }
                    if (!modal_drawn) {
                        std::puts("Confirmation two-pass composition submitted; driver state restored");
                        modal_drawn = true;
                    }
                }
                if (!text_drawn) {
                    std::puts("Original menu bitmap glyphs submitted");
                    text_drawn = true;
                }
            }
            if (cursor) {
                custom_cursor_active = cursor->draw(window);
                if (custom_cursor_active) {
                    SetCursor(nullptr);
                    if (!cursor_drawn) {
                        std::puts("Original cursor quad submitted");
                        cursor_drawn = true;
                    }
                }
            }
            methods->present(graphics);
            ++frames;
            if (frame_limit != 0 && frames == frame_limit) {
                if (!PostMessageW(window, WM_CLOSE, 0, 0)) {
                    windows_error("PostMessageW(WM_CLOSE)");
                    result = 1;
                    break;
                }
            }
            Sleep(16);
        }
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Menu preview failed: %s\n", error.what());
        result = 1;
    }
    custom_cursor_active = false;
    if (menu_model) {
        HCURSOR arrow = LoadCursorW(nullptr, IDC_ARROW);
        if (arrow == nullptr) {
            windows_error("LoadCursorW(IDC_ARROW)");
            result = 1;
        } else {
            SetCursor(arrow);
        }
    }
    SetLastError(0);
    if (SetWindowLongPtrW(window, GWLP_WNDPROC,
                         reinterpret_cast<LONG_PTR>(previous_window_proc)) == 0) {
        windows_error("Restore window procedure");
        result = 1;
    }
    std::printf("Submitted %u frames\n", static_cast<unsigned>(frames));
    return finish(graphics, result);
}

}

int wmain(int argc, wchar_t** argv) {
    if (std::freopen("host-runtime.log", "w", stdout) == nullptr) {
        return 1;
    }
    if (std::freopen("host-runtime.log", "a", stderr) == nullptr ||
            _dup2(_fileno(stdout), _fileno(stderr)) != 0) {
        std::fputs("Cannot redirect host error diagnostics\n", stdout);
        return 1;
    }
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::setvbuf(stderr, nullptr, _IONBF, 0);
    if (argc < 2) {
        std::fputs("Usage: engine_window.exe ABSOLUTE_ENGINE_DLL [--frames COUNT] "
                   "[--menu-model | --menu-ui | --confirm-model | --menu-confirm] "
                   "[--probe-confirm-picking] [--use-confirm-picking] [--use-engine-mouse] [--probe-groups] [--probe-runtime-confirmation]\n"
                   "Use the fingerprint-checking Python launcher. Default: 180 frames; "
                   "0 waits for Escape/Close.\n", stderr);
        return 2;
    }
    std::uint32_t frame_limit = 180;
    bool menu_model = false;
    bool menu_ui = false;
    bool confirmation_preview = false;
    bool confirmation_enabled = false;
    bool confirmation_pick_probe = false;
    bool confirmation_pick_active = false;
    bool use_engine_mouse = false;
    bool group_probe = false;
    bool runtime_probe = false;
    bool frames_seen = false;
    try {
        for (int i = 2; i < argc; ++i) {
            if (std::wstring_view(argv[i]) == L"--menu-model" && !menu_model) {
                menu_model = true;
            } else if (std::wstring_view(argv[i]) == L"--menu-ui" && !menu_model) {
                menu_model = true;
                menu_ui = true;
            } else if (std::wstring_view(argv[i]) == L"--confirm-model" && !menu_model) {
                menu_model = true;
                confirmation_preview = true;
            } else if (std::wstring_view(argv[i]) == L"--menu-confirm" && !menu_model) {
                menu_model = true;
                menu_ui = true;
                confirmation_enabled = true;
            } else if (std::wstring_view(argv[i]) == L"--probe-confirm-picking" &&
                       !confirmation_pick_probe) {
                confirmation_pick_probe = true;
            } else if (std::wstring_view(argv[i]) == L"--use-confirm-picking" &&
                       !confirmation_pick_active) {
                confirmation_pick_active = true;
            } else if (std::wstring_view(argv[i]) == L"--use-engine-mouse" && !use_engine_mouse) {
                use_engine_mouse = true;
            } else if (std::wstring_view(argv[i]) == L"--probe-groups" && !group_probe) {
                group_probe = true;
            } else if (std::wstring_view(argv[i]) == L"--probe-runtime-confirmation" && !runtime_probe) {
                runtime_probe = true;
            } else if (std::wstring_view(argv[i]) == L"--frames" &&
                       !frames_seen && i + 1 < argc) {
                frame_limit = hd2::host::parse_frame_limit(argv[++i]);
                frames_seen = true;
            } else {
                throw std::invalid_argument("Unknown, duplicate, or incomplete host option");
            }
        }
        if (use_engine_mouse && (!confirmation_enabled || !confirmation_pick_active)) {
            throw std::invalid_argument("Engine mouse requires --menu-confirm --use-confirm-picking");
        }
        if ((confirmation_pick_probe || confirmation_pick_active) && !confirmation_enabled) {
            throw std::invalid_argument("Confirmation picking requires --menu-confirm");
        }
        if (group_probe && !confirmation_enabled) {
            throw std::invalid_argument("Group probe requires --menu-confirm");
        }
        if (runtime_probe && (!confirmation_enabled || confirmation_pick_probe ||
                              confirmation_pick_active || use_engine_mouse)) {
            throw std::invalid_argument("Runtime confirmation probe requires --menu-confirm without picking or engine mouse");
        }
    } catch (const std::invalid_argument& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 2;
    }
    std::puts("Engine-window host started (i386)");
    std::fflush(stdout);
    HMODULE module = LoadLibraryExW(argv[1], nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (module == nullptr) {
        windows_error("LoadLibraryExW(LS3DF.dll)");
        return 1;
    }
    FARPROC entry = GetProcAddress(module, "GetIGraph");
    int result = 1;
    if (entry == nullptr || !expected_method(entry, module, 0x68f90)) {
        std::fputs("Missing or unsupported GetIGraph export\n", stderr);
    } else {
        using GetGraphics = Graphics* (__cdecl *)();
        GetGraphics get_graphics = nullptr;
        static_assert(sizeof(get_graphics) == sizeof(entry));
        std::memcpy(&get_graphics, &entry, sizeof(entry));
        Graphics* graphics = get_graphics();
        if (graphics == nullptr || !verify_methods(graphics, module)) {
            std::fputs("Missing graphics singleton or unsupported graphics vtable\n", stderr);
        } else {
            result = run_window(graphics, module, frame_limit, menu_model, menu_ui,
                                confirmation_preview, confirmation_enabled, confirmation_pick_probe,
                                confirmation_pick_active, use_engine_mouse, group_probe, runtime_probe);
        }
    }
    if (!FreeLibrary(module)) {
        windows_error("FreeLibrary(LS3DF.dll)");
        result = 1;
    }
    return result;
}
