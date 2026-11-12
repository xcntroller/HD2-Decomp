#ifndef HD2_HOST_GAME_SESSION_FRAMES_H
#define HD2_HOST_GAME_SESSION_FRAMES_H

#include "host/game_mission_module_frames.h"
#include "host/game_blast_waves.h"
#include "game/session_frame_loading.h"

namespace hd2::host {
//Required OTHER owners. UI's unmerged receivers and OWN/controller identity
//must use the given SAME World. No successful fallback, default flag or input
//buffer is installed. Human0053BF90 and non7 control/peer bodies remain explicit.
class GameSessionFrameExternal {
public:
    using Human=world_loading::Human;
    virtual ~GameSessionFrameExternal()=default;
    virtual world_loading::World& world()const=0;
    virtual bool is_current_application_state()=0; //Actual controller14 identity.
    virtual void request_application_state(std::uint32_t)=0; //Actual controller04, no immediate activation.
    virtual bool mission_save_blocked()=0; //0067FAA0, queried only with missionA8.
    virtual bool application_saved_state_present()=0; //Actual application descriptor+4.
    virtual void show_session_warning(std::uint32_t caption,bool format_zero)=0; //0069BF70, RGB255;2536 uses00405100(format,0).
    virtual void filter_hud_input(std::uint32_t*,std::uint32_t*)=0; //0049FC10/006C71C0.
    virtual bool hud_nonoffline_input_blocked()=0; //0049F090.
    virtual bool hud_player_input_blocked()=0; //0049D300.
    virtual void process_hud_controls(std::uint32_t*,std::uint32_t*)=0; //0049C3C0, actual64-word input views.
    virtual void update_loaded_hud(std::uint32_t)=0; //Latest UI advance_hud with its real sources.
    virtual void advance_first_person(std::uint32_t)=0; //00491E90, latest UI advance_first_person.
    virtual void process_player_input(Human&,std::uint32_t*,std::uint32_t*)=0; //005A3DE0→0053BF90.
    virtual void end_session_cutscene()=0; //00423EB0 actual script process; no end flag substitute.
    virtual void process_nonoffline_session_input(std::uint32_t*,std::uint32_t*)=0; //0069AAB0.
    virtual Human* network_input_player()=0; //00730460 actual local peer3C borrower.
};
class GameSessionFrameCallbacks {
public:
    virtual ~GameSessionFrameCallbacks()=default;
    virtual void input(std::uint32_t*,std::uint32_t*)=0;
    virtual void fixed_update(std::uint32_t)=0;
    virtual void update(std::uint32_t)=0;
    virtual void retire()noexcept=0;
};

//Callable0069A560/0069AF10/0069B230 composition. OWN retains this borrower,
//the supplied actual Runtime and External until retire(), BEFORE lifecycle
//finish/unload/World destruction. No scheduling, presentation or activation is
//moved out of the application frame. No second camera/clock/process is owned.
template<class Runtime>class GameSessionFrames final:public GameSessionFrameCallbacks {
public:
    using World=world_loading::World;using Human=world_loading::Human;using Parent=World::Parent;
    GameSessionFrames(World& world,Parent& parent,const game::SessionSelection& selection,
            GameBlastWaves& blast,GameSessionFrameExternal& external,Runtime& runtime)
        :world_(world),parent_(parent),d_(world.owners().dependencies()),selection_(selection),
         blast_(blast),external_(external),modules_(world,runtime),camera_(world.owners()){
        if(!parent_.constructed()||&external_.world()!=&world_||&blast_.dependencies()!=&d_||
           &d_.request!=&parent_.request()||selection_.accepted_request()!=&parent_.request()||
           d_.scene.frames().world_scene_frame()!=parent_.scene()->frame())
            throw std::logic_error("Session FRAME requires its SAME World/parent/selection/blast/external owners");
    }
    GameSessionFrames(const GameSessionFrames&)=delete;GameSessionFrames& operator=(const GameSessionFrames&)=delete;
    void input(std::uint32_t* first,std::uint32_t* second)override{
        require_open();io::observe_data_operation(store(),"0069A560 session input",[&]{(void)game::input_loading_session(d_.session,*this,first,second);});
    }
    void fixed_update(std::uint32_t elapsed)override{
        require_open();io::observe_data_operation(store(),"0069AF10 fixed session frame",[&]{game::fixed_loading_session(d_.session,*this,elapsed);});
    }
    void update(std::uint32_t elapsed)override{
        require_open();io::observe_data_operation(store(),"0069B230 variable session frame",[&]{game::update_loading_session(d_.session,d_.music,*this,elapsed);});
    }
    void retire()noexcept override{retired_=true;} //Consumes the source borrower; does not finish/simulate the World.
    std::uint32_t mode_flags()const{return d_.startup.application_flags();}
    bool is_current_application_state(){return external_.is_current_application_state();}
    bool full_viewport()const{return d_.startup.scene_presentation_flags().full_viewport;}
    bool hud_enabled()const{return parent_.hud()->enabled();}
    void filter_hud_input(std::uint32_t* a,std::uint32_t* b){external_.filter_hud_input(a,b);}
    void update_loaded_hud(std::uint32_t elapsed){external_.update_loaded_hud(elapsed);}
    void show_loaded_hud(bool)noexcept{} //0049B7C0 is actual RET4, not a flags1 setter.
    void players_control_fixed(std::uint32_t elapsed){d_.players.control().advance_fixed(elapsed,d_.session.mission,*this);}
    void players_control_presentation(std::uint32_t elapsed){d_.players.control().advance_presentation(elapsed);}
    void tick_authority_modules(std::uint32_t elapsed){modules_.tick(true,elapsed);}
    void tick_presentation_modules(std::uint32_t elapsed){modules_.tick(false,elapsed);}
    void advance_authority_only_humans(std::uint32_t elapsed){modules_.tick_authority_only_humans(elapsed);}
    void advance_shared_transient_records(std::uint32_t elapsed){blast_.update(elapsed);}
    void recycle_collision_buckets(){d_.scene.buckets().recycle_empty();}
    void advance_first_person(std::uint32_t elapsed){external_.advance_first_person(elapsed);}
    void advance_engine_scene(std::uint32_t elapsed){parent_.scene()->advance(elapsed);}
    void advance_animated_models(std::uint32_t elapsed){d_.scene.frames().advance_animated_models(elapsed);}
    void advance_dubbing(std::uint32_t elapsed){d_.startup.update_game_dubbing(elapsed);}
    void synchronize_dubbing_attachments(){d_.startup.synchronize_game_dubbing();}
    void advance_mission_text(std::uint32_t elapsed){if(parent_.text())d_.subtitles.tick(elapsed);}
    void advance_car_particles(std::uint32_t elapsed){
        auto* callbacks=dynamic_cast<GameCarCallbacks*>(&d_.scene.car_callbacks());
        if(!callbacks)throw std::logic_error("FRAME requires its SAME Car particle pool");callbacks->update_particles(elapsed);
    }
    void advance_camera(std::uint32_t elapsed){camera_.simulate(elapsed);}
    void advance_paused_mission_particles(std::uint32_t elapsed){world_.modules().particles().update(elapsed);}
    void synchronize_selected_camera_direction(){
        if(auto* human=d_.players.control().selected_player(mode_flags())){
            game::HumanVector direction{};
            if(!d_.camera.query_direction(3,direction))throw std::logic_error("Camera query3 lost actual direction20");
            human->base().world_direction(direction); //Native Human+40, NOT B0 target direction.
        }
    }
    bool finish_offline_objective_delay(){
        return game::finish_loading_objective_delay(d_.session,*this);
    }
    bool application_saved_state_present(){return external_.application_saved_state_present();}
    bool current_player_available(){return d_.players.control().current_player()!=nullptr;}
    void objective_exit_camera(){ //0046CB20 command2(1,0) is a genuine offline7 early return.
        if((mode_flags()&7u)!=7u)throw std::runtime_error("Objective camera command requires actual nonoffline receiver");
    }
    void finish_nonoffline_session_exit(){throw std::runtime_error("Session exit requires actual0072E4E0/0072A0B0 match/network receivers");}
    std::uint32_t player_control_surface_flags(Human& human){ //005A9720/004DD090.
        if(human.field160){
            if(human.field160->base().type()!=4)return 0;
            auto* car=dynamic_cast<game::CarSceneActor<GameActorServices>*>(human.field160);
            if(!car||!car->loading)throw std::logic_error("Control surface flags require the actual initialized Car");
            const auto& wheels=car->loading->cold_state().wheels;
            return game::car_control_surface_flags(wheels,[](const auto& wheel){
                using Descriptor=world_loading::Scene::Descriptor;
                if(wheel.query_kind==game::CarCollisionIdentityKind::dynamic)return static_cast<const Descriptor*>(wheel.query_identity)->flags;
                if(wheel.query_kind==game::CarCollisionIdentityKind::shape)return static_cast<const game::CollisionShape<EngineFrame*>*>(wheel.query_identity)->flags;
                return static_cast<const game::CollisionTriangle<EngineFrame*>*>(wheel.query_identity)->flags;
            });
        }
        auto* control=d_.first_players.inline_control(human).control();
        if(!control)throw std::logic_error("Control surface flags require actual Human DD0");
        const auto& contact=control->physical().ground_contact();return contact?world_.owners().human_services().human_geometry_flags(*contact)&0x6000u:0;
    }
    void fail_player_control_objective(bool special){world_.owners().scripts().fail_objectives(special);}
    void player_control_hint(std::uint32_t id){d_.hud.set_hint(static_cast<std::int32_t>(id));}
    bool selected_player_available(){return d_.players.control().selected_player(mode_flags())!=nullptr;}
    void process_selected_player_input(std::uint32_t* a,std::uint32_t* b){
        auto* human=d_.players.control().selected_player(mode_flags());
        if(!human)throw std::logic_error("Session input lost its actual selected Human after HUD callbacks");
        external_.process_player_input(*human,a,b);
    }
    void toggle_session_first_person(){
        auto& view=d_.first_person.view();view.enable(mode_flags(),!(view.enabled()||view.secondary_enabled()));
        d_.startup.first_person_preference()=static_cast<std::uint8_t>(view.enabled()||view.secondary_enabled());
    }
    auto neutral_first_input()const{return parent_.neutral_first_input().data();}
    auto neutral_second_input()const{return parent_.neutral_second_input().data();}
    bool mission_save_blocked(){return external_.mission_save_blocked();}
    unsigned session_difficulty(){return d_.startup.human_difficulty();}
    void request_application_state(std::uint32_t status){external_.request_application_state(status);}
    void show_session_warning(std::uint32_t caption,bool format){external_.show_session_warning(caption,format);}
    std::uint32_t application_root()const{return selection_.field_e8;}
    std::uint32_t selected_mission_ordinal(std::uint32_t root)const{return selection_.selected_mission_ordinal(root);}
    bool hud_nonoffline_input_blocked(){return external_.hud_nonoffline_input_blocked();}
    bool hud_player_input_blocked(){return external_.hud_player_input_blocked();}
    void process_hud_controls(std::uint32_t* a,std::uint32_t* b){external_.process_hud_controls(a,b);}
    void process_nonoffline_session_input(std::uint32_t* a,std::uint32_t* b){external_.process_nonoffline_session_input(a,b);}
    void end_session_cutscene(){external_.end_session_cutscene();}
    Human* network_input_player(){return external_.network_input_player();}
    void process_network_player_input(Human& human,std::uint32_t* a,std::uint32_t* b){external_.process_player_input(human,a,b);}
private:
    auto store(){return d_.scene.frames().resource_observations();}
    void require_open(){
        if(retired_)throw std::logic_error("Session FRAME borrower is retired");
        if(!parent_.constructed()||!world_.owners().session_loaded())throw std::logic_error("Session FRAME requires its actual loaded containing World");
        (void)world_.owners().dependencies();
    }
    World& world_;Parent& parent_;world_loading::Dependencies& d_;const game::SessionSelection& selection_;
    GameBlastWaves& blast_;GameSessionFrameExternal& external_;GameMissionModuleFrames<Runtime> modules_;GameCameraLoading camera_;bool retired_=false;
};
} //namespace hd2::host
#endif
