#ifndef HD2_HOST_GAME_SESSION_ACTIVATION_H
#define HD2_HOST_GAME_SESSION_ACTIVATION_H
#include "host/game_mission_loading.h"
#include "host/game_prepared_human_loading.h"
#include "game/mission_scene_activation.h"
#include "game/session_world_input.h"
#include "game/session_selection.h"
#include "game/session_context_marker.h"
#include "game/human_squad_snapshot.h"
#include "game/session_finish.h"
namespace hd2::host {

//Actual006989B0 ordinary offline stage receivers around the completed cold
//loading allocation. Every service borrows the SAME parent/process/world; no
//second squad/Items/input/roster/music state. Controller is the real containing
//application controller (006DC940); null is its genuine absent-pointer branch.
//Still source-only: this does not initialize protection observations, render a
//first frame, supply full00698F80 finish or enable the application entry path.
class GameSessionTransientFinish {
public:
    virtual ~GameSessionTransientFinish()=default;
    virtual void clear_all_shared_records()=0; //006DF390(0,true); actual008AED48 owner, not proximity008AED54.
};
template<class Controller>
class GameSessionActivation {
public:
    using World=world_loading::World;using Parent=World::Parent;
    using InputBinding=game::SessionWorldInput<world_loading::Inputs,frontend::ControlStartupState>;
    GameSessionActivation(GameMissionLoadingSession& loading,Parent& parent,GameOwnerBackend& backend,
            StartupSession& startup,Graphics& graphics,HMODULE module,FrameClockPlatform& clock,
            game::SessionActivationState& session,Controller* controller)
        :loading_(loading),parent_(parent),backend_(backend),startup_(startup),graphics_(graphics),clock_(clock),session_(session),controller_(controller),
         d_(loading.world().owners().dependencies()),queries_(module),
         selection_(startup.pre_mission().game(),startup.pre_mission().route(),parent.request().preparation),
         input_(loading.world().owners().input_profiles(),startup.human_input_shared(),&d_.inputs,startup.game_control_records(),
                startup.local_player_identity(),startup.application_flags_owner()),lights_(d_.scene.frames()),prepared_(d_){
        if(&d_.startup!=&startup||&d_.session!=&session||&d_.request!=&parent.request()||
           &parent.live_squad()!=&startup.pre_mission().squad())
            throw std::logic_error("Activation requires SAME actual loading/session/application squad owners");
    }
    bool activate(){return game::activate_offline_session(session_,*this,parent_.request().mission.get());}
    bool finish(GameSessionTransientFinish& transients){
        transients_.bind(transients);
        struct Unbind {game::WorldLoadingDependencyLink<GameSessionTransientFinish>& link;GameSessionTransientFinish& owner;~Unbind(){link.unbind(owner);}} unbind{transients_,transients};
        return game::finish_offline_session(session_,*this);
    }
    std::uint32_t mode_flags()const{return startup_.application_flags();}
    const game::SessionSelection& application_data()const{return selection_;}
    std::uint32_t selected_mission_ordinal(std::uint32_t root)const{return selection_.selected_mission_ordinal(root);}
    std::uint32_t activation_flags()const{
        const auto* settings=graphics_.methods->settings(&graphics_);
        if(!settings)throw std::runtime_error("Activation requires actual graphics settings");return settings->flags;
    }
    std::int32_t activation_flags(std::uint32_t flags){return graphics_.methods->set_flags(&graphics_,flags);}
    bool load_mission(const std::string& resource,const frontend::GameDataNode* mission){return loading_.loader().load(resource,mission);}
    const std::string& current_mission_resource()const{return parent_.request().preparation.mission_resource;}
    void loading_progress(unsigned stage,float value){loading_.loader().loading_progress(stage,value);}
    DWORD query_activation_ticks(){return queries_.query_activation_ticks();}
    BOOL query_activation_visible_window(){return queries_.query_activation_visible_window();}
    HWND query_activation_capture(){return queries_.query_activation_capture();}
    auto& files(){return startup_.pre_mission_files();}
    auto& car_definitions(){return startup_.car_definitions();}
    auto& car_tyres(){return startup_.car_tyres();}
    auto& car_materials(){return startup_.car_materials();}
    auto& car_particles(){
        auto* callbacks=dynamic_cast<GameCarCallbacks*>(&d_.scene.car_callbacks());
        if(!callbacks)throw std::logic_error("Activation requires SAME actual Car callback pool");return *callbacks;
    }
    void clear_game_queues(){backend_.clear_game_queues();}
    void rebuild_session_world_bounds(){d_.bounds.rebuild(d_.scene.frames(),input_);}
    auto& mission_light_visibility(){return lights_;}
    bool human_face_loading_enabled(){return startup_.human_face_quality_enabled();}
    auto& human_faces(){return d_.faces;}
    auto& module_registry(){return startup_.game_module_registry();}
    void activate_mission_module(frontend::ClientModuleRegistration& module,bool authority){
        loading_.world().modules().dispatch().activate_mission_module(module,authority);
    }
    void initialize_loaded_hud(){
        if(!d_.hud.bind(current_mission_resource(),mode_flags(),false))throw std::runtime_error("Actual mission HUD initialization failed");
    }
    void initialize_loaded_first_person(){d_.first_person.activate(mode_flags(),files(),clock_);}
    auto& live_squad(){return parent_.live_squad();}
    auto* loading_spawn_point(std::uint32_t team){return d_.scene.frames().loading_spawn_point(team);}
    void spawn_prepared_human(GameSceneActors::Spawn& spawn,frontend::PreMissionSoldier& soldier){
        game::spawn_prepared_human(d_.scene.frames(),d_.scene.frames(),d_.catalogue,prepared_,
            loading_.world().owners().spawn_records(),prepared_,spawn,soldier,mode_flags());
    }
    void initialize_loaded_dubbing(){startup_.initialize_game_dubbing();}
    bool has_mission_text_definition()const{return session_.resource!=nullptr;}
    void initialize_loaded_mission_text(){(void)d_.subtitles.initialize(files(),startup_.game_text_directory(),startup_.wide_characters(),backend_);}
    auto& game_music(){return d_.music;}
    std::int32_t game_music_request()const{return startup_.game_music_request();}
    void queue_game_music(std::int32_t track,std::int32_t fade,std::uint32_t position){startup_.queue_game_music(track,fade,position);}
    void reset_loading_clock(){loading_.loader().finish_activation_screen();} //006C95C0 is loading-screen finish gate.
    bool has_application_game_context()const{return controller_!=nullptr;}
    bool restore_session_context(){
        if(!controller_)return false;
        const auto* active=controller_->active();if(!active||active->type()!=1)return false;
        const auto& request=parent_.request().preparation;
        const auto path=game::session_quick_context_path(startup_.pre_mission_profile_directory(),request.standalone?1u:0u,request.mode);
        return game::read_session_context_marker(files(),path,session_.words.template get<std::uint32_t>(0x3c),active->type());
    }
    void snapshot_live_squad(){game::snapshot_live_squad(d_.scene.frames(),parent_.live_squad(),d_.world_actors.items(),d_.catalogue,d_.profiles,mode_flags());}
    void close_first_person(){d_.first_person.close(mode_flags());}
    void deactivate_mission_module(frontend::ClientModuleRegistration& module,bool authority){loading_.world().modules().dispatch().deactivate_mission_module(module,authority);}
    void finish_loaded_dubbing(){startup_.finish_game_dubbing();}
    void close_loaded_hud(){(void)d_.hud.close();}
    HWND query_finish_focus(){return queries_.query_finish_focus();}
    void release_finished_collision_tree(){loading_.world().release_collision_tree();}
    void clear_car_definition_tables(){startup_.car_definitions().finish_mission();}
    void clear_loaded_mission_text(){parent_.text()->clear();}
    void reset_navigation_runtime_links(){d_.navigation.reset_runtime_links();}
    void clear_car_particle_pool(){car_particles().clear_particles();}
    void clear_human_face_resources(){d_.faces.clear();}
    void clear_shared_finish_resources(){transients_.get().clear_all_shared_records();} //Required containing process receiver; no successful default.
    auto& world_bounds(){return d_.bounds;}
private:
    GameMissionLoadingSession& loading_;Parent& parent_;GameOwnerBackend& backend_;StartupSession& startup_;
    Graphics& graphics_;FrameClockPlatform& clock_;game::SessionActivationState& session_;Controller* controller_;
    world_loading::Dependencies& d_;GameLoadingQueries queries_;game::SessionSelection selection_;InputBinding input_;
    game::WorldLoadingDependencyLink<GameSessionTransientFinish> transients_;
    game::MissionLightVisibility<GameActorServices> lights_;GamePreparedHumanLoading prepared_;
};
} //namespace hd2::host
#endif
