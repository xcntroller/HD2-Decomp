#ifndef HD2_HOST_STARTUP_SESSION_H
#define HD2_HOST_STARTUP_SESSION_H

#include "frontend/application_startup_data.h"
#include "frontend/control_startup.h"
#include "frontend/control_actor_records.h"
#include "frontend/control_frame.h"
#include "game/session_local_packets.h"
#include "game/session_packet_world.h"
#include "game/blast_wave_process.h"
#include "game/human_projection_process.h"
#include <ctime>
#include <cstdlib>
#include "frontend/application_controller.h"
#include "frontend/profile_save.h"
#include "frontend/options_save.h"
#include "input/binding_editor.h"
#include "frontend/opening_subtitles.h"
#include "frontend/input_setup.h"
#include "frontend/sound_manager_data.h"
#include "frontend/sound_listeners.h"
#include "frontend/pre_mission_flow.h"
#include "host/game_animations.h"
#include "host/game_scene_actors.h"
#include "host/game_spatial_sound.h"
#include "host/dubbing_sound_services.h"
#include "frontend/dubbing_sound.h"
#include "game/human_sound_pool.h"
#include "game/human_morph_tables.h"
#include "game/human_physics_globals.h"
#include "game/mission_integrity_tables.h"
#include "game/mission_script_variables.h"
#include "game/mission_effects_loading.h"
#include "game/shot_manager_loading.h"
#include "game/heavy_fire_module_loading.h"
#include "game/effects_loading_process.h"
#include "game/item_hand_models.h"
#include "game/human_ai_event_queue.h"
#include "game/human_ai_action_pool.h"
#include "game/human_ai_navigation_jobs.h"
#include "game/human_frame_inputs.h"
#include "game/human_scene_movement.h"
#include "game/actor_proximity_loading.h"
#include "game/car_seat_camera.h"
#include "game/car_definition_tables.h"
#include "game/car_tyres.h"
#include "game/players_process.h"
#include "game/human_input_records.h"
#include "game/world_views.h"
#include "game/hud_scene_loading.h"
#include "game/hud_signal_tables.h"
#include "game/hud_compass_loading.h"
#include "game/console_variables.h"
#include "game/text_render.h"
#include "frontend/multiplayer_connection.h"
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
#include "host/menu_presentation.h"
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
    using PreMission = frontend::PreMissionFlow<frontend::HumanStore<HumanPortraits>>;
    using PlayerHuman=game::HumanSceneActor<GameActorServices,GameSceneActors::Runtime>;
    std::optional<frontend::MultiplayerLaunchRequest>& multiplayer_request() { return multiplayer_request_; }
    StartupSession(HMODULE engine, HMODULE rw_data, HMODULE joy,const std::string& selected_language={})
        : engine_(engine), files_(rw_data), text_files_(rw_data), directory_(platform_), language_(platform_),
          portraits_(engine), data_(portraits_, files_, directory_),
          archive_backend_(rw_data), archives_(archive_backend_),
          graphics_directories_(engine), caches_(graphics_directories_), loaded_resources_(engine, caches_),
          game_animations_(files_, loaded_resources_),broadcast_packets_(human_input_shared_),
          joysticks_backend_(engine, joy), joysticks_(joysticks_backend_),
          input_backend_(engine, joysticks_backend_), input_(input_backend_),
          input_setup_(engine, joysticks_), settings_backend_(engine),
          sound_backend_(engine, game_views_.sound), sound_catalog_(sound_backend_),
          dubbing_sound_(sound_backend_,files_),
          profile_platform_(directory_), sound_data_(registry_), modules_(registry_),
          control_frame_(control_data_, gate_, *this,broadcast_packets_),
          local_packets_(control_data_,control_frame_.state(),broadcast_packets_,human_input_shared_,gate_,*this,packet_world_) {
        if (!selected_language.empty()) language_.select(selected_language);
        menu_cursor_={menu_width()*.5f,menu_height()*.5f};
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
                if(source_packet_path())control_frame_.shutdown_packets([&](std::uint32_t elapsed){local_packets_.flush(elapsed);},[&]{local_packets_.clear();});
                else control_frame_.shutdown();
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
        blast_wave_process_.clear(); //Actual process008AED48 fallback retirement before device/module owners close.
        pre_mission_.reset(); // Borrowed HumanStore/descriptor owners end before application data.
        multiplayer_request_.reset();
        errors.attempt([&] { data_.close(); }); //006B6010 four actually owned components.
        errors.attempt([&] { modules_.shutdown(gate_,human_projection_process_); }); //0048DA80, after data; actual Items3 pool retirement.
        item_hand_models_.clear();
        game_animations_.close();
        std::printf("Shutdown: input/sound/data/module cleanup completed=%u\n",
                    errors.failed() ? 0u : 1u);
        errors.rethrow();
    }
    void close_archives() { archives_.close(); }
    EngineFrame* graphics() const { return static_cast<EngineFrame*>(input_setup_.graphics()); }
    std::uint32_t application_flags() const { return gate_; }
    std::uint32_t& application_flags_owner(){return gate_;} //SAME00982100 for source module/runtime borrowers; no per-world mode copy.
    std::int32_t& local_player_identity(){return control_frame_.state().local_peer;} //SAME00982104, not a per-world ID copy.
    const std::int32_t& local_player_identity()const{return control_frame_.state().local_peer;}
    bool application_input_enabled() const { return application_input_enabled_; }
    void application_input_enabled(bool value) { application_input_enabled_ = value; } // 006DCAD0
    std::uint32_t random_word() { return settings_backend_.sound_random_word(); }
    void seed_random(std::uint32_t seed){std::srand(seed);} //006E9EA0 uses the SAME CRT stream as sound/AI.
    bool& script_advance_override(){return script_advance_override_;} //008692D0;00424900 reset scope owns its writes.
    game::MissionIntegrityTables& mission_integrity_tables(){return mission_integrity_tables_;} //SAME process owner; no invented initialized/healthy state.
    game::MissionScriptVariables& mission_script_variables(){return mission_script_variables_;} //Module4 survives world finish/reconstruction;004270D0 applies actual selection kind afterward.
    game::MissionEffectsProcess<EngineFrame*>& mission_effects_process(){return mission_effects_process_;}
    game::ShotManagerLoadingState<GameActorServices>& shot_manager_process(){return shot_manager_process_;}
    game::HeavyFireLoadingState& heavy_fire_process(){return heavy_fire_process_;}
    auto& effects_loading_process(){return effects_loading_process_;}
    using BlastWaves=game::BlastWaveProcess<MenuDriver::Owner,GameSceneActors::Instance>;
    BlastWaves& blast_wave_process(){return blast_wave_process_;} //SAME fixed-update and finish owner.
    auto& human_projection_process(){return human_projection_process_;} //0098AD28/38/44, SAME records/pool and process timers.
    auto& item_hand_models(){return item_hand_models_;}
    std::uint8_t& first_person_preference(){return data_.first_person_preference();}
    const frontend::LocalizedTextCache& text() const { return text_; }
    LoadedResources& loaded_resources() { return loaded_resources_; }
    GameAnimations& game_animations() { return game_animations_; }
    game::HumanSoundVariants& human_sound_variants() {return human_sound_variants_;}
    game::HumanInputShared& human_input_shared() {return human_input_shared_;}
    const game::HumanMorphTables& human_morph_tables() {human_morph_tables_.load(files_);return human_morph_tables_;}
    const float& human_animation_time_scale()const{return human_animation_time_scale_;} //0084E80C initial FLOAT1.4; original direct references are reads.
    game::HumanPhysicsGlobals& human_physics_globals(){return human_physics_globals_;}
    game::HumanAIEventPool& human_ai_event_pool(){return human_ai_events_;}
    game::HumanAIFrameInputs& human_ai_frame_inputs(){return human_ai_frame_inputs_;}
    game::HumanAIActionPools& human_ai_action_pools(){return human_ai_actions_;}
    game::HumanAINavigationJobPool& human_ai_navigation_jobs(){return human_ai_navigation_jobs_;}
    auto& human_scene_movement_process(){return human_scene_movement_process_;} //008AADD0, shared by Scene2 owners.
    auto& human_perception_record_pool(){return human_perception_records_;}
    auto& actor_proximity_state(){return actor_proximity_;} //Actual process008AED54, preserves bucket timers across world clear.
    EngineFrame*& car_loading_scene(){return car_loading_scene_;} //0089C0C0 borrowed engine scene,006D3F20 publication.
    auto& car_definitions(){return car_definitions_;} //0071F9B0 process definitions, retained across worlds.
    auto& car_tyres(){return car_tyres_;} //004D6900/004EC980 process registry, reloaded in place.
    auto& car_materials(){return car_materials_;} //004D57A0 canonical process material lookup.
    std::int32_t& game_vertex_clock(){return game_vertex_clock_;} //0089B398, persists across instance cleanup.
    float& game_sight_work_clock(){return game_sight_work_clock_;} //00839390 initialized1, persists across World/module activation.
    auto& car_seat_camera_shared(){return car_seat_camera_;} //Actual process0089B0F8, separate from MissionCamera258.
    auto& car_light_receivers(){return car_light_receivers_;} //0089C0A4,004E2810 initializes the actual process list empty.
    auto& car_sound_receivers(){return car_sound_receivers_;} //0089C0B4, separate sound registrations.
    auto& players_process(){return players_process_;} //Actual CRT0089AC30; module reset, not a per-world copy.
    auto& delayed_player_speech(){return delayed_player_speech_;} //Separate0089AC20, survives00456440 reset.
    template<class Resolve,class Type>std::int32_t add_game_sound_listener(EngineFrame* frame,void* callback,Resolve&& resolve,Type&& type) {
        return frontend::add_sound_listener(sound_data_.runtime,gate_,frame,callback,
            std::forward<Resolve>(resolve),std::forward<Type>(type)); //00433ED0, SAME process manager.
    }
    bool remove_game_sound_listener(std::int32_t slot){return frontend::remove_sound_listener(sound_data_.runtime,gate_,slot);} //00433E80.
    void set_game_sound_voice_limit(std::uint32_t group,std::uint32_t limit) {
        frontend::set_sound_voice_limit(sound_data_.runtime,group,limit); //004324C0, SAME client sound manager.
    }
    void finish_game_sound_loading(bool authority) { //004338D0, module6 virtual14.
        if(!authority&&(gate_&1u))return;
        frontend::reset_sound_runtime(sound_data_.runtime,sound_catalog_,files_,
            sound_data_.settings,sound_data_.fade,sound_data_.secondary,sound_data_.selection,
            gate_,game_sound_application(),1,1,sound_backend_);
    }
    void activate_game_sound(bool authority) { //00433840, module6 virtual1C.
        frontend::start_sound_runtime(sound_data_.runtime,sound_catalog_,files_,
            sound_data_.settings,sound_data_.fade,sound_data_.secondary,sound_data_.selection,
            gate_,game_sound_application(),std::uint8_t(authority),sound_backend_);
    }
    void unload_game_sound(bool authority) { //004338F0, virtual18; no sector clearing here.
        if(!authority&&(gate_&1u))return;
        frontend::reset_sound_runtime(sound_data_.runtime,sound_catalog_,files_,
            sound_data_.settings,sound_data_.fade,sound_data_.secondary,sound_data_.selection,
            gate_,game_sound_application(),1,0,sound_backend_);
    }
    void finish_game_sound(bool authority) { //004338A0, module6 virtual20.
        frontend::stop_sound_runtime(sound_data_.runtime,sound_catalog_,files_,
            sound_data_.settings,sound_data_.fade,sound_data_.secondary,sound_data_.selection,
            gate_,game_sound_application(),std::uint8_t(authority),sound_backend_);
    }
    void stop_game_sound_voice(std::int32_t& handle) {
        frontend::stop_sound_voice(sound_data_.runtime,handle,sound_backend_); //00437340.
    }
    void resume_game_sound_voice(std::int32_t handle) { //SOUND+78/004374A0→actual frame+30(1), used by Door00601F90.
        set_game_sound_voice_enabled(handle,true);
    }
    void set_game_sound_voice_enabled(std::int32_t handle,bool enabled) {
        auto* frame=frontend::sound_voice_frame(sound_data_.runtime,handle);
        if(!frame)throw std::runtime_error("Door sound control requires its actual live voice frame");
        sound_backend_.enable_sound(frame,std::uint32_t(enabled));
    }
    std::uint32_t game_sound_voice_cursor(std::int32_t handle) {
        auto* frame=frontend::sound_voice_frame(sound_data_.runtime,handle);
        if(!frame)throw std::runtime_error("Door sound cursor requires its actual live voice frame");
        return sound_backend_.sound_play_cursor(frame);
    }
    void game_sound_voice_cursor(std::int32_t handle,std::uint32_t cursor) {
        auto* frame=frontend::sound_voice_frame(sound_data_.runtime,handle);
        if(!frame)throw std::runtime_error("Door sound cursor requires its actual live voice frame");
        sound_backend_.sound_play_cursor(frame,cursor);
    }
    template<class World>std::int32_t emit_game_sound(const frontend::SpatialSoundRequest& request,World& world) {
        GameSpatialSoundServices<World> backend(engine_,game_views_.sound,loaded_resources_,world);
        return frontend::emit_spatial_sound(sound_data_.runtime,sound_catalog_,&request,gate_,backend);
    }
    template<class World>bool play_game_speech(void* actor,EngineFrame* frame,std::int32_t id,float near_range,float far_range,bool subtitles,World& world) {
        return dubbing_sound_.play_speech(actor,frame,id,near_range,far_range,subtitles,gate_,world);
    }
    std::uint32_t game_speech_length(std::int32_t id){return sound_catalog_.speech_sample_size(id);} //SOUND virtual94, SAME catalogue mappings.
    const char* missing_speech_caption()const{return missing_speech_caption_;} //Process0089BB44 cold null; no populated writer is fabricated.
    void initialize_game_dubbing(){dubbing_sound_.initialize_loading();}
    void synchronize_game_dubbing(){dubbing_sound_.synchronize_attachments();}
    void update_game_dubbing(std::uint32_t elapsed){dubbing_sound_.update(elapsed);sound_backend_.rethrow_dubbing_cleanup_failure();}
    void retire_game_dubbing(EngineFrame* frame){dubbing_sound_.retire_model(frame);sound_backend_.rethrow_dubbing_cleanup_failure();}
    void finish_game_dubbing(){dubbing_sound_.clear(false);sound_backend_.rethrow_dubbing_cleanup_failure();} //0071DE60: no morph neutral on native mission finish.
    void clear_game_dubbing(){dubbing_sound_.clear();sound_backend_.rethrow_dubbing_cleanup_failure();}
    frontend::VertexAnimations& game_vertex_definitions(){return modules_.vertices;} //Same process module9 definitions; world registry borrows these.
    const io::ResourceDirectoryList& model_resource_directories() const { return caches_.client(1); }
    void clear_controller_resources() { loaded_resources_.clear_controller(); }
    void clear_final_resources() { loaded_resources_.clear_final(); }
    //006B6740: resolve the selected profile's real settings each call. No
    // menu-local defaults/copies or cache of a prior selected-profile pointer.
    frontend::ProfileSettings& profile_settings() { return data_.profile().options.settings; }
    bool human_face_quality_enabled(){return profile_settings().quality_byte(3)!=0;} //006B6740→006B33E0(3).
    frontend::GameConfiguration& game_configuration(){return data_.game_configuration();}
    frontend::SoundApplicationMode game_sound_application(){
        const auto& configuration=game_configuration();
        //006B66B0 tests this opaque native DWORD for nonzero only; it is never dereferenced.
        return {reinterpret_cast<void*>(std::uintptr_t(configuration.field04)),std::uint32_t(configuration.style224)};
    }
    bool human_remote_ai_enabled(){return game_configuration().remote_ai_enabled();} //006B66B0, SAME application24 data.
    unsigned human_difficulty(){ //006B6700; remote predicate precedes offline7/profile test.
        return frontend::human_difficulty_preference(application_flags(),game_configuration(),[this]{return profile_difficulty();});
    }
    bool car_camera_setting264(){return game_configuration().car_camera264();}
    std::uint8_t hud_crosshair_allowed(){return frontend::hud_crosshair_preference(gate_,game_configuration(),profile_settings());}

    bool mission_active() const { return game_views_.owner()!=nullptr; } //00698500 actual owner, distinct from loaded camera.
    game::WorldViews& game_views() { return game_views_; }
    frontend::ClientModuleRegistry& game_module_registry(){return registry_;}
    frontend::ClientModuleRegistration& game_module_registration(std::uint32_t kind){return modules_.registration(kind);}
    input::BindingConfiguration& profile_bindings() { return data_.profile().options.bindings; }
    const std::string& profile_name() { return data_.profile().directories.current_name; }
    PreMission& pre_mission() {
        if (!pre_mission_) pre_mission_=std::make_unique<PreMission>(data_.humans());
        return *pre_mission_;
    }
    bool prepare_pre_mission() {
        if (profile_name().empty())
            throw std::logic_error("Single-player preparation requires the selected profile");
        auto& flow=pre_mission();
        flow.cancel();
        if (!reload_pre_mission_humans()) return false;
        return flow.load(files_,pre_mission_profile_directory());
    }
    ChunkFiles& pre_mission_files() { return files_; }
    frontend::HumanStore<HumanPortraits>& pre_mission_humans() { return data_.humans(); }
    std::string pre_mission_profile_directory() {
        return data_.profile().directories.current_directory(wide_characters());
    }
    bool reload_pre_mission_humans() {
        return data_.humans().load(files_, "tables\\HumanStore.def", text_,
                                  pre_mission_profile_directory(), wide_characters());
    }
    void save_pre_mission_names() {
        if(!data_.humans().save_user_names(files_,pre_mission_profile_directory()))
            throw std::runtime_error("Cannot save selected profile soldier names");
    }
    std::int32_t pre_mission_progress(std::uint32_t root, std::uint32_t mode) {
        //006B13A0/006B13C0: mode0 queries the standalone unlock word;
        //mode1/2/3 have distinct per-root campaign progress words.
        if (root >= 2 || mode > 3)
            return -1;
        const auto value = data_.profile().payload.words[(0x74 + root * 16 + mode * 4) / 4];
        std::int32_t result;
        std::memcpy(&result, &value, sizeof(result));
        return result;
    }
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
    const std::string& last_save_caption() {return data_.profile().payload.strings[2];}
    unsigned profile_difficulty() {return data_.profile().payload.words[0x34/4];}
    void profile_difficulty(unsigned value) {data_.profile().payload.words[0x34/4]=value;}
    bool save_managed_profile(){return frontend::save_profile_payload(data_.profile(),files_,wide_characters());}
    int create_managed_profile(const std::string& name) {
        if(wide_characters())throw std::runtime_error("Profile editing requires a supported byte-font language");
        return ProfileFilesystem(directory_.get()).create(name);
    }
    int rename_managed_profile(const std::string& from,const std::string& to) {
        if(wide_characters())throw std::runtime_error("Profile editing requires a supported byte-font language");
        const int result=ProfileFilesystem(directory_.get()).rename(from,to);
        if(result==1) {data_.profile().directories.current_name=to;save_managed_profile();
                      data_.configuration().set_profile(files_,to);}
        return result;
    }
    bool delete_managed_profile(const std::string& name) {
        if(wide_characters())throw std::runtime_error("Profile editing requires a supported byte-font language");
        return ProfileFilesystem(directory_.get()).remove(name);
    }
    bool save_options() {
        const bool result=frontend::save_profile_options(data_.profile().options,
            data_.profile().directories,files_,wide_characters());
        std::printf("Options persistence: result=%u\n",static_cast<unsigned>(result));
        return result;
    }
    void apply_bindings() { input::apply_dirty_bindings(profile_bindings(),input_.state(),input_); }
    void select_game_binding_mode(std::uint32_t mode) { //006B7400→006A8690 on SAME profile/input owners.
        input::select_binding_mode(profile_bindings(),input_.state(),input_,mode);
    }
    void reset_bindings(std::uint32_t layout) {
        input::reset_binding_defaults(profile_bindings(),input_.state(),input_,layout);
    }
    bool gamma_available() const {
        //0061BEA0 checks the initialization settings, NOT adapter capabilities.
        //1006DA30 returns100D1D18; +2c is the fullscreen flag word. The
        //launch mode controls bit2 independently of the quality preset.
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
    std::string game_text_directory(){return language_.text_directory();} //0048FBC0 actual installed language prefix.
    const auto& profile_movies() { return data_.profile().payload.movies.records(); }
    bool subtitles_enabled() { return data_.profile().options.settings.bytes[0x3a] != 0; }
    bool load_movie_subtitles(const char* filename, frontend::LocalizedTextCache& text) {
        const auto path = frontend::opening_subtitle_path(directory_.get(), language(), filename);
        if (!platform_.probe_text_file(path)) return false;
        return frontend::load_localized_text_file(text, text_files_, path);
    }
    frontend::OpeningTextStyles& opening_text_styles() { return opening_text_styles_; }
    game::HudSignalTables& hud_signal_tables(){return hud_signal_tables_;}
    game::HudCompassShared& hud_compass_state(){return hud_compass_state_;}
    game::HudClickSound<MenuDriver::Owner>& hud_click_sound(){return hud_click_sound_;}
    game::ConsoleVariables& game_console_variables(){return game_console_variables_;}
    game::TextRenderControls& game_text_render_controls(){return game_text_render_controls_;}
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
    float& camera_breathing_amount(){return camera_breathing_;} //00839268 persistent process FLOAT .005, modified by00466970.
    bool camera_mouse_inverted(){return profile_settings().bytes[0x39]!=0;} //00469300 reads current006B6740 settings.
    const std::array<float, 2> &menu_cursor() const { return menu_cursor_; }
    void menu_cursor(float x, float y) { menu_cursor_ = {x, y}; }
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
    std::int32_t game_music_request()const{return sound_data_.selection.requested;} //SOUND+70, SAME primary owner.
    void queue_game_music(std::int32_t track,std::int32_t fade,std::uint32_t position){ //SOUND+6C.
        frontend::queue_primary_sound_movie(sound_data_.settings,sound_data_.fade,
            sound_data_.secondary,sound_data_.selection,track,fade,position);
    }
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
        errors.attempt([&] {clear_game_dubbing();});
        errors.attempt([&] { frontend::reset_sound_runtime(sound_data_.runtime, sound_catalog_, files_,
            sound_data_.settings, sound_data_.fade, sound_data_.secondary, sound_data_.selection,
            gate_, frontend::SoundApplicationMode{nullptr, 0}, 1, 0, sound_backend_); });
        errors.attempt([&] { frontend::clear_sound_sectors(sound_data_.runtime, sound_backend_); });
        errors.attempt([&] { sound_catalog_.clear(); });
        errors.rethrow();
    }
    void control_phase(std::uint32_t milliseconds,bool fixed){
        if(source_packet_path())local_packets_.phase(fixed);else control_frame_.phase(milliseconds,fixed);
    }
    void control_maintenance(std::uint32_t milliseconds){
        if(source_packet_path())local_packets_.maintain(milliseconds);else control_frame_.maintain(milliseconds);
    }
    void reset_control(){
        if(source_packet_path())control_frame_.reset_packets([&](std::uint32_t elapsed){local_packets_.flush(elapsed);},[&]{local_packets_.clear();});
        else control_frame_.reset();
    }
    auto& game_broadcast_packets(){return broadcast_packets_;}
    auto& game_local_packets(){return local_packets_;}
    void bind_game_packet_world(game::SessionPacketWorld& world){packet_world_.bind(world);}
    void unbind_game_packet_world(game::SessionPacketWorld& world)noexcept{packet_world_.unbind(world);}
    void enter_control_lock(unsigned index) { lock_services_.enter_control_lock(index); }
    void leave_control_lock(unsigned index) noexcept { lock_services_.leave_control_lock(index); }
    void clear_retired_actor_record(std::int32_t identity){frontend::clear_retired_actor_record(control_data_,lock_services_,identity);}
    void clear_human_player_slot(std::uint32_t identity){frontend::clear_human_player_slot(control_data_,lock_services_,identity);}
    frontend::ControlStartupState& game_control_records(){return control_data_;} //PreparedSpawnPackets borrows SAME1DC storage and acquires lock0 for each access.
    std::uint32_t control_packet_clock()const {return static_cast<std::uint32_t>(std::clock());} //007E814F CRT clock(), packet timestamps.
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
    std::unique_ptr<PreMission> pre_mission_;
    std::optional<frontend::MultiplayerLaunchRequest> multiplayer_request_;
    ChunkFiles files_;
    DtaTextFiles text_files_;
    StartupLocalizationPlatform platform_;
    frontend::ExecutableDirectory<StartupLocalizationPlatform> directory_;
    frontend::ApplicationLanguage<StartupLocalizationPlatform> language_;
    frontend::OpeningTextStyles opening_text_styles_;
    game::HudSignalTables hud_signal_tables_;
    game::HudCompassShared hud_compass_state_;
    game::HudClickSound<MenuDriver::Owner> hud_click_sound_;
    bool source_packet_path()const{
        return packet_world_.bound()||broadcast_packets_.pending()||!local_packets_.authority_queue().empty()||!local_packets_.presentation_queue().empty();
    }
    game::ConsoleVariables game_console_variables_;
    game::TextRenderControls game_text_render_controls_;
    HumanPortraits portraits_;
    frontend::ApplicationStartupData<HumanPortraits> data_;
    StartupArchiveBackend archive_backend_;
    io::StartupArchives<StartupArchiveBackend> archives_;
    GraphicsResourceDirectories graphics_directories_;
    io::ResourceDirectoryCaches<GraphicsResourceDirectories> caches_;
    LoadedResources loaded_resources_;
    GameAnimations game_animations_;
    game::HumanSoundVariants human_sound_variants_;
    game::HumanInputShared human_input_shared_;
    game::SessionBroadcastPackets broadcast_packets_; //SAME native208/244/280, process/control-lived, not World-owned.
    game::ActorProximityState<GameSceneActors::Instance*> actor_proximity_;
    EngineFrame* car_loading_scene_=nullptr;
    game::CarDefinitionTables car_definitions_;game::CarTyres car_tyres_;game::CarMaterialRegistry car_materials_;
    std::int32_t game_vertex_clock_=0;
    float game_sight_work_clock_=1.f;
    game::CarSeatCameraShared car_seat_camera_;
    std::vector<EngineFrame*> car_light_receivers_,car_sound_receivers_;
    game::PlayersProcess<PlayerHuman> players_process_;
    std::vector<game::PlayersProcess<PlayerHuman>::DelayedSpeech> delayed_player_speech_;
    game::HumanMorphTables human_morph_tables_;
    float camera_breathing_=0x1.47ae14p-8f; //Original00839268 bytes0A D7 A3 3B, retained across worlds.
    const float human_animation_time_scale_=0x1.666666p+0f;
    game::HumanPhysicsGlobals human_physics_globals_;
    game::MissionIntegrityTables mission_integrity_tables_;
    game::MissionScriptVariables mission_script_variables_;
    game::MissionEffectsProcess<EngineFrame*> mission_effects_process_;
    game::ShotManagerLoadingState<GameActorServices> shot_manager_process_;
    game::HeavyFireLoadingState heavy_fire_process_;
    game::EffectsLoadingProcess<EngineFrame*> effects_loading_process_;
    BlastWaves blast_wave_process_;
    game::HumanProjectionProcess<MenuDriver::Owner> human_projection_process_;
    game::ItemHandModels<GameActorServices> item_hand_models_;
    game::HumanAIEventPool human_ai_events_;
    game::HumanAIFrameInputs human_ai_frame_inputs_;
    game::HumanSceneMovementProcess<GameSceneActors::Instance,GameSceneActors::Human,
        game::CarSceneActor<GameActorServices>> human_scene_movement_process_;
    game::HumanPerceptionRecordPool<GameSceneActors::Instance*> human_perception_records_;
    game::HumanAIActionPools human_ai_actions_;
    game::HumanAINavigationJobPool human_ai_navigation_jobs_;
    ResourceDirectoryPlatform directory_platform_;
    JoystickBackend joysticks_backend_;
    input::JoystickDevices<JoystickBackend> joysticks_;
    GraphicsInputServices<JoystickBackend> input_backend_;
    input::InputManager<GraphicsInputServices<JoystickBackend>> input_;
    GraphicsInputSetup<input::JoystickDevices<JoystickBackend>> input_setup_;
    SettingsServices settings_backend_;
    game::WorldViews game_views_; // Borrowed owner/scene; audio stays null until an actual loaded listener.
    DubbingSoundServices sound_backend_;
    frontend::SoundCatalog<SoundServices> sound_catalog_;
    frontend::DubbingSound<DubbingSoundServices,ChunkFiles> dubbing_sound_;
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
    const char* missing_speech_caption_=nullptr; //Actual process fallback borrower; distinct from game+60 text entries.
    frontend::ColdControlFrame<StartupSession> control_frame_;
    game::SessionPacketWorldLink packet_world_;
    game::SessionLocalPackets<StartupSession,game::SessionPacketWorldLink> local_packets_;
    std::uint8_t input_flag_ = 0;
    std::array<float, 2> menu_cursor_{{400, 300}}; //0066A4E0 centers only the first shared owner.
    bool application_input_enabled_ = true; // Original00843470 initial byte1.
    bool script_advance_override_=false; //Original008692D0 BSS; never a fabricated loading-success flag.
    bool control_ready_ = false, setup_started_ = false;
    bool input_setup_started_ = false, mouse_attempted_ = false;
    Phase phase_ = Phase::constructed;
    HWND archive_focus_ = nullptr;
    BOOL directory_window_ = FALSE;
    LARGE_INTEGER control_frequency_{}, control_origin_{};
};

}
#endif
