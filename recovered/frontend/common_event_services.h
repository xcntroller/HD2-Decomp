#ifndef HD2_FRONTEND_COMMON_EVENT_SERVICES_H
#define HD2_FRONTEND_COMMON_EVENT_SERVICES_H

#include "frontend/control_events.h"

#include <algorithm>
#include <stdexcept>

namespace hd2::frontend {

class FrontendCaptureOwner {
public:
    virtual ~FrontendCaptureOwner() = default;
    virtual bool release(std::uint32_t argument) = 0; // Original +64.
    virtual void acquire(std::uint32_t argument) = 0; // Original +68.
};

// Native storage for front-end +10/+2d/+11c/+130 protocol state. Tokens stand
// for original pointer identity in the two lists; no dereference/destruction is
// performed by either original erase helper (0066e2e0 / 00653930). Population
// of the first list belongs to its owning resource service.
struct CommonFrontendState {
    std::shared_ptr<FrontendCaptureOwner> capture_owner;
    bool motion_only = false;
    std::vector<std::uint32_t> resource_tokens;
    std::vector<std::uint32_t> notification_tokens;
};

// Remaining non-stack branches of 0066c950. Backend calls are explicit engine,
// application and input services. Reacquire state after external callbacks;
// native retained capture adapters survive replacement during their own calls.
template<class Services>
bool dispatch_common_frontend_service(CommonFrontendState& state, Services& services,
                                      ControlEvent& event) {
    const auto erase_first = [](auto& values, std::uint32_t token) {
        const auto found = std::find(values.begin(), values.end(), token);
        if (found != values.end()) values.erase(found);
    };
    const auto update_input_mode = [&] {
        services.set_application_input_enabled(!state.motion_only); // 006dcad0.
        services.set_input_motion_only(state.motion_only); // 0066a860.
    };
    switch (event.code & 0x03000fffu) {
    case 0x01000068: {
        const auto found = std::find(state.resource_tokens.begin(), state.resource_tokens.end(), event.word_0c);
        if (found == state.resource_tokens.end()) return true;
        const auto index = static_cast<std::size_t>(found - state.resource_tokens.begin());
        // 007dc460 acts on the found frame before the vector erase. It may
        // mutate resources; original rechecks size and erases the SAVED index.
        services.release_resource(state.resource_tokens[index]); // Zero is permitted.
        if (index < state.resource_tokens.size()) {
            state.resource_tokens.erase(state.resource_tokens.begin() + index);
        }
        return true;
    }
    case 0x010000a1:
        if (!services.has_current_screen() || !services.current_model()) return true;
        if ((event.argument & 0x04000000u) == 0) return true;
        if (event.argument == 0x04005000) {
            const auto model = services.current_model();
            const auto receiver = services.current_screen();
            const auto target = services.pick_model(model);
            // The receiver is saved BEFORE the picker, which may change stack.
            services.focus_screen(receiver, target);
        } else if (event.argument == 0x04004000) {
            const auto receiver = services.current_screen();
            services.leave_screen(receiver, 0);
        }
        return true;
    case 0x02000007:
    case 0x02000008: {
        const auto next_state = (event.code & 0x0fffu) == 7 ? 3u : 2u;
        services.clear_screens(); // Hide LIFO; preserve previous-screen pointer.
        services.select_application_state(next_state);
        return true;
    }
    case 0x02000009:
        services.dispatch_immediate({0, 0x02000068, 0, 0, 0, {}});
        services.select_application_state(10);
        return true;
    case 0x02000068:
        services.reset_resources(); // 006b6530 body remains a resource service.
        return true;
    case 0x0200007a:
        if (event.word_0c != 0 && std::find(state.notification_tokens.begin(),
                state.notification_tokens.end(), event.word_0c) == state.notification_tokens.end()) {
            state.notification_tokens.push_back(event.word_0c);
        }
        return true;
    case 0x0200007b:
        if (event.word_0c != 0) erase_first(state.notification_tokens, event.word_0c);
        return true;
    case 0x020000bf: {
        auto previous = state.capture_owner;
        if (!previous || previous->release(0)) {
            if (!event.capture_owner) throw std::invalid_argument("Capture acquire requires a typed native owner");
            state.capture_owner = event.capture_owner;
            auto installed = state.capture_owner;
            installed->acquire(0);
        }
        return true;
    }
    case 0x020000c0: {
        auto previous = state.capture_owner;
        if (previous) previous->release(0); // Result ignored, including false.
        if (state.capture_owner == event.capture_owner) {
            state.capture_owner.reset();
            state.motion_only = false;
            update_input_mode();
        }
        return true;
    }
    case 0x020000c2:
        state.motion_only = event.argument != 0;
        update_input_mode();
        return true;
    default:
        return false;
    }
}

}

#endif
