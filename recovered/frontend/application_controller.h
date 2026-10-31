#ifndef HD2_FRONTEND_APPLICATION_CONTROLLER_H
#define HD2_FRONTEND_APPLICATION_CONTROLLER_H

#include "frontend/opening_video.h"
#include "frontend/cleanup.h"
#include <cstring>
#include <memory>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace hd2::frontend {

// 006c72b0: expose the selected vector's begin pointer without consuming it.
// An empty vector leaves the caller's output untouched; only the low mode bit
// selects the second buffer. Native storage replaces the original vector ABI.
inline std::int32_t query_controller_input(const std::vector<std::uint32_t>& first,
                                          const std::vector<std::uint32_t>& second,
                                          const std::uint32_t*& output,
                                          std::uint8_t mode) {
    const auto& selected = (mode & 1u) ? second : first;
    if (selected.empty()) return -1;
    if (selected.size() > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))
        throw std::length_error("Controller input buffer exceeds signed count range");
    output = selected.data();
    return static_cast<std::int32_t>(selected.size());
}

// Native representation of the shared controller paths below, not its original
// object layout. State methods correspond to the original state vtable slots:
// initialize +04, shutdown +08, input +0c, fixed_update +10, update +14,
// render +18, type +1c, status +20, after_update +30. Video-specific fields
// and begin_video are supplied by the concrete reconstructed video state.
//
// Services retain the original prerequisite boundaries, not successful stubs:
// prepare_application=006b65d0, reset_profile=006b6620,
// alternate_start=006c0af0, query_input=006c72b0,
// delay_elapsed=006b32a0, finish_frame=0072b1c0,
// finish_fixed_frame=0072a0b0, plus flags, mouse initialization and rand.
// Concrete service bodies and an engine-backed State are not supplied here.
template<class State, class Services>
class ApplicationController {
public:
    explicit ApplicationController(Services& services) : services_(services) {}
    ApplicationController(const ApplicationController&) = delete;
    ApplicationController& operator=(const ApplicationController&) = delete;

    State* active() const { return active_.get(); }
    void install(std::unique_ptr<State> state) { active_ = std::move(state); }
    std::unique_ptr<State> detach() { return std::move(active_); }
    std::uint32_t phase() const { return phase_; }
    std::uint32_t accumulated_time() const { return accumulated_time_; }
    std::uint32_t random_seed() const { return random_seed_; }
    void suspend(bool value) { suspended_ = value; }
    void delay(std::uint32_t value) { delay_ = value; }
    std::uint32_t delay() const { return delay_; }

    //006C0920: state shutdown, actual loaded cache clear, optional state
    // deletion, then mouse close. Search directories survive the cache clear.
    void shutdown() {
        CleanupErrors errors;
        errors.attempt([&] { if (active_) required().shutdown(); });
        errors.attempt([&] { if (retained_) retained_->shutdown(); });
        retained_.reset();
        errors.attempt([&] { services_.clear_controller_resources(); });
        if (!reinitialize_ || errors.failed()) active_.reset();
        errors.attempt([&] { if (services_.application_flags() & 2u) services_.close_mouse(); });
        errors.rethrow();
    }

    // 006c0820 normal/reinitialize startup. Original deliberately ignores the
    // initial-state helper's result, including alternate-start success.
    bool start(bool reinitialize) {
        accumulated_time_ = 0;
        reinitialize_ = reinitialize;
        if (active_ && runnable()) return false;
        if (services_.application_flags() & 2u) services_.initialize_mouse(1);
        if (!reinitialize_) {
            if (!services_.prepare_application()) return false;
            services_.reset_profile();
            initial_video();
        } else {
            required().initialize(nullptr);
        }
        const std::uint32_t high = services_.random_word();
        const std::uint32_t low = services_.random_word();
        random_seed_ = (high << 16) | low;
        return true;
    }

    // 006c0ca0 normal path. Alternate service owns its command-line handling.
    bool initial_video() {
        if (services_.alternate_start(*this)) return false;
        // Original alternate handler clears +14 before its normal return.
        // Reject an invalid native owner rather than reproducing its raw leak.
        if (active_) throw std::logic_error("Normal startup requires an empty controller");
        install(require_created(services_.create_video_state()));
        const auto first = normal_opening_start();
        required().begin_video(first.filename, first.sequence_index);
        return true;
    }

    // Null-descriptor branch of 006c0ef0: shutdown, release, construct, install,
    // then initialize(nullptr). Its initializer result is intentionally ignored.
    bool enter_frontend() {
        if (active_) {
            required().shutdown();
            active_.reset(); // Fresh owner after a shutdown callback may replace it.
        }
        install(require_created(services_.create_frontend_state()));
        required().initialize(nullptr);
        return true;
    }

    // 006c31d0 reads BOTH buffer views even if the first query fails. The
    // returned words are begin pointers, not individual scan/character words.
    void dispatch_input() {
        const std::uint32_t* first = nullptr;
        const std::uint32_t* second = nullptr;
        const auto first_count = services_.query_input(first, 0);
        const auto second_count = services_.query_input(second, 1);
        if (second_count >= 0 && first_count >= 0) required().input(first, second);
    }

    // Shared prefix and normal video-completion branch of 006c20f0. Other
    // descriptor/game transitions remain explicit errors.
    bool update(std::uint32_t milliseconds) {
        phase_ = 1;
        if (signed_word(delay_) > 0) {
            delay_ -= milliseconds;
            if (signed_word(delay_) <= 0) {
                delay_ = 0;
                services_.delay_elapsed();
            }
        }
        if (required().status() == 4) return finish_frame(false);
        if (!active_ || !runnable() || suspended_) return finish_frame(true);
        dispatch_input();
        required().update(milliseconds);
        required().after_update();
        if (required().status() == 0) return finish_frame(true);
        const auto status = required().status();
        if (status == 12) {
            enter_credits();
        } else if (status == 13) {
            video_completion();
        } else if (status == 4) {
            shutdown();
            services_.reset_control(); // Actual0072E4E0(1), supported cold branch.
            return finish_frame(false);
        } else if (status == 2 || status == 3 ||
                   (status >= 7 && status <= 17)) {
            throw std::logic_error("Unrecovered application state transition");
        }
        return finish_frame(true);
    }

    // 006c3100 subtracts 100 ONCE, not a catch-up loop. Signed x86 comparisons
    // apply to the wrapping 32-bit accumulator. Pause still consumes the tick.
    bool fixed_update(std::uint32_t milliseconds) {
        accumulated_time_ += milliseconds;
        if (signed_word(accumulated_time_) < 100) return true;
        accumulated_time_ -= 100;
        phase_ = 2;
        if (required().status() == 4) return finish_fixed_frame(false);
        if (active_ && runnable() && !suspended_) required().fixed_update(100);
        return finish_fixed_frame(true);
    }

    // 006c3190 does not add the controller's suspended flag as a render gate.
    void render() {
        if (active_ && runnable()) required().render();
    }

private:
    static std::int32_t signed_word(std::uint32_t value) {
        std::int32_t result;
        std::memcpy(&result, &value, sizeof(result));
        return result;
    }
    static std::unique_ptr<State> require_created(std::unique_ptr<State> state) {
        if (!state) throw std::runtime_error("Application state allocation failed");
        return state;
    }
    State& required() const {
        if (!active_) throw std::logic_error("Application controller has no active state");
        return *active_;
    }
    bool runnable() const {
        // Fresh pointer and status each time, matching the original virtual calls.
        return required().status() == 1 || required().status() == 6 || required().status() == 5;
    }
    //006C20F0 status12: retain the frontend without shutdown or reconstruction.
    void enter_credits() {
        if (required().type()!=2 || retained_)
            throw std::logic_error("Credits requires one retained frontend");
        auto video=require_created(services_.create_credits_state());
        retained_=std::move(active_);
        active_=std::move(video);
        required().begin_video("credits.bik",0);
        required().status(1);
    }
    void video_completion() {
        if (required().type() != 10) return;
        const auto sequence = required().video_sequence();
        if (sequence==0) {
            if (!retained_) throw std::logic_error("Video has no retained frontend");
            auto video=std::move(active_);
            active_=std::move(retained_); // Restore BEFORE video shutdown/deletion.
            video->shutdown();
            video.reset();
            required().status(1); // Original retained owner virtual+24(1).
            return;
        }
        const auto next = normal_opening_completion(sequence);
        if (next.action == OpeningVideoAction::enter_frontend) {
            enter_frontend();
        } else if (next.action != OpeningVideoAction::none) {
            required().shutdown();
            required().begin_video(next.filename, next.sequence_index);
        } else if (sequence == 0 || sequence == 5 || sequence == 6) {
            throw std::logic_error("Unrecovered retained/restart/exit video transition");
        }
    }
    bool finish_frame(bool keep_running) {
        const auto flags = services_.application_flags();
        if ((flags & 2u) && (flags & 7u) != 7u) services_.finish_frame(0, 0, 0, 1);
        phase_ = 0;
        return keep_running;
    }
    bool finish_fixed_frame(bool keep_running) {
        const auto flags = services_.application_flags();
        if ((flags & 1u) && (flags & 7u) != 7u) services_.finish_fixed_frame(0, 0, 0, 0, 1);
        phase_ = 0;
        return keep_running;
    }
    Services& services_;
    std::unique_ptr<State> active_, retained_;
    std::uint32_t accumulated_time_ = 0, delay_ = 0, phase_ = 0, random_seed_ = 0;
    bool reinitialize_ = false, suspended_ = false;
};

} // namespace hd2::frontend
#endif
