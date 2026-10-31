#ifndef HD2_FRONTEND_POINTER_DISPATCH_H
#define HD2_FRONTEND_POINTER_DISPATCH_H

#include "frontend/pointer_wheel.h"

#include <cstdint>

namespace hd2::frontend {

// Screen vtable byte offsets used by client 0066b990. Names describe the
// observed call sites; original class/type names and ABI remain unknown.
enum class ScreenPointerCallback : std::uint32_t {
    focus = 0x40,
    release = 0x44,
    press = 0x48,
    drag = 0x50,
    drag_end = 0x54,
    drag_begin = 0x58,
    click = 0x5c,
    double_click = 0x60
};

struct PointerDispatchState {
    std::int32_t gate = -1;         // Front-end +24.
    std::uint32_t previous_target = 0xffffffffu; // Front-end +0c.
    bool dragging = false;          // Front-end +2c.
    bool motion_only = false;       // Standalone probes; live host uses CommonFrontendState +2d.
};

struct PointerDispatchInput {
    bool pressed_mask3 = false;
    bool released_mask3 = false;
    bool held_mask3 = false;
    bool click = false;
    bool double_click = false;
    bool moved = false;
    bool wheel_changed = false;
};

// Portable sequencing subset of client 0066b990. Context supplies picking,
// input update, captured-control callbacks, and fresh screen-stack lookup for
// EVERY screen callback. Do not cache a screen across callbacks: they may
// change the stack. input() must expose the current flags after callbacks.
// Wheel dispatch is followed by a fresh current-screen lookup and pick/focus.
// Original segmented stack storage and frame ownership remain context adapters.
template<class Context>
int dispatch_menu_pointer(PointerDispatchState& state, Context& context,
                          std::uint32_t delta, const bool& motion_only) {
    if (state.gate != -1) return 0;
    const auto target = context.pick_target();
    const bool processed = context.update_input(delta, target);
    if (!processed && state.previous_target == target) return 1;
    state.previous_target = target;

    if (motion_only) {
        if (context.input().moved) {
            state.dragging = false;
            context.screen_event(ScreenPointerCallback::focus, target);
        }
        return 1;
    }

    if (context.input().pressed_mask3) {
        const auto captured = context.captured_control_target();
        if (captured && *captured != target) {
            if (context.captured_control_event(target) != 1) return 1;
            context.clear_captured_control();
        }
        context.screen_event(ScreenPointerCallback::press, target);
    } else if (context.input().released_mask3) {
        if (state.dragging) {
            state.dragging = false;
            context.screen_event(ScreenPointerCallback::drag_end, target);
            context.screen_event(ScreenPointerCallback::focus, target);
        }
        context.screen_event(ScreenPointerCallback::release, target);
    }

    if (context.input().double_click) {
        context.screen_event(ScreenPointerCallback::double_click, target);
    }
    if (context.input().click) {
        const auto captured = context.captured_control_target();
        if (!captured || *captured == target) {
            if (context.screen_event(ScreenPointerCallback::click, target) == 1) {
                context.accepted_click(); // Original 0066b110; side effect delegated.
            }
        }
    }

    if (context.input().moved) {
        if (!state.dragging) context.screen_event(ScreenPointerCallback::focus, target);
        if (context.input().held_mask3) {
            if (!state.dragging) {
                context.screen_event(ScreenPointerCallback::drag_begin, target);
                state.dragging = true;
            }
            if (state.dragging) {
                context.screen_event(ScreenPointerCallback::drag, target);
                return 1; // Original skips wheel handling on this path.
            }
        }
    }
    if (context.input().wheel_changed) {
        context.screen_wheel(target, pointer_wheel_argument(context.wheel_delta()));
        // A callback may replace the screen/model. Resolve again after it.
        const auto screen = context.current_screen();
        const auto refreshed = context.pick_for_screen(screen);
        context.focus_screen(screen, refreshed);
    }
    return 1;
}

template<class Context>
int dispatch_menu_pointer(PointerDispatchState& state, Context& context, std::uint32_t delta) {
    return dispatch_menu_pointer(state, context, delta, state.motion_only);
}

}

#endif
