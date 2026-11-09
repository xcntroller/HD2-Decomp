#ifndef HD2_NETWORKING_HEADLESS_APPLICATION_DATA_H
#define HD2_NETWORKING_HEADLESS_APPLICATION_DATA_H

#include "networking/headless_profile.h"
#include "networking/headless_startup_data.h"
#include "networking/server_initial_settings.h"
#include <memory>

namespace hd2::networking {

//006F71B0/006F8DD0's recovered startup DATA subset: +04 GameData, +08 startup
// configuration, +20 profile, +10 HumanStore and +24 cold configuration fields.
// Other original managers (+18/+1C/+14/+28/+0C), activation/module registry,
// controller, engine and live-session ownership are NOT substituted here.
// Byte readers/directory must already be available: game.cfg loads immediately,
// before profile construction and before later archive/resource setup.
class HeadlessApplicationData {
public:
    template<class Files, class Directory>
    HeadlessApplicationData(Files& files, Directory& directory,
                            CommandFiles& command_files, CommandFiles& catalogue_files) {
        game_ = std::make_unique<frontend::GameData>();
        configuration_ = std::make_unique<frontend::StartupConfiguration>();
        configuration_->load(files, directory); //006E8F70, not deferred to selection.
        profile_ = std::make_unique<HeadlessProfile>();
        // Original fresh profile has word34=0, eight save words=-1 and settings
        // defaults, but does not execute the later payload reset006F0670.
        for (std::size_t at = 0x74 / 4; at <= 0x90 / 4; ++at)
            profile_->payload.words[at] = 0xffffffffu;
        humans_ = std::make_unique<HeadlessHumanStore>();
        // Single authoritative cold field owner for command dispatch and its
        // consumers. Construction does not read a catalogue or create a host.
        commands_ = std::make_unique<ColdServerCommands>(initial_server_command_settings(),
                                                        command_files, catalogue_files);
    }
    ~HeadlessApplicationData() { close(); }
    HeadlessApplicationData(const HeadlessApplicationData&) = delete;
    HeadlessApplicationData& operator=(const HeadlessApplicationData&) = delete;

    void close() noexcept {
        // Preserve the relative +04,+08,+20,+10,+24 order in006F8DD0. Normal
        // close is idempotent. Native constructor failure also uses RAII.
        game_.reset();
        configuration_.reset();
        profile_.reset();
        humans_.reset();
        commands_.reset();
    }
    bool open() const noexcept { return static_cast<bool>(game_); }
    frontend::GameData& game() { return required(game_); }
    frontend::StartupConfiguration& configuration() { return required(configuration_); }
    HeadlessProfile& profile() { return required(profile_); }
    HeadlessHumanStore& humans() { return required(humans_); }
    ColdServerCommands& commands() { return required(commands_); }
    const ServerStartupSettings& settings() const { return required(commands_).settings(); }

    // Data preparation precedes profile selection and retains the actual
    // preselection profile directory for HumanStore's optional overlay.
    template<class Files>
    bool prepare_application(Files& files, const frontend::LocalizedTextCache& text, bool wide) {
        return prepare_headless_startup_data(game(), humans(), files, text,
            [&] { return profile().directories.current_directory(wide); }, wide);
    }
    template<class Files, class Platform, class Backend>
    void prepare_profile(HeadlessProfileSelection& selection, Files& files, Platform& platform,
                          bool wide, Backend& backend, const frontend::LocalizedTextCache& text) {
        prepare_headless_startup_profile(configuration(), profile(), selection, files,
                                         platform, wide, backend, &game(), text);
    }
private:
    template<class T> static T& required(const std::unique_ptr<T>& value) {
        if (!value) throw std::logic_error("Headless application data is closed");
        return *value;
    }
    // Declaration order also supplies native failure cleanup; normal retirement
    // follows close(), not the automatic reverse member order.
    std::unique_ptr<frontend::GameData> game_;
    std::unique_ptr<frontend::StartupConfiguration> configuration_;
    std::unique_ptr<HeadlessProfile> profile_;
    std::unique_ptr<HeadlessHumanStore> humans_;
    std::unique_ptr<ColdServerCommands> commands_;
};

}
#endif
