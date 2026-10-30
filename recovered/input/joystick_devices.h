#ifndef HD2_INPUT_JOYSTICK_DEVICES_H
#define HD2_INPUT_JOYSTICK_DEVICES_H

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace hd2::input {

struct JoystickDeviceView {
    void* const* devices;
    std::int32_t count;
};

// Client 006dc640/006dc5e0, whose callers are unresolved. This list owns the
// factory references, independently of InputManager's additional retained refs.
// Services/modules must outlive it; release callbacks must not throw or mutate
// ownership storage. Original global storage starts empty. Native vector growth
// replaces the original raw-pointer vector helpers, without retaining copies.
template<class Services>
class JoystickDevices {
public:
    explicit JoystickDevices(Services& services) : services_(services) {}
    ~JoystickDevices() { close(); }
    JoystickDevices(const JoystickDevices&) = delete;
    JoystickDevices& operator=(const JoystickDevices&) = delete;

    void initialize(std::uintptr_t instance) {
        // The original overwrites its base on repeat setup and can leak it.
        // Require explicit close before repeating a native ownership cycle.
        if (base_ || !devices_.empty())
            throw std::logic_error("Joystick list is already initialized");
        auto* graphics = services_.graphics();
        const auto window = services_.window(graphics);
        if (!services_.create_base(base_, instance, window, nullptr, 0)) return;
        if (!base_) throw std::runtime_error("Joystick factory returned a null base");
        const auto count = services_.joystick_count(base_); // One signed snapshot.
        for (std::int32_t index = 0; index < count; ++index) {
            void* device = nullptr;
            const auto created = services_.create_joystick(base_, device, index, 0);
            // IJoy clears the output on failure; the original false branch
            // dereferences that null. Never silently skip a failed device.
            if (!device) throw std::runtime_error("Joystick creation returned a null device");
            try {
                if (!created) services_.configure_joystick_binding(device, 1, 2);
                devices_.push_back(device); // Also append on false + valid output.
            } catch (...) {
                // Native allocation/adapter errors cannot strand this new ref.
                services_.release_joystick(device);
                throw;
            }
        }
    }

    JoystickDeviceView view() const {
        if (devices_.size() > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))
            throw std::length_error("Joystick view exceeds the original signed count");
        return {devices_.data(), static_cast<std::int32_t>(devices_.size())};
    }
    void* base() const { return base_; }

    // 006dceb0..006dcedb: application polls the list in reverse before invoking
    // InputManager::produce(false). Do not poll again through produce(true).
    void poll() {
        for (auto count = devices_.size(); count != 0; --count)
            services_.poll_joystick(devices_[count - 1], 1);
    }

    void close() {
        // Original list releases FORWARD while its length remains visible;
        // InputManager's separate retained records release in reverse order.
        for (auto* device : devices_) services_.release_joystick(device);
        devices_.clear(); // Keep allocation capacity, then release the base.
        if (base_) {
            services_.release_joystick_base(base_);
            base_ = nullptr; // Clear only after the callback.
        }
    }

private:
    Services& services_;
    void* base_ = nullptr;
    std::vector<void*> devices_;
};

}
#endif
