#ifndef HD2_HOST_GAME_MISSION_LOADING_H
#define HD2_HOST_GAME_MISSION_LOADING_H
#include "host/game_world_loading.h"
#include "host/game_loading_screen.h"
#include "host/game_loading_queries.h"
#include "game/mission_loading.h"
#include "game/loading_progress.h"
namespace hd2::host {

//Concrete006CA780/006D3F20 stage adapter for the SAME cold World, real loading
//screen, SessionOwner and process resources. Screen backend/runtime must be
//actual containing owners; no external-progress or GEffects tick defaults.
//This class does not activate the session, supply integrity observations, render
//a first frame or wire GameApplicationState. Retained-game status1 is separate.
class GameMissionLoading final:public GameSceneLoadingProgress {
public:
    using World=world_loading::World;using Parent=World::Parent;
    using ScratchOwner=std::unique_ptr<world_loading::Scene::Scratch>;
    GameMissionLoading(World& world,Parent& parent,StartupSession& startup,
                       game::SessionActivationState& session,GameLoadingScreen& screen,HMODULE module)
        :world_(world),parent_(parent),startup_(startup),session_(session),screen_(screen),queries_(module){
        auto& d=world_.owners().dependencies();
        if(&d.startup!=&startup||&d.session!=&session||&d.request!=&parent.request())
            throw std::logic_error("Mission loader requires SAME world/process/session/request owners");
    }
    bool load(const std::string& resource,const frontend::GameDataNode* mission){
        if(resource!=parent_.request().preparation.mission_resource||
           (mission&&mission!=parent_.request().mission.get()))
            throw std::logic_error("Mission loader must consume its actual owned request and descriptor");
        return game::load_mission(*this,resource,mission);
    }
    void reset_loading_fields(){
        session_.words.set(0x66,std::uint16_t{0});session_.words.set(0x68,std::uint16_t{0});
        world_.scene().documents().fields().field_d8=0;
    }
    ScratchOwner make_loading_scratch(){return std::make_unique<world_loading::Scene::Scratch>();}
    void prepare_loading(const std::string& resource,const frontend::GameDataNode* mission){
        const auto mode=startup_.application_flags();progress_.reset(true);
        (void)screen_.prepare(resource,(mode&2u)!=0,(mode&7u)==7u,true,mission); //006D3F75, native status ignored.
        unload_loaded_mission(); //006D3F81, BEFORE resource and Car scene publication/cache clear.
        session_.resource=std::make_shared<const std::string>(resource); //game6A, source COW allocation.
        startup_.car_loading_scene()=parent_.scene()->frame(); //006D3FF8, borrowed SAME scene.
        resource_=session_.resource; //Containing loading controller30 shares the actual resource.
        startup_.loaded_resources().prepare_mission(); //Actual cache1/4/5 order, not controller shutdown.
    }
    void unload_loaded_mission(){ //0069A3D0, supported non-retained file-unload path.
        if(session_.status==1)throw std::runtime_error("Retained session unload requires actual game virtual08 finish receiver");
        if(!session_.loaded)return;
        auto& d=world_.owners().dependencies();
        if(d.world_actors.has_human_loading_owners())
            throw std::logic_error("Finish actual Human/module lifetimes before file unload");
        auto& fields=world_.scene().documents().fields();fields.field_cc=fields.field_c8=0xffffffffu;
        world_.scene().frames().clear_scene_frame_data(); //006E9B50 before ascending+18 passes.
        auto& modules=world_.modules().dispatch();
        if(startup_.application_flags()&1u)modules.unload_modules(true);
        if(startup_.application_flags()&2u)modules.unload_modules(false);
        world_.scene().frames().clear_scene_object_references(); //Native game48 model references, reverse release.
        world_.release_collision_tree(); //Additional source tree/grid owners retire before engine scene reset.
        parent_.tables()->clear();
        parent_.scene()->reset_mission_scene();
        session_.resource.reset();session_.loaded=false; //Only after required native stage retirement.
    }
    bool load_animation_tables(){return startup_.game_animations().ensure_loaded();}
    void open_scene(const std::string& resource){(void)parent_.scene()->open_mission_scene(resource,*this);} //006D4180 ignores engine result.
    void refresh_scene(){parent_.scene()->refresh();}
    auto load_scene2(const std::string& resource,world_loading::Scene::Scratch& scratch){return world_.scene().documents().load_scene2(resource,scratch);}
    void load_actors(const std::string& resource,world_loading::Scene::Scratch& scratch){world_.scene().documents().load_actors(resource,scratch);}
    void load_sounds(const std::string& resource,world_loading::Scene::Scratch& scratch){world_.scene().documents().load_sounds(resource,scratch);}
    void load_check_tables(const std::string& resource){world_.scene().documents().load_check_tables(resource);}
    void load_collision_tree(const std::string& resource,bool dynamic){
        if(!dynamic)throw std::logic_error("Mission loader requires its actual dynamic grid");world_.load_collision_tree(resource);
    }
    void publish_effects_scene(){world_.modules().mission_effects().publish_scene();}
    void load_effects(const std::string& resource){world_.modules().mission_effects().load(resource);}
    void mark_mission_loaded(){
        if(!world_.scene().collision_tree_if_loaded()||!world_.scene().buckets().bound())
            throw std::logic_error("Mission stages have not produced their actual collision owners");
        session_.loaded=true;
    }
    void finish_loading_screen(){screen_.finish_loading();}
    void loading_progress(unsigned stage,float fraction){
        progress_.update(stage,fraction,screen_.active(),[this](float value){screen_.value(value);screen_.draw();});
    }
    void scene_loading_progress(float fraction)override{loading_progress(0,fraction);} //006D4C70 engine callback; event10/20 filtered by GameScene.
    HWND query_parent_window(){return queries_.query_parent_window();}
    BOOL query_visible_window(){return queries_.query_visible_window();}
    BOOL query_iconic_window(){return queries_.query_iconic_window();}
    void finish_activation_screen(){screen_.finish_activation();}
    void finish_loading_modules(){world_.modules().dispatch().finish_loading_modules();}
private:
    World& world_;Parent& parent_;StartupSession& startup_;game::SessionActivationState& session_;
    GameLoadingScreen& screen_;GameLoadingQueries queries_;game::LoadingProgress progress_;
    std::shared_ptr<const std::string> resource_;
};
//Actual config24+170 owner; positive external transport remains a required
//later endpoint. Its ordinary reset flag is supplied by GameConfiguration,
//never by a successful hardcoded loading-progress receiver.
class GameLoadingPresentationRuntime final:public LoadingPresentationRuntime {
public:
    GameLoadingPresentationRuntime(StartupSession& startup):startup_(startup){}
    void bind(world_loading::World& world){world_.bind(world);}
    void unbind(world_loading::World& world)noexcept{world_.unbind(world);}
    bool external_loading_progress()const override{return startup_.game_configuration().flag170!=0;}
    void send_loading_progress(std::int32_t)override{throw std::runtime_error("External loading progress requires actual00731720 control/transport receiver");}
    void advance_loading_effects(std::uint32_t elapsed)override{
        auto& world=world_.get();auto& d=world.owners().dependencies();
        if(&d.startup!=&startup_)throw std::logic_error("Loading effects changed their process owner");
        world.modules().game_effects().update_loading(elapsed,d.session,d.players.control());
    }
private:
    StartupSession& startup_;game::WorldLoadingDependencyLink<world_loading::World> world_;
};

//Full real source loading allocation, including checked progress/runtime cycles.
//Constructing callbacks captures only already-constructed links. No callback
//can reach an unconstructed World/loader, and neither link has a successful default.
//The application remains unconnected until activation/guard/first-frame owners
//and desktop acceptance are ready; native module finish still belongs to caller.
class GameMissionLoadingSession {
public:
    using Parent=GameMissionLoading::Parent;
    GameMissionLoadingSession(Parent& parent,GameOwnerBackend& backend,StartupSession& startup,
            MenuDriver& driver,HMODULE module,Graphics& graphics,FrameClockPlatform& clock,
            const frontend::PreMissionItemCatalogue& catalogue,const std::string& suffix,game::SessionActivationState& state)
        :runtime_(startup),screen_backend_(driver,module,graphics,startup.pre_mission_files(),startup.text(),runtime_),screen_(screen_backend_),
         world_(parent,backend,startup,driver,module,graphics,clock,catalogue,suffix,
             [this](unsigned stage,float value){progress_.get().loading_progress(stage,value);},state),
         loader_(world_,parent,startup,state,screen_,module){runtime_.bind(world_);progress_.bind(loader_);}
    ~GameMissionLoadingSession(){progress_.unbind(loader_);runtime_.unbind(world_);}
    GameMissionLoadingSession(const GameMissionLoadingSession&)=delete;
    GameMissionLoadingSession& operator=(const GameMissionLoadingSession&)=delete;
    auto& loader(){return loader_;}auto& world(){return world_;}
private:
    game::WorldLoadingDependencyLink<GameMissionLoading> progress_;GameLoadingPresentationRuntime runtime_;
    GameLoadingScreenBackend screen_backend_;GameLoadingScreen screen_;world_loading::World world_;GameMissionLoading loader_;
};

} //namespace hd2::host
#endif
