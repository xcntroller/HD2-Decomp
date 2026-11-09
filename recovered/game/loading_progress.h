#ifndef HD2_GAME_LOADING_PROGRESS_H
#define HD2_GAME_LOADING_PROGRESS_H

#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <utility>

namespace hd2::game {

//006C95D0, tables00842F68/00842FB0. The six entries contain a base, scale
//and label pointer; only the first two values participate in progress math.
class LoadingProgress {
public:
    void reset(bool includes_activation) { value_ = 0; activation_ = includes_activation; }
    template<class Draw>
    void update(unsigned stage, float fraction, bool active, Draw&& draw) {
        if (!active) return; //006CA2C0's real loading-screen availability gate.
        if (stage >= 6 || !std::isfinite(fraction))
            throw std::runtime_error("Invalid loading progress callback");
        const auto& entry = (activation_ ? activation_stages : load_stages)[stage];
        const auto next = entry[1] * fraction + entry[0];
        // Original has no clamp; a new frame requires a STRICT .003 advance.
        if (value_ + 0.003f < next) { value_ = next; draw(value_); }
    }
    template<class Draw>
    void scene_callback(std::uint32_t event, float fraction, bool active, Draw&& draw) {
        //006D4C70's event10/20 paths leave progress untouched.
        if (event != 0x10 && event != 0x20) update(0, fraction, active, std::forward<Draw>(draw));
    }
    float value() const { return value_; }
private:
    inline static constexpr std::array<std::array<float, 2>, 6> load_stages{{
        {{0, .3f}}, {{.3f, .5f}}, {{.8f, .05f}}, {{.85f, .05f}}, {{.9f, .1f}}, {{1, 0}}
    }};
    inline static constexpr std::array<std::array<float, 2>, 6> activation_stages{{
        {{0, .15f}}, {{.15f, .25f}}, {{.4f, .05f}}, {{.45f, .05f}}, {{.5f, .05f}}, {{.55f, .45f}}
    }};
    float value_ = 0;
    bool activation_ = false;
};

} // namespace hd2::game
#endif
