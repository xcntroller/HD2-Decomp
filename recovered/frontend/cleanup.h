#ifndef HD2_FRONTEND_CLEANUP_H
#define HD2_FRONTEND_CLEANUP_H

#include <exception>
#include <utility>

namespace hd2::frontend {

// Native exception safety, not an original successful service implementation.
// Each operation owns its acquisition checks and consumes its release once.
class CleanupErrors {
public:
    explicit CleanupErrors(std::exception_ptr first = {}) noexcept : first_(std::move(first)) {}
    template<class Operation> void attempt(Operation&& operation) noexcept {
        try { operation(); }
        catch (...) { if (!first_) first_ = std::current_exception(); }
    }
    bool failed() const noexcept { return first_ != nullptr; }
    void rethrow() const { if (first_) std::rethrow_exception(first_); }
private:
    std::exception_ptr first_;
};

// Reached native ownership corresponding to006DCAE0/006DC3A0. Unacquired
// original global systems are not represented by successful empty callbacks.
template<class Owners>
void close_launch_owners(Owners& owners, CleanupErrors& errors) noexcept {
    errors.attempt([&] { owners.restore_window_hook(); });
    errors.attempt([&] { owners.close_application(); });
    errors.attempt([&] { owners.close_consumers(); });
    errors.attempt([&] { owners.save_window(); });
    errors.attempt([&] { owners.close_3d(); });
    errors.attempt([&] { owners.close_sound(); });
    errors.attempt([&] { owners.close_graphics(); });
    errors.attempt([&] { owners.close_archives(); });
}

}
#endif
