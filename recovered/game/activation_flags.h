#ifndef HD2_GAME_ACTIVATION_FLAGS_H
#define HD2_GAME_ACTIVATION_FLAGS_H

#include <stdexcept>

namespace hd2::game {

//006989B0's graphics flag scope: clear bit20 for activation and restore the
// captured value on both results. Native exception/failure paths also restore;
// no default backend or load-success value is supplied here.
template<class Graphics, class Activate>
auto with_activation_flags(Graphics& graphics, Activate activate) -> decltype(activate()) {
    const auto saved = graphics.activation_flags();
    if (graphics.activation_flags(saved & ~0x20u) < 0) {
        if (graphics.activation_flags(saved) < 0)
            throw std::runtime_error("Cannot restore mission graphics flags after setup failure");
        throw std::runtime_error("Cannot set mission graphics flags");
    }
    try {
        auto result = activate();
        if (graphics.activation_flags(saved) < 0)
            throw std::runtime_error("Cannot restore mission graphics flags");
        return result;
    } catch (...) {
        // A retry is also required if the first restoration itself failed.
        if (graphics.activation_flags(saved) < 0)
            throw std::runtime_error("Cannot restore mission graphics flags after activation failure");
        throw;
    }
}

} // namespace hd2::game
#endif
