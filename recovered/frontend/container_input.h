#ifndef HD2_FRONTEND_CONTAINER_INPUT_H
#define HD2_FRONTEND_CONTAINER_INPUT_H

#include <cstdint>

namespace hd2::frontend {

// 00656aa0, kind-five +20. Unlike the screen override, no Enter/Space
// activation fallback. Eligibility (+08), rather than only disabled, gates keys.
template<class Container>
bool container_key(Container& container, std::uint32_t key) {
    const int focused = container.focused_control();
    if (!container.valid_control(focused) || !container.control_eligible(container.focused_control()))
        return false;
    return container.key_control(container.focused_control(), key);
}

// 00656b20, shared container/screen +30. Special target 15 calls the
// object's virtual key method and maps handled to seven, without ID lookup.
// Ordinary entry resolves the target three times: validate, eligibility, store.
template<class Container>
int container_focus_enter(Container& container, std::uint32_t target) {
    if (target == 15) return container.key(target) ? 7 : 0;
    if (!container.valid_control(container.control_for_target(target))) return 0;
    if (!container.control_eligible(container.control_for_target(target))) return 0;
    const int index = container.control_for_target(target);
    container.set_focused_control(index);
    return container.enter_control(index, target);
}

// 00656bd0, +38. Activation forwards to the focused child without an
// eligibility/disabled check at this layer; the child owns that policy.
template<class Container>
int container_activate(Container& container, std::uint32_t target) {
    if (!container.valid_control(container.focused_control())) return 0;
    return container.activate_control(container.focused_control(), target);
}

// 00656c00 / +3c: forward to the freshly resolved focused child's +3c.
template<class Container>
int container_deactivate(Container& container, std::uint32_t target) {
    if (!container.valid_control(container.focused_control())) return 0;
    return container.deactivate_control(container.focused_control(), target);
}

// 00656c30: retain virtual leave/enter calls and repeat target lookup after
// validity. The focused child receives movement when the target stays in it.
template<class Container>
int container_pointer_focus(Container& container, std::uint32_t target) {
    if (!container.valid_control(container.control_for_target(target))) return container.leave_focus(target);
    const int focused = container.focused_control();
    if (focused == container.control_for_target(target)) return container.move_control(focused, target);
    if (container.leave_focus(target) == 0) return 1;
    return container.enter_focus(target);
}

}

#endif
