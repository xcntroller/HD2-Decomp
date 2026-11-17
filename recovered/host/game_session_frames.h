#ifndef HD2_HOST_GAME_SESSION_FRAMES_H
#define HD2_HOST_GAME_SESSION_FRAMES_H

#include "host/game_mission_module_frames.h"
#include "host/game_blast_waves.h"
#include "game/session_frame_loading.h"
#include "game/session_match_rules.h"
#include "game/session_match_input.h"
#include "game/session_match_peers.h"
#include "game/session_received_match.h"
#include "game/session_round_reset.h"
#include "game/network_mission_metadata.h"
#include "game/human_damage_score.h"
#include "host/game_ui_presentation.h"

namespace hd2::host {
//Actual process/transport/application/interactive-camera owners, distinct from
//the containing FRAME policy. No default successes or manufactured data.
class GameSessionMatchExternal {
public:
    virtual ~GameSessionMatchExternal()=default;
    virtual world_loading::World& world()const=0;
    virtual void system_console_command(const std::string&)=0; //0069F250/00742480.
    virtual void defer_network_close()=0; //Legacy compatibility loan; FRAME now writes actual process5070 directly.
    virtual void current_application_status(std::uint32_t)=0; //Current application virtual2C, NOT controller04 request.
    virtual std::uint32_t current_game_phase()=0; //Fresh00698500 game88.
    virtual void reset_round_world(bool)=0; //Legacy external loan; FRAME006997B0 now invokes its SAME-World source directly.
    virtual void balance_peer_sides(std::uint32_t,std::uint32_t)=0; //Legacy compatibility loan; FRAME now balances actual process records directly.
    virtual void camera_command(std::uint32_t,std::uint32_t,std::uint32_t)=0; //Actual mission camera virtual44.
};
//Required UI/controller and conditional non7 endpoints use the SAME World.
//Source implementations on the owning branches still need actual application
//binding; no successful fallback, default flag or copied input is installed.
class GameSessionFrameExternal {
public:
    using Human=world_loading::Human;
    virtual ~GameSessionFrameExternal()=default;
    virtual world_loading::World& world()const=0;
    virtual bool is_current_application_state()=0; //Actual controller14 identity.
    virtual void request_application_state(std::uint32_t)=0; //Actual controller04, no immediate activation.
    virtual bool mission_save_blocked()=0; //0067FAA0, queried only with missionA8.
    virtual bool application_saved_state_present()=0; //Actual ApplicationStartupData mode04 (008AE924+4).
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
    virtual GameSessionMatchExternal* match_frame_sources(){return nullptr;} //Required only at reached external leaves.
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
template<class Runtime>class GameSessionFrames final:public GameSessionFrameCallbacks,public game::SessionRoundResetReceiver {
public:
    using World=world_loading::World;using Human=world_loading::Human;using Parent=World::Parent;
    GameSessionFrames(World& world,Parent& parent,const game::SessionSelection& selection,
            GameBlastWaves& blast,GameSessionFrameExternal& external,Runtime& runtime,GameUiPresentation* presentation=nullptr)
        :world_(world),parent_(parent),d_(world.owners().dependencies()),selection_(selection),
         blast_(blast),external_(external),modules_(world,runtime),camera_(world.owners()),presentation_(presentation){
        if(!parent_.constructed()||&external_.world()!=&world_||&blast_.dependencies()!=&d_||
           (presentation_&&presentation_->world_identity()!=&world_)||
           &d_.request!=&parent_.request()||selection_.accepted_request()!=&parent_.request()||
           d_.scene.frames().world_scene_frame()!=parent_.scene()->frame())
            throw std::logic_error("Session FRAME requires its SAME World/parent/selection/blast/external owners");
        world_.bind_frame_reset(*this); //Publish only AFTER all actual members and same-owner checks.
    }
    ~GameSessionFrames(){retire();}
    const void* round_reset_world_identity()const noexcept override{return &world_;}
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
    void reset_round(bool restart)override{
        require_open();RoundServices services{*this};
        io::observe_data_operation(store(),"0069C390 retained round reset",[&]{game::reset_retained_session_round(d_.session,restart,services);});
    }
    void receive_frame_control(std::uint32_t kind,std::uint32_t value)override{
        require_open();MatchServices services{*this};
        if(kind==6){(void)frontend::select_network_map(d_.startup.game_configuration(),nullptr);services.frame_current_application_status(9);return;} //0069B9D0.
        if(kind==13){game::receive_session_match_result(d_.session,value,services);return;}
        throw std::runtime_error("Required current-Frame presentation control receiver is unavailable");
    }
    void retire()noexcept override{if(retired_)return;retired_=true;world_.unbind_frame_reset(*this);} //Consumes the source borrower; does not finish/simulate the World.
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
    void finish_nonoffline_session_exit(){MatchServices q{*this};q.frame_defer_network_close();if(mode_flags()&1u)q.frame_queue_command(1,0,0,0);}
    void advance_nonoffline_fixed_frame(std::uint32_t elapsed){MatchServices q{*this};game::advance_nonoffline_fixed_frame(d_.session,q,elapsed);}
    void advance_nonoffline_variable_frame(std::uint32_t elapsed){MatchServices q{*this};game::advance_nonoffline_variable_frame(d_.session,q,elapsed);}
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
    void process_nonoffline_session_input(std::uint32_t*,std::uint32_t* b){MatchServices q{*this};game::process_nonoffline_frame_input(d_.session,q,b);}
    void end_session_cutscene(){external_.end_session_cutscene();}
    Human* network_input_player(){MatchServices q{*this};return q.frame_local_human();}
    void process_network_player_input(Human& human,std::uint32_t* a,std::uint32_t* b){external_.process_player_input(human,a,b);}
private:
    struct RoundServices {
        GameSessionFrames& f;
        auto& d(){return f.d_;}auto& startup(){return d().startup;}
        std::uint32_t mode_flags(){return f.mode_flags();}
        void enter_control_lock(std::size_t kind){startup().enter_control_lock(kind);}
        void leave_control_lock(std::size_t kind){startup().leave_control_lock(kind);}
        auto& round_peer_records(){return startup().game_control_records();}
        bool round_shared_lives(){return startup().game_configuration().rules228.at((0x27c-0x228)/4)!=0;}
        void round_query_focus(){GameLoadingQueries query(d().scene.frames().engine_module(),startup());(void)query.query_round_reset_focus();}
        void round_query_iconic(){GameLoadingQueries query(d().scene.frames().engine_module(),startup());(void)query.query_round_reset_iconic();}
        void round_reset_hud(){d().hud.reset_round(d().request.preparation.mission_resource,mode_flags(),false);}
        void round_finish_dubbing(){startup().finish_game_dubbing();}
        void round_initialize_dubbing(){startup().initialize_game_dubbing();}
        void round_reset_navigation(){d().navigation.reset_runtime_links();}
        auto* round_actor_identity(std::int32_t id){return d().scene.frames().scene_actor_command_identity(id);}
        auto round_actor_count(){return d().scene.frames().scene_actor_identity_count();}
        template<class Actor>void round_deactivate_actor(Actor& actor,bool authority){d().scene.frames().deactivate_scene_actor(actor,authority,d().world_actors,d().scene.buckets());}
        template<class Actor>void round_remove_actor(Actor* actor){d().scene.frames().remove_scene_actor(actor);}
        template<class Actor>void round_activate_spawn(Actor& actor,bool authority){
            if(actor.base().type()!=10)throw std::logic_error("Round spawn activation requires actual type10");
            actor.loading_activation_passes|=authority?1u:2u;
            static_cast<GameSceneActors::Spawn&>(actor).activate(authority,d().world_actors);
        }
        void round_reset_modules(bool restart){f.world_.modules().dispatch().reset_modules(restart);}
        auto round_squad_count(){return f.parent_.live_squad().size();}
        bool round_squad_active(std::size_t n){return f.parent_.live_squad().at(n).active;}
        auto* round_loading_spawn(){return d().scene.frames().loading_spawn_point(0);}
        template<class Spawn>void round_spawn_soldier(Spawn& spawn,std::size_t n){
            f.world_.owners().human_services().spawn_authority_human(spawn,0,&f.parent_.live_squad().at(n),nullptr);
        }
        void round_reset_first_person(){d().first_person.reset(mode_flags(),startup().pre_mission_files(),d().clock,startup().first_person_preference());}
        void round_reset_text_selection(){if(!f.parent_.text())throw std::logic_error("Round reset lost actual mission Text");f.parent_.text()->reset_selection();}
        void round_clear_transients(){f.blast_.clear_all_shared_records();}
        void round_reset_objectives(){game::reset_network_mission_objectives(*d().session.mission);}
        void round_begin_entry(){startup().begin_game_entry_connections();}
        void round_reset_entry_packets(){startup().reset_control_entry_packets(true);}
        auto* round_local_peer_human(){return game::local_session_peer_human(round_peer_records(),startup().local_player_identity());}
        bool round_is_current_application(){return f.external_.is_current_application_state();}
        void round_queue_entry(){startup().queue_application_command(0,0,0,0);}
        void round_reset_music_references(){d().music.countdown=d().music.references=0;}
        void round_select_music(){d().music.select(false,mode_flags(),startup());}
        void round_reset_speech_clocks(){d().audio.reset_category_clocks();}
        bool round_face_quality(){return startup().human_face_quality_enabled();}
        void round_initialize_faces(){d().faces.initialize();}
    };
    struct MatchServices {
        GameSessionFrames& f;
        auto& data(){return f.d_.startup.game_control_records();}auto& startup(){return f.d_.startup;}
        auto& world(){return f.world_.owners().human_services();}auto& state(){return f.d_.session;}
        GameSessionMatchExternal& required(const char* address){auto* p=f.external_.match_frame_sources();
            if(!p)throw std::runtime_error(std::string("Nonoffline FRAME requires actual external owner at ")+address);
            if(&p->world()!=&f.world_)throw std::logic_error("Nonoffline FRAME external producer belongs to a different World");return *p;}
        auto& presentation(){if(!f.presentation_)throw std::logic_error("Nonoffline FRAME requires its SAME lifecycle presentation");return *f.presentation_;}
        std::uint32_t mode_flags(){return f.mode_flags();}
        std::uint32_t frame_rule(unsigned offset){auto& c=startup().game_configuration();if(offset==0x224)return static_cast<std::uint32_t>(c.style224);
            if(offset<0x228||offset>0x27c||(offset&3))throw std::out_of_range("FRAME configuration word");return static_cast<std::uint32_t>(c.rules228.at((offset-0x228)/4));}
        bool frame_console_enabled(){return startup().game_configuration().flag170!=0;}
        auto take_frame_console_command(){return startup().take_game_console_command();}
        void process_frame_console_command(const std::string& text){required("0069F250/00742480").system_console_command(text);}
        bool frame_remote_ai_enabled(){return startup().game_configuration().remote_ai_enabled();}
        void frame_reset_scores(){game::reset_human_match_scores(data(),startup());}
        Human* frame_local_human(){auto* p=game::local_session_peer_human(data(),startup().local_player_identity());
            return p?&f.d_.world_actors.human_loading_actor(static_cast<GameSceneActors::Instance*>(p)):nullptr;}
        auto* frame_human_state(Human& human){return human.loading_state;}
        template<class State>bool frame_state_suppressed(State& value){return value.selection_suppressed();}
        template<class State>void frame_suppress_state(State& value,bool suppressed){value.selection_suppressed(static_cast<std::uint8_t>(suppressed));}
        template<class State>std::uint32_t frame_state_flags(State& value){return value.words().template get<std::uint32_t>(0x50);}
        std::int32_t frame_human_age(Human& human){return game::human_frame_signed_word(human.field204);}
        std::int32_t frame_human_health(Human& human){return game::human_frame_signed_word(human.properties.stats_ec[0]);}
        std::int32_t frame_human_identity(Human& human){return human.base().id10();}
        bool frame_human_controlled(Human& human){return game::script_human_player_controlled(human,mode_flags());}
        std::uint32_t frame_human_side(Human& human){return world().human_specialization(human,false);}
        std::uint32_t frame_local_human_side(){auto* p=frame_local_human();if(!p)throw std::logic_error("Spectator team query lost actual local Human");return frame_human_side(*p);}
        Human* frame_spectator_candidate(std::uint32_t index){auto* p=f.d_.scene.frames().scene_actor_command_identity(game::human_frame_signed_word(index));
            return p?&f.d_.world_actors.human_loading_actor(p):nullptr;}
        bool frame_peer_spectator_only(std::int32_t identity){const auto i=static_cast<std::uint32_t>(identity);if(i>=data().records.size())return false;
            auto& record=data().records[i];if(!record.byte(8).value())return false;
            auto* p=record.human_actor();return game::hud_info_peer_unavailable(p?&f.d_.world_actors.human_loading_actor(static_cast<GameSceneActors::Instance*>(p)):nullptr);}
        void frame_override_player(Human* human){f.d_.players.control().override_player(human);}
        void frame_enable_first_person(bool enabled){f.d_.first_person.view().enable(mode_flags(),enabled);}
        void frame_hud_spawn_mode(std::uint32_t value){world().spawn_hud_mode(value);}
        void frame_hud_peer_human(Human* human){f.d_.hud.refresh_peer_human(human,mode_flags(),startup().local_player_identity());}
        std::uint32_t frame_current_match_phase(){return required("00698500/88").current_game_phase();}
        void frame_camera_command(std::uint32_t command,std::uint32_t present,std::uint32_t value){required("0046CB20").camera_command(command,present,value);}
        void frame_reset_local_suppression(){if(auto* human=frame_local_human()){auto* state=human->loading_state;if(!state)throw std::logic_error("Match reset lost actual local B0");state->selection_suppressed(0);}}
        void frame_clear_flag_actor_states(){for(std::size_t n=0;n<f.d_.scene.frames().scene_actor_identity_count();++n)
            if(auto* actor=f.d_.scene.frames().scene_actor_command_identity(static_cast<std::int32_t>(n)))if(actor->base().type()==37u)static_cast<game::ZoneSceneActor<GameActorServices>*>(actor)->cursor100=0;}
        void frame_prepare_peer_lives(){game::prepare_session_match_peer_lives(data(),startup());}
        auto frame_live_team_counts(){return game::live_session_match_team_counts(data(),startup());}
        void frame_queue_command(std::uint32_t kind,std::uint32_t target,std::uint32_t first,std::uint32_t second){startup().queue_application_command(kind,target,first,second);}
        void frame_queue_pending_spawn(const std::shared_ptr<game::SessionPendingSpawn>& pending){startup().queue_application_spawn(pending);}
        void frame_queue_request(std::uint32_t kind,std::uint32_t first,std::uint32_t second){startup().queue_application_request(kind,first,second);}
        bool frame_first_prepared_player_valid(){const auto& squad=f.parent_.live_squad();if(squad.empty())throw std::logic_error("FRAME pending request requires actual first live soldier");return squad.front().network_identity!=-1;}
        void frame_defer_network_close(){startup().defer_control_close();}
        void frame_current_application_status(std::uint32_t status){required("current application virtual2C").current_application_status(status);}
        std::uint32_t frame_connection_status(){return startup().game_connection_status();}
        void frame_control_heartbeat(std::uint32_t elapsed){startup().advance_game_match_heartbeat(elapsed);}
        void frame_countdown_hint(std::uint32_t id,std::uint32_t time){f.world_.owners().hud_panels().small().set_countdown_hint(id,time);}
        void frame_set_hint(std::int32_t id){f.d_.hud.set_hint(id);}
        void frame_connection_warning(std::uint32_t id,std::uint32_t time){f.d_.hud.set_injury_message(id,time);}
        void frame_hide_map(){f.d_.hud.hide_map();}
        std::uint32_t frame_team_win(unsigned side){return data().team_wins458.at(side);}
        void frame_result_hint(std::uint32_t label,std::uint32_t first,std::uint32_t second){f.d_.hud.set_result_hint(label,first,second);}
        void frame_show_info(bool enabled){presentation().show_info(enabled);f.d_.hud.control_latches()[0]=static_cast<std::uint8_t>(enabled);}
        void frame_advance_sync(std::uint32_t elapsed){game::advance_session_match_sync(startup().game_session_frame_process(),*this,elapsed);}
        void frame_evaluate_rules(std::uint32_t elapsed){game::evaluate_session_match_rules(state(),startup().game_session_frame_process(),*this,elapsed);}
        void frame_advance_hazards(std::uint32_t elapsed){game::advance_session_match_hazards(state(),startup().game_session_frame_process(),*this,elapsed);}
        auto player_control_surface_flags(Human& human){return f.player_control_surface_flags(human);}
        void frame_authority_hazards(){
            alignas(16) std::array<unsigned char,32> payload; //Original0069EED0 unwritten stack vectors; SAME storage for the whole peer walk.
            game::with_session_match_peers(data(),startup(),[&](auto& peers){for(std::size_t n=0;n<peers.size();++n)
            if(peers[n].byte(8).value())if(auto* p=peers[n].human_actor()){
                auto& human=f.d_.world_actors.human_loading_actor(static_cast<GameSceneActors::Instance*>(p));
                if(player_control_surface_flags(human)&0x2000u){auto* fresh=peers.at(n).human_actor();if(!fresh)throw std::logic_error("Hazard lost fresh peer Human3C");
                    auto& target=f.d_.world_actors.human_loading_actor(static_cast<GameSceneActors::Instance*>(fresh));
                    if(!target.loading_lifetime)throw std::logic_error("FRAME hazard requires actual Human D4 owner");
                    const auto vectors=f.d_.scene.frames().frame_hazard_stack_vectors(payload);
                    target.loading_lifetime->damage(12,-1.f,vectors.first,vectors.second,&target,0);}
            }});}
        bool frame_any_peer_below_life_limit(std::uint32_t limit){return game::any_session_match_peer_below_limit(data(),startup(),limit);}
        std::int32_t frame_maximum_peer_score(){return game::maximum_session_match_score(data(),startup());}
        float frame_team_score(std::uint32_t side){return data().team_scores44c.at(side);}
        void frame_increment_team_win(std::uint32_t side){++data().team_wins458.at(side);}
        auto& frame_objectives(){if(!state().mission)throw std::logic_error("Match objectives require actual missionA8");return state().mission->related;}
        const auto& frame_objective_fields(std::size_t n){const auto& node=frame_objectives().at(n);if(!node||!node->network_objective)throw std::logic_error("Match requires actual kind12 objective metadata");return *node->network_objective;}
        auto frame_first_objective_side(){return frame_objective_fields(0).side18;}
        auto frame_objective_score(std::size_t n){return frame_objective_fields(n).score24;}
        std::uint32_t frame_opposite_side(std::uint32_t side){return static_cast<std::uint32_t>(game::human_frame_signed_word(side+1u)%2);}
        void frame_award_objective_points(std::uint32_t value,std::uint32_t side){world().award_human_objective_score(value,side);}
        void frame_finish_objectives(std::uint32_t side){for(std::size_t n=0;n<frame_objectives().size();++n)if(frame_objectives().at(n)->word10==0u)
            f.d_.script_link.set_human_objective_status(static_cast<std::int32_t>(n+1),frame_first_objective_side()!=side?2:1);}
        bool frame_peer_team_alive(std::uint32_t side){return game::with_session_match_peers(data(),startup(),[&](auto& peers){
            for(std::size_t n=0;n<peers.size();++n)if(peers[n].byte(8).value())if(auto* p=peers[n].human_actor())if(peers[n].word(0x64)==side){
                auto& human=f.d_.world_actors.human_loading_actor(static_cast<GameSceneActors::Instance*>(p));
                if(frame_human_health(human)>0&&human.loading_state&&!human.loading_state->selection_suppressed())return true;}
            return false;});}
        std::array<std::uint32_t,4> frame_flag_side_counts(){std::array<std::uint32_t,4> counts{};
            for(std::size_t n=0;n<f.d_.scene.frames().scene_actor_identity_count();++n)if(auto* actor=f.d_.scene.frames().scene_actor_command_identity(static_cast<std::int32_t>(n)))if(actor->base().type()==37u){
                ++counts[0];const auto side=static_cast<game::ZoneSceneActor<GameActorServices>*>(actor)->field98;
                if(side>2u)throw std::out_of_range("Flag actor side exceeds native three counters");++counts.at(side+1);}
            return counts;}
        void frame_update_all_ranks(){game::with_session_match_peers(data(),startup(),[&](auto& peers){for(std::size_t n=0;n<peers.size();++n)game::update_human_score_rank(data(),static_cast<std::uint32_t>(n),world());});}
        void frame_publish_match_result(std::uint32_t winner){if(!(mode_flags()&2u)){
            startup().append_game_console(game::format_human_game_message(startup().application_localize(winner?0x22e6:0x22e5)));
            const auto second=data().team_wins458[1];const auto right=startup().application_localize(0x22e8);
            const auto first=data().team_wins458[0];const auto left=startup().application_localize(0x22e7);
            startup().append_game_console(game::frame_match_result_text(left,first,right,second));}
            frame_queue_command(8,0,0,0);frame_queue_command(13,0,winner,0);}
        void frame_choose_objective_actor(){ //00699680: inclusive count+1 draw; floor is fed a DOUBLE, not a rounded FLOAT.
            if(frame_rule(0x224)!=2u)return;
            const auto side=frame_first_objective_side();const auto counts=game::count_session_match_teams(data(),startup(),true,true);
            const auto count=counts[side==1u?0u:1u]+1u;
            const double product=static_cast<double>(static_cast<long double>(game::human_frame_signed_word(startup().random_word()))*.000030518509447574615f*game::human_frame_signed_word(count));
            const auto value=std::floor(product);
            if(!std::isfinite(value)||value<std::numeric_limits<std::int32_t>::min()||value>std::numeric_limits<std::int32_t>::max())throw std::runtime_error("Objective actor draw exceeds native signed range");
            auto draw=static_cast<std::int32_t>(value);if(game::human_frame_signed_word(count)<draw)draw=game::human_frame_signed_word(count);
            game::with_session_match_peers(data(),startup(),[&](auto& peers){for(std::size_t n=0;n<peers.size();++n)
                if(peers[n].word(0)!=0&&peers[n].word(0x64)!=side&&peers[n].human_actor()){
                    draw=game::human_frame_signed_word(static_cast<std::uint32_t>(draw)-1u);
                    if(draw<1){state().words.set(0x78,static_cast<std::uint32_t>(n));break;}
                }});
            state().words.set(0x70,std::uint8_t{0});
        }
        void frame_disable_peer_bodies(bool disabled){ //00699AA0,418 remains held across collision rebuild/model visibility.
            game::with_session_match_peers(data(),startup(),[&](auto& peers){for(std::size_t n=0;n<peers.size();++n)if(peers[n].byte(8).value())if(auto* p=peers[n].human_actor()){
                auto& human=f.d_.world_actors.human_loading_actor(static_cast<GameSceneActors::Instance*>(p));auto& owner=world().human_inline(human);
                auto* control=owner.control();auto* look=owner.look();if(!control||!look)throw std::logic_error("Match body toggle requires actual Human A0/DC4/posture owner");
                control->frame_collision_disabled(disabled,*look);
            }});
        }
        void frame_begin_match_countdown(){ //00699890.
            game::publish_session_match_peer_sides(data(),startup());frame_disable_peer_bodies(true);
            if(frame_rule(0x248)){
                if(!state().mission||!state().mission->network_mission)throw std::logic_error("Match balance requires actual missionA8 kind13 fields58/5C");
                const auto& fields=state().mission->network_mission->words50;
                game::balance_session_match_peer_sides(data(),startup(),fields[3].value(),fields[2].value());
            }
            state().words.set(0x88,3u);state().words.set(0x8c,state().words.template get<std::uint32_t>(0x28));frame_queue_command(12,0,0,0);
        }
        void frame_synchronize_other_peers(std::uint32_t selector){ //00699290.
            game::with_session_match_peers(data(),startup(),[&](auto& peers){for(std::size_t n=0;n<peers.size();++n)
                if(n!=static_cast<std::uint32_t>(startup().local_player_identity())&&peers[n].byte(8).value())frame_queue_command(selector,peers[n].word(0),0,0);});
        }
        void frame_restart_round(){ //006997B0, real shared round reset/snapshot and process packet owners.
            f.reset_round(true);frame_synchronize_other_peers(7);startup().drain_application_commands();
            startup().flush_application_command_packets(1000);startup().begin_game_entry_connections();startup().reset_control_entry_packets(true);
        }
        void frame_rotate_map(const char* name){ //0069B9F0: nothing follows false006AD4D0; null means next map.
            if(!frontend::select_network_map(startup().game_configuration(),name))return;
            frame_synchronize_other_peers(6);startup().drain_application_commands();startup().flush_application_command_packets(1000);
            data().team_wins458.fill(0);frame_current_application_status(9);
        }
    };
    auto store(){return d_.scene.frames().resource_observations();}
    void require_open(){
        if(retired_)throw std::logic_error("Session FRAME borrower is retired");
        if(!parent_.constructed()||!world_.owners().session_loaded())throw std::logic_error("Session FRAME requires its actual loaded containing World");
        (void)world_.owners().dependencies();
    }
    World& world_;Parent& parent_;world_loading::Dependencies& d_;const game::SessionSelection& selection_;
    GameBlastWaves& blast_;GameSessionFrameExternal& external_;GameMissionModuleFrames<Runtime> modules_;GameCameraLoading camera_;GameUiPresentation* presentation_;bool retired_=false;
};
} //namespace hd2::host
#endif
