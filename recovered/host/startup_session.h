#ifndef HD2_HOST_STARTUP_SESSION_H
#define HD2_HOST_STARTUP_SESSION_H

#include "frontend/application_startup_data.h"
#include "frontend/control_startup.h"
#include "frontend/control_frame.h"
#include "frontend/application_controller.h"
#include "frontend/profile_save.h"
#include "frontend/options_save.h"
#include "input/binding_editor.h"
#include "frontend/opening_subtitles.h"
#include "frontend/input_setup.h"
#include "frontend/sound_manager_data.h"
#include "frontend/startup_modules.h"
#include "frontend/options_settings.h"
#include "host/chunk_files.h"
#include "host/control_startup.h"
#include "host/graphics_input_services.h"
#include "host/human_store.h"
#include "host/joystick_backend.h"
#include "host/profile_directories.h"
#include "host/profile_management.h"
#include "host/resource_directories.h"
#include "host/loaded_resources.h"
#include "host/settings_services.h"
#include "host/sound_services.h"
#include "host/graphics_settings.h"
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
          graphics_directories_(engine), caches_(graphics_directories_), loaded_resources_(engine, caches_),
          joysticks_backend_(engine, joy), joysticks_(joysticks_backend_),
          input_backend_(engine, joysticks_backend_), input_(input_backend_),
          input_setup_(engine, joysticks_), settings_backend_(engine),
          sound_backend_(engine, world_), sound_catalog_(sound_backend_),
          profile_platform_(directory_), sound_data_(registry_), modules_(registry_),
          control_frame_(control_data_, gate_, *this) {
        // Actual00748940/00748960 clock basis, separate from frame pacing.
        if (!QueryPerformanceFrequency(&control_frequency_) || control_frequency_.QuadPart <= 0 ||
                !QueryPerformanceCounter(&control_origin_))
            throw std::runtime_error("Control performance clock initialization failed");
    }

    void before_devices() {
        if (phase_ != Phase::constructed) throw std::logic_error("Startup initialization repeated");
        frontend::initialize_control_startup(control_data_, gate_, lock_services_);
        control_ready_ = true;
        phase_ = Phase::control_ready;
    }
    // Called only after real graphics, sound and I3D initialization. The caller
    // retains device ownership; close_consumers must run before device close.
    void setup_devices() {
        if (phase_ != Phase::control_ready) throw std::logic_error("Startup device order mismatch");
        phase_ = Phase::loading;
        setup_started_ = true;
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
        input_setup_started_ = true;
        frontend::setup_graphics_input(input_, input_setup_, language_, text_, platform_, text_files_, directory_);
        modules_.initialize(gate_, files_); // Original006DBFB0 includes this phase.
        phase_ = Phase::resources_ready;
    }
    bool prepare_application() {
        if (phase_ != Phase::resources_ready) throw std::logic_error("Application preparation repeated/out of order");
        const auto wide = language_.state().wide_characters;
        const auto prepared = data_.prepare_application(files_, text_, wide);
        if (const auto rejected = data_.game().rejected_root())
            std::printf("Startup GameData: rejected-root=%u\n", static_cast<unsigned>(*rejected));
        std::printf("Startup: gate=%u text=%zu game-roots=%zu vertex-tables=%zu prepare=%u\n",
            static_cast<unsigned>(gate_), text_.size(), data_.game().roots().size(),
            modules_.vertices.entries().size(), static_cast<unsigned>(prepared));
        if (prepared) phase_ = Phase::data_ready;
        return prepared;
    }
    void reset_profile() {
        if (phase_ != Phase::data_ready) throw std::logic_error("Profile preparation repeated/out of order");
        const auto wide = language_.state().wide_characters;
        frontend::SettingsSound<SettingsServices> sound(sound_data_.settings, gate_, settings_backend_);
        frontend::SettingsControl<SettingsServices> control(control_data_.wrapper0c, settings_backend_);
        data_.reset_profile(wide, profile_words_, files_, profile_platform_, wide, input_.state(),
            input_, settings_backend_, sound, control, input_flag_, text_);
        std::printf("Startup: profile/options applied; bindings=%zu joysticks=%zu input-words=%zu/%zu\n",
            input_.state().bindings.size(), input_.state().joysticks.size(),
            input_.state().first.size(), input_.state().second.size());
        phase_ = Phase::ready;
    }
    // Keep the accepted diagnostic as a composition of the same ordered phases.
    bool after_devices() {
        setup_devices();
        if (!prepare_application()) return false;
        reset_profile();
        input_.produce(false);
        return true;
    }
    void close_consumers(bool graphics_alive = true) {
        if (phase_ == Phase::closed) return;
        phase_ = Phase::closed;
        frontend::CleanupErrors errors;
        //0072E460 final reset is required even after status4 reset. Failed
        // pre-device acquisition instead abandons only acquired native locks.
        errors.attempt([&] {
            if (control_ready_ && graphics_alive) {
                control_frame_.shutdown();
                std::puts("Shutdown: final cold control reset and startup locks released");
            }
        });
        // Also reclaim partial0072E1A0 initialization/unsupported native errors.
        lock_services_.delete_control_critical_section(0);
        lock_services_.delete_control_critical_section(1);
        control_data_.locks5018 = 0;
        control_data_.records.clear();
        if (graphics_alive && setup_started_) errors.attempt([&] { close_sound(); });
        if (graphics_alive && mouse_attempted_) errors.attempt([&] { close_mouse(); });
        if (graphics_alive && input_setup_started_) errors.attempt([&] { close_keyboard(); });
        errors.attempt([&] { input_.detach(); });
        errors.attempt([&] { joysticks_.close(); });
        errors.attempt([&] { data_.close(); }); //006B6010 four actually owned components.
        errors.attempt([&] { modules_.shutdown(gate_); }); //0048DA80, after data.
        std::printf("Shutdown: input/sound/data/module cleanup completed=%u\n",
                    errors.failed() ? 0u : 1u);
        errors.rethrow();
    }
    void close_archives() { archives_.close(); }
    EngineFrame* graphics() const { return static_cast<EngineFrame*>(input_setup_.graphics()); }
    std::uint32_t application_flags() const { return gate_; }
    bool application_input_enabled() const { return application_input_enabled_; }
    void application_input_enabled(bool value) { application_input_enabled_ = value; } // 006DCAD0
    std::uint32_t random_word() { return settings_backend_.sound_random_word(); }
    const frontend::LocalizedTextCache& text() const { return text_; }
    LoadedResources& loaded_resources() { return loaded_resources_; }
    void clear_controller_resources() { loaded_resources_.clear_controller(); }
    void clear_final_resources() { loaded_resources_.clear_final(); }
    //006B6740: resolve the selected profile's real settings each call. No
    // menu-local defaults/copies or cache of a prior selected-profile pointer.
    frontend::ProfileSettings& profile_settings() { return data_.profile().options.settings; }
    bool mission_active() const { return world_!=nullptr; } //00698500's actual mission-owner condition.
    input::BindingConfiguration& profile_bindings() { return data_.profile().options.bindings; }
    const std::string& profile_name() { return data_.profile().directories.current_name; }
    std::vector<std::string> profile_names() {
        std::vector<std::string> result;
        data_.profile().directories.scan(profile_platform_,files_,wide_characters(),
            [](const char* name,void* owner){static_cast<std::vector<std::string>*>(owner)->emplace_back(name);},&result);
        return result;
    }
    void select_managed_profile(const std::string& name) {
        frontend::SettingsSound<SettingsServices> sound(sound_data_.settings,gate_,settings_backend_);
        frontend::SettingsControl<SettingsServices> control(control_data_.wrapper0c,settings_backend_);
        frontend::select_and_remember_profile(data_.configuration(),data_.profile(),name.c_str(),profile_words_,
            files_,profile_platform_,wide_characters(),input_.state(),input_,settings_backend_,sound,control,
            input_flag_,&data_.game(),text_);
    }
    unsigned profile_difficulty() {return data_.profile().payload.words[0x34/4];}
    void profile_difficulty(unsigned value) {data_.profile().payload.words[0x34/4]=value;}
    bool save_managed_profile(){return frontend::save_profile_payload(data_.profile(),files_,wide_characters());}
    int create_managed_profile(const std::string& name) {
        if(wide_characters())throw std::runtime_error("Profile editing requires the delivered English mode");
        return ProfileFilesystem(directory_.get()).create(name);
    }
    int rename_managed_profile(const std::string& from,const std::string& to) {
        if(wide_characters())throw std::runtime_error("Profile editing requires the delivered English mode");
        const int result=ProfileFilesystem(directory_.get()).rename(from,to);
        if(result==1) {data_.profile().directories.current_name=to;save_managed_profile();
                      data_.configuration().set_profile(files_,to);}
        return result;
    }
    bool delete_managed_profile(const std::string& name) {
        if(wide_characters())throw std::runtime_error("Profile editing requires the delivered English mode");
        return ProfileFilesystem(directory_.get()).remove(name);
    }
    bool save_options() {
        const bool result=frontend::save_profile_options(data_.profile().options,
            data_.profile().directories,files_,wide_characters());
        std::printf("Options persistence: result=%u\n",static_cast<unsigned>(result));
        return result;
    }
    void apply_bindings() { input::apply_dirty_bindings(profile_bindings(),input_.state(),input_); }
    void reset_bindings(std::uint32_t layout) {
        input::reset_binding_defaults(profile_bindings(),input_.state(),input_,layout);
    }
    bool gamma_available() const {
        //0061BEA0 checks the initialization settings, NOT adapter capabilities.
        //1006DA30 returns100D1D18; +2c is the fullscreen flag word. The
        //windowed launcher deliberately leaves bit2 clear, regardless of preset.
        using Info=const unsigned char* (WINAPI *)(EngineFrame*);
        const auto* info=scene_method<Info>(graphics(),engine_,4,0x6da30)(graphics());
        if(!info) throw std::runtime_error("Graphics adapter information is null");
        std::uint32_t flags=0;
        std::memcpy(&flags,info+0x2c,sizeof(flags));
        std::printf("Options gamma: graphics flags=%08x fullscreen=%u enabled=%u\n",
                    static_cast<unsigned>(flags),static_cast<unsigned>((flags&2)!=0),
                    static_cast<unsigned>((flags&2)!=0));
        return (flags&2)!=0;
    }
    std::optional<input::BindingCandidate> poll_options_binding() {
        //006C84A0(mask9A,poll1): no joystick candidate is enabled by this mask.
        input_.pump_graphics(); input_.poll_mouse(); input_.poll_keyboard();
        using Query=std::uint32_t (WINAPI *)(EngineFrame*);
        using Modifiers=std::uint32_t (WINAPI *)(EngineFrame*,std::uint8_t);
        const auto modifiers=scene_method<Modifiers>(graphics(),engine_,0xc8,0x6f5b0)(graphics(),0);
        const auto scan=checked_engine_method<Query>(graphics(),engine_,0xa0)(graphics());
        return input::options_binding_candidate(modifiers,scan,input_.mouse_buttons(),
                                                input_.mouse_x(),input_.mouse_y(),input_.mouse_z());
    }
    std::string binding_name(std::size_t index) {
        //006AB590 English branch. Joystick labels are empty in the original.
        const auto& binding=profile_bindings().settings.at(index);
        const auto label=[&](std::uint32_t id) { return frontend::lookup_localized_text(text_,static_cast<std::int32_t>(id)); };
        if(binding.device_kind==2) {
            switch(binding.code) {
            case 1:return label(0xb9a); case 2:return label(0xb9b); case 4:return label(0xb9c);
            case 8:return label(0xb98); case 16:return label(0xb99);
            case 0x100:case 0x200:return label(binding.code==0x100?0xb9d:0xb9e)+
                    (binding.direction==1?" +":binding.direction==2?" -":"");
            case 0x300:return label(0xb9f)+" "+(binding.direction==1?label(0xbac):binding.direction==2?label(0xbad):"");
            default:return {};
            }
        }
        if(binding.device_kind!=1) return {};
        constexpr std::pair<std::uint16_t,std::uint16_t> named[]={
            {0x1d,0xbab},{0x2a,0xba9},{0x36,0xba8},{0x37,0xba1},{0x38,0xba7},
            {0x47,0xbb5},{0x48,0xbb6},{0x49,0xbb7},{0x4a,0xba2},{0x4b,0xbb2},
            {0x4c,0xbb3},{0x4d,0xbb4},{0x4e,0xba3},{0x4f,0xbaf},{0x50,0xbb0},
            {0x51,0xbb1},{0x52,0xbae},{0x53,0xba5},{0x9c,0xba4},{0x9d,0xbaa},
            {0xb5,0xba0},{0xb8,0xba6}};
        for(const auto& item:named) if(item.first==binding.code) return label(item.second);
        //1006F6A0 RET8 takes only self/scan. The31 pushed before the
        // original virtual call belongs to the FOLLOWING006C8D70 conversion.
        using Name=const wchar_t* (WINAPI *)(EngineFrame*,std::uint32_t);
        const auto* name=scene_method<Name>(graphics(),engine_,0xd0,0x6f6a0)(graphics(),binding.code&0xff);
        if(!name) return {};
        char bytes[32]{};
        WideCharToMultiByte(CP_ACP,0,name,-1,bytes,31,nullptr,nullptr);
        //006AB4F0 uppercases first, lowercases the remaining bytes (CRT locale).
        CharUpperBuffA(bytes,1);
        const auto length=std::strlen(bytes);
        if(length>1) CharLowerBuffA(bytes+1,static_cast<DWORD>(length-1));
        return bytes;
    }
    void apply_graphics_options() {
        if (settings_backend_.driver()) {
            auto* driver = settings_backend_.driver();
            settings_backend_.driver_state(driver, 0x2b, profile_settings().quality_byte(0));
        }
    }
    void apply_option(frontend::OptionsFloatSetting setting) {
        using Setting = frontend::OptionsFloatSetting;
        switch (setting) {
        case Setting::gamma:
            settings_backend_.gamma(frontend::profile_gamma_ramp(profile_settings()), 0); break;
        case Setting::sound_volume: {
            frontend::SettingsSound<SettingsServices> sound(sound_data_.settings, gate_, settings_backend_);
            sound.driver_volume(static_cast<float>(static_cast<long double>(profile_settings().real(8)) * 0.01f));
            break;
        }
        case Setting::music_volume: {
            frontend::SettingsSound<SettingsServices> sound(sound_data_.settings, gate_, settings_backend_);
            sound.movie_volume(static_cast<float>(static_cast<long double>(profile_settings().real(12)) * 0.01f));
            break;
        }
        case Setting::voice_volume: {
            frontend::SettingsControl<SettingsServices> control(control_data_.wrapper0c, settings_backend_);
            control.set_value(frontend::settings_truncated_word(static_cast<long double>(profile_settings().real(16)) * 0.01));
            break;
        }
        case Setting::mouse_sensitivity: {
            const auto bits = frontend::settings_truncated_word(static_cast<long double>(profile_settings().real(0)) * -9.0f + 1000.0f);
            std::int32_t limit; std::memcpy(&limit, &bits, sizeof(limit));
            input::set_binding_axis_limits(input_.state(), limit, limit, 0x78);
            input_flag_ = profile_settings().bytes[0x39];
            break;
        }
        }
    }
    bool wide_characters() const { return language_.state().wide_characters; }
    std::string language() { return language_.get(); }
    const auto& profile_movies() { return data_.profile().payload.movies.records(); }
    bool subtitles_enabled() { return data_.profile().options.settings.bytes[0x3a] != 0; }
    bool load_movie_subtitles(const char* filename, frontend::LocalizedTextCache& text) {
        const auto path = frontend::opening_subtitle_path(directory_.get(), language(), filename);
        if (!platform_.probe_text_file(path)) return false;
        return frontend::load_localized_text_file(text, text_files_, path);
    }
    frontend::OpeningTextStyles& opening_text_styles() { return opening_text_styles_; }
    void set_subtitle_driver_volume() {
        settings_backend_.sound_driver_volume(static_cast<float>(
            static_cast<long double>(data_.profile().options.settings.real(8)) * 0.01f));
    }
    void initialize_mouse(std::uint32_t flags) {
        mouse_attempted_ = true;
        using Initialize = std::int32_t (WINAPI *)(EngineFrame*, std::uint32_t);
        const auto result = scene_method<Initialize>(graphics(), engine_, 0xdc, 0x6f710)(graphics(), flags);
        if (result < 0) throw std::runtime_error("Controller mouse initialization failed");
    }
    void close_mouse() {
        using Enabled = std::uint8_t (WINAPI *)(EngineFrame*);
        using Close = std::int32_t (WINAPI *)(EngineFrame*);
        if (scene_method<Enabled>(graphics(), engine_, 0xe0, 0x6f840)(graphics()))
            scene_method<Close>(graphics(), engine_, 0xe4, 0x6f850)(graphics());
    }
    void close_keyboard() {
        using Enabled = std::uint8_t (WINAPI *)(EngineFrame*);
        using Close = void (WINAPI *)(EngineFrame*);
        if (scene_method<Enabled>(graphics(), engine_, 0x94, 0x6f150)(graphics()))
            scene_method<Close>(graphics(), engine_, 0x98, 0x6f160)(graphics());
    }
    void sample_frame_input() {
        input_backend_.poll_mouse(graphics());
        joysticks_.poll();
        input_.produce(false);
    }
    std::int32_t query_input(const std::uint32_t*& output, std::uint8_t mode) const {
        return frontend::query_controller_input(input_.state().first, input_.state().second, output, mode);
    }
    std::uint32_t mouse_buttons() const { return input_backend_.mouse_buttons(graphics()); }
    std::int32_t mouse_wheel() const { return input_backend_.mouse_z(graphics()); }
    std::int32_t mouse_x() const { return input_backend_.mouse_x(graphics()); }
    std::int32_t mouse_y() const { return input_backend_.mouse_y(graphics()); }
    void delay_elapsed() {
        frontend::SettingsSound<SettingsServices> sound(sound_data_.settings, gate_, settings_backend_);
        sound.driver_volume(static_cast<float>(static_cast<long double>(data_.profile().options.settings.real(8)) * 0.01f));
    }
    float movie_volume() {
        return static_cast<float>(static_cast<long double>(data_.profile().options.settings.real(8)) *
                                  0.01f * 1.1f);
    }
    bool sound_available() const { return settings_backend_.sound_driver_initialized(); }
    void fade_movie_music(float target, std::int32_t milliseconds) {
        frontend::schedule_sound_movie_fade(sound_data_.fade, sound_data_.settings.movie_scale, target, milliseconds);
    }
    struct CreditsMusic { std::int32_t request; std::uint32_t position; };
    CreditsMusic credits_music_position() {
        return {sound_data_.selection.requested,
            frontend::primary_sound_movie_position(sound_data_.settings,sound_backend_)};
    }
    void begin_credits_music(const CreditsMusic& saved,bool force=false) {
        if(force||saved.request!=37)
            frontend::queue_primary_sound_movie(sound_data_.settings,sound_data_.fade,
                sound_data_.secondary,sound_data_.selection,37,2000,0);
    }
    void restore_credits_music(const CreditsMusic& saved) {
        if(saved.request==-1 || saved.position==0xffffffffu) fade_movie_music(1,2000);
        else frontend::queue_primary_sound_movie(sound_data_.settings,sound_data_.fade,
            sound_data_.secondary,sound_data_.selection,saved.request,2000,saved.position);
    }
    void begin_menu_music() {
        const auto selected = sound_data_.selection.requested;
        if (selected == -1)
            frontend::select_primary_sound_movie(sound_catalog_, files_, sound_data_.settings,
                sound_data_.fade, sound_data_.selection, gate_, 0, 0, sound_backend_);
        else if (selected == 37)
            frontend::queue_primary_sound_movie(sound_data_.settings, sound_data_.fade,
                sound_data_.secondary, sound_data_.selection, 0, 2000, 0);
    }
    void update_sound(std::uint32_t milliseconds) {
        frontend::advance_sound_runtime(sound_data_.runtime, sound_catalog_, files_,
            sound_data_.settings, sound_data_.fade, sound_data_.secondary, sound_data_.selection,
            gate_, sound_clock_, static_cast<std::int32_t>(milliseconds), sound_backend_);
    }
    void remember_movie(const char* filename) {
        frontend::remember_profile_movie(data_.profile(), filename, 1, files_, wide_characters());
    }
    void close_sound() {
        frontend::CleanupErrors errors;
        errors.attempt([&] { frontend::reset_sound_runtime(sound_data_.runtime, sound_catalog_, files_,
            sound_data_.settings, sound_data_.fade, sound_data_.secondary, sound_data_.selection,
            gate_, frontend::SoundApplicationMode{nullptr, 0}, 1, 0, sound_backend_); });
        errors.attempt([&] { frontend::clear_sound_sectors(sound_data_.runtime, sound_backend_); });
        errors.attempt([&] { sound_catalog_.clear(); });
        errors.rethrow();
    }
    void control_phase(std::uint32_t milliseconds, bool fixed) { control_frame_.phase(milliseconds, fixed); }
    void control_maintenance(std::uint32_t milliseconds) { control_frame_.maintain(milliseconds); }
    void reset_control() { control_frame_.reset(); }
    void enter_control_lock(unsigned index) { lock_services_.enter_control_lock(index); }
    void leave_control_lock(unsigned index) noexcept { lock_services_.leave_control_lock(index); }
    void delete_control_critical_section(unsigned index) noexcept {
        lock_services_.delete_control_critical_section(index);
    }
    std::uint32_t control_milliseconds() const {
        LARGE_INTEGER counter{};
        if (!QueryPerformanceCounter(&counter)) throw std::runtime_error("Control performance clock read failed");
        const auto bits = static_cast<std::uint64_t>(counter.QuadPart) - static_cast<std::uint64_t>(control_origin_.QuadPart);
        std::int64_t ticks; std::memcpy(&ticks, &bits, sizeof(ticks));
        return frontend::settings_truncated_word(static_cast<long double>(ticks) / control_frequency_.QuadPart * 1000.0f);
    }
    void restore_graphics_flags(std::uint32_t flags) {
        using Set = std::int32_t (WINAPI *)(EngineFrame*, std::uint32_t);
        scene_method<Set>(graphics(), engine_, 8, 0x6c2c0)(graphics(), flags);
    }
    void restore_cold_firewall(bool modified) {
        // 00732360(0,0): same cold static flag means no COM/firewall mutation.
        if (modified) throw std::logic_error("Live firewall ownership is not reconstructed");
        OSVERSIONINFOA version{};
        version.dwOSVersionInfoSize = sizeof(version);
        if (!GetVersionExA(&version)) {
            version.dwOSVersionInfoSize = 0x94;
            GetVersionExA(&version);
        }
    }
private:
    HMODULE engine() const { return engine_; }
    enum class Phase { constructed, control_ready, loading, resources_ready, data_ready, ready, closed };
    HMODULE engine_;
    ChunkFiles files_;
    DtaTextFiles text_files_;
    StartupLocalizationPlatform platform_;
    frontend::ExecutableDirectory<StartupLocalizationPlatform> directory_;
    frontend::ApplicationLanguage<StartupLocalizationPlatform> language_;
    frontend::OpeningTextStyles opening_text_styles_;
    HumanPortraits portraits_;
    frontend::ApplicationStartupData<HumanPortraits> data_;
    StartupArchiveBackend archive_backend_;
    io::StartupArchives<StartupArchiveBackend> archives_;
    GraphicsResourceDirectories graphics_directories_;
    io::ResourceDirectoryCaches<GraphicsResourceDirectories> caches_;
    LoadedResources loaded_resources_;
    ResourceDirectoryPlatform directory_platform_;
    JoystickBackend joysticks_backend_;
    input::JoystickDevices<JoystickBackend> joysticks_;
    GraphicsInputServices<JoystickBackend> input_backend_;
    input::InputManager<GraphicsInputServices<JoystickBackend>> input_;
    GraphicsInputSetup<input::JoystickDevices<JoystickBackend>> input_setup_;
    SettingsServices settings_backend_;
    frontend::SoundWorldView* world_ = nullptr; // Actual mission owner absent before mission entry.
    SoundServices sound_backend_;
    frontend::SoundCatalog<SoundServices> sound_catalog_;
    std::uint32_t sound_clock_ = 0;
    ProfileDirectoryPlatform<frontend::ExecutableDirectory<StartupLocalizationPlatform>> profile_platform_;
    frontend::ClientModuleRegistry registry_{};
    frontend::SoundManagerData sound_data_;
    frontend::StartupModules modules_;
    frontend::ControlStartupState control_data_;
    ControlStartupServices lock_services_;
    frontend::ProfileSelectionWords profile_words_;
    frontend::LocalizedTextCache text_;
    std::uint32_t gate_ = 0;
    frontend::ColdControlFrame<StartupSession> control_frame_;
    std::uint8_t input_flag_ = 0;
    bool application_input_enabled_ = true; // Original00843470 initial byte1.
    bool control_ready_ = false, setup_started_ = false;
    bool input_setup_started_ = false, mouse_attempted_ = false;
    Phase phase_ = Phase::constructed;
    HWND archive_focus_ = nullptr;
    BOOL directory_window_ = FALSE;
    LARGE_INTEGER control_frequency_{}, control_origin_{};
};

}
#endif
