#ifndef HD2_FRONTEND_FRAME_CLOCK_H
#define HD2_FRONTEND_FRAME_CLOCK_H

#include <cstdint>

namespace hd2::frontend {

// 006dc950: unsigned low-32-bit microsecond subtraction precedes conversion
// to milliseconds. Maximum is checked first, including inverted bounds. Every
// branch rereads the clock for the next baseline; a waited frame returns the
// requested minimum, independently of timer/wait success or actual elapsed time.
template<class Clock>
class FrameClock {
public:
    explicit FrameClock(Clock& clock, std::uint32_t previous = 0)
        : clock_(clock), previous_(previous) {}

    std::uint32_t next(std::uint32_t minimum, std::uint32_t maximum) {
        const std::uint32_t now = clock_.microseconds();
        const auto elapsed = (now - previous_) / 1000u;
        std::uint32_t result = elapsed;
        if (maximum < elapsed) {
            result = maximum;
        } else if (elapsed < minimum) {
            clock_.wait_for_frame(minimum - elapsed);
            result = minimum;
        }
        previous_ = clock_.microseconds();
        return result;
    }
    std::uint32_t previous() const { return previous_; }

private:
    Clock& clock_;
    std::uint32_t previous_;
};

} // namespace hd2::frontend
#endif
