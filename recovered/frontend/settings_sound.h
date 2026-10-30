#ifndef HD2_FRONTEND_SETTINGS_SOUND_H
#define HD2_FRONTEND_SETTINGS_SOUND_H

#include <array>
#include <cstdint>

namespace hd2::frontend {

// Bounded fields of client singleton00899728, NOT the LS3DF sound singleton.
// Caller supplies the existing manager state and live gate00982100. Its full
// constructor/movie ownership remains a prerequisite for controller integration.
struct SettingsSoundState {
    float driver_volume; // +3C; final constructor does not initialize this word.
    float movie_volume;  // +44; final constructor writes 1.
    void* movie;        // +48; final constructor writes null.
    float movie_scale;  // +54; final constructor writes 1.
};

template<class Backend>
class SettingsSound {
public:
    SettingsSound(SettingsSoundState& state, const std::uint32_t& gate, Backend& backend)
        : state_(state), gate_(gate), backend_(backend) {}
    // 00433550: gate clear leaves BOTH stored value and backend untouched.
    float driver_volume(float value) {
        if (gate_ & 2) {
            state_.driver_volume = value;
            backend_.sound_driver_volume(state_.driver_volume);
        }
        return state_.driver_volume;
    }
    // 004323E0 / 00433510. Re-resolve movie after the opened callback as the
    // original does. This channel controls the active movie, not global music.
    bool movie_opened() {
        return state_.movie && backend_.movie_opened(state_.movie);
    }
    float movie_volume(float value) {
        if (gate_ & 2) {
            state_.movie_volume = value;
            if (movie_opened()) backend_.movie_volume(state_.movie,
                static_cast<float>(static_cast<long double>(state_.movie_scale) * state_.movie_volume));
        }
        return state_.movie_volume;
    }
private:
    SettingsSoundState& state_;
    const std::uint32_t& gate_;
    Backend& backend_;
};

using SettingsControlBlock = std::array<std::uint32_t, 8>;

// Bounded 00742210 wrapper+04. Interface lifetime belongs to its actual creator;
// this view borrows it, and does not create or silently initialize a substitute.
struct SettingsControlInterface { void* object; };
template<class Backend>
std::int32_t submit_settings_control(SettingsControlInterface& wrapper,
                                     SettingsControlBlock& block, Backend& backend) {
    if (!wrapper.object) return static_cast<std::int32_t>(-2147221008); // 800401F0
    const auto result = backend.submit_settings_control(wrapper.object, block);
    return result < 0 ? result : 0;
}

// Owner0097D070+0C and 0072EA20. Both absent-owner and absent-interface branches
// are real, separate cases. Initialization and semantic name are still open.
template<class Backend>
class SettingsControl {
public:
    SettingsControl(SettingsControlInterface*& wrapper, Backend& backend)
        : wrapper_(wrapper), backend_(backend) {}
    void set_value(std::uint32_t value) {
        if (!wrapper_) return;
        SettingsControlBlock block{{0x20,0x28,1,value,0xfffffffeu,0,0,0}};
        submit_settings_control(*wrapper_, block, backend_); // Original ignores HRESULT.
    }
private:
    SettingsControlInterface*& wrapper_;
    Backend& backend_;
};

}
#endif
