#ifndef HD2_INPUT_BINDING_INSTALLATION_H
#define HD2_INPUT_BINDING_INSTALLATION_H

#include "input/binding_producer.h"
#include <algorithm>

namespace hd2::input {

inline std::size_t binding_scan_slot(const BindingInputState& state, std::uint8_t scan) {
    const auto found = std::find(state.scans.begin(), state.scans.end(), scan);
    return static_cast<std::size_t>(found - state.scans.begin());
}

// Only the five original button selectors participate in reference counts.
inline std::size_t binding_mouse_slot(std::uint16_t code) {
    switch (code) {
    case 1: return 0;
    case 2: return 1;
    case 4: return 2;
    case 8: return 3;
    case 16: return 4;
    default: return 5;
    }
}

// Client 006c7480. Services supply graphics +b4(map,64) and joystick
// +30(selector,flags). Their results are intentionally ignored. Replacement
// storage must stay valid across vector growth and callbacks, as with the
// original stack record in 006c7440; don't borrow a growing binding vector.
// A false return does NOT roll back growth, removed counts or scan-map updates.
template<class Services>
bool install_input_binding(BindingInputState& state, Services& services,
                            std::int32_t index, const InputBinding* replacement) {
    if (index < 0) return false;
    const auto slot = static_cast<std::size_t>(index);
    if (slot >= state.bindings.size()) {
        state.bindings.resize(slot + 1, InputBinding{});
        // Original grows each output vector only in this binding-growth branch.
        if (state.first.size() <= slot) state.first.resize(slot + 1, 0);
        if (state.second.size() <= slot) state.second.resize(slot + 1, 0);
    }

    if (state.bindings.at(slot).device_kind == 1) {
        const auto source = binding_scan_slot(state,
            static_cast<std::uint8_t>(state.bindings.at(slot).code));
        if (source < state.scans.size() && --state.key_binding_counts[source] == 0) {
            state.scans[source] = 0;
            services.configure_keyboard_snapshot(state.scans.data(), 64);
        }
    }
    // Reread the record after the map callback, just as the original does.
    if (state.bindings.at(slot).device_kind == 2) {
        const auto source = binding_mouse_slot(state.bindings.at(slot).code);
        if (source < 5) --state.mouse_binding_counts[source];
    }
    if (!replacement) {
        state.bindings.at(slot) = {};
        return true; // Does not clear the old output words or persistent gates.
    }

    const auto kind = replacement->device_kind;
    if (kind == 1) {
        const auto scan = static_cast<std::uint8_t>(replacement->code);
        const auto source = binding_scan_slot(state, scan);
        if (source < state.scans.size()) {
            ++state.key_binding_counts[source];
        } else {
            const auto empty = binding_scan_slot(state, 0);
            if (empty == state.scans.size()) return false;
            state.scans[empty] = scan;
            services.configure_keyboard_snapshot(state.scans.data(), 64);
            ++state.key_binding_counts[empty];
        }
    } else if (kind == 2) {
        const auto source = binding_mouse_slot(replacement->code);
        if (source < 5) ++state.mouse_binding_counts[source];
    } else if (kind == 3) {
        const auto device = replacement->device_index;
        if (device >= state.joysticks.size()) return false;
        const auto flags = replacement->direction == 3 && replacement->code < 0xff ? 4u : 0u;
        services.configure_joystick_binding(device, replacement->code, flags);
    }
    // Other kinds are accepted verbatim. Callback results don't gate this copy.
    state.bindings.at(slot) = *replacement;
    return true;
}

// 006c7440 packs its five arguments into the original twelve-byte record.
// It truncates kind/selector to sixteen bits and forwards the installer AL result.
template<class Services>
bool set_input_binding(BindingInputState& state, Services& services,
                        std::int32_t index, std::uint32_t kind, std::uint32_t code,
                        std::uint32_t device, std::uint32_t direction) {
    const InputBinding replacement{direction, static_cast<std::uint16_t>(kind),
                                    static_cast<std::uint16_t>(code), device};
    return install_input_binding(state, services, index, &replacement);
}

}
#endif
