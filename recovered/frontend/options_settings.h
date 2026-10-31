#ifndef HD2_FRONTEND_OPTIONS_SETTINGS_H
#define HD2_FRONTEND_OPTIONS_SETTINGS_H

#include "frontend/options_observers.h"
#include "frontend/profile_settings.h"
#include <functional>

namespace hd2::frontend {

//006B6740 resolves CURRENT application+20 settings, not a menu-local default.
// Services owns the profile and actual apply helpers. Unused options controls
// remain source-only until their engine construction/screen flow is delivered.
template<class Services>
class OptionsSettings {
public:
    OptionsSettings(Services& services, std::function<void()> click)
        : services_(services), click_(std::move(click)) {}
    std::uint32_t geometry_detail() { return services_.profile_settings().word(0x18); }
    void set_geometry_detail(std::uint32_t value) { services_.profile_settings().word(0x18, value); }
    std::uint8_t graphics_feature(std::size_t index) { return services_.profile_settings().quality_byte(index); }
    void set_graphics_feature(std::size_t index, std::uint32_t value) {
        if (index >= 7) throw std::out_of_range("Invalid profile graphics feature");
        auto& settings = services_.profile_settings();
        const auto detail = settings.word(0x18);
        settings.bytes[(detail < 3 ? 0x1c + detail * 7 : 0x31) + index] = static_cast<std::uint8_t>(value);
        //006B3380 only stores; it does not select custom mode or apply.
    }
    float get_float(OptionsFloatSetting setting) { return services_.profile_settings().real(offset(setting)); }
    void set_float(OptionsFloatSetting setting, float value) {
        services_.profile_settings().word(offset(setting), widget_float_bits(value));
    }
    void set_byte(OptionsByteSetting setting, std::uint8_t value) {
        const auto offset = setting == OptionsByteSetting::invert_mouse ? 0x39 :
                            setting == OptionsByteSetting::subtitles ? 0x3a : 0x3b;
        services_.profile_settings().bytes[offset] = value;
    }
    void set_voice_enabled(std::uint8_t value) { services_.profile_settings().bytes[0x38] = value; }
    void apply_float(OptionsFloatSetting setting) { services_.apply_option(setting); }
    void apply_mouse() { services_.apply_option(OptionsFloatSetting::mouse_sensitivity); }
    void apply_graphics() { services_.apply_graphics_options(); }
    void play_click_sound() { click_(); }
private:
    static std::size_t offset(OptionsFloatSetting setting) {
        switch (setting) {
        case OptionsFloatSetting::mouse_sensitivity: return 0;
        case OptionsFloatSetting::gamma: return 4;
        case OptionsFloatSetting::sound_volume: return 8;
        case OptionsFloatSetting::music_volume: return 12;
        case OptionsFloatSetting::voice_volume: return 16;
        }
        throw std::logic_error("Unknown profile float setting");
    }
    Services& services_;
    std::function<void()> click_;
};

}
#endif
