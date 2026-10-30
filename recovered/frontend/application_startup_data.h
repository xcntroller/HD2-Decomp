#ifndef HD2_FRONTEND_APPLICATION_STARTUP_DATA_H
#define HD2_FRONTEND_APPLICATION_STARTUP_DATA_H

#include "frontend/application_data.h"
#include "frontend/profile_selection.h"

namespace hd2::frontend {

// Owning composition of the four already recovered startup data components.
// This is ONLY the application+04/+08/+20/+10 subset of 006B4A60/006B6010;
// remaining application owners, services, protection and full lifecycle remain
// unrecovered. It cannot stand in for the full original application object.
// Files/graphics/directory must be available when construction begins; original
// configuration loading happens before application resource/archive setup.
template<class Graphics>
class ApplicationStartupData {
public:
    template<class Files, class Directory>
    ApplicationStartupData(Graphics& graphics, Files& files, Directory& directory) {
        game_ = std::make_unique<GameData>();
        configuration_ = std::make_unique<StartupConfiguration>();
        configuration_->load(files, directory);
        profile_ = std::make_unique<ProfileState>();
        // Original profile construction initializes defaults directly (no
        // binding reset/installation), raw word34=0 and eight save words=-1.
        // Unwritten settings+14 follows the established native zero policy.
        initialize_profile_settings(profile_->options.settings);
        for (std::size_t at = 0x74 / 4; at <= 0x90 / 4; ++at)
            profile_->payload.words[at] = 0xffffffffu;
        humans_ = std::make_unique<HumanStore<Graphics>>(graphics);
    }
    ~ApplicationStartupData() {
        // 006B6010 destroys these four owners in this order, not reverse member
        // declaration order. Other original application owners are outside scope.
        game_.reset(); configuration_.reset(); profile_.reset(); humans_.reset();
    }
    ApplicationStartupData(const ApplicationStartupData&) = delete;
    ApplicationStartupData& operator=(const ApplicationStartupData&) = delete;
    GameData& game() { return *game_; }
    StartupConfiguration& configuration() { return *configuration_; }
    ProfileState& profile() { return *profile_; }
    HumanStore<Graphics>& humans() { return *humans_; }

    template<class Files>
    bool prepare_application(Files& files, const LocalizedTextCache& text, bool wide) {
        return prepare_application_data(*game_, *humans_, profile_->directories, files, text, wide);
    }
    template<class Files, class Platform, class Services, class Backend, class Sound, class Control>
    void reset_profile(bool scan, ProfileSelectionWords& selection, Files& files, Platform& platform,
                        bool wide, input::BindingInputState& input, Services& services,
                        Backend& backend, Sound& sound, Control& control, std::uint8_t& input_flag,
                        const LocalizedTextCache& text) {
        prepare_startup_profile(scan, *configuration_, *profile_, selection, files, platform, wide,
                                input, services, backend, sound, control, input_flag, game_.get(), text);
    }
private:
    std::unique_ptr<GameData> game_;
    std::unique_ptr<StartupConfiguration> configuration_;
    std::unique_ptr<ProfileState> profile_;
    std::unique_ptr<HumanStore<Graphics>> humans_;
};

}
#endif
