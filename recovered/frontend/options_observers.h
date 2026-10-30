#ifndef HD2_FRONTEND_OPTIONS_OBSERVERS_H
#define HD2_FRONTEND_OPTIONS_OBSERVERS_H

#include "frontend/options_widget_base.h"

namespace hd2::frontend {

enum class OptionsFloatSetting { gamma, sound_volume, music_volume, mouse_sensitivity, voice_volume };
enum class OptionsByteSetting { invert_mouse, subtitles, parental_lock };

inline std::uint32_t options_screen_family(std::uint8_t screen) {
    return static_cast<std::uint32_t>(screen) << 26;
}

// 006202a0. Query the sender's inner slider before identifying its setting.
// Native backend performs typed writes/apply calls instead of accessing original
// global preference records. This helper does not reconstruct those services.
template<class Backend, class Sender>
bool options_slider_observer(Backend& backend, std::uint8_t screen, Sender* sender) {
    if (!sender) return false;
    ControlEvent query{0, 0x01000022, 0, 0, 0, {}};
    sender->dispatch(query);
    const auto family = options_screen_family(screen);
    const auto id = sender->identifier();
    OptionsFloatSetting setting;
    if (id == (family | 0x00604000)) setting = OptionsFloatSetting::gamma;
    else if (id == (family | 0x00706000)) setting = OptionsFloatSetting::sound_volume;
    else if (id == (family | 0x00707000)) setting = OptionsFloatSetting::music_volume;
    else if (id == (family | 0x00806000)) setting = OptionsFloatSetting::mouse_sensitivity;
    else if (id == (family | 0x00b06000)) setting = OptionsFloatSetting::voice_volume;
    else return false;
    float value = widget_float_word(query.word_0c);
    if (setting == OptionsFloatSetting::music_volume && !(value >= 1.0f)) value = 0;
    backend.set_float(setting, value);
    backend.apply_float(setting);
    if (setting == OptionsFloatSetting::sound_volume) backend.play_click_sound();
    return true;
}

// 00620440. Full argument is truncated to the original stored byte. Only
// invert-mouse requests an immediate application helper in this callback.
template<class Backend, class Sender>
bool options_toggle_observer(Backend& backend, std::uint8_t screen, Sender* sender, std::uint32_t argument) {
    if (!sender) return false;
    const auto family = options_screen_family(screen);
    const auto id = sender->identifier();
    OptionsByteSetting setting;
    if (id == (family | 0x00804000)) setting = OptionsByteSetting::invert_mouse;
    else if (id == (family | 0x00a02000)) setting = OptionsByteSetting::subtitles;
    else if (id == (family | 0x00a03000)) setting = OptionsByteSetting::parental_lock;
    else return false;
    backend.set_byte(setting, static_cast<std::uint8_t>(argument));
    if (setting == OptionsByteSetting::invert_mouse) backend.apply_mouse();
    return true;
}

// 00620720. This observer ignores sender identity. The stored low byte and
// the full-word zero test deliberately have different behavior for 0x100.
template<class Backend>
bool options_voice_observer(Backend& backend, std::uint8_t screen, std::uint32_t argument,
                            ControlEventDestination& queue) {
    backend.set_voice_enabled(static_cast<std::uint8_t>(argument));
    ControlEvent event{options_screen_family(screen) | 0x00b06000,
                       argument ? 0x01000002u : 0x01000001u, 0, 0, 0, {}};
    queue.enqueue(event);
    return true;
}

// 0061fca0: use only sender bits 12..19, as the original does. Preset changes
// queue seven silent value updates, then seven availability changes, then apply.
// Direct feature changes use the service helper; unknown ordinals are handled.
template<class Backend, class Sender>
bool options_graphics_observer(Backend& backend, std::uint8_t screen, Sender* sender,
                               std::uint32_t argument, ControlEventDestination& queue) {
    if (!sender) return false;
    const auto ordinal = (sender->identifier() >> 12) & 0xff;
    if (ordinal == 5) {
        if (argument < 4) backend.set_geometry_detail(argument);
        const auto family = options_screen_family(screen);
        for (std::uint32_t feature = 0; feature < 7; ++feature) {
            ControlEvent event{family | 0x00600000u | ((feature + 6) << 12),
                               0x01000076, static_cast<std::uint8_t>(backend.graphics_feature(feature)), 0, 0, {}};
            queue.enqueue(event);
        }
        const bool custom = backend.geometry_detail() == 3;
        for (std::uint32_t feature = 0; feature < 7; ++feature) {
            ControlEvent event{family | 0x00600000u | ((feature + 6) << 12),
                               custom ? 0x01000002u : 0x01000001u, 0, 0, 0, {}};
            queue.enqueue(event);
        }
        backend.apply_graphics();
    } else if (ordinal >= 6 && ordinal <= 12) {
        backend.set_graphics_feature(ordinal - 6, argument);
    }
    return true;
}

}

#endif
