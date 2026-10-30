#ifndef HD2_FRONTEND_SCREEN_STACK_H
#define HD2_FRONTEND_SCREEN_STACK_H

#include "frontend/screen_registry.h"

#include <stdexcept>
#include <vector>

namespace hd2::frontend {

// Bounded 0066c950 lifecycle branches, with native borrowed registration
// pointers instead of the original global segmented deque and object ABI.
// Backend supplies registered-screen availability and activate/deactivate hooks.
// Registry and its registrations outlive this stack, including previous_.
template<class Backend, class Registry = MenuScreenRegistry>
class ScreenStack {
public:
    explicit ScreenStack(Backend& backend, const Registry& registry = default_menu_screen_registry)
        : backend_(backend), registry_(registry) {}
    void seed(std::uint32_t index) {
        if (!screens_.empty()) throw std::logic_error("Screen stack already seeded");
        screens_.push_back({index, &resolve(index)});
    }
    std::size_t size() const { return screens_.size(); }
    const ScreenRegistration* current() const { return screens_.empty() ? nullptr : screens_.back().screen; }
    const ScreenRegistration* previous() const { return previous_; }
    std::uint32_t current_index() const {
        if (!current()) throw std::logic_error("Empty screen stack");
        return screens_.back().index;
    }
    // 02000001 hides current before stacking a regular screen; 02000002
    // leaves it visible and activates the new screen with focus 10200000.
    // Validate unsupported native bindings before changing any live state.
    void push(std::uint32_t index, bool modal) {
        push_with_focus(index, modal, modal ? 0x10200000u : 0u);
    }
    // 02000096 saves the previous screen and stacks registry slot zero, but
    // activates with focus zero (unlike the regular 02000002 modal branch).
    void push_runtime_confirmation() { push_with_focus(0, true, 0); }
    // 02000097 checks the current screen's ID family, not its registry index.
    // Remove/hide only that screen, clear +f8 and DO NOT reactivate underneath.
    void dismiss_runtime_confirmation() {
        if (!current() || (current()->identifier & 0xfc000000u) != 0) return;
        backend_.deactivate_screen(*current());
        screens_.pop_back();
        previous_ = nullptr;
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
    void push_with_focus(std::uint32_t index, bool modal, std::uint32_t focus) {
        const auto& next = resolve(index);
        if (modal && !current()) throw std::logic_error("Modal push requires a current screen");
        if (!modal && current()) backend_.deactivate_screen(*current());
        // Regular selection jumps directly to 0066d561, bypassing the +f8
        // store at 0066d55b. Only modal branches save the previous screen.
        if (modal) previous_ = current();
        screens_.push_back({index, &next});
        backend_.activate_screen(next, focus);
    }
    const ScreenRegistration& resolve(std::uint32_t index) const {
        if (index >= registry_.size() || !backend_.screen_available(index)) {
            throw std::logic_error("Menu screen has no reconstructed runtime binding");
        }
        return registry_.registration(index);
    }
    Backend& backend_;
    const Registry& registry_;
    struct Entry { std::uint32_t index; const ScreenRegistration* screen; };
    std::vector<Entry> screens_;
    const ScreenRegistration* previous_ = nullptr;
};

}

#endif
