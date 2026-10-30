#ifndef HD2_HOST_STARTUP_SESSION_H
#define HD2_HOST_STARTUP_SESSION_H

#include "frontend/application_startup_data.h"
#include "frontend/control_startup.h"
#include "frontend/input_setup.h"
#include "frontend/sound_manager_data.h"
#include "frontend/startup_modules.h"
#include "host/chunk_files.h"
#include "host/control_startup.h"
#include "host/graphics_input_services.h"
#include "host/human_store.h"
#include "host/joystick_backend.h"
#include "host/profile_directories.h"
#include "host/resource_directories.h"
#include "host/settings_services.h"
#include "host/startup_archives.h"
#include "host/startup_localization.h"
#include "input/input_manager.h"
#include <cstdio>

namespace hd2::host {

// Concrete composition of recovered startup components for a bounded startup
// executable. Construct BEFORE graphics initialization (game.cfg timing).
// This does not supply application/video/menu states or the binary wrapper VM.
// Original Win32 import targets are called directly; no protection success API.
class StartupSession {
public:
    StartupSession(HMODULE engine, HMODULE rw_data, HMODULE joy)
        : engine_(engine), files_(rw_data), text_files_(rw_data), directory_(platform_), language_(platform_),
          portraits_(engine), data_(portraits_, files_, directory_),
          archive_backend_(rw_data), archives_(archive_backend_),
          graphics_directories_(engine), caches_(graphics_directories_),
          joysticks_backend_(engine, joy), joysticks_(joysticks_backend_),
          input_backend_(engine, joysticks_backend_), input_(input_backend_),
          input_setup_(engine, joysticks_), settings_backend_(engine),
          profile_platform_(directory_), sound_data_(registry_), modules_(registry_) {}

    void before_devices() {
        if (phase_ != Phase::constructed) throw std::logic_error("Startup initialization repeated");
        frontend::initialize_control_startup(control_data_, gate_, lock_services_);
        phase_ = Phase::control_ready;
    }
    // Called only after real graphics, sound and I3D initialization. The caller
    // retains device ownership; close_consumers must run before device close.
    bool after_devices() {
        if (phase_ != Phase::control_ready) throw std::logic_error("Startup device order mismatch");
        phase_ = Phase::loading;
        auto random = [&] { return settings_backend_.sound_random_word(); };
        auto mask = io::begin_archive_key_mask(random);
        const auto selected_language = language_.get();
        archive_focus_ = GetFocus(); // Original006DB8C7 wrapper → real GetFocus.
        archives_.create(io::language_archive_name(selected_language));
        archives_.configure(mask, random);
        io::setup_resource_directory_trees(caches_, directory_platform_);
        using Window = HWND (WINAPI *)(EngineFrame*);
        auto* graphics = static_cast<EngineFrame*>(input_setup_.graphics());
        const auto window = scene_method<Window>(graphics, engine(), 0x48, 0x6a240)(graphics);
        directory_window_ = IsZoomed(window); // Original006DBF2B wrapper target.
        frontend::setup_graphics_input(input_, input_setup_, language_, text_, platform_, text_files_, directory_);
        modules_.initialize(gate_, files_); // Original006DBFB0 includes this phase.
        const auto wide = language_.state().wide_characters;
        const auto prepared = data_.prepare_application(files_, text_, wide);
        std::printf("Startup: gate=%u text=%zu game-roots=%zu vertex-tables=%zu prepare=%u\n",
            static_cast<unsigned>(gate_), text_.size(), data_.game().roots().size(),
            modules_.vertices.entries().size(), static_cast<unsigned>(prepared));
        if (!prepared) return false; // Original controller does not select profile on failure.
        frontend::SettingsSound<SettingsServices> sound(sound_data_.settings, gate_, settings_backend_);
        frontend::SettingsControl<SettingsServices> control(control_data_.wrapper0c, settings_backend_);
        data_.reset_profile(wide, profile_words_, files_, profile_platform_, wide, input_.state(),
            input_, settings_backend_, sound, control, input_flag_, text_);
        input_.produce(false);
        std::printf("Startup: profile/options applied; bindings=%zu joysticks=%zu input-words=%zu/%zu\n",
            input_.state().bindings.size(), input_.state().joysticks.size(),
            input_.state().first.size(), input_.state().second.size());
        phase_ = Phase::ready;
        return true;
    }
    void close_consumers() {
        input_.detach();
        joysticks_.close();
        data_.close();
        modules_.vertices.clear();
        phase_ = Phase::closed;
    }
    void close_archives() { archives_.close(); }
private:
    HMODULE engine() const { return engine_; }
    enum class Phase { constructed, control_ready, loading, ready, closed };
    HMODULE engine_;
    ChunkFiles files_;
    DtaTextFiles text_files_;
    StartupLocalizationPlatform platform_;
    frontend::ExecutableDirectory<StartupLocalizationPlatform> directory_;
    frontend::ApplicationLanguage<StartupLocalizationPlatform> language_;
    HumanPortraits portraits_;
    frontend::ApplicationStartupData<HumanPortraits> data_;
    StartupArchiveBackend archive_backend_;
    io::StartupArchives<StartupArchiveBackend> archives_;
    GraphicsResourceDirectories graphics_directories_;
    io::ResourceDirectoryCaches<GraphicsResourceDirectories> caches_;
    ResourceDirectoryPlatform directory_platform_;
    JoystickBackend joysticks_backend_;
    input::JoystickDevices<JoystickBackend> joysticks_;
    GraphicsInputServices<JoystickBackend> input_backend_;
    input::InputManager<GraphicsInputServices<JoystickBackend>> input_;
    GraphicsInputSetup<input::JoystickDevices<JoystickBackend>> input_setup_;
    SettingsServices settings_backend_;
    ProfileDirectoryPlatform<frontend::ExecutableDirectory<StartupLocalizationPlatform>> profile_platform_;
    frontend::ClientModuleRegistry registry_{};
    frontend::SoundManagerData sound_data_;
    frontend::StartupModules modules_;
    frontend::ControlStartupState control_data_;
    ControlStartupServices lock_services_;
    frontend::ProfileSelectionWords profile_words_;
    frontend::LocalizedTextCache text_;
    std::uint32_t gate_ = 0;
    std::uint8_t input_flag_ = 0;
    Phase phase_ = Phase::constructed;
    HWND archive_focus_ = nullptr;
    BOOL directory_window_ = FALSE;
};

}
#endif
