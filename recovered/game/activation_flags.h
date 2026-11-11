#ifndef HD2_GAME_ACTIVATION_FLAGS_H
#define HD2_GAME_ACTIVATION_FLAGS_H

#include <stdexcept>
#include <exception>

namespace hd2::game {

//006989B0's graphics flag scope: clear bit20 for activation and restore the
// captured value on both results. Native exception/failure paths also restore;
// no default backend or load-success value is supplied here.
template<class Graphics, class Activate>
auto with_activation_flags(Graphics& graphics, Activate activate,
                           std::exception_ptr* restoration_failure=nullptr) -> decltype(activate()) {
    const auto saved = graphics.activation_flags();
    try {
        if (graphics.activation_flags(saved & ~0x20u) < 0)
            throw std::runtime_error("Cannot set mission graphics flags");
        auto result = activate();
        if (graphics.activation_flags(saved) < 0)
            throw std::runtime_error("Cannot restore mission graphics flags");
        return result;
    } catch (...) {
        const auto original=std::current_exception();
        //Also restore after a throwing setup call. Never replace the actual
        //load/activation error with a secondary restoration error. The concrete
        //lifecycle supplies the diagnostic sink; no successful setter is faked.
        try {
            if(graphics.activation_flags(saved)<0)
                throw std::runtime_error("Cannot restore mission graphics flags after activation failure");
        }catch(...){if(restoration_failure)*restoration_failure=std::current_exception();}
        std::rethrow_exception(original);
    }
}

} // namespace hd2::game
#endif
