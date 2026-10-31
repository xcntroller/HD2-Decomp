#ifndef HD2_FRONTEND_CONTROL_INPUT_BRIDGE_H
#define HD2_FRONTEND_CONTROL_INPUT_BRIDGE_H
#include "frontend/pointer_dispatch.h"
#include <type_traits>
namespace hd2::frontend {
// Existing leaf backends can retain their target-independent interface. The
// concrete heterogeneous product opts in to preserving the internal-part bits.
template<class Control,class=void> struct HasTargetedMenuInput : std::false_type {};
template<class Control> struct HasTargetedMenuInput<Control,std::void_t<typename Control::targeted_menu_input>> : std::true_type {};
template<class Control> int enter_menu_control(Control& c,std::uint32_t t) { if constexpr (HasTargetedMenuInput<Control>::value) return c.enter_focus(t); else return c.apply(true); }
template<class Control> int leave_menu_control(Control& c,std::uint32_t t) { if constexpr (HasTargetedMenuInput<Control>::value) return c.leave_focus(t); else return c.apply(false); }
template<class Control> int move_menu_control(Control& c,std::uint32_t t) { if constexpr (HasTargetedMenuInput<Control>::value) return c.pointer_focus(t); else return c.pointer_move(); }
template<class Control,class Events> int pointer_menu_control(Control& c,ScreenPointerCallback callback,std::uint32_t target,Events& events) { if constexpr (HasTargetedMenuInput<Control>::value) return c.pointer_target(callback,target,events); else return c.pointer_event(callback,events); }
template<class Control,class Events> int wheel_menu_control(Control& c,std::int32_t delta,Events& events) { if constexpr (HasTargetedMenuInput<Control>::value) return c.wheel_target(delta,events); else return c.wheel_event(events); }
}
#endif
