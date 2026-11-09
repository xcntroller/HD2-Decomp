#ifndef HD2_HOST_GAME_OWNER_H
#define HD2_HOST_GAME_OWNER_H

#include "frontend/application_states.h"
#include "game/session_owner.h"
#include "game/session_activation.h"
#include "game/mission_scene_activation.h"
#include "game/prepared_human_spawn.h"
#include "game/prepared_human_clothing.h"
#include "game/prepared_spawn_packets.h"
#include "game/session_world_input.h"
#include "game/offline_spawn_presentation.h"
#include "game/session_local_packets.h"
#include "game/scene_callback.h"
#include "game/activation_flags.h"
#include "game/hud_owner.h"
#include "game/text_state.h"
#include "game/table_storage.h"
#include "game/actor_base.h"
#include "game/actor_references.h"
#include "game/human_passive_states_loading.h"
#include "game/human_input_session_profiles.h"
#include "game/human_input_loading_manager.h"
#include "game/human_player_state_loading.h"
#include "game/human_player_roster.h"
#include "game/game_music_references.h"
#include "game/first_person_animation_table.h"
#include "game/first_person_model_loading.h"
#include "game/first_person_animation_playback.h"
#include "game/first_person_animation_sound.h"
#include "game/first_person_human_items.h"
#include "game/first_person_view_loading.h"
#include "game/hud_cursor_loading.h"
#include "game/hud_artwork_loading.h"
#include "game/hud_command_routes.h"
#include "game/hud_text_loading.h"
#include "game/hud_small_panels_loading.h"
#include "game/hud_auxiliary_loading.h"
#include "game/hud_weapon_selector_loading.h"
#include "game/hud_injury_movies_loading.h"
#include "game/hud_dialogs_loading.h"
#include "game/hud_hands_loading.h"
#include "game/hud_objective_icons_loading.h"
#include "game/hud_soldier_loading.h"
#include "game/hud_compass_loading.h"
#include "game/hud_messages_loading.h"
#include "game/hud_crosshair_loading.h"
#include "game/hud_scoreboard_loading.h"
#include "game/hud_explosive_timers_loading.h"
#include "game/hud_scroll_loading.h"
#include "game/hud_inventory_loading.h"
#include "game/first_person_hands_material.h"
#include "game/scene_actor_registry.h"
#include "game/simple_scene_actors.h"
#include "game/ladder_geometry.h"
#include "game/supply_box_geometry.h"
#include "game/switch_actor_state.h"
#include "game/door_portal_binding.h"
#include "game/car_physics.h"
#include "game/car_contact_callbacks.h"
#include "game/car_particles.h"
#include "game/car_particle_pool.h"
#include "game/car_definition_tables.h"
#include "game/effect_definitions.h"
#include "game/mission_module_dispatch.h"
#include "game/mission_tracks.h"
#include "game/mission_camera.h"
#include "game/mission_vertex_instances.h"
#include "game/mission_script_bindings.h"
#include "game/runtime_item.h"
#include "game/item_identity.h"
#include "game/mission_item_records.h"
#include "game/item_model_cache.h"
#include "game/volumetric_lights.h"
#include "game/dynamic_collision_grid.h"
#include "game/collision_ray.h"
#include "game/rigid_body.h"
#include "game/dynamic_item_actor.h"
#include "game/car_tyres.h"
#include "game/car_collision_parts.h"
#include "game/car_wheel_geometry.h"
#include "game/car_deformable_parts.h"
#include "game/car_axes.h"
#include "game/car_auxiliary_geometry.h"
#include "game/car_strip_connections.h"
#include "game/car_default_parameters.h"
#include "game/car_lighting.h"
#include "game/car_geometry.h"
#include "game/car_ancillary_model.h"
#include "host/game_scene.h"
#include "host/game_collision_resources.h"
#include "host/game_car_frames.h"
#include "host/game_actor_frames.h"
#include "host/game_actor_services.h"
#include "host/game_scene_actors.h"
#include "host/game_mission_scene.h"
#include "host/game_runtime_item.h"
#include "host/game_mission_items.h"
#include "host/game_physics_objects.h"
#include "host/game_speech_definitions.h"
#include "host/game_human_audio_loading.h"
#include "host/game_runtime_actors.h"
#include "host/game_mesh_movie.h"
#include "host/game_car_callback_frames.h"
#include "host/game_loading_screen.h"
#include "host/game_fonts.h"
#include "host/game_text_renderer.h"
#include "host/startup_session.h"
#include "game/hud_info_loading.h"
#include "game/hud_command_loading.h"
#include "game/hud_map_loading.h"
#include "game/hud_panels_loading.h"
#include "game/hud_scene_loading.h"
#include "game/hud_actor_identity_view.h"

namespace hd2::host {

class GameOwnerBackend : public game::TableStorage {
public:
    using DisplayText = game::TextState<GameFonts>;
    using Hud = game::HudOwner<DisplayText>;
    using Text = game::MissionText<DisplayText>;
    using Scene = GameScene;
    struct FrameRelease {
        MenuDriver::Release release = nullptr;
        void operator()(EngineFrame* frame) const noexcept { if (frame) release(frame); }
    };
    using FrameOwner = std::unique_ptr<EngineFrame, FrameRelease>;
    GameOwnerBackend(StartupSession& startup, MenuDriver& driver, HMODULE module,
                     Graphics& graphics, GameSceneCallbacks& callbacks)
        : startup_(startup), driver_(driver), module_(module), graphics_(graphics), callbacks_(callbacks) {}
    std::shared_ptr<GameFonts> make_hud_fonts() {
        auto fonts=fonts_.lock();
        if(!fonts){fonts=std::make_shared<GameFonts>(startup_.pre_mission_files(),&graphics_,module_);fonts_=fonts;}
        return fonts;
    }
    std::uint8_t hud_crosshair_allowed(){return startup_.hud_crosshair_allowed();}
    std::int32_t hud_style224(){return startup_.game_configuration().hud_style224();}
    std::uint32_t hud_teams240(){return startup_.game_configuration().hud_teams240();}
    void hud_binding_mode(std::uint32_t mode){startup_.select_game_binding_mode(mode);}
    std::int32_t hud_no_gi(){return startup_.game_console_variables().integer(10);} //0049BA12→00720B80(10).
    std::int32_t hud_display_height(){return graphics_.methods->height(&graphics_);}
    std::int32_t hud_display_width(){return graphics_.methods->width(&graphics_);}
    void hud_draw_text(DisplayText& text){
        GameTextRenderer renderer(&graphics_,driver_,*text.fonts());
        game::draw_text_byte_atlas(text,startup_.opening_text_styles(),renderer,startup_.game_text_render_controls());
    }
    template<class Messages>void hud_draw_messages(Messages& messages){
        GameTextRenderer renderer(&graphics_,driver_,*messages.fonts());messages.draw_bytes(renderer);
    }
    void hud_draw_command_route(const game::HudRouteVertex* vertices,std::uint32_t count){
        //004B7780, Graph84→1006B360. Original00810550 is the identity matrix;
        //repeat texture/transform setup for EACH nonempty native line strip.
        constexpr std::array<float,16> identity{{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1}};
        if(!graphics_.methods->bind_texture(&graphics_,nullptr,0))throw std::runtime_error("Cannot clear HUD route texture");
        if(graphics_.methods->set_world_transform(&graphics_,identity.data())<0)throw std::runtime_error("Cannot set HUD route world transform");
        if(graphics_.methods->draw_primitive(&graphics_,2,count,vertices,0)<0)throw std::runtime_error("Cannot draw HUD command route");
    }
    void hud_draw_selection_vertices(const std::array<game::TextGlyphVertex,16>& vertices){
        if(!graphics_.methods->bind_texture(&graphics_,nullptr,0))throw std::runtime_error("Cannot clear HUD selection texture");
        if(graphics_.methods->draw_primitive(&graphics_,1,8,vertices.data(),1)<0)throw std::runtime_error("Cannot draw HUD actor selection");
    }
    std::uint32_t hud_graphics_state(std::uint32_t state){return graphics_.methods->state(&graphics_,state);}
    void hud_graphics_state(std::uint32_t state,std::uint32_t value){
        graphics_.methods->set_state(&graphics_,state,value); //1006ABC0 has no defined status on its cached no-op return.
    }
    std::unique_ptr<DisplayText> make_hud_text() {
        auto fonts=make_hud_fonts();
        return std::make_unique<DisplayText>(graphics_.methods->width(&graphics_),graphics_.methods->height(&graphics_),std::move(fonts));
    }
    frontend::SharedText hud_localized(std::uint32_t id) {
        return std::make_shared<const std::string>(frontend::lookup_localized_text(startup_.text(),static_cast<std::int32_t>(id)));
    }
    std::unique_ptr<Hud> make_hud() {return std::make_unique<Hud>([this]{return make_hud_text();});}
    std::unique_ptr<Text> make_mission_text() { return std::make_unique<Text>(); }
    std::unique_ptr<Scene> make_game_scene() {
        return std::make_unique<Scene>(driver_, module_, graphics_, callbacks_);
    }
    //004ECDE0/004ECCD0's transient references and optional owned queue payloads.
    // These are real native queues, initially empty, not calls into the original
    // client. Future producers must use these same owners instead of private copies.
    void clear_game_queues() noexcept {
        transient_references_.clear();
        while (!transient_payloads_.empty()) transient_payloads_.pop_back();
    }
private:
    struct QueuePayload { std::vector<unsigned char> bytes; };
    StartupSession& startup_;
    MenuDriver& driver_;
    HMODULE module_;
    Graphics& graphics_;
    GameSceneCallbacks& callbacks_;
    std::weak_ptr<GameFonts> fonts_;
    std::vector<void*> transient_references_;
    std::vector<std::unique_ptr<QueuePayload>> transient_payloads_;
};

// Kind7's actual type1 game owner,00698510/006987E0, installed by the controller.
// Construction/teardown are available. Activation006989B0 still cannot succeed:
// P2.2 supplies its real006CA780 loader and post-load systems. Do not report this
// constructed scene as a loaded world or render it with a substitute camera.
class GameApplicationState final : public frontend::ApplicationState, public GameSceneCallbacks {
public:
    using Owner = game::SessionOwner<GameOwnerBackend>;
    GameApplicationState(StartupSession& startup, MenuDriver& driver, HMODULE module,
                         Graphics& graphics, game::MissionRequest request)
        : frontend::ApplicationState(1), startup_(startup), graphics_(graphics),
          backend_(startup, driver, module, graphics, *this),
          request_(std::move(request)) {}
    ~GameApplicationState() override { shutdown(); }
    const frontend::GameDataNode* mission_descriptor() const { return request_.mission.get(); }
    bool initialize(const void* descriptor) override {
        if (!descriptor || descriptor != request_.mission.get())
            throw std::logic_error("Game initializer requires its owned mission descriptor");
        if (owner_ || attempted_) throw std::logic_error("Game initialization repeated");
        attempted_ = true;
        try {
            owner_ = std::make_unique<Owner>(backend_, registry_, std::move(request_));
            startup_.game_views().bind(this, owner_->scene()->frame());
            owner_->scene()->rethrow_callback_failure();
            std::printf("Game owner constructed: mission=%s soldiers=%zu text-owners=%zu scene=%p status=%u loaded=0\n",
                owner_->request().preparation.mission_resource.c_str(),
                owner_->request().preparation.squad.size(), owner_->hud()->text_owners().size(),
                static_cast<void*>(owner_->scene()->frame()), status());
            // Preserve006989B0's graphics-flags restoration on a failed load.
            game::with_activation_flags(*this, [] { return false; });
            failure_ = "Mission loading is not implemented in this build.";
            std::puts("Game activation declined:006CA780 loading pipeline is unavailable; graphics flags restored; loaded=0");
        } catch (const std::exception& error) {
            failure_ = error.what();
            std::fprintf(stderr, "Game owner initialization failed: %s\n", failure_.c_str());
        }
        return false; //006989B0 zero=failure; never set running/status5 here.
    }
    const std::string& failure() const { return failure_; }
    void shutdown() override {
        startup_.game_views().unbind(this);
        owner_.reset();
        status(0);
    }
    std::uint32_t dispatch_game_scene(std::uint32_t stage) override {
        return game::scene_callback(stage, *this);
    }
    std::uint32_t activation_flags() const {
        const auto* settings = graphics_.methods->settings(&graphics_);
        if (!settings) throw std::runtime_error("Mission graphics settings are unavailable");
        return settings->flags;
    }
    std::int32_t activation_flags(std::uint32_t flags) { return graphics_.methods->set_flags(&graphics_, flags); }
    bool has_game() const { return owner_ != nullptr; }
    std::uint32_t mode_flags()const{return startup_.application_flags();} //SAME process00982100; offline single-player7.
    std::uint32_t game_status() const { return status(); }
    bool suppress_hud() const { return false; } //0086922D initial byte; no loaded HUD mutation yet.
    bool full_viewport() const { return false; } //0086922C initial byte.
    bool debug_enabled() const { return false; } //0098AF98 initial byte.
    bool hud_enabled() const {
        if (!owner_ || !owner_->hud()) throw std::logic_error("HUD owner is not available");
        return owner_->hud()->enabled();
    }
    CursorViewport graphics_viewport() const {
        const auto* viewport = graphics_.methods->viewport(&graphics_);
        if (!viewport) throw std::runtime_error("Game viewport is unavailable");
        return *viewport;
    }
    void graphics_viewport(const CursorViewport& viewport) {
        if (graphics_.methods->set_viewport(&graphics_, &viewport) < 0)
            throw std::runtime_error("Cannot set game graphics viewport");
    }
    std::uint32_t graphics_width() const { return graphics_.methods->width(&graphics_); }
    std::uint32_t graphics_height() const { return graphics_.methods->height(&graphics_); }
    std::uint32_t game_word24() const { require_world(); }
    void before_hud() const { require_world(); }
    void render_hud() const { require_world(); }
    void control_before_scene() const { require_world(); }
    void control_after_scene() const { require_world(); }
    void render_debug() const { require_world(); }
    void render_network_overlay() const { require_world(); }
    void render_game_fade() const { require_world(); }
    void render_effects_overlay() const { require_world(); }
    void render_mission_text() const {
        if (!owner_ || !owner_->text()) throw std::logic_error("Mission text owner is not available");
        if (owner_->text()->drawable(startup_.profile_settings().bytes[0x3a] != 0)) require_world();
        //0048F390's actual disabled/missing-renderer branch requires no draw.
    }
    void input(const std::uint32_t*, const std::uint32_t*) override { require_world(); }
    void fixed_update(std::uint32_t) override { require_world(); }
    void update(std::uint32_t) override { require_world(); }
    void render() override { require_world(); }
private:
    [[noreturn]] static void require_world() { throw std::logic_error("Game world is not activated"); }
    inline static Owner::Registry registry_;
    StartupSession& startup_;
    Graphics& graphics_;
    GameOwnerBackend backend_;
    game::MissionRequest request_;
    std::unique_ptr<Owner> owner_;
    std::string failure_;
    bool attempted_ = false;
};

} // namespace hd2::host
#endif
