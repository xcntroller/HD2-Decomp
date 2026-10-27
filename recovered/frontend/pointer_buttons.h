#ifndef HD2_FRONTEND_POINTER_BUTTONS_H
#define HD2_FRONTEND_POINTER_BUTTONS_H

#include <cstdint>

namespace hd2::frontend {

// Client 0066a050.
inline bool pointer_buttons_selected(std::uint32_t selector, std::uint32_t buttons) {
    return selector == 0 ? buttons != 0 : (buttons & selector) != 0;
}

class PointerButtons {
public:
    void update(std::uint32_t buttons) {
        previous_ = current_;
        current_ = buttons;
    }

    void clear() { previous_ = current_ = 0; }
    std::uint32_t previous() const { return previous_; }
    std::uint32_t current() const { return current_; }

    // Client 0066a080 and 0066a0a0.
    bool previously_selected(std::uint32_t selector) const {
        return pointer_buttons_selected(selector, previous_);
    }
    bool currently_selected(std::uint32_t selector) const {
        return pointer_buttons_selected(selector, current_);
    }

    // Client 0066a0c0 and 0066a0f0.
    bool pressed(std::uint32_t selector) const {
        return currently_selected(selector) && !previously_selected(selector);
    }
    bool released(std::uint32_t selector) const {
        return !currently_selected(selector) && previously_selected(selector);
    }

private:
    std::uint32_t previous_ = 0;
    std::uint32_t current_ = 0;
};

}

#endif
