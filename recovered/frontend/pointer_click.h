#pragma once

#include <cstdint>

namespace hd2::frontend {

class PointerClick {
public:
    using Target = std::uint32_t;

    void press(Target target) { pressed_ = target; }

    bool release(Target target) {
        const bool matches = pressed_ == target;
        pressed_ = 0;
        return matches;
    }

    void clear() { pressed_ = 0; }
    Target pressed() const { return pressed_; }

private:
    Target pressed_ = 0;
};

} // namespace hd2::frontend
