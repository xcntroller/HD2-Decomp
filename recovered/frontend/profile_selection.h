#ifndef HD2_FRONTEND_PROFILE_SELECTION_H
#define HD2_FRONTEND_PROFILE_SELECTION_H

#include "frontend/profile_load.h"
#include "frontend/profile_settings.h"
#include "frontend/startup_configuration.h"

namespace hd2::frontend {

// Native composition of SAME application+20 profile owner. The application+08
// startup-name owner below is distinct; it must survive profile clearing/loading.
struct ProfileState {
    ProfileDirectories directories;
    ProfileOptions options;
    ProfilePayload payload;
};

// Explicit shared storage for 00868A2C..00868E28 inclusive. Wider consumers
// remain open; passing the owner keeps the clear observable to later startup.
struct ProfileSelectionWords { std::array<std::uint32_t, 256> words{}; };

// 006AF1D0 retains the candidate before clearing, restores executable cwd even
// after failed chdir, clears all 256 words, loads/applies options then profile,
// ignores both results and returns true. Native exceptions are not success.
template<class Files, class Platform, class Services, class Backend, class Sound, class Control>
bool select_profile(ProfileState& profile, const char* requested, ProfileSelectionWords& selection,
                     Files& files, Platform& platform, bool wide,
                     input::BindingInputState& input, Services& services,
                     Backend& backend, Sound& sound, Control& control, std::uint8_t& input_flag,
                     const GameData* game, const LocalizedTextCache& text) {
    const std::string retained(requested ? requested : "");
    const auto candidate = profile.directories.resolve(retained, wide);
    profile.directories.current_name.clear();
    if (!candidate.empty() && platform.change_to_directory("PlayersProfiles\\" + candidate))
        profile.directories.current_name = retained;
    platform.restore_executable_directory();
    for (auto count = selection.words.size(); count != 0; --count) selection.words[count - 1] = 0;
    load_and_apply_profile_options(profile.options, profile.directories, files, wide,
                                   input, services, backend, sound, control, input_flag);
    load_profile(profile.payload, profile.directories, files, platform, game, text, wide);
    return true;
}

// 006B6620: scan flag at application+30, selection through application+20,
// initial name queried AFTER the optional scan through application+08.
template<class Files, class Platform, class Services, class Backend, class Sound, class Control>
void prepare_startup_profile(bool scan, const StartupProfileName& startup, ProfileState& profile,
                              ProfileSelectionWords& selection, Files& files, Platform& platform,
                              bool wide, input::BindingInputState& input, Services& services,
                              Backend& backend, Sound& sound, Control& control, std::uint8_t& input_flag,
                              const GameData* game, const LocalizedTextCache& text) {
    if (scan) profile.directories.scan(platform, files, wide);
    select_profile(profile, startup.current_profile().c_str(), selection, files, platform, wide,
                   input, services, backend, sound, control, input_flag, game, text);
}

// 006B6670 selects through application+20, then persists the requested spelling
// through application+08 only on selection success. This is distinct from
// 006B6620: normal startup selection does not write game.cfg. Native retains
// input across selection to guard an aliased caller string becoming invalid.
template<class Files, class Platform, class Services, class Backend, class Sound, class Control>
bool select_and_remember_profile(StartupConfiguration& startup, ProfileState& profile,
                                 const char* requested, ProfileSelectionWords& selection,
                                 Files& files, Platform& platform, bool wide,
                                 input::BindingInputState& input, Services& services,
                                 Backend& backend, Sound& sound, Control& control, std::uint8_t& input_flag,
                                 const GameData* game, const LocalizedTextCache& text) {
    const std::string retained(requested ? requested : "");
    if (!select_profile(profile, retained.c_str(), selection, files, platform, wide,
                         input, services, backend, sound, control, input_flag, game, text)) return false;
    startup.set_profile(files, retained);
    return true;
}

}
#endif
