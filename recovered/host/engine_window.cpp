#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "host/graphics_settings.h"
#include "host/menu_scene.h"
#include "host/menu_cursor.h"
#include "host/menu_text.h"
#include "host/menu_button_visuals.h"
#include "frontend/menu_event_queue.h"
#include "frontend/pointer_click.h"
#include "host/pointer_dispatch_probe.h"
#include "host/engine_mouse.h"
#include "frontend/pointer_motion.h"
#include "frontend/screen_focus.h"
#include "frontend/screen_input.h"
#include "frontend/screen_target.h"

#include <cstdio>
#include <cstring>
#include <exception>
#include <io.h>

namespace {

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
    HostMenuEventActions(const hd2::host::MenuScene& scene, HMODULE module, bool& confirming)
        : scene_(scene), module_(module), confirming_(confirming) {}

    void show_confirmation() override {
        confirming_ = true;
        scene_.set_confirmation_visible(module_, true);
        std::puts("Original event flow: confirmation opened");
    }

    void hide_confirmation() override {
        confirming_ = false;
        scene_.set_confirmation_visible(module_, false);
        std::puts("Original event flow: confirmation dismissed");
    }

    void quit() override {
        std::puts("Original event flow: closing reconstruction host");
        running = false;
    }

private:
    const hd2::host::MenuScene& scene_;
    HMODULE module_;
    bool& confirming_;
};

class ConfirmationFocus {
public:
    ConfirmationFocus(hd2::host::ButtonVisualPair& yes, hd2::host::ButtonVisualPair& no,
                      hd2::frontend::ControlEventDestination& events)
        : yes_(yes), no_(no), events_(events) {}

    static std::uint32_t target_for_hit(int hit) {
        if (hit == 0) return 0;
        if (hit == 1 || hit == 2) return control_ids_[static_cast<std::size_t>(hit)];
        throw std::logic_error("Invalid confirmation hit");
    }
    static int hit_for_target(std::uint32_t target) {
        const int index = hd2::frontend::find_screen_control(screen_id_, control_ids_, target);
        return index == 1 || index == 2 ? index : 0;
    }
    int control_for_target(std::uint32_t target) const {
        return hd2::frontend::find_screen_control(screen_id_, control_ids_, target);
    }
    bool valid_control(int index) const {
        return index >= 0 && static_cast<std::size_t>(index) < control_ids_.size();
    }
    bool control_eligible(int index) const {
        if (index != 1 && index != 2) throw std::logic_error("Unsupported confirmation control");
        return true;
    }
    int focused_control() const { return focused_; }
    void set_focused_control(int index) { focused_ = index; }
    int leave_control(int index, std::uint32_t) { return button(index).apply(false); }
    int enter_control(int index, std::uint32_t) { return button(index).apply(true); }
    int move_control(int index, std::uint32_t) { return button(index).pointer_move(); }
    int click_control(int index, std::uint32_t) { return button(index).click(events_); }
    int pointer_control(int index, hd2::frontend::ScreenPointerCallback callback, std::uint32_t) {
        return button(index).pointer_event(callback, events_);
    }

private:
    static constexpr std::uint32_t screen_id_ = 0x10000000;
    static constexpr std::array<std::uint32_t, 4> control_ids_{{
        0x10100000, 0x10200000, 0x10300000, 0x10400000
    }};

    hd2::host::ButtonVisualPair& button(int index) {
        if (index == 1) return yes_;
        if (index == 2) return no_;
        throw std::logic_error("Invalid confirmation control");
    }

    hd2::host::ButtonVisualPair& yes_;
    hd2::host::ButtonVisualPair& no_;
    hd2::frontend::ControlEventDestination& events_;
    int focused_ = -1;
};

// Functional opt-in bridge. Positions/focus and departure cancellation remain
// host-owned; button snapshots come from the rebuilt LS3DF polling subset.
class ConfirmationFrameInput {
public:
    explicit ConfirmationFrameInput(ConfirmationFocus& focus) : focus_(focus) {}
    void clear() {
        pointer_.clear();
        state_ = {};
        input_ = {};
        previous_point_.reset();
        armed_ = false;
    }
    void tick(const hd2::host::CursorPoint& point, std::uint32_t target,
              std::uint32_t buttons, std::uint32_t delta) {
        if (!armed_) {
            if (buttons != 0) return; // Reentry must observe all buttons up first.
            pointer_.cursor().initialize(point.x, point.y, 800, 600);
            previous_point_ = point;
            armed_ = true;
        }
        pending_ = {buttons, target, delta, point.x - previous_point_->x,
                    point.y - previous_point_->y};
        previous_point_ = point;
        pointer_.cancel_departed_press(target); // Retain established host safety.
        hd2::frontend::dispatch_menu_pointer(state_, *this, delta);
    }
    std::uint32_t pick_target() const { return pending_.target; }
    bool update_input(std::uint32_t delta, std::uint32_t target) {
        pending_.delta = delta;
        pending_.target = target;
        const auto result = pointer_.update(pending_);
        const auto& buttons = pointer_.buttons();
        input_ = {buttons.pressed(3), buttons.released(3), buttons.currently_selected(3),
                  result.click.click, false, // Double-click remains observational only.
                  hd2::frontend::pointer_moved(pointer_.cursor().horizontal(),
                                             pointer_.cursor().vertical()), false};
        return result.processed;
    }
    const hd2::frontend::PointerDispatchInput& input() const { return input_; }
    std::optional<std::uint32_t> captured_control_target() const { return std::nullopt; }
    int captured_control_event(std::uint32_t) {
        throw std::logic_error("No captured confirmation control object");
    }
    void clear_captured_control() {
        throw std::logic_error("No captured confirmation control object");
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
            std::printf("Engine mouse confirmation click: target=%08x focus=%d\n",
                        static_cast<unsigned>(target), focus_.focused_control());
            return hd2::frontend::screen_click(focus_, target);
        }
        throw std::logic_error("Unsupported confirmation pointer callback");
    }
    void accepted_click() {} // Original +3c object/helper is still absent.
    void wheel_and_refresh(std::uint32_t) {
        throw std::logic_error("Live wheel routing is not reconstructed");
    }

private:
    ConfirmationFocus& focus_;
    hd2::frontend::PointerUpdate pointer_;
    hd2::frontend::PointerDispatchState state_;
    hd2::frontend::PointerDispatchInput input_;
    hd2::frontend::PointerUpdateSample pending_{};
    std::optional<hd2::host::CursorPoint> previous_point_;
    bool armed_ = false;
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
               bool confirmation_pick_probe, bool confirmation_pick_active, bool use_engine_mouse) {
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
        std::unique_ptr<hd2::host::MenuScene> menu;
        if (menu_model) {
            menu = std::make_unique<hd2::host::MenuScene>(
                module, confirmation_preview, confirmation_enabled);
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
        std::unique_ptr<hd2::host::MenuText<Graphics>> text;
        std::unique_ptr<hd2::host::MenuButtonVisuals> buttons;
        hd2::host::ExitButton exit_button;
        hd2::frontend::PointerClick confirmation_exit_click;
        hd2::frontend::PointerClick confirmation_click;
        hd2::host::PointerDispatchProbe confirmation_probe_input;
        std::optional<hd2::host::CursorPoint> confirmation_probe_point;
        std::optional<DWORD> confirmation_probe_time;
        std::optional<hd2::host::ButtonVisualPair> confirm_yes;
        std::optional<hd2::host::ButtonVisualPair> confirm_no;
        bool confirming = false;
        int previous_confirm_hover = 0;
        std::optional<hd2::host::CursorPoint> previous_confirmation_point;
        std::optional<ConfirmationFocus> confirmation_focus;
        std::optional<ConfirmationFrameInput> frame_input;
        std::unique_ptr<hd2::host::EngineMouse> engine_mouse;
        std::optional<std::uint32_t> previous_mouse_status;
        std::optional<std::array<std::uint32_t, 2>> previous_pick_probe;
        std::optional<HostMenuEventActions> menu_actions;
        std::optional<hd2::frontend::MenuEventQueue> menu_events;
        if (menu_ui) {
            if (!expected_method(methods->viewport, module, 0x6c2b0)) {
                throw std::runtime_error("Unsupported original viewport getter ABI");
            }
            if (confirmation_enabled) menu->initialize_confirmation_text_frame(module);
            text = std::make_unique<hd2::host::MenuText<Graphics>>(
                graphics, module, *menu, confirmation_enabled);
            buttons = std::make_unique<hd2::host::MenuButtonVisuals>(*menu, module);
            if (confirmation_enabled) {
                confirm_yes.emplace(*menu, module, "byes?", "normal02", "actived03", true);
                confirm_no.emplace(*menu, module, "bno?", "normal01", "actived01", true);
                confirm_yes->bind_activation_events({
                    {0, 0x14001100, 0, 0, 0, {}}, {0, 0x06001005, 0, 0, 0, {}}
                });
                confirm_no->bind_activation_events({
                    {0, 0x14001200, 1, 0, 0, {}}, {0, 0x06001005, 0, 0, 0, {}}
                });
                if (confirmation_pick_probe || confirmation_pick_active) {
                    confirm_yes->bind_pick_target(ConfirmationFocus::target_for_hit(1));
                    confirm_no->bind_pick_target(ConfirmationFocus::target_for_hit(2));
                }
                if (confirmation_pick_probe) {
                    std::puts("Confirmation engine-picking comparison enabled");
                    std::puts("Confirmation input transition probe enabled (Win32 message snapshots)");
                }
                if (confirmation_pick_active) {
                    std::puts("Confirmation engine picking controls hover and activation");
                }
                menu->hide_confirmation_ok(module);
                menu_actions.emplace(*menu, module, confirming);
                menu_events.emplace(*menu_actions);
                confirmation_focus.emplace(*confirm_yes, *confirm_no, *menu_events);
                if (use_engine_mouse) {
                    frame_input.emplace(*confirmation_focus);
                    engine_mouse = std::make_unique<hd2::host::EngineMouse>(
                        reinterpret_cast<hd2::host::EngineFrame*>(graphics), module);
                }
            }
        }
        if (confirmation_preview) {
            const hd2::host::ButtonVisualPair yes(*menu, module, "byes?", "normal02", "actived03");
            const hd2::host::ButtonVisualPair no(*menu, module, "bno?", "normal01", "actived01");
            const hd2::host::ButtonVisualPair ok(*menu, module, "bok", "normal03", "actived04");
            for (const auto& binding : std::array<std::array<const char*, 2>, 3>{{
                     {{"text", "text"}}, {{"byes?", "normal02"}}, {{"bno?", "normal01"}}
                 }}) {
                const auto bounds = menu->label_bounds(binding[0], binding[1], module);
                const auto rectangle = hd2::host::menu_rectangle(bounds, 800, 600);
                std::printf("Confirmation diagnostic frame=%s bounds=(%.1f,%.1f)-(%.1f,%.1f)\n",
                            binding[0], rectangle.left, rectangle.top,
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
            return confirmation_pick_active
                ? ConfirmationFocus::hit_for_target(pick_confirmation(point))
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
                if (text && !(use_engine_mouse && confirming &&
                              mouse_button_state_message(message.message))) {
                    const bool inside = text->over_exit(cursor->position(window, &message.pt));
                    if (confirming && message.message == WM_LBUTTONDOWN) {
                        confirmation_click.press(confirmation_hit(cursor->position(window, &message.pt)));
                    } else if (confirming && message.message == WM_LBUTTONUP) {
                        const int hit = confirmation_hit(cursor->position(window, &message.pt));
                        const bool matched = confirmation_click.release(hit);
                        if (hit != 0 && matched) {
                            int result;
                            if (confirmation_focus->focused_control() == hit) {
                                std::printf("Confirmation click: routed through focused control %d\n",
                                            hit);
                                result = hd2::frontend::screen_click(
                                    *confirmation_focus, ConfirmationFocus::target_for_hit(hit));
                            } else {
                                std::printf("Confirmation click: sampled focus=%d target=%d; "
                                            "using host target selection\n",
                                            confirmation_focus->focused_control(), hit);
                                auto& button = hit == 1 ? *confirm_yes : *confirm_no;
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
                                if (!buttons->click_exit(*menu_events)) {
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
                exit_hover = !confirming && text->over_exit(point);
                if (!exit_hover) {
                    exit_button.cancel();
                    confirmation_exit_click.clear();
                }
                buttons->set_exit_hover(exit_hover);
                const std::uint32_t picked =
                    confirming && (confirmation_pick_active || confirmation_pick_probe)
                        ? pick_confirmation(point) : 0;
                const int rectangle_hit = confirming ? text->confirmation_hit(point) : 0;
                const int hover = confirmation_pick_active
                    ? ConfirmationFocus::hit_for_target(picked) : rectangle_hit;
                frame_target = ConfirmationFocus::target_for_hit(hover);
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
                            *confirmation_focus, ConfirmationFocus::target_for_hit(hover)) == 0) {
                        throw std::logic_error("Confirmation focus transition failed");
                    }
                    previous_confirm_hover = hover;
                }
                if (confirmation_pick_probe && confirming) {
                    const std::array<std::uint32_t, 2> comparison{{
                        ConfirmationFocus::target_for_hit(rectangle_hit), picked
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
                if (confirming && frame_point && mouse_snapshot->status == 0) {
                    frame_input->tick(*frame_point, frame_target,
                                      mouse_snapshot->buttons, mouse_snapshot->delta);
                } else {
                    frame_input->clear();
                }
                menu_events->pump();
                if (!running) break;
            }
            if (menu) {
                if (confirming) menu->set_confirmation_visible(module, false);
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
                    hd2::host::MenuScene::ModalPass modal(*menu, module);
                    const auto status = menu->render();
                    if (status < 0) {
                        std::fprintf(stderr, "Confirmation scene render failed: 0x%08x\n",
                                     static_cast<unsigned>(status));
                        result = 1;
                        break;
                    }
                    text->draw(false, true, previous_confirm_hover);
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
                   "[--probe-confirm-picking] [--use-confirm-picking] [--use-engine-mouse]\n"
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
                                confirmation_pick_active, use_engine_mouse);
        }
    }
    if (!FreeLibrary(module)) {
        windows_error("FreeLibrary(LS3DF.dll)");
        result = 1;
    }
    return result;
}
