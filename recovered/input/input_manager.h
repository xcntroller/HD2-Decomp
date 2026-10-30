#ifndef HD2_INPUT_INPUT_MANAGER_H
#define HD2_INPUT_INPUT_MANAGER_H

#include "input/binding_producer.h"
#include <algorithm>

namespace hd2::input {

// Client 006c6c90: signed integers converted to floats, with no range/default
// policy. The original constructor uses 20/20/20; attachment uses 400/400/1.
inline bool set_binding_axis_limits(BindingInputState& state, std::int32_t x,
                                     std::int32_t y, std::int32_t z) {
    state.mouse_axis_limits = {{static_cast<float>(x), static_cast<float>(y),
                                static_cast<float>(z)}};
    return true;
}

// Native owner for 006c6a20/6ab0, 006c6b60, 006c6cc0 and 006c6ff0. The normal
// original instance has zero-initialized static storage before its constructor;
// native value initialization preserves that context, not arbitrary placement.
// Graphics is borrowed without AddRef/Release. Only joystick records retain
// devices. Services must supply real interface bodies; none are default no-ops.
// Device reference callbacks must not throw or structurally mutate ownership
// storage. The services object must outlive this owner.
template<class Services>
class InputManager {
public:
    explicit InputManager(Services& services) : services_(services) {
        set_binding_axis_limits(state_, 20, 20, 20);
    }
    ~InputManager() {
        detach();
        // Original destructor releases vector allocations in this order.
        std::vector<std::uint32_t>{}.swap(state_.second);
        std::vector<std::uint32_t>{}.swap(state_.first);
        std::vector<InputBinding>{}.swap(state_.bindings);
        std::vector<JoystickInput>{}.swap(state_.joysticks);
    }
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    BindingInputState& state() { return state_; }
    const BindingInputState& state() const { return state_; }
    void* graphics() const { return graphics_; }
    std::uint8_t attachment_flag() const { return attachment_flag_; }

    // 006c6cc0 validates graphics and every supplied joystick before mutation.
    // It appends to existing records: it does not detach/clear prior devices or
    // bindings. The caller's device array must remain stable across callbacks.
    bool attach(void* graphics, void* const* devices, std::int32_t count) {
        if (!graphics) return false;
        if (count < 0 || (count && !devices))
            throw std::invalid_argument("Invalid input-manager joystick array");
        for (auto index = count; index != 0; --index)
            if (!devices[index - 1]) return false;

        clear_source_counts();
        graphics_ = graphics;
        for (std::int32_t index = 0; index < count; ++index) {
            JoystickInput record{};
            record.device = devices[index];
            services_.retain_joystick(record.device); // Original +04, before append.
            try {
                state_.joysticks.push_back(record);
            } catch (...) {
                // Native allocation failure must not leak the just-retained ref.
                services_.release_joystick(record.device);
                throw;
            }
        }
        state_.key_gate = ~std::uint64_t{0};
        set_binding_axis_limits(state_, 400, 400, 1);
        attachment_flag_ = 0;
        return true;
    }

    // 006c6ff0 clears only binding/output lengths and source tables. Preserve
    // capacities, device refs, graphics, limits, gates and attachment flag.
    void clear_bindings() {
        state_.bindings.clear();
        state_.first.clear();
        state_.second.clear();
        clear_source_counts();
    }

    // 006c6b60 clears binding/output lengths before reverse joystick release,
    // then clears joystick length. Maps/counts, graphics and other fields stay.
    // Null records are skipped, as in original; each retained occurrence owns
    // its own reference, including duplicates.
    void detach() {
        state_.bindings.clear();
        state_.first.clear();
        state_.second.clear();
        for (auto count = state_.joysticks.size(); count != 0; --count) {
            auto* device = state_.joysticks.at(count - 1).device;
            if (device) services_.release_joystick(device);
        }
        state_.joysticks.clear();
    }

    // These methods connect the recovered installer/producer contracts to the
    // manager's actual borrowed graphics and retained joystick records. Native
    // calls resolve device/graphics afresh instead of caching across callbacks.
    void produce(bool poll) { produce_binding_input(state_, *this, poll); }
    decltype(auto) configure_keyboard_snapshot(const std::uint8_t* scans, std::uint32_t count) {
        return services_.configure_keyboard_snapshot(required_graphics(), scans, count);
    }
    decltype(auto) configure_joystick_binding(std::size_t index, std::uint16_t selector,
                                              std::uint32_t flags) {
        return services_.configure_joystick_binding(required_device(index), selector, flags);
    }
    decltype(auto) poll_joystick(std::size_t index, std::uint32_t flags) {
        return services_.poll_joystick(required_device(index), flags);
    }
    decltype(auto) read_joystick_current(std::size_t index, JoystickInputState& output) {
        return services_.read_joystick_current(required_device(index), output);
    }
    decltype(auto) read_joystick_secondary(std::size_t index, JoystickInputState& output) {
        return services_.read_joystick_secondary(required_device(index), output);
    }
    decltype(auto) pump_graphics() { return services_.pump_graphics(required_graphics()); }
    decltype(auto) poll_mouse() { return services_.poll_mouse(required_graphics()); }
    decltype(auto) poll_keyboard() { return services_.poll_keyboard(required_graphics()); }
    std::uint64_t keyboard_snapshot() { return services_.keyboard_snapshot(required_graphics()); }
    std::uint32_t mouse_buttons() { return services_.mouse_buttons(required_graphics()); }
    std::int32_t mouse_x() { return services_.mouse_x(required_graphics()); }
    std::int32_t mouse_y() { return services_.mouse_y(required_graphics()); }
    std::int32_t mouse_z() { return services_.mouse_z(required_graphics()); }

private:
    void clear_source_counts() {
        state_.scans.fill(0);
        state_.key_binding_counts.fill(0);
        // Original writes ONE DWORD at +a0, leaving button-five count at +a4
        // and spare byte +a5 unchanged. Do not clear the whole native array.
        std::fill_n(state_.mouse_binding_counts.begin(), 4, std::uint8_t{0});
    }
    void* required_graphics() const {
        if (!graphics_) throw std::logic_error("Input manager has no graphics attachment");
        return graphics_;
    }
    void* required_device(std::size_t index) const {
        auto* device = state_.joysticks.at(index).device;
        if (!device) throw std::logic_error("Input manager has a null joystick record");
        return device;
    }
    Services& services_;
    BindingInputState state_;
    void* graphics_ = nullptr;
    std::uint8_t attachment_flag_ = 0;
};

}
#endif
