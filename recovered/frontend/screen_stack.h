#ifndef HD2_FRONTEND_SCREEN_STACK_H
#define HD2_FRONTEND_SCREEN_STACK_H

#include "frontend/screen_registry.h"

#include <stdexcept>
#include <vector>

namespace hd2::frontend {

// Bounded 0066c950 lifecycle branches, with native borrowed registration
// pointers instead of the original global segmented deque and object ABI.
// Backend supplies registered-screen availability and activate/deactivate hooks.
template<class Backend>
class ScreenStack {
public:
    explicit ScreenStack(Backend& backend) : backend_(backend) {}
    void seed(std::uint32_t index) {
        if (!screens_.empty()) throw std::logic_error("Screen stack already seeded");
        screens_.push_back(&resolve(index));
    }
    std::size_t size() const { return screens_.size(); }
    const ScreenRegistration* current() const { return screens_.empty() ? nullptr : screens_.back(); }
    const ScreenRegistration* previous() const { return previous_; }
    std::uint32_t current_index() const {
        if (!current()) throw std::logic_error("Empty screen stack");
        return static_cast<std::uint32_t>(current() - menu_screen_registry.data());
    }
    // 02000001 hides current before stacking a regular screen; 02000002
    // leaves it visible and activates the new screen with focus 10200000.
    // Validate unsupported native bindings before changing any live state.
    void push(std::uint32_t index, bool modal) {
        const auto& next = resolve(index);
        if (modal && !current()) throw std::logic_error("Modal push requires a current screen");
        if (!modal && current()) backend_.deactivate_screen(*current());
        // Regular selection jumps directly to 0066d561, bypassing the +f8
        // store at 0066d55b. Only the modal branch saves the previous screen.
        if (modal) previous_ = current();
        screens_.push_back(&next);
        backend_.activate_screen(next, modal ? 0x10200000u : 0u);
    }
    // 02000005 clears front-end +f8; the general 02000003 back branch does
    // not clear it. Neither operation destroys registry-owned screen objects.
    void pop(bool clear_previous) {
        if (!current()) return;
        backend_.deactivate_screen(*current());
        screens_.pop_back();
        if (clear_previous) previous_ = nullptr;
        if (current()) backend_.activate_screen(*current(), 0);
    }
    // 02000007/08 hide every stacked occurrence in LIFO order. Their caller
    // selects application state three/two; +f8 is not cleared in these branches.
    void clear() {
        while (current()) {
            backend_.deactivate_screen(*current());
            screens_.pop_back();
        }
    }
private:
    const ScreenRegistration& resolve(std::uint32_t index) const {
        if (index >= menu_screen_registry.size() || !backend_.screen_available(index)) {
            throw std::logic_error("Menu screen has no reconstructed runtime binding");
        }
        return menu_screen_registry[index];
    }
    Backend& backend_;
    std::vector<const ScreenRegistration*> screens_;
    const ScreenRegistration* previous_ = nullptr;
};

}

#endif
