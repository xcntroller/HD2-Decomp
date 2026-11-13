#ifndef HD2_HOST_GAME_UI_PRESENTATION_H
#define HD2_HOST_GAME_UI_PRESENTATION_H

#include "host/game_world_loading.h"
#include "host/game_effects_fade_render.h"
#include "host/borrowed_input.h"
#include "game/hud_crosshair_visibility.h"
#include "game/hud_signal_actions.h"
#include "game/hud_vehicle_presentation.h"
#include "game/hud_inventory_presentation.h"
#include "game/hud_command_presentation.h"
#include "game/hud_controls.h"
#include "game/console_command_dispatch.h"
#include "game/hud_car_occupants.h"
#include "game/hud_process_data.h"
#include "game/actor_occupants_presentation.h"
#include "input/binding_consumption.h"

namespace hd2::host {
//Required Human producer boundary005A5ED0/005A5F00/005A5F50. Agent #1's
//actual B0 order owner supplies these read queries; UI never creates tasks or
//treats a missing populated producer as a successful empty list.
class GameUiHumanOrderSource {
public:
    virtual ~GameUiHumanOrderSource()=default;
    virtual bool changed(world_loading::Human& human)=0;
    virtual std::int32_t count(world_loading::Human& human)=0;
    virtual game::HudOrderDisplay order(world_loading::Human& human,std::uint32_t index)=0;
};
class GameUiHumanCommandSource : public GameUiHumanOrderSource {
public:
    //Required mutable005A5F50 order44. The caller borrows the actual Human
    //order word for this call, never a copied policy or another command owner.
    virtual std::uint32_t& flags(world_loading::Human& human,std::uint32_t index)=0;
};
//Actual Human order74 positions/84 model are owned by its order list. This
//view is consumed before the call returns; UI retains none of its borrowers.
struct GameUiCommandRouteView {
    const std::vector<std::array<float,4>>& positions;
    world_loading::Frames::Frame model;
    GameSceneActors::Instance* const& target;
    const std::array<float,4>& position;
};
class GameUiHumanRouteSource:public GameUiHumanCommandSource {
public:
    //0058CCE0 mutates the actual order's path/model using mission navigation;
    //it is not a UI-generated straight line or a copied order/model owner.
    virtual GameUiCommandRouteView prepare_route(world_loading::Human& human,std::uint32_t index,const std::array<float,4>& from)=0;
};
class GameUiCommandFrameSource:public GameUiHumanRouteSource {
public:
    //Required005AC470 (attachment00793490, weapon00786160); argument is
    //the roster/temporary-selection INDEX, as confirmed at004B653E/004B6678.
    virtual void present_command_human(world_loading::Human& human,std::uint32_t index)=0;
    //00463BA0 consumes the SAME mission-camera virtual88 controller. General
    //camera simulation belongs to FRAME, not the UI presentation receiver.
    virtual void advance_command_camera(game::DetachedCamera<world_loading::Frames,world_loading::Human>& camera,std::uint32_t elapsed)=0;
};
class GameUiCommandControls {
public:
    virtual ~GameUiCommandControls()=default;
    //0069B8B0 must apply the actual game28/mode gate, then time, gameplay
    //and SOUND00428F00 notifications. FRAME/OWN supplies the containing policy.
    virtual void request_realtime(world_loading::World& world,bool enabled)=0;
};
struct GameUiCommandOrderLoan {
    world_loading::Human* human=nullptr;std::uint32_t index=0;const void* identity=nullptr;std::uint32_t kind=0;
    GameSceneActors::Instance* target=nullptr;std::array<float,4> position{},direction{};world_loading::Frames::Frame model{};
};
//Actual Human order/B0 queues, Players0045DD40 and detached-camera input
//receivers. The UI implements selection/button/drag policy; these producers
//must use the given SAME World and owners and consume all loans before return.
class GameUiCommandInputSource:public GameUiHumanCommandSource,public GameUiCommandControls {
public:
    virtual GameUiCommandOrderLoan read_order(world_loading::Human&,std::uint32_t)=0;
    virtual GameUiCommandOrderLoan pick_order_model(world_loading::World&,world_loading::Frames::Frame)=0;
    virtual void mark_order_selected(world_loading::Human&,std::uint32_t,bool)=0; //Native order4A.
    virtual void remove_order(world_loading::Human&,std::int32_t)=0;virtual void clear_orders(world_loading::Human&)=0;
    virtual void create_order(world_loading::Human&,std::uint32_t kind,std::int32_t parameter,const std::array<float,4>& position,const std::array<float,4>& direction,
        const std::array<std::uint32_t,3>& policies,GameSceneActors::Instance* target,std::uint32_t row,std::int32_t insert_before)=0; //005825C0→005A5F20/30.
    virtual void change_order(world_loading::Human&,std::int32_t,const std::array<float,4>& position,const std::array<float,4>& direction,const std::array<std::uint32_t,3>& policies)=0; //0058C860→005A5F40.
    virtual void select_command_human(world_loading::World&,world_loading::Human&)=0; //Actual Players0045DD40, not a scalar index substitute.
    virtual void camera_mode(world_loading::World&,game::DetachedCamera<world_loading::Frames,world_loading::Human>&,std::uint32_t,const std::array<float,4>* position)=0; //004632C0.
    virtual void camera_axes(game::DetachedCamera<world_loading::Frames,world_loading::Human>&,float,float)=0; //004633A0.
    virtual std::int32_t actor20_occupant_count(GameSceneActors::Instance&)=0;virtual world_loading::Human* actor20_occupant(GameSceneActors::Instance&,std::int32_t)=0;
};
class GameUiCommandModalSource:public GameUiCommandInputSource {
public:
    virtual void enable_command_camera(world_loading::World&,game::DetachedCamera<world_loading::Frames,world_loading::Human>&,bool)=0; //00462AE0; existing camera virtual88 borrower.
    virtual void prepare_command_sight(world_loading::World&)=0; //0045F080→actual Human Sight0046F280, not selection reset.
    virtual void retire_command_special_model(world_loading::World&)=0; //Actual GEffects0089B770 virtual44=004870A0(false,0,true); restores/releases its2E0 model. RENDER owns that producer.
};
//Required external network/typed Human producers. Ordinary peer/name/stat/team
//reads and sorting use SAME ControlStartupState under its actual lock0. All
//keys/strings/Humans are in-call loans; the HUD retains no peer borrowers.
class GameUiInfoSource {
public:
    virtual ~GameUiInfoSource()=default;
    virtual world_loading::Human* peer_human(const frontend::ControlStartupRecord& peer)=0; //Actual typed3C; NEVER cast a serialized word or substitute an actor ID.
    virtual std::uint32_t objective_team_caption(const frontend::GameDataNode& objective,std::int32_t team)=0;
    virtual std::int32_t objective_team(const frontend::GameDataNode& objective)=0;
    virtual std::shared_ptr<const std::string> mission_title(world_loading::World& world)=0; //Actual gameA8/6C descriptor publication.
    virtual std::string network_address()=0; //Actual process00981E70, no discovery/menu substitute.
    virtual std::int32_t team_count(std::uint32_t team)=0; //97D4C8/CC (team0/team1).
};
class GameUiMapSource {
public:
    virtual ~GameUiMapSource()=default;
    virtual std::int32_t zone_state(game::ActorBase<world_loading::Frames>& actor)=0; //Actual kind37 actor98; no actor/simulation substitute.
    virtual std::uint32_t objective_caption(const frontend::GameDataNode& objective,std::uint32_t team)=0; //Native1C/20.
    virtual std::int32_t objective_team(const frontend::GameDataNode& objective)=0; //Native18.
    virtual std::string human_speech(world_loading::Human& human)=0; //Actual Human214 text, distinct from cold first-byte flag.
};
//Required original console/chat dispatch00720130/00720340. Input editing
//does not manufacture command parsing, transport or gameplay side effects.
class GameUiMessageCommands {
public:
    virtual ~GameUiMessageCommands()=default;
    virtual void console(char* input)=0;
    virtual void say(char* input,bool team)=0;
};
//Effects outside the UI parser. These receivers must use the supplied SAME
//World/process owners and consume each loan before return. There are no
//successful defaults for absent gameplay, application FIFO or packet services.
class GameUiConsoleActions {
public:
    virtual ~GameUiConsoleActions()=default;
    virtual void system_command(world_loading::World&,const std::string& command)=0; //Actual process98C0FC/007422D0/00742480 application/network console.
    virtual void chat(world_loading::World&,std::string_view text,std::int32_t sender,bool team)=0; //0072EA90: one typed payload, application kind3 THEN control scalar kind3; actual application supplies unwritten target108 before serialization.
    virtual void network_kill(world_loading::World&,std::int32_t identity)=0; //Application kind4 THEN control scalar4, zero remaining arguments.
    virtual void kill_human(world_loading::World&,world_loading::Human&)=0; //Required original D4(6,-1,zero,zero,NULL,0) only on older Human checkpoints.
    virtual void set_health_percent(world_loading::World&,world_loading::Human&,std::int32_t)=0; //Actual005A6060, not direct HUD/stat mutation.
    virtual void give_item(world_loading::World&,world_loading::Human&,std::int32_t definition)=0; //Actual live007D7930(definition,0,Human,-1); cold preparation is not this producer.
};
//Kind37 itself belongs to the actual Actor owner. Only its positive F1 branch
//requires this source, consumed before returning; no synthetic zone/state owner.
class GameUiZoneSource {
public:
    virtual ~GameUiZoneSource()=default;
    virtual bool progress_active(GameSceneActors::Instance& zone)=0; //Actual F1.
    virtual game::HudZoneProgressData progress_data(GameSceneActors::Instance& zone)=0; //Actual98/A0/A8/EC in-call snapshot, no UI-owned state.
};
//Positive producers belonging to actual Human/occupied-Car/item-transfer
//owners. Each loan is consumed in-call; no default successful response exists.
class GameUiRollExternalSource {
public:
    virtual ~GameUiRollExternalSource()=default;
    virtual GameSceneActors::Instance* human_focus(world_loading::Human& human)=0; //Actual typed Human174.
    virtual bool car_interaction_available(game::CarSceneActor<world_loading::Frames>& car)=0; //004DDD10,254A/004D5430.
    virtual std::int32_t loose_item_pickup_capacity(world_loading::Human& human,GameSceneActors::Instance& item)=0; //007DAF80 dry-run.
    //Read-only/eligibility receivers of Actor families absent from the current
    //factory. Their owners supply these values; UI supplies all row policy.
    virtual bool switch_available(GameSceneActors::Instance&,world_loading::Human&)=0; //Kind7,0060B460.
    virtual std::int32_t switch_definition(GameSceneActors::Instance&)=0; //Kind7 native98.
    virtual std::int32_t switch_state(GameSceneActors::Instance&)=0; //Kind7 nativeA0.
    virtual std::int32_t special_caption(GameSceneActors::Instance&)=0; //Kind23 native9C.
    virtual std::int32_t special_definition(GameSceneActors::Instance&)=0; //Kind23 native98.
};
class GameUiSignalExternalSource : public GameUiRollExternalSource {
public:
    virtual std::int32_t definition_pickup_capacity(world_loading::Human& human,std::int32_t definition)=0; //007DB170 dry-run.
};
class GameUiSignalCommands {
public:
    virtual ~GameUiSignalCommands()=default;
    //Required actual Player0053D910 kind12/embedded kind3 queue and downstream
    //serializer, including queued actor retirement. The receiver copies the
    //request before return; UI neither retains it nor simulates Human orders.
    virtual void submit(world_loading::Human& selected,const game::HudSignalCommand<GameSceneActors::Instance>& request)=0;
};
class GameUiRollCommands:public GameUiSignalCommands{
public:
    virtual void submit_action(world_loading::Human& human,const game::HudRollCommand& request)=0; //Human005A45B0, actual B0 virtualD8.
    virtual void pickup_supply(world_loading::Human& human,game::SupplyBoxSceneActor<world_loading::Frames>& supply,std::int32_t row)=0; //005A2CF0, B0 virtual7C and Human178 publication.
    virtual void pickup_item(world_loading::Human& human,GameSceneActors::Instance& item)=0; //005A2D20, B0 virtual6C(id,7) and Human178 publication.
    virtual void request_inventory(world_loading::Human& human,std::int32_t target_identity)=0; //005A45A0; nonoffline kind3 request producer, not an immediate transfer.
};
//Actual Human005A2B40/005A2BB0/005A2C50 command receivers. UI chooses the
//original source/destination/holder/index and consumes each loan in-call. This
//does not replace Human B0 queues, cross-actor transfers or world item spawning.
class GameUiInventoryCommands {
public:
    using Human=world_loading::Human;using Actor=GameSceneActors::Instance;using Entry=world_loading::Actors::Items::Entry;
    virtual ~GameUiInventoryCommands()=default;
    virtual void move(Human& destination,Entry item,Actor& source,std::int32_t source_holder,std::int32_t destination_holder,std::int32_t index)=0;
    virtual void swap(Human& destination,Entry displaced,std::int32_t destination_holder,Actor& source,Entry item,std::int32_t source_holder)=0;
    virtual void drop(Human& source,Entry item,std::int32_t holder,const std::array<float,4>& direction)=0;
};
class GameUiSelectorCommands:public GameUiInventoryCommands {
public:
    virtual bool selector_busy(Human&)=0; //Actual Human state virtual48, before005A45C0.
    virtual void use_selector_item(Human&,Entry)=0; //005A45B0 command2/23 with actual runtime item identity10.
};
class GameUiPlayerControls {
public:
    virtual ~GameUiPlayerControls()=default;
    virtual void resume_player_control(world_loading::World&)=0; //0045DA40.
    virtual void select_player_index(world_loading::World&,std::int32_t)=0; //0045DC20.
    virtual void select_command_player_index(world_loading::World&,std::int32_t)=0; //0045DCC0.
    virtual std::int32_t next_player_index(world_loading::World&,bool reverse)=0; //0045D660/0045D9D0.
    virtual void focus_command_human(world_loading::World&,game::DetachedCamera<world_loading::Frames,world_loading::Human>&,world_loading::Human*)=0; //00462780 on SAME detached camera, nullable original Human loan.
};
//In-call loans only. A required positive branch rejects an absent producer;
//closed gates do not require unrelated network/command/console services.
struct GameUiHudFrameSources {
    GameUiHumanOrderSource* orders=nullptr;GameUiCommandFrameSource* command=nullptr;
    GameUiSignalExternalSource* signals=nullptr;GameUiMessageCommands* messages=nullptr;
    GameUiInfoSource* info=nullptr;GameUiMapSource* map=nullptr;
    GameUiZoneSource* zone=nullptr;
    GameUiConsoleActions* console=nullptr;
};
struct GameUiHudControlSources {
    GameUiCommandModalSource* command=nullptr;GameUiPlayerControls* players=nullptr;GameUiSelectorCommands* selector=nullptr;
    GameUiCommandControls* realtime=nullptr;GameUiSignalCommands* signals=nullptr;GameUiRollCommands* roll=nullptr;GameUiInventoryCommands* inventory=nullptr;
    GameUiHumanOrderSource* orders=nullptr;GameUiSignalExternalSource* external=nullptr;GameUiInfoSource* info=nullptr;GameUiMapSource* map=nullptr;
};
//Source presentation endpoints and ordinary HUD dispatcher over ONE containing
//World. Required Human producers and containing frame coordinator stay separate.
//The caller must retire this borrower before Owners::close or World retirement.
class GameUiPresentation {
public:
    using RollDialogs=std::remove_reference_t<decltype(std::declval<world_loading::World&>().owners().hud_panels().dialogs())>;
    GameUiPresentation(world_loading::World& world,world_loading::World::Parent& parent,
                       GameOwnerBackend& backend,Graphics& graphics)
        :world_(world),d_(world.owners().dependencies()),parent_(parent),backend_(backend),graphics_(graphics),fade_(world.owners(),graphics) {
        if(&backend_!=&parent_.backend()||&backend_.presentation_graphics()!=&graphics_||
           &backend_.presentation_startup()!=&d_.startup||!parent_.constructed()||
           d_.scene.frames().world_scene_frame()!=parent_.scene()->frame()||
           &d_.request!=&parent_.request())throw std::logic_error("UI requires its SAME World/session owners");
        auto& panels=world_.owners().hud_panels();panels.dialogs().check_retirement_binding();panels.command_roll().check_retirement_binding();panels.inventory().check_refresh_binding();d_.world_actors.items().check_ui_retirement_binding();
        std::function<void(const void*,bool)> ordinary=[this](const void* actor,bool reset){retire_dialog_actor(false,actor,reset);};
        std::function<void(const void*,bool)> command=[this](const void* actor,bool reset){retire_dialog_actor(true,actor,reset);};
        std::function<void()> inventory=[this]{refresh_inventory();};
        panels.dialogs().bind_retirement(this,std::move(ordinary));panels.command_roll().bind_retirement(this,std::move(command));panels.inventory().bind_refresh(this,std::move(inventory));
        d_.world_actors.items().bind_ui_retirement(this,[](void* context,world_loading::Actors::Items::Entry item)noexcept{auto& ui=*static_cast<GameUiPresentation*>(context);auto& panels=ui.world_.owners().hud_panels();panels.inventory().retire_item(item);panels.selector().retire_item(item);});
    }
    ~GameUiPresentation(){auto& panels=world_.owners().hud_panels();d_.world_actors.items().unbind_ui_retirement(this);panels.inventory().unbind_refresh(this);panels.dialogs().unbind_retirement(this);panels.command_roll().unbind_retirement(this);unbind_signal_sources();}
    GameUiPresentation(const GameUiPresentation&)=delete;
    GameUiPresentation& operator=(const GameUiPresentation&)=delete;
    void before_hud(){require_open();d_.hud.render_prepass(d_.camera);} //0049B9B0.
    void render_hud(){require_open();d_.hud.render(d_.camera,d_.startup.application_flags(),*this);} //0049B9F0.
    void advance_mission_text(std::uint32_t elapsed){require_open();d_.subtitles.tick(elapsed);} //0048F2D0.
    void advance_hud_messages(std::uint32_t elapsed,GameUiConsoleActions* actions=nullptr){
        require_open();auto input=backend_.presentation_input();MessageCommands commands{*this,actions};
        world_.owners().hud_panels().messages().update(elapsed,d_.startup.wide_characters(),input,commands);
    }
    std::uint32_t dispatch_console(char* input,std::size_t capacity,GameUiConsoleActions* actions=nullptr){
        require_open();ConsoleQueries queries{*this,actions};game::ConsoleCommandDispatch worker(d_.startup.game_console_variables(),queries);return worker.console(input,capacity);
    }
    std::uint32_t dispatch_console_command(std::size_t index,char* input,std::size_t capacity,GameUiConsoleActions* actions=nullptr){
        require_open();ConsoleQueries queries{*this,actions};game::ConsoleCommandDispatch worker(d_.startup.game_console_variables(),queries);return worker.command(index,input,capacity);
    }
    void advance_hud(std::uint32_t elapsed,const GameUiHudFrameSources& sources={}){require_open();HudFrameQueries queries{*this,sources};d_.hud.update_loading(elapsed,queries);} //0049BD50, no World/camera simulation.
    void process_hud_controls(const GameUiHudControlSources& sources,std::uint32_t* first,std::uint32_t* second,std::size_t count){
        require_open();if(!parent_.hud()->flags)return;if(!first||!second||count<64)throw std::invalid_argument("HUD controls require original containing64-word input pair");
        DialogInput input{d_.startup.presentation_input_bindings(),backend_.presentation_input(),first,second,count};HudControlQueries queries{*this,sources,first,second,count};game::HudControls worker(input,queries);worker.run();
    }
    void advance_hud_messages(std::uint32_t elapsed,GameUiMessageCommands& commands){
        require_open();auto input=backend_.presentation_input();
        world_.owners().hud_panels().messages().update(elapsed,d_.startup.wide_characters(),input,commands);
    }
    void hud_message_input_mode(std::uint8_t enabled){
        require_open();auto input=backend_.presentation_input();
        world_.owners().hud_panels().messages().input_mode(enabled,input);
    }
    void hud_message_mode(std::uint32_t mode,GameUiCommandControls& controls){require_open();auto input=backend_.presentation_input();CommandControlQueries queries{*this,controls};world_.owners().hud_panels().messages().mode(mode,(d_.startup.application_flags()&7u)==7u,input,queries);}
    void hud_message_say(bool team){require_open();auto input=backend_.presentation_input();world_.owners().hud_panels().messages().say(team,input);}
    void render_mission_text(){require_open();d_.subtitles.draw(d_.startup.profile_settings().bytes[0x3a]!=0,backend_);} //0048F390.
    void render_effects_overlay(){require_open();d_.startup.frame_text_overlay().render();} //006E8490.
    bool has_game()const{return true;}
    std::uint32_t game_status()const{return d_.session.status;}
    std::uint32_t game_word24()const{return d_.session.words.get<std::uint32_t>(0x24);}
    bool suppress_hud()const{return d_.startup.scene_presentation_flags().suppress_hud;}
    bool full_viewport()const{return d_.startup.scene_presentation_flags().full_viewport;}
    bool debug_enabled()const{return d_.startup.scene_presentation_flags().debug;}
    bool hud_enabled(){require_open();return parent_.hud()->enabled();}
    CursorViewport graphics_viewport(){
        const auto* viewport=graphics_.methods->viewport(&graphics_);
        if(!viewport)throw std::runtime_error("UI callback requires actual graphics viewport");
        return *viewport;
    }
    void graphics_viewport(const CursorViewport& value){
        if(graphics_.methods->set_viewport(&graphics_,&value)<0)throw std::runtime_error("Cannot set UI callback viewport");
    }
    std::uint32_t graphics_width(){return graphics_.methods->width(&graphics_);}
    std::uint32_t graphics_height(){return graphics_.methods->height(&graphics_);}
    void render_game_fade(){require_open();game::render_session_fade(d_.session,fade_);} //0069AE10.
    void control_after_scene(){require_open();world_.modules().game_effects().render_after_scene(fade_);} //00486C10.
    void advance_hud_pointer(std::uint32_t elapsed){require_open();world_.owners().hud_panels().auxiliary().update_pointer(elapsed);}
    void process_hud_pointer(std::uint32_t* first=nullptr,std::uint32_t* second=nullptr,std::size_t count=0){require_open();auto& pointer=world_.owners().hud_panels().auxiliary();if(!pointer.pointer_accepts_input())return;
        input::BindingInputViews input(d_.startup.presentation_input_bindings(),first,second,count);pointer.process_pointer(input);
    }
    void show_injury_movie(bool enabled){require_open();world_.owners().hud_panels().injury_movies().show(enabled,d_.audio.random_source());}
    void show_camera_snapshot(bool enabled){require_open();world_.owners().hud_panels().small().show_camera(enabled);}
    void advance_camera_snapshot(std::uint32_t elapsed){
        require_open();auto& panel=world_.owners().hud_panels().small();const auto root=panel.camera();
        if(root&&d_.scene.frames().hud_frame_visible(root)){SnapshotPresentation presentation{*this};panel.update_camera(elapsed,presentation);}
    }
    void advance_injury_movie(std::uint32_t elapsed){require_open();world_.owners().hud_panels().injury_movies().update(elapsed);}
    void advance_hud_soldiers(){
        require_open();auto& panels=world_.owners().hud_panels();SoldierQueries queries{world_,d_};
        for(unsigned n=6;n;){auto& panel=panels.soldiers()[--n];
            if(!panel.artwork().root)throw std::logic_error("Soldier frame requires actual loaded artwork");
            if(!d_.scene.frames().hud_frame_visible(panel.artwork().root))continue;
            auto* human=find_human_borrower(panel.human_borrower());
            panel.update(*human,d_.world_actors.items(),queries,d_.startup.application_flags(),d_.startup.local_player_identity());
        }
    }
    void advance_hud_compass(std::uint32_t elapsed){require_open();CompassQueries queries{*this};world_.owners().hud_panels().compass().update(elapsed,queries);}
    void advance_hud_progress(GameUiZoneSource* source=nullptr){
        require_open();ProgressQueries queries{*this,source};world_.owners().hud_panels().auxiliary().update_progress([&]()->std::optional<game::HumanHudProgress>{
            auto* human=d_.first_players.current_player();if(!human)return std::nullopt;
            return game::human_hud_progress(*human,d_.startup.application_flags(),queries);
        });
    }
    void advance_hud_orders(GameUiHumanOrderSource& source){
        require_open();auto* human=d_.first_players.current_player();const auto mode=d_.startup.application_flags();
        //0049BD50 uses0045D1B0 PREVIOUS30 in offline command mode, not
        //0045D160 selected28, then falls back to the actual current Human.
        if((mode&7u)==7u&&world_.owners().hud_panels().command().visible())if(auto* previous=d_.players.control().previous_player())human=previous;
        if(!human)throw std::logic_error("HUD orders require actual current/previous Human");
        (void)d_.first_players.state(*human);
        OrderQueries queries{*this,*human,source};world_.owners().hud_panels().auxiliary().update_order_display(human,queries);
    }
    void show_weapon_selector(bool visible){
        require_open();auto& items=d_.world_actors.items();ItemIcons icons{items};
        world_.owners().hud_panels().selector().show(visible,d_.startup.application_flags(),d_.first_players,items,icons);
    }
    void advance_weapon_selector(std::uint32_t elapsed){
        require_open();auto& items=d_.world_actors.items();ItemIcons icons{items};
        world_.owners().hud_panels().selector().update(elapsed,d_.startup.application_flags(),d_.first_players,items,icons);
    }
    void process_weapon_selector(GameUiSelectorCommands& commands,std::uint32_t* first=nullptr,std::uint32_t* second=nullptr,std::size_t count=0){
        require_open();SelectorControlQueries queries{*this,commands};input::BindingInputViews input(d_.startup.presentation_input_bindings(),first,second,count);world_.owners().hud_panels().selector().process_controls(input,queries);
    }
    void cycle_hud_layout(){require_open();d_.hud.cycle_layout(d_.startup.application_flags(),d_.startup.selected_game_mode());}
    void advance_hud_hands(){require_open();world_.owners().hud_panels().hands().update(d_.first_players,d_.world_actors.items(),d_.session.words.get<std::uint32_t>(0x28));}
    void advance_crosshair(std::uint32_t elapsed){
        require_open();world_.owners().hud_panels().crosshair().update_spread(elapsed,[&]()->std::optional<long double>{
            auto* human=d_.first_players.current_player();if(!human)return std::nullopt;
            return game::human_hud_weapon_spread(*human,d_.world_actors.items(),d_.first_players);
        });
    }
    void publish_crosshair_target(GameSceneActors::Instance* actor,std::uint8_t allowed,GameUiRollExternalSource* external=nullptr){
        require_open();if(actor)require_actor_borrower(actor);
        TargetQueries queries{*this,external?external:signal_external_};world_.owners().hud_panels().crosshair().publish_target(actor,allowed,d_.startup.application_flags(),queries);
    }
    void update_crosshair_visibility(){require_open();VisibilityQueries queries{*this};game::update_hud_crosshair_visibility(parent_.hud()->flags,world_.owners().hud_panels().crosshair(),queries);}
    void advance_hud_selection(std::uint32_t elapsed,GameUiHumanOrderSource* orders=nullptr,GameUiSignalExternalSource* external=nullptr){
        require_open();SelectionQueries queries{SignalQueries{RollQueries{CommandQueries{TargetQueries{*this}}}}};queries.orders=orders?orders:signal_orders_;queries.external=external?external:signal_external_;d_.hud.update_selection(elapsed,queries);
    }
    void advance_hud_status(std::uint32_t elapsed,GameUiMapSource* source=nullptr){require_open();MapQueries queries{*this,source};world_.owners().hud_panels().small().update_status(elapsed,d_.first_players,d_.startup.application_flags(),backend_.hud_style224(),[&](world_loading::Human& human){return queries.team(human);});}
    void advance_first_person(std::uint32_t elapsed){require_open();d_.first_person.advance(d_.startup.application_flags(),elapsed);}
    void complete_first_person_animation(std::uint32_t elapsed){require_open();d_.first_person.complete_animation(d_.startup.application_flags(),elapsed);}
    void update_first_person_opacity(){require_open();d_.first_person.update_opacity(d_.startup.application_flags());}
    void advance_injury_message(std::uint32_t elapsed){require_open();world_.owners().hud_panels().small().update_injury_message(elapsed);}
    void update_carnage(){require_open();world_.owners().hud_panels().small().update_carnage(d_.scene.frames().selected_human_count());}
    void update_hud_timers(){
        require_open();auto& panels=world_.owners().hud_panels();
        panels.timers().update(full_viewport(),[&](game::ActorBase<world_loading::Frames>* actor){
            //Membership must precede RTTI or any borrowed actor dereference.
            GameSceneActors::Instance* found=nullptr;
            for(std::size_t n=0;n<d_.scene.frames().scene_actor_identity_count();++n){
                auto* candidate=d_.scene.frames().scene_actor_identity(static_cast<std::int32_t>(n));
                if(candidate && &candidate->base()==actor){found=candidate;break;}
            }
            if(!found)throw std::logic_error("HUD explosive borrower is no longer in the actual World");
            auto* explosive=dynamic_cast<game::ExplosivePlaceSceneActor<world_loading::Frames>*>(found);
            if(!explosive)throw std::logic_error("HUD timer borrower is not an actual ExplosivePlace");
            return std::make_pair(explosive->field94,explosive->countdown());
        });
    }
    void advance_vehicle_display(){require_open();VehicleQueries queries{RollQueries{CommandQueries{TargetQueries{*this}}}};game::advance_hud_vehicle(world_.owners().hud_panels().artwork(),queries);}
    void advance_scoreboard(std::uint32_t elapsed){require_open();ScoreboardQueries queries{*this};world_.owners().hud_panels().scoreboard().update(elapsed,queries);}
    void advance_info(std::uint32_t elapsed,GameUiInfoSource* source=nullptr){require_open();InfoQueries queries{*this,source};world_.owners().hud_panels().info().update(elapsed,queries);}
    void refresh_info(GameUiInfoSource* source=nullptr){require_open();InfoQueries queries{*this,source};world_.owners().hud_panels().info().refresh(queries);}
    void show_info(bool enabled,GameUiInfoSource* source=nullptr){require_open();InfoQueries queries{*this,source};world_.owners().hud_panels().info().show(enabled,queries);}
    void advance_map(std::uint32_t elapsed,GameUiMapSource* source=nullptr){require_open();MapQueries queries{*this,source};world_.owners().hud_panels().map().update(elapsed,queries);}
    void show_map(bool enabled,bool camera=false,GameUiMapSource* source=nullptr){require_open();MapQueries queries{*this,source};auto& panels=world_.owners().hud_panels();panels.map().show(enabled,camera,queries,panels.auxiliary());}
    void process_map_controls(std::uint32_t* first=nullptr,std::uint32_t* second=nullptr,std::size_t count=0,GameUiMapSource* source=nullptr){
        require_open();auto& panels=world_.owners().hud_panels();if(panels.map().kind()==0||panels.map().kind()==1)return;
        MapQueries queries{*this,source};input::BindingInputViews input(d_.startup.presentation_input_bindings(),first,second,count);panels.map().process_controls(queries,panels.auxiliary(),input);
    }
    void refresh_inventory(){require_open();InventoryQueries queries{RollQueries{CommandQueries{TargetQueries{*this}}}};world_.owners().hud_panels().inventory().refresh(queries);}
    void publish_inventory_actors(std::uint32_t flags,world_loading::Human* primary,GameSceneActors::Instance* secondary){require_open();InventoryQueries queries{RollQueries{CommandQueries{TargetQueries{*this}}}};
        world_.owners().hud_panels().inventory().publish_actors(flags,static_cast<GameSceneActors::Instance*>(primary),secondary,queries);}
    void show_inventory_panel(bool enabled){require_open();InventoryQueries queries{RollQueries{CommandQueries{TargetQueries{*this}}}};world_.owners().hud_panels().inventory().show_inventory(enabled,queries);}
    void advance_inventory_skills(){require_open();InventoryQueries queries{RollQueries{CommandQueries{TargetQueries{*this}}}};world_.owners().hud_panels().inventory().update_skills(queries);}
    void advance_inventory(std::uint32_t elapsed){require_open();InventoryQueries queries{RollQueries{CommandQueries{TargetQueries{*this}}}};auto& panels=world_.owners().hud_panels();panels.inventory().update(elapsed,panels.auxiliary(),queries);}
    void process_inventory_controls(std::uint32_t* first=nullptr,std::uint32_t* second=nullptr,std::size_t count=0,GameUiInventoryCommands* commands=nullptr){
        require_open();InventoryControlQueries queries{InventoryQueries{RollQueries{CommandQueries{TargetQueries{*this}}}},commands};auto& panels=world_.owners().hud_panels();
        input::BindingInputViews input(d_.startup.presentation_input_bindings(),first,second,count);panels.inventory().process_controls(panels.auxiliary(),input,queries);
    }
    void show_inventory(bool enabled,GameSceneActors::Instance* target=nullptr,GameUiRollCommands* commands=nullptr){require_open();if(target)require_actor_borrower(target);
        InventoryPresentationQueries queries{InventoryQueries{RollQueries{CommandQueries{TargetQueries{*this}}}}};queries.commands=commands;game::show_hud_inventory(enabled,target,queries);}
    void command_before_scene(){
        require_open();CommandQueries queries{TargetQueries{*this}};auto& panels=world_.owners().hud_panels();
        panels.command().draw_before_scene(queries,panels.auxiliary());
    }
    void advance_command_hover(){require_open();auto& panels=world_.owners().hud_panels();panels.command().update_hover_caption(panels.auxiliary());}
    void advance_command(std::uint32_t elapsed,GameUiCommandFrameSource& source,GameUiSignalExternalSource* external=nullptr){
        require_open();CommandFrameQueries queries{*this,source,external?external:signal_external_};auto& panels=world_.owners().hud_panels();
        panels.command().update(elapsed,queries,panels.auxiliary());
    }
    void process_command_controls(GameUiCommandInputSource& source,std::uint32_t* first,std::uint32_t* second,std::size_t count,GameUiSignalExternalSource* external=nullptr){
        require_open();auto& panels=world_.owners().hud_panels();if(!panels.command().input_bound()||!panels.command().visible())return;
        if(!first||!second||count<64)throw std::invalid_argument("Command controls require original containing64-word input pair, not extra fabricated bindings");
        CommandInputQueries queries{*this,source,external?external:signal_external_};DialogInput input{d_.startup.presentation_input_bindings(),backend_.presentation_input(),first,second,count};panels.command().process_controls(queries,panels.auxiliary(),input);
    }
    void advance_command_routes(world_loading::Human& human,std::size_t index,GameUiHumanRouteSource& source){
        require_open();CommandRouteQueries queries{*this,source,signal_external_};(void)queries.human(human);
        world_.owners().hud_panels().command().update_routes(human,index,queries);
    }
    void show_command_panel(bool enabled,GameUiCommandModalSource& source,GameUiSignalExternalSource* external=nullptr){
        require_open();CommandModalQueries queries{*this,source,external?external:signal_external_};world_.owners().hud_panels().command().show(enabled,queries);
    }
    void show_command(bool enabled,GameUiCommandModalSource& source,GameUiSignalExternalSource* external=nullptr){
        require_open();CommandPresentationQueries queries{*this,source,external?external:signal_external_};game::show_hud_command(enabled,queries);
    }
    void refresh_hud_layout(){require_open();d_.hud.refresh_layout(d_.startup.application_flags());}
    void activate_command_misc_button(std::uint32_t tag,GameUiCommandControls& controls){
        require_open();CommandControlQueries queries{*this,controls};world_.owners().hud_panels().command().activate_misc_button(tag,queries);
    }
    void show_command_roll(bool enabled,GameSceneActors::Instance* target=nullptr,GameUiRollExternalSource* external=nullptr){require_open();RollQueries queries{CommandQueries{TargetQueries{*this}}};queries.external=external?external:signal_external_;
        auto& panels=world_.owners().hud_panels();panels.command().show_nested_roll(enabled,target,queries,panels.auxiliary(),d_.session.words.get<std::uint8_t>(0x64)!=0);}
    void advance_command_point_target(){
        require_open();CommandQueries queries{TargetQueries{*this}};auto& panels=world_.owners().hud_panels();panels.command().update_point_target(queries,panels.auxiliary());
    }
    void refresh_command_policies(GameUiHumanCommandSource& source){
        require_open();CommandPolicyQueries queries{*this,source};world_.owners().hud_panels().command().update_policies(queries);
    }
    void update_command_order_buttons(std::uint32_t actual_kind){require_open();world_.owners().hud_panels().command().update_order_buttons(actual_kind);}
    void select_hud_pointer(std::int32_t kind){require_open();world_.owners().hud_panels().auxiliary().select_pointer(kind);}
    void publish_roll_target(GameSceneActors::Instance* actor,float distance,bool command=false,GameUiRollExternalSource* source=nullptr){
        require_open();if(actor)require_actor_borrower(actor);RollQueries queries{CommandQueries{TargetQueries{*this}}};
        queries.external=source?source:signal_external_;
        roll_dialog(command).publish_roll_target(actor,distance,queries);
    }
    void show_roll(bool enabled,bool command=false,GameUiRollExternalSource* source=nullptr){require_open();RollQueries queries{CommandQueries{TargetQueries{*this}}};queries.external=source?source:signal_external_;roll_dialog(command).show_roll(enabled,queries);}
    void cycle_roll(bool command=false){require_open();RollQueries queries{CommandQueries{TargetQueries{*this}}};roll_dialog(command).cycle_roll(queries);}
    void hover_roll(bool command=false){require_open();RollQueries queries{CommandQueries{TargetQueries{*this}}};roll_dialog(command).hover_roll(world_.owners().hud_panels().auxiliary().pointer_position(),queries);}
    void advance_roll(std::uint32_t elapsed,bool command=false){require_open();RollQueries queries{CommandQueries{TargetQueries{*this}}};roll_dialog(command).advance_roll(elapsed,queries);}
    void position_roll(const std::array<float,4>& position,bool restore=false,bool command=false){require_open();auto& dialog=roll_dialog(command);if(restore)dialog.restore_roll_position();else dialog.position_roll(position);}
    void activate_roll(GameUiRollCommands& commands,GameUiRollExternalSource* external=nullptr){
        require_open();RollControlQueries queries{SignalQueries{RollQueries{CommandQueries{TargetQueries{*this}}}}};queries.roll_commands=&commands;queries.commands=&commands;queries.external=external?external:signal_external_;
        world_.owners().hud_panels().dialogs().activate_roll(queries);
    }
    void process_roll_controls(GameUiRollCommands& commands,std::uint32_t* first=nullptr,std::uint32_t* second=nullptr,std::size_t count=0,GameUiRollExternalSource* external=nullptr){
        require_open();RollControlQueries queries{SignalQueries{RollQueries{CommandQueries{TargetQueries{*this}}}}};queries.roll_commands=&commands;queries.commands=&commands;queries.external=external?external:signal_external_;
        DialogInput input{d_.startup.presentation_input_bindings(),backend_.presentation_input(),first,second,count};world_.owners().hud_panels().dialogs().process_roll_controls(input,queries);
    }
    void bind_signal_sources(world_loading::World& owner,GameUiHumanOrderSource& orders,GameUiSignalExternalSource& external){
        require_open();if(&owner!=&world_)throw std::logic_error("Signal producers require SAME containing World");
        if(signal_orders_||signal_external_)throw std::logic_error("Signal producers already bound");
        //Actual producer owners must outlive this UI borrower, or explicitly
        //unbind before destruction. No actor/font/frame is captured here.
        signal_orders_=&orders;signal_external_=&external;
    }
    void unbind_signal_sources()noexcept{signal_orders_=nullptr;signal_external_=nullptr;}
    void show_signals(bool enabled,GameSceneActors::Instance* target,world_loading::Human* human,GameUiHumanOrderSource* orders=nullptr,GameUiSignalExternalSource* external=nullptr){
        require_open();SignalQueries queries{RollQueries{CommandQueries{TargetQueries{*this}}}};queries.orders=orders?orders:signal_orders_;queries.external=external?external:signal_external_;
        world_.owners().hud_panels().dialogs().show_signals(enabled,target,static_cast<GameSceneActors::Instance*>(human),queries);
    }
    void publish_signal_target(GameSceneActors::Instance* target,GameUiHumanOrderSource* orders=nullptr,GameUiSignalExternalSource* external=nullptr){
        require_open();SignalQueries queries{RollQueries{CommandQueries{TargetQueries{*this}}}};queries.orders=orders?orders:signal_orders_;queries.external=external?external:signal_external_;world_.owners().hud_panels().dialogs().publish_signal_target(target,queries);
    }
    void advance_signals(std::uint32_t elapsed,GameUiHumanOrderSource* orders=nullptr,GameUiSignalExternalSource* external=nullptr){
        require_open();SignalQueries queries{RollQueries{CommandQueries{TargetQueries{*this}}}};queries.orders=orders?orders:signal_orders_;queries.external=external?external:signal_external_;world_.owners().hud_panels().dialogs().advance_signals(elapsed,queries);
    }
    void select_signal(GameUiSignalCommands& commands,GameUiHumanOrderSource* orders=nullptr,GameUiSignalExternalSource* external=nullptr){
        require_open();SignalQueries queries{RollQueries{CommandQueries{TargetQueries{*this}}}};queries.orders=orders?orders:signal_orders_;queries.external=external?external:signal_external_;queries.commands=&commands;
        world_.owners().hud_panels().dialogs().select_signal(queries);
    }
    void cycle_signal(bool previous,GameUiHumanOrderSource* orders=nullptr,GameUiSignalExternalSource* external=nullptr){
        require_open();SignalQueries queries{RollQueries{CommandQueries{TargetQueries{*this}}}};queries.orders=orders?orders:signal_orders_;queries.external=external?external:signal_external_;
        auto& dialogs=world_.owners().hud_panels().dialogs();if(previous)dialogs.previous_signal(queries);else dialogs.cycle_signals(queries);
    }
    void process_signal_controls(GameUiSignalCommands& commands,std::uint32_t* first=nullptr,std::uint32_t* second=nullptr,std::size_t count=0,
                                 GameUiHumanOrderSource* orders=nullptr,GameUiSignalExternalSource* external=nullptr){
        require_open();SignalQueries queries{RollQueries{CommandQueries{TargetQueries{*this}}}};queries.orders=orders?orders:signal_orders_;queries.external=external?external:signal_external_;queries.commands=&commands;
        DialogInput input{d_.startup.presentation_input_bindings(),backend_.presentation_input(),first,second,count};world_.owners().hud_panels().dialogs().process_signal_controls(input,queries);
    }
private:
    struct ConsoleQueries {
        GameUiPresentation& ui;GameUiConsoleActions* actions;
        GameUiConsoleActions& required(){if(!actions)throw std::runtime_error("Console command requires its actual Human/application/packet producer");return *actions;}
        std::uint32_t mode(){return ui.d_.startup.application_flags();}std::uint32_t difficulty(){return ui.d_.startup.human_difficulty();}
        void sensitivity(std::int32_t value){(void)input::set_binding_axis_limits(ui.d_.startup.presentation_input_bindings(),value,value,1);} //006C6C90, SAME input manager, no profile/settings rewrite.
        void system_command(const std::string& line){required().system_command(ui.world_,line);}
        void chat(std::string_view text,bool team){required().chat(ui.world_,text,ui.d_.startup.local_player_identity(),team);}
        auto* current_human(){auto* human=ui.d_.first_players.current_player();if(human)ui.require_actor_borrower(human);return human;}
        std::int32_t identity(world_loading::Human& human){ui.require_actor_borrower(&human);return human.base().id10();}
        std::int32_t local_identity(){return ui.d_.startup.local_player_identity();}
        void kill(world_loading::Human& human){ui.require_actor_borrower(&human);game::console_human_kill(ui.world_.owners().human_services(),human,[&](auto& target){required().kill_human(ui.world_,target);});}
        void network_kill(std::int32_t id){required().network_kill(ui.world_,id);}
        void heal_roster(){game::heal_console_roster(*this);} //0045F020 repeats native mode/difficulty gates.
        std::size_t roster_count(){return ui.d_.players.control().human_roster().size();}
        auto* roster_human(std::size_t index){if(index>0x7fffffffu)throw std::out_of_range("Console roster index");auto* human=ui.d_.players.control().human_roster().at(static_cast<std::int32_t>(index));if(human)ui.require_actor_borrower(human);return human;}
        std::int32_t health(world_loading::Human& human){return ui.d_.first_players.health(human);}
        void set_health_percent(world_loading::Human& human,std::int32_t value){required().set_health_percent(ui.world_,human,value);}
        bool has_definition(std::int32_t id){return id>=0&&id<500&&ui.d_.world_actors.items().catalogue().item(id)!=nullptr;}
        const auto& definition(std::int32_t id){const auto* value=ui.d_.world_actors.items().catalogue().item(id);if(!value)throw std::logic_error("Console item definition is not in SAME catalogue");return *value;}
        std::int32_t ammunition(std::int32_t id){return definition(id).ammunition();}
        std::uint32_t kind(std::int32_t id){return definition(id).kind();}
        void give_item(std::int32_t id,world_loading::Human* human){ //007D7930's actual mode/null/id/definition admission.
            if((mode()&7u)!=7u||!human||!has_definition(id))return;
            ui.require_actor_borrower(human);required().give_item(ui.world_,*human,id);
        }
    };
    struct MessageCommands final:GameUiMessageCommands {
        GameUiPresentation& ui;GameUiConsoleActions* actions;
        MessageCommands(GameUiPresentation& value,GameUiConsoleActions* source):ui(value),actions(source){}
        std::size_t capacity(char* input){const auto& buffer=ui.world_.owners().hud_panels().messages().state().input68;
            if(input!=buffer.data())throw std::logic_error("HUD command must borrow SAME actual message edit buffer");return buffer.size();}
        void console(char* input)override{(void)ui.dispatch_console(input,capacity(input),actions);}
        void say(char* input,bool team)override{(void)ui.dispatch_console_command(team?2u:1u,input,capacity(input),actions);}
    };
    RollDialogs& roll_dialog(bool command){auto& panels=world_.owners().hud_panels();return command?panels.command_roll():panels.dialogs();}
    void require_actor_borrower(const GameSceneActors::Instance* actor){
        if(!actor)throw std::logic_error("HUD target requires an actual actor");
        for(std::size_t n=0;n<d_.scene.frames().scene_actor_identity_count();++n)if(d_.scene.frames().scene_actor_identity(static_cast<std::int32_t>(n))==actor)return;
        throw std::logic_error("HUD target borrower is absent from SAME World identity registry");
    }
    world_loading::Human* find_human_borrower(const void* borrower){
        for(std::size_t i=0;i<d_.scene.frames().scene_actor_identity_count();++i){
            auto* actor=d_.scene.frames().scene_actor_identity(static_cast<std::int32_t>(i));
            auto* human=dynamic_cast<world_loading::Human*>(actor);if(human&&human==borrower)return human;
        }
        throw std::logic_error("HUD soldier borrower is absent from SAME World identity registry");
    }
    struct SoldierQueries{
        world_loading::World& world;world_loading::Dependencies& d;
        void validate_human(world_loading::Human& human){(void)d.first_players.inline_control(human);}
        auto* current_player(){return d.first_players.current_player();}
        bool remote_ai_enabled(){return d.startup.game_configuration().remote_ai_enabled();}
        std::uint32_t selected_objective_identity(){return game::hud_selected_objective_identity(d.session,d.scene.frames());}
        long double visibility_range(world_loading::Human& human){return game::human_sight_range(human,world.owners().human_services());}
        bool has_orders(world_loading::Human& human){
            auto& state=d.first_players.state(human);if(state.words().template get<std::uint32_t>(0x48)!=3)return false;
            auto* active=dynamic_cast<world_loading::Active*>(&state);
            if(!active)throw std::logic_error("Soldier order query requires its actual kind3 Active AI owner");
            return active->has_pending_orders_loading(); //Same250 owner; populated unsupported producer rejects.
        }
    };
    struct CompassQueries{
        GameUiPresentation& ui;
        auto* current_player(){return ui.d_.first_players.current_player();}
        auto* resolve_human(const void* borrower){return ui.find_human_borrower(borrower);}
        auto target_direction(world_loading::Human& human){return ui.d_.first_players.state(human).target_direction();}
        auto current_direction(world_loading::Human& human){return ui.d_.first_players.state(human).current_direction();}
        auto model(world_loading::Human& human){(void)ui.d_.first_players.inline_control(human);if(!human.base().frame())throw std::logic_error("Compass Human model is retired");return human.base().frame();}
        bool moving(world_loading::Human& human){return (ui.d_.first_players.state(human).words().template get<std::uint32_t>(0x50)&0xf8u)!=0;}
        std::uint32_t movement_flags(world_loading::Human& human){return ui.d_.first_players.state(human).words().template get<std::uint32_t>(0x5c);}
        bool movement_side(world_loading::Human& human){auto* control=ui.d_.first_players.inline_control(human).control();if(!control)throw std::logic_error("Compass movement sample requires actual DD0");return game::human_hud_movement_side(human,ui.d_.scene.frames(),control->physical().bones());}
        std::int32_t health(world_loading::Human& human){return ui.d_.first_players.health(human);}
        bool unavailable(world_loading::Human& human){(void)ui.d_.first_players.inline_control(human);return ui.world_.owners().human_services().human_unavailable(human);}
        bool suppressed(world_loading::Human& human){return human.properties.field19c==2&&!unavailable(human)&&human.loading_state&&ui.d_.first_players.state(human).suppress_hostile_selection();}
        std::uint32_t random_word(){return ui.d_.audio.random_source()();}
        std::uint32_t time(){return ui.d_.session.words.template get<std::uint32_t>(0x2c);}
    };
    struct ProgressQueries{
        GameUiPresentation& ui;GameUiZoneSource* source=nullptr;
        void validate_human(world_loading::Human& human){(void)ui.d_.first_players.inline_control(human);}
        std::uint32_t attachment_kind(world_loading::Human& human){return ui.d_.first_players.seat_presentation().kind(*human.field160);}
        bool attachment_has_control(world_loading::Human& human){return ui.d_.first_players.seat_presentation().has_control(*human.field160,human.field164);}
        std::optional<game::HumanHudProgress> attachment_progress(world_loading::Human& human){
            return ui.d_.first_players.seat_presentation().progress(*human.field160,human.field164);
        }
        auto* zone_actor(std::int32_t identity){return ui.d_.scene.frames().scene_actor_command_identity(identity);}
        std::uint32_t zone_kind(GameSceneActors::Instance& zone){return zone.base().type();} //Fresh actual00475D50 loan, including pending reserved identities.
        bool zone_active(GameSceneActors::Instance& zone){if(const auto* owner=game::actor_zone_presentation(zone))return owner->hud_zone_progress_active();
            if(!source)throw std::runtime_error("HUD zone progress requires actual kind37 F1 owner");return source->progress_active(zone);}
        game::HudZoneProgressData zone_data(GameSceneActors::Instance& zone){if(const auto* owner=game::actor_zone_presentation(zone))return owner->hud_zone_progress();
            if(!source)throw std::runtime_error("HUD zone progress requires actual kind37 fields");return source->progress_data(zone);}
        std::optional<game::HumanHudProgress> remote_progress(world_loading::Human& human){return game::human_remote_hud_progress(human,*this);}
    };
    struct OrderQueries{
        GameUiPresentation& ui;world_loading::Human& human;GameUiHumanOrderSource& source;
        void validate(){(void)ui.d_.first_players.state(human);}
        const void* actor_identity(){validate();return &human.base();}
        bool changed(){validate();return source.changed(human);}
        std::int32_t count(){validate();return source.count(human);}
        game::HudOrderDisplay order(std::uint32_t index){validate();return source.order(human,index);}
    };
    struct SnapshotPresentation{
        GameUiPresentation& ui;
        void snapshot_sound(){
            frontend::SpatialSoundRequest request;request.group=1; //Original004374D0 defaults, no persistent sound owner.
            const auto camera=ui.d_.scene.frames().active_game_camera();if(!camera)throw std::logic_error("HUD camera snapshot requires SAME scene active camera");
            const auto matrix=ui.d_.scene.frames().actor_world_matrix(camera);request.position={{matrix[12],matrix[13],matrix[14],matrix[15]}};
            request.source=nullptr;request.relative=nullptr;request.flags=0x41;request.entry=0x2e;
            (void)ui.d_.sound.emit(request); //SOUND virtual54, native ignores returned handle.
        }
        void snapshot_flash(){const std::array<float,4> black{{0,0,0,0}};ui.fade_(1.f,&black);} //GEffects008101F0+88=004867F0, direct draw; no fade-state/camera simulation.
    };
    struct TargetQueries{
        GameUiPresentation& ui;GameUiRollExternalSource* focus_source=nullptr;using Actor=GameSceneActors::Instance;using Human=world_loading::Human;
        const void* identity(Actor& actor){ui.require_actor_borrower(&actor);return &actor.base();}
        Human& human(Actor& actor){ui.require_actor_borrower(&actor);auto* value=dynamic_cast<Human*>(&actor);if(!value)throw std::logic_error("HUD target Human query requires actual Human actor");(void)ui.d_.first_players.inline_control(*value);return *value;}
        Human* current_player(){auto* value=ui.d_.first_players.current_player();if(value)(void)human(*value);return value;}
        Actor* attachment(Actor& actor){auto* value=human(actor).field160;if(value)ui.require_actor_borrower(value);return value;}
        Actor* focus(Actor& actor){auto& value=human(actor);if(!value.field174)return nullptr;
            auto* target=value.field174;ui.require_actor_borrower(target);return target;}
        std::int32_t health(Actor& actor){return ui.d_.first_players.health(human(actor));}
        std::uint32_t kind(Actor& actor){ui.require_actor_borrower(&actor);return actor.base().type();}
        std::uint32_t human_kind(Actor& actor){return human(actor).properties.field19c;}
        bool hostile(Actor& observer,Actor& target){(void)human(observer);(void)human(target);return ui.d_.sight.human_sight_hostile(&observer,&target);}
        bool in_roster(Actor& actor){return ui.d_.players.control().human_roster().contains(&human(actor));}
        std::int32_t roster_index(Actor& actor){return ui.d_.players.control().human_roster().index(&human(actor));}
        std::string full_name(Actor& actor){auto& value=human(actor);if((ui.d_.startup.application_flags()&7u)!=7u&&value.properties.field19c==1)return ui.backend_.hud_human_peer_name(value);return game::offline_human_target_name(ui.d_.scene.frames(),value);}
        std::string caption(Actor& actor){
            ui.require_actor_borrower(&actor);
            if(actor.base().type()==3)return game::human_target_caption(ui.d_.scene.frames(),human(actor),ui.d_.startup.application_flags(),[&](std::uint32_t id){const auto text=ui.backend_.hud_localized(id);return text?*text:std::string{};},[&](Human& value){return ui.backend_.hud_human_peer_name(value);});
            return actor.base().name();
        }
        std::string rank_text(Actor& actor){auto& value=human(actor);const auto rank=ui.backend_.hud_human_rank(value,ui.d_.startup.application_flags());if(rank>=8)throw std::out_of_range("HUD target rank exceeds original eight caption rows");const auto text=ui.backend_.hud_localized(0x27a6u+rank);return text?*text:std::string{};}
        std::int32_t medic_skill(Actor& actor){return game::human_hud_signed(human(actor).properties.stats_ec[7]);}
        std::int32_t health_percent(Actor& actor){return game::human_hud_health_percent(human(actor));}
        auto& car(Actor& actor){ui.require_actor_borrower(&actor);auto* value=dynamic_cast<game::CarSceneActor<world_loading::Frames>*>(&actor);if(!value||!value->geometry)throw std::logic_error("HUD Car target requires SAME loaded Car geometry");if(value->occupants.size()!=value->geometry->auxiliary().seats().size())throw std::logic_error("HUD Car seat population differs from actual geometry");return *value;}
        auto& static_weapon(Actor& actor){
            ui.require_actor_borrower(&actor);
            auto* weapon=dynamic_cast<game::StaticWeaponSceneActor<world_loading::Frames>*>(&actor);
            using Loading=game::StaticWeaponLoading<world_loading::Frames,world_loading::Actors,world_loading::Actors::Collisions,world_loading::Actors::Items>;
            auto* loading=weapon?dynamic_cast<Loading*>(weapon->loading.get()):nullptr;
            if(!loading)throw std::logic_error("HUD static weapon target requires SAME actual loading owner");return *loading;
        }
        std::size_t occupant_count(Actor& actor){ui.require_actor_borrower(&actor);
            if(actor.base().type()==4)return car(actor).occupants.size();
            if(actor.base().type()==21)return static_weapon(actor).seats().size();
            return game::actor_occupants_presentation(actor).hud_occupant_count();}
        Actor* occupant(Actor& actor,std::size_t n){
            ui.require_actor_borrower(&actor);Actor* value=nullptr;
            if(actor.base().type()==4)value=car(actor).occupants.at(n);
            else if(actor.base().type()==21){auto& loading=static_weapon(actor);if(n<loading.seats().size())value=static_cast<Actor*>(loading.seats()[n].occupant);}
            else value=game::actor_occupants_presentation(actor).hud_occupant(n);
            if(value)(void)human(*value);return value;
        }
        bool car_player_controlled(Actor& actor){return game::script_human_player_controlled(human(actor),ui.d_.startup.application_flags());}
        bool car_player_suppressed(Actor& actor){return ui.d_.presentation.human_selected_suppressed(human(actor));}
        std::uint32_t car_human_team(Actor& actor){return ui.backend_.hud_human_team(human(actor),ui.d_.startup.application_flags());}
        bool car_hostile(Actor& actor,Actor& observer){return game::hud_car_hostile_occupants(actor,observer,ui.d_.startup.application_flags(),*this);} //004DDE20.
    };
    struct CommandQueries:TargetQueries{
        std::vector<Actor*> selected_humans(){std::vector<Human*> selected;ui.d_.players.control().selection().selected(selected);return {selected.begin(),selected.end()};}
        Actor* resolve(const void* identity){
            for(std::size_t n=0;n<ui.d_.scene.frames().scene_actor_identity_count();++n){auto* actor=ui.d_.scene.frames().scene_actor_identity(static_cast<std::int32_t>(n));if(actor&&&actor->base()==identity)return actor;}
            throw std::logic_error("Command cached actor is absent from SAME World registry");
        }
        auto& base(Actor& actor){ui.require_actor_borrower(&actor);return actor.base();}
        bool unavailable(Actor& actor){return ui.world_.owners().human_services().human_unavailable(human(actor));}
        std::int32_t occupant_count(Actor& actor){
            const auto size=TargetQueries::occupant_count(actor);
            if(size>0x7fffffffu)throw std::length_error("Command attachment seat count exceeds native signed range");return static_cast<std::int32_t>(size);
        }
        bool human_hidden(Actor& actor){return human(actor).activation.flag150;}
        bool static_weapon_ready(Actor& actor){ui.require_actor_borrower(&actor);auto* weapon=dynamic_cast<game::StaticWeaponSceneActor<world_loading::Frames>*>(&actor);if(!weapon)throw std::logic_error("Command target requires actual StaticWeapon actor");return weapon->field2f0==1;}
        bool animation_excludes_target(Actor& actor){ui.require_actor_borrower(&actor);auto* animation=dynamic_cast<game::AnimationSceneActor<world_loading::Frames>*>(&actor);if(!animation)throw std::logic_error("Command target requires actual Animation actor");return animation->properties.flaga4!=0;}
        struct RayHit{std::array<float,4> position;std::uint32_t flags;Actor* actor;};
        std::optional<RayHit> command_ray(const std::array<float,4>& origin,const std::array<float,4>& travel){
            const auto hit=ui.world_.owners().human_services().human_geometry_ray(origin,travel,0,0x610000u,false);if(!hit)return std::nullopt;
            auto* actor=hit->dynamic_identity?static_cast<Actor*>(hit->dynamic_identity->actor):nullptr;
            const auto flags=hit->dynamic_identity?hit->dynamic_identity->flags:0u;
            if(actor&&(flags&0x40u))ui.require_actor_borrower(actor);return RayHit{hit->position,flags,actor};
        }
    };
    struct RollQueries:CommandQueries{
        using Entry=world_loading::Actors::Items::Entry;
        GameUiRollExternalSource* external=nullptr;
        Actor* focus(Actor& actor){auto& value=human(actor);if(!value.field174)return nullptr;
            auto* focus=value.field174;ui.require_actor_borrower(focus);return focus;}
        bool offline(){return (ui.d_.startup.application_flags()&7u)==7u;}
        Actor* selected_human(){auto* value=ui.d_.players.control().selected_player(ui.d_.startup.application_flags());if(value)(void)human(*value);return value;}
        Actor* previous_human(){auto* value=ui.d_.players.control().previous_player();if(value)(void)human(*value);return value;}
        template<class Dialogs>Actor* target(Dialogs& dialog){const auto identity=dialog.roll().field88;return identity?resolve(identity):nullptr;}
        template<class Dialogs>void set_target(Dialogs& dialog,const void* identity){(void)resolve(identity);dialog.roll_target_identity(identity);}
        auto& state(Actor& actor){return ui.d_.first_players.state(human(actor));}
        std::uint32_t player_mode(Actor& actor){return ui.d_.first_players.player_mode(human(actor),ui.d_.startup.application_flags());}
        bool action_active(Actor& actor){return state(actor).words().template get<std::int32_t>(0x60)!=-1;} //005A45C0.
        std::uint32_t movement_animation(Actor& actor){auto* control=ui.d_.first_players.inline_control(human(actor)).control();if(!control)throw std::logic_error("Roll movement query requires actual DD0");
            const auto clip=control->animation_state_word(0x1d8);return clip>=106&&clip<=108?1u:clip==8||clip==14?2u:0u;}
        bool navigation_floor(Actor& actor){return ui.d_.first_players.inline_control(human(actor)).frame_navigation_floor();}
        bool flag18(Actor& actor){return (state(actor).words().template get<std::uint32_t>(0x50)&0x18u)!=0;}
        std::uint32_t pose(Actor& actor){return state(actor).words().template get<std::uint32_t>(0x50)&7u;}
        bool human_flag1a5(Actor& actor){return human(actor).properties.flags1a4[1]!=0;}
        std::int32_t human_seat(Actor& actor){return human(actor).field164;}
        bool suppressed(Actor& actor){auto& value=human(actor);return value.properties.field19c==2&&value.loading_state&&value.loading_state->suppress_hostile_selection();}
        std::uint32_t team(Actor& actor){return ui.backend_.hud_human_team(human(actor),ui.d_.startup.application_flags());}
        std::int32_t hud_style(){return ui.backend_.hud_style224();}bool special_game_mode(){return ui.d_.startup.human_remote_ai_enabled();}
        std::int32_t actor_id(Actor& actor){ui.require_actor_borrower(&actor);return actor.base().id10();}
        Actor* human_by_identity(std::int32_t index){auto* value=ui.d_.scene.frames().scene_actor_identity(index);if(!value)return nullptr;(void)human(*value);return value;}
        std::string short_name(Actor& actor){return ui.backend_.hud_human_short_name(human(actor),ui.d_.startup.application_flags());}
        Entry weapon(Actor& actor){game::FirstPersonHumanItems<world_loading::Actors::Items> items(ui.d_.world_actors.items());return items.weapon(human(actor));}
        bool weapon_pending(Actor& actor){if(!weapon(actor))return false;const auto entry=weapon(actor);return ui.d_.session.words.get<std::uint32_t>(0x28)<ui.d_.world_actors.items().runtime_item(entry).field5c;}
        Entry medical_item(Actor& actor){ //005A7E00 checks holder5 before reverse holsters kind13, with no holder cooldown test.
            auto& value=human(actor);auto& items=ui.d_.world_actors.items();auto* storage=value.base().storage();
            if(!storage||storage->single()||!storage->holder(5))throw std::logic_error("Roll medical query requires actual Human holder5");
            auto* holder=storage->holder(5);if(holder->entry()&&items.runtime_item(holder->entry()).definition().kind()==13)return holder->entry();
            return game::find_player_mode_holster(value,items,13,ui.d_.session.words.get<std::uint32_t>(0x28));
        }
        bool placed_item_available(Actor& actor){return game::find_player_mode_holster(human(actor),ui.d_.world_actors.items(),19,ui.d_.session.words.get<std::uint32_t>(0x28))!=Entry{};}
        bool item_kind_count(Actor& actor,std::int32_t kind){return game::find_player_mode_holster(human(actor),ui.d_.world_actors.items(),kind,ui.d_.session.words.get<std::uint32_t>(0x28))!=Entry{};}
        bool has_definition(Actor& actor,std::int32_t definition){return game::find_human_inventory_definition(human(actor),ui.d_.world_actors.items(),definition)!=Entry{};}
        std::int32_t definition_count(Actor& actor,std::int32_t definition){return game::count_human_definition(human(actor),ui.d_.world_actors.items(),definition);}
        std::int32_t ammunition_count(Actor& actor,std::int32_t definition){return game::count_human_ammunition(human(actor),ui.d_.world_actors.items(),definition,false);}
        std::int32_t ammunition_definition(Entry entry){return ui.d_.world_actors.items().runtime_item(entry).definition().ammunition();}
        std::int32_t item_caption(std::int32_t definition){const auto* item=ui.d_.world_actors.items().catalogue().item(definition);if(!item)throw std::logic_error("Roll supply row requires actual populated item definition");return game::human_hud_signed(item->caption);}
        template<class T>T& typed(Actor& actor){ui.require_actor_borrower(&actor);auto* value=dynamic_cast<T*>(&actor);if(!value)throw std::logic_error("Roll actor query disagrees with actual loaded actor owner");return *value;}
        auto& door(Actor& actor){return typed<game::DoorSceneActor<world_loading::Frames>>(actor);}
        bool door_locked(Actor& actor){return door(actor).properties.flag234!=0;}
        std::uint32_t door_state(Actor& actor){return door(actor).properties.fields220[1];}
        std::int32_t door_key(Actor& actor){return game::human_hud_signed(door(actor).properties.field230);}
        bool door_pickable(Actor& actor){return door(actor).properties.flag235!=0;}
        bool door_use_blocked(Actor& actor){return door(actor).properties.flag118!=0;}
        bool door_has_link(Actor& actor){auto& value=door(actor);return value.portal.portal()||value.portal.attachment();}
        bool animation_active(Actor& actor){return typed<game::AnimationSceneActor<world_loading::Frames>>(actor).flaga1;}
        auto& supply(Actor& actor){return typed<game::SupplyBoxSceneActor<world_loading::Frames>>(actor);}
        std::uint32_t supply_state(Actor& actor){return supply(actor).field250;}
        std::uint32_t supply_flags(Actor& actor){auto& value=supply(actor);
            using Loading=game::SupplyBoxLoading<world_loading::Frames,world_loading::Actors,world_loading::Actors::Collisions,world_loading::Actors::Vertices,game::EffectsFrameFades<world_loading::Frames>>;
            auto* loading=dynamic_cast<Loading*>(value.loading.get());if(!loading)throw std::logic_error("Roll requires actual SupplyBox loading owner");return loading->state();}
        bool supply_limited(Actor& actor){return supply(actor).field130!=0;}
        std::int32_t supply_maximum(Actor& actor){return game::human_hud_signed(supply(actor).properties.field128);}
        bool supply_uses_list(Actor& actor){return supply(actor).properties.field11c!=0;}
        const auto& supply_items(Actor& actor,bool second){return supply(actor).properties.items[second?1:0];}
        auto& car_loading(Actor& actor){auto& value=car(actor);using Queries=decltype(ui.d_.scene.car_queries());
            using Loading=game::CarLoadingActivation<world_loading::Frames,world_loading::Actors,world_loading::Actors::Collisions,Queries>;
            auto* loading=dynamic_cast<Loading*>(value.loading.get());if(!loading)throw std::logic_error("Roll requires actual Car loading owner");return *loading;}
        GameUiRollExternalSource& actor_source(Actor& actor){ui.require_actor_borrower(&actor);if(!external)throw std::runtime_error("Roll requires the actual conditional Actor owner/data receiver");return *external;}
        std::int32_t seat_caption(Actor& actor,std::int32_t index){if(kind(actor)==4)return car_loading(actor).seats().seats().at(static_cast<std::size_t>(index)).name;
            ui.require_actor_borrower(&actor);return game::actor_occupants_presentation(actor).hud_seat_caption(index);}
        bool car_destroyed(Actor& actor){return car_loading(actor).cold_state().health.body==0.f;}
        bool car_interaction_blocked(Actor& actor,Actor& observer){return this->car_hostile(actor,observer);} //SAME004DDE20, observer is not reversed.
        bool car_interaction_available(Actor& actor){auto& value=car(actor);if(!external)throw std::runtime_error("Ordinary roll eligibility requires actual Car254A and004D5430 occupied-seat producer");return external->car_interaction_available(value);}
        auto& explosive(Actor& actor){return typed<game::ExplosivePlaceSceneActor<world_loading::Frames>>(actor);}
        std::int32_t explosive_state(Actor& actor){return game::human_hud_signed(explosive(actor).field94);}
        std::int32_t explosive_definition(Actor& actor){return game::human_hud_signed(explosive(actor).properties.fielda0);}
        bool switch_available(Actor& actor,Actor& observer){return actor_source(actor).switch_available(actor,human(observer));}
        std::int32_t switch_definition(Actor& actor){return actor_source(actor).switch_definition(actor);}
        std::int32_t switch_state(Actor& actor){return actor_source(actor).switch_state(actor);}
        std::int32_t item_container_count(Actor& actor){ui.require_actor_borrower(&actor);auto* selected=selected_human();if(!selected)throw std::logic_error("Roll pickup query requires actual selected Human");
            if(!external)throw std::runtime_error("Roll loose-item pickup requires actual007DAF80 dry-run transfer receiver");return external->loose_item_pickup_capacity(human(*selected),actor);}
        std::int32_t special_caption(Actor& actor){return actor_source(actor).special_caption(actor);}
        std::int32_t special_definition(Actor& actor){return actor_source(actor).special_definition(actor);}
    };
    struct SignalQueries:RollQueries{
        using Actor=TargetQueries::Actor;
        GameUiHumanOrderSource* orders=nullptr;
        GameUiSignalCommands* commands=nullptr;
        template<class Signals>void dispatch_signal(const Signals& signals){game::dispatch_hud_signal(signals,*this);}
        auto signal_direction(Actor& actor){return game::human_camera_target_direction(human(actor));}
        float signal_range(){return ui.d_.scene.frames().sight_view_distance();} //Mission camera virtual58=00465DE0.
        auto signal_origin(){game::MissionCameraState::Vector origin;if(!ui.d_.camera.query_vector(0,origin))throw std::logic_error("Signal point requires SAME active camera");return origin;}
        void exclude_signal_human(Actor& actor){ui.d_.first_players.inline_control(human(actor)).exclude_query(false);}
        void restore_signal_human(Actor& actor){ui.d_.first_players.inline_control(human(actor)).restore_query(false);}
        std::optional<std::array<float,4>> signal_ray(const std::array<float,4>& origin,const std::array<float,4>& travel){
            const auto hit=ui.world_.owners().human_services().human_geometry_ray(origin,travel,0x110000u,0,false);if(!hit)return std::nullopt;return hit->position;
        }
        auto signal_point(float range){std::array<float,4> position;game::hud_signal_point(range,position,*this);return position;}
        void submit_signal(Actor& actor,const game::HudSignalCommand<Actor>& request){if(!commands)throw std::runtime_error("Signal execution requires actual Player0053D910 command producer");commands->submit(human(actor),request);}
        std::size_t roster_count(){return ui.d_.players.control().human_roster().size();}
        Actor* roster_human(std::size_t index){auto* value=ui.d_.players.control().human_roster().at(static_cast<std::int32_t>(index));if(!value)throw std::logic_error("Signal roster query requires actual Human");(void)human(*value);return value;}
        std::uint32_t live_humans(){return ui.d_.players.control().human_roster().live_count();}
        std::int32_t orders_count(Actor& actor){auto& value=human(actor);if(!orders)throw std::runtime_error("Signal population requires actual Human005A5F00 order-count producer");return orders->count(value);}
        std::vector<Actor*> signal_recipients(unsigned mode){ //Process0089AC30 virtual08=004568D0, ordinary modes0..3 copy SAME group buffers.
            if(mode>3)throw std::out_of_range("Signal recipient query requires original special-mode collector");
            const auto& group=ui.d_.startup.players_process().groups().at(mode);std::vector<Actor*> output;output.reserve(group.size());
            for(auto* value:group){if(!value)throw std::logic_error("Signal process group has null Human");(void)human(*value);output.push_back(value);}return output;
        }
        std::int32_t formation_kind(Actor& actor){auto& value=human(actor);return value.formation?value.formation->mode():13;} //005A9960/00597B40.
        bool formation_scale_minimum(){return ui.d_.startup.players_process().words().template get<std::int32_t>(0x120)==0;} //004595D0 virtual40.
        bool formation_scale_maximum(){return ui.d_.startup.players_process().words().template get<std::int32_t>(0x120)==7;} //004595C0 virtual3C.
        bool any_definition(std::int32_t definition){ //0045E9A0, reverse roster and fresh Human after health query.
            for(auto n=roster_count();n;){--n;if(health(*roster_human(n))>0&&has_definition(*roster_human(n),definition))return true;}return false;
        }
        bool any_medical_for(Actor& target){ //0045EA00; candidate identity BEFORE fresh health and item lookups.
            for(auto n=roster_count();n;){--n;if(roster_human(n)!=&target&&health(*roster_human(n))>0&&medical_item(*roster_human(n)))return true;}return false;
        }
        bool attachment_seat_special(Actor& actor,Actor& attachment){auto& value=human(actor);ui.require_actor_borrower(&attachment);
            return ui.d_.first_players.seat_presentation().has_control(attachment,value.field164);}
        std::int32_t definition_pickup_capacity(Actor& actor,std::int32_t definition){auto& value=human(actor);auto* producer=dynamic_cast<GameUiSignalExternalSource*>(external);
            if(!producer)throw std::runtime_error("Signal supply action requires actual007DB170 dry-run transfer producer");return producer->definition_pickup_capacity(value,definition);}
        void publish_signal_selection(Actor* filter,unsigned mode){ //0049D340, SAME roster-indexed HUD soldier panels, fresh count after callbacks.
            Actor* previous=mode==3?previous_human():nullptr;std::vector<Actor*> recipients;
            if(mode!=3&&mode!=5)recipients=signal_recipients(mode);
            for(std::size_t n=0;n<roster_count();++n){bool selected=false;
                if(mode!=5){auto* candidate=roster_human(n);if(mode==3)selected=candidate==previous;
                    else if(!filter||filter==candidate)for(auto i=recipients.size();i;)if(recipients[--i]==candidate){selected=true;break;}}
                ui.world_.owners().hud_panels().soldiers().at(n).set_selected(selected);
            }
        }
    };
    struct VehicleQueries:RollQueries{
        std::uint32_t hud_flags(){return ui.parent_.hud()->flags;}
        bool command_visible(){return ui.world_.owners().hud_panels().command().visible();}
        bool inventory_visible(){const auto root=ui.world_.owners().hud_panels().inventory().root();if(!root)throw std::logic_error("Vehicle HUD requires actual inventory artwork");return ui.d_.scene.frames().hud_frame_visible(root);}
        bool roll_visible(){return ui.world_.owners().hud_panels().dialogs().roll().flag30!=0;}
        Actor* current_human(){return current_player();}
        float fuel_ratio(Actor& actor){auto& loading=car_loading(actor);return static_cast<float>(static_cast<long double>(loading.fuel_amount())/car(actor).geometry->physics().scalars().get(0x2220));}
        float car_speed(Actor& actor){return car_loading(actor).cold_state().solver.speed;}
        float car_rpm(Actor& actor){const auto& drive=car_loading(actor).cold_state().drive;return drive.running?static_cast<float>(static_cast<long double>(drive.engine_speed)*60.f):0.f;} //004F4C50 actor22BF/core222F and864/core7D4.
    };
    struct FrameVehicleQueries:VehicleQueries{world_loading::Human* cached;Actor* current_human(){if(cached)ui.require_actor_borrower(cached);return cached;}};
    struct HudFrameQueries{
        GameUiPresentation& ui;const GameUiHudFrameSources& sources;
        std::uint32_t mode(){return ui.d_.startup.application_flags();}
        bool paused(){return ui.d_.session.words.get<std::uint8_t>(0x64)!=0;}
        auto* current_human(){TargetQueries queries{ui};return queries.current_player();}
        auto* previous_human(){auto* human=ui.d_.players.control().previous_player();if(human)ui.require_actor_borrower(human);return human;}
        bool inventory_changed(world_loading::Human& human){ui.require_actor_borrower(&human);return human.activation.flag178;}
        auto* order_source(){return sources.orders?sources.orders:sources.command?sources.command:ui.signal_orders_;}
        void orders(world_loading::Human* human){if(!human)throw std::logic_error("HUD frame order display requires current/previous Human");ui.require_actor_borrower(human);
            auto* source=order_source();if(!source)throw std::runtime_error("HUD frame requires actual Human order producer");OrderQueries queries{ui,*human,*source};ui.world_.owners().hud_panels().auxiliary().update_order_display(human,queries);}
        void messages(std::uint32_t elapsed){if(sources.messages)ui.advance_hud_messages(elapsed,*sources.messages);else ui.advance_hud_messages(elapsed,sources.console);}
        void info(std::uint32_t elapsed){ui.advance_info(elapsed,sources.info);}
        void map(std::uint32_t elapsed){ui.advance_map(elapsed,sources.map);}
        void camera_snapshot(std::uint32_t elapsed){ui.advance_camera_snapshot(elapsed);}
        void crosshair(std::uint32_t elapsed){ui.advance_crosshair(elapsed);}
        void selection(std::uint32_t elapsed){ui.advance_hud_selection(elapsed,order_source(),sources.signals);}
        void signals(std::uint32_t elapsed){ui.advance_signals(elapsed,order_source(),sources.signals);}
        void roll(std::uint32_t elapsed){RollQueries queries{CommandQueries{TargetQueries{ui}}};queries.external=sources.signals?sources.signals:ui.signal_external_;ui.roll_dialog(false).advance_roll(elapsed,queries);}
        void selector(std::uint32_t elapsed){ui.advance_weapon_selector(elapsed);}
        void progress(world_loading::Human* human){ProgressQueries queries{ui,sources.zone};ui.world_.owners().hud_panels().auxiliary().update_progress([&]()->std::optional<game::HumanHudProgress>{
            if(!human)return std::nullopt;ui.require_actor_borrower(human);return game::human_hud_progress(*human,mode(),queries);});}
        void hands(std::uint32_t){ui.advance_hud_hands();}void status(std::uint32_t elapsed){ui.advance_hud_status(elapsed,sources.map);}void soldiers(){ui.advance_hud_soldiers();}
        void timers(std::uint32_t){ui.update_hud_timers();}void carnage(std::uint32_t){ui.update_carnage();}void compass(std::uint32_t elapsed){ui.advance_hud_compass(elapsed);}
        void command(std::uint32_t elapsed){if(!sources.command)throw std::runtime_error("HUD frame requires actual command Human/camera producers");ui.advance_command(elapsed,*sources.command,sources.signals);}
        void inventory(std::uint32_t elapsed){ui.advance_inventory(elapsed);}
        void vehicle(world_loading::Human* human){FrameVehicleQueries queries{VehicleQueries{RollQueries{CommandQueries{TargetQueries{ui}}}},human};game::advance_hud_vehicle(ui.world_.owners().hud_panels().artwork(),queries);}
        void scoreboard(std::uint32_t elapsed){ui.advance_scoreboard(elapsed);}
    };
    struct SelectionQueries:SignalQueries{
        void selection_visibility(){ui.update_crosshair_visibility();}
        bool selection_command_visible(){return ui.world_.owners().hud_panels().command().visible();}
        auto selection_direction(){game::MissionCameraState::Vector value;if(!ui.d_.camera.query_direction(0,value))throw std::logic_error("HUD selection requires SAME actual active camera direction");return value;}
        auto selection_origin(Actor& actor){auto& value=human(actor);const auto model=value.base().frame();if(!model)throw std::logic_error("HUD selection Human model is retired");
            const auto marker=ui.d_.scene.frames().actor_find(model,ui.d_.first_person.view().enabled()?"cameraFPV":"camera3RD",0x20);if(!marker)throw std::logic_error("HUD selection requires original Human camera marker");
            const auto matrix=ui.d_.scene.frames().actor_world_matrix(marker);return std::array<float,4>{{matrix[12],matrix[13],matrix[14],matrix[15]}};}
        float selection_range(){return signal_range();}
        auto selection_head(Actor& actor){auto& value=human(actor);if(!value.head_marker)throw std::logic_error("HUD selection requires original current Human260 head marker");const auto matrix=ui.d_.scene.frames().actor_world_matrix(value.head_marker);return std::array<float,4>{{matrix[12],matrix[13],matrix[14],matrix[15]}};}
        struct Hit{std::array<float,4> position;std::uint32_t flags;Actor* actor;const void* dynamic_identity;std::optional<game::CollisionCellReference> static_identity;};
        std::optional<Hit> selection_ray(const std::array<float,4>& origin,const std::array<float,4>& travel,std::uint32_t flags){const auto hit=ui.world_.owners().human_services().human_geometry_ray(origin,travel,0,flags,false);if(!hit)return std::nullopt;
            std::uint32_t found_flags=0;Actor* actor=nullptr;
            if(hit->dynamic_identity){found_flags=hit->dynamic_identity->flags;actor=static_cast<Actor*>(hit->dynamic_identity->actor);if(actor&&(found_flags&0x40u))ui.require_actor_borrower(actor);}
            else if(hit->static_identity){const auto id=*hit->static_identity;const auto& tree=ui.d_.scene.collision_tree();found_flags=id.kind?tree.shapes.at(id.kind-0x80).at(id.index).flags:tree.triangles.at(id.index).flags;}
            return Hit{hit->position,found_flags,actor,hit->dynamic_identity,hit->static_identity};
        }
        bool same_selection_hit(const Hit& first,const Hit& second){if(first.dynamic_identity||second.dynamic_identity)return first.dynamic_identity==second.dynamic_identity;
            if(!first.static_identity||!second.static_identity)return false;return first.static_identity->kind==second.static_identity->kind&&first.static_identity->index==second.static_identity->index;}
        auto* selection_identity(Actor& actor){ui.require_actor_borrower(&actor);return &actor.base();}
        bool selection_door_blocked(Actor& actor){return door_use_blocked(actor);}
        void selection_roll_target(Actor* actor,float distance){ui.world_.owners().hud_panels().dialogs().publish_roll_target(actor,distance,*this);}
        void selection_crosshair(Actor* actor,bool allowed){ui.world_.owners().hud_panels().crosshair().publish_target(actor,allowed,ui.d_.startup.application_flags(),*this);}
        bool selection_signals_active(){return ui.world_.owners().hud_panels().dialogs().signals_active();}
        void selection_signal_target(const void* identity){ui.world_.owners().hud_panels().dialogs().publish_signal_target(identity?resolve(identity):nullptr,*this);}
    };
    struct InventoryQueries:RollQueries{
        using Entry=world_loading::Actors::Items::Entry;
        auto& storage(Actor& actor){ui.require_actor_borrower(&actor);auto* value=actor.base().storage();if(!value||value->single())throw std::logic_error("Inventory body requires actual Human storage");return *value;}
        Entry holder_entry(Actor& actor,unsigned index){auto* holder=storage(actor).holder(index);return holder?holder->entry():nullptr;}
        auto* array_storage(Actor& actor,unsigned index){return storage(actor).slots(index);}
        const void* item_identity(Entry entry){(void)ui.d_.world_actors.items().runtime_item(entry);return entry;}
        auto& item(const void* identity){return ui.d_.world_actors.items().runtime_item(static_cast<Entry>(const_cast<void*>(identity)));} //Membership checked BEFORE RTTI/dereference by SAME module.
        unsigned item_family(const void* identity){return static_cast<unsigned>(item(identity).family());}
        float item_amount(const void* identity){return item(identity).amount();}
        void validate_item(const void* identity){(void)item(identity);}
        std::uint32_t item_caption(const void* identity){return item(identity).definition().caption;}
        float item_weight(const void* identity){return item(identity).definition().weight;}
        std::uint32_t item_kind(const void* identity){return item(identity).definition().kind();}
        std::int32_t item_ammunition(const void* identity){return item(identity).definition().ammunition();}
        float item_capacity(const void* identity){return item(identity).capacity();}
        static std::uint32_t payload_word(const frontend::PreMissionItemPayload& payload,std::size_t offset){if(offset>payload.bytes.size()||payload.bytes.size()-offset<4)throw std::runtime_error("Inventory descriptor lacks original field");return frontend::pre_mission_detail::Bytes::word(payload.bytes.data()+offset);}
        std::optional<float> item_description_speed(const void* identity){const auto& payload=item(identity).definition().payloads[0];
            //Native payload kinds2/3 have different virtual14 fields:007E0320 +38,007E0380 +34. Serialized omits vtable.
            if(payload.type==3)return frontend::pre_mission_detail::Bytes::real(payload_word(payload,0x34));
            if(payload.type==4)return frontend::pre_mission_detail::Bytes::real(payload_word(payload,0x30));return std::nullopt;
        }
        std::optional<float> item_description_zoom(const void* identity){const auto& definition=item(identity).definition();if(definition.kind()!=4)return std::nullopt;const auto& payload=definition.payloads[1];
            if(payload.type!=5||game::human_hud_signed(payload_word(payload,8))>=4)return std::nullopt;
            const auto fov=frontend::pre_mission_detail::Bytes::real(payload_word(payload,12));return static_cast<float>(std::tan(.6981317202250164)/std::tan(static_cast<long double>(fov)*.5f));
        }
        auto item_icon(Entry entry){return ui.d_.world_actors.items().item_icon(item(entry).definition());}
        float weight(Actor& actor){ui.require_actor_borrower(&actor);float value;const auto word=actor.base().field7c();std::memcpy(&value,&word,4);return value;}
        std::int32_t weight_limit(Actor& actor){ui.require_actor_borrower(&actor);return ui.d_.scene.frames().actor_inventory_capacity(actor.base());}
        std::string rank(Actor& actor){auto& value=human(actor);const auto rank=ui.backend_.hud_human_rank(value,ui.d_.startup.application_flags());if(rank>=8)throw std::out_of_range("Inventory rank exceeds original eight caption rows");const auto text=ui.backend_.hud_localized(0x27a6u+rank);return text?*text:std::string{};}
        std::int32_t raw_stat(Actor& actor,unsigned index){return game::human_hud_signed(human(actor).properties.stats_ec.at(index));}
        long double stat(Actor& actor,unsigned index){return game::human_loading_stat(human(actor),index);}
        bool inventory_changed(Actor& actor){ui.require_actor_borrower(&actor);if(kind(actor)==3)return human(actor).activation.flag178;
            throw std::runtime_error("Inventory frame requires actual non-Human178 producer");}
    };
    struct RollControlQueries:SignalQueries{
        GameUiRollCommands* roll_commands=nullptr;
        void submit_roll(Actor& actor,const game::HudRollCommand& request){if(!roll_commands)throw std::runtime_error("Roll execution requires actual Human B0 virtualD8 producer");roll_commands->submit_action(human(actor),request);}
        void pickup_roll_supply(Actor& actor,Actor& target,std::int32_t row){auto& value=supply(target);if(!roll_commands)throw std::runtime_error("Roll pickup requires actual Human B0 virtual7C producer");roll_commands->pickup_supply(human(actor),value,row);}
        void pickup_roll_item(Actor& actor,Actor& item){ui.require_actor_borrower(&item);if(kind(item)!=11)throw std::logic_error("Roll item pickup requires actual kind11 actor");if(!roll_commands)throw std::runtime_error("Roll pickup requires actual Human B0 virtual6C producer");roll_commands->pickup_item(human(actor),item);}
        void show_roll_inventory(Actor& target){ui.show_inventory(true,&target,roll_commands);}
    };
    struct InventoryControlQueries:InventoryQueries{
        GameUiInventoryCommands* commands=nullptr;
        std::uint32_t game_status(){return ui.d_.session.status;}
        bool uniform_integrity_mismatch(){auto& tables=ui.d_.startup.mission_integrity_tables();return tables.expected(25)!=tables.observed(25);} //Actual8CAB62/8CB262 indices, not invented healthy values.
        auto identity_icon(const void* identity){return ui.d_.world_actors.items().item_icon(item(identity).definition());}
        Entry entry(const void* identity){validate_item(identity);return static_cast<Entry>(const_cast<void*>(identity));}
        void move(Actor& destination,const void* identity,Actor& source,unsigned from,unsigned to,unsigned index){
            auto& receiver=human(destination);ui.require_actor_borrower(&source);const auto selected=entry(identity);
            if(commands){commands->move(receiver,selected,source,from,to,index);return;}
            //Use the actual existing Human command owner for its recovered
            //same-Human loading moves; unsupported queues/transfers still reject there.
            (void)ui.d_.world_actors.template human_loading_owner<world_loading::Composition>(receiver).inventory().move(selected,human(source),from,to,index);
        }
        void swap(Actor& destination,const void* displaced,unsigned to,Actor& source,const void* identity,unsigned from){
            auto& receiver=human(destination);ui.require_actor_borrower(&source);const auto current=entry(displaced),selected=entry(identity);
            if(commands){commands->swap(receiver,current,to,source,selected,from);return;}
            (void)ui.d_.world_actors.template human_loading_owner<world_loading::Composition>(receiver).inventory().swap(current,to,human(source),selected,from);
        }
        void drop(Actor& source,const void* identity,unsigned holder){auto& receiver=human(source);const auto selected=entry(identity);const auto direction=game::human_camera_target_direction(receiver);
            if(!commands)throw std::runtime_error("Inventory drop requires actual Human005A2C50 command/queue producer");commands->drop(receiver,selected,holder,direction);
        }
    };
    struct InventoryPresentationQueries:InventoryQueries{
        GameUiRollCommands* commands=nullptr;
        bool command_visible(){return ui.world_.owners().hud_panels().command().visible();}
        void request_inventory(Actor& actor,std::int32_t identity){if(!commands)throw std::runtime_error("Remote inventory requires actual Human005A45A0 producer");commands->request_inventory(human(actor),identity);}
        void publish_inventory_actors(std::uint32_t flags,Actor* primary,Actor* secondary){ui.world_.owners().hud_panels().inventory().publish_actors(flags,primary,secondary,*this);}
        void hide_inventory_signals(){ui.show_signals(false,nullptr,nullptr);}
        void hide_inventory_roll(){ui.world_.owners().hud_panels().dialogs().hide_roll();}
        void hide_inventory_selector(){ui.show_weapon_selector(false);}
        void hide_inventory_info(){ui.world_.owners().hud_panels().info().hide();}
        void hide_inventory_map(){auto& panels=ui.world_.owners().hud_panels();panels.map().hide(panels.auxiliary());}
        void select_inventory_pointer(std::int32_t kind){ui.select_hud_pointer(kind);}
        void show_inventory_panel(bool enabled){ui.world_.owners().hud_panels().inventory().show_inventory(enabled,*this);}
        void refresh_inventory_compass(){ui.world_.owners().hud_panels().post_bind(ui.d_.startup.application_flags(),ui.parent_.hud()->flags);}
    };
    struct DialogInput:input::BindingInputViews{
        BorrowedGraphicsInput graphics;
        DialogInput(input::BindingInputState& state,BorrowedGraphicsInput graphics,std::uint32_t* first,std::uint32_t* second,std::size_t count)
            :BindingInputViews(state,first,second,count),graphics(graphics){}
        auto next_scan(){return graphics.next_scan();}auto scan_character(std::uint32_t scan){return graphics.scan_character(scan,0);}
        bool key_down(std::uint32_t scan){return graphics.key_down(scan);}
    };
    void retire_dialog_actor(bool command,const void* actor,bool reset){
        require_open();auto& dialog=roll_dialog(command);SignalQueries queries{RollQueries{CommandQueries{TargetQueries{*this}}}};
        queries.orders=signal_orders_;queries.external=signal_external_;
        if(dialog.roll().field88==actor){
            if(command){dialog.roll_target_identity(nullptr);dialog.hide_roll();} //Source lifetime guard for nested590, not native null command-target dereference.
            else dialog.publish_roll_target(static_cast<GameSceneActors::Instance*>(nullptr),0,queries);
        }
        if(reset&&dialog.signals().root)dialog.reset_signal_target(queries);
    }
    struct CommandPolicyQueries{
        GameUiPresentation& ui;GameUiHumanCommandSource& source;
        std::uint32_t* selected_order_flags(){ //004AB7E0: fresh count after each unselected order.
            const auto* identity=ui.world_.owners().hud_panels().auxiliary().info().human;if(!identity)return nullptr;
            auto& human=*ui.find_human_borrower(identity);std::uint32_t index=0;
            while(static_cast<std::int64_t>(index)<source.count(human)){if(source.order(human,index).selected)return &source.flags(human,index);++index;}
            return nullptr;
        }
        auto& state(const void* identity){CommandQueries query{TargetQueries{ui}};auto* actor=query.resolve(identity);return ui.d_.first_players.state(query.human(*actor));}
        std::uint32_t human_movement(const void* identity){return state(identity).words().template get<std::uint32_t>(0x5c);} //005A5C80.
        std::uint32_t human_mind(const void* identity){return state(identity).words().template get<std::uint32_t>(0x50)&0x700u;} //005A5CB0.
        std::uint32_t human_pose(const void* identity){return state(identity).words().template get<std::uint32_t>(0x50)&7u;} //005A5D10.
    };
    struct CommandRouteQueries:SignalQueries{
        GameUiHumanRouteSource& source;
        CommandRouteQueries(GameUiPresentation& value,GameUiHumanRouteSource& producer,GameUiSignalExternalSource* external_source)
            :SignalQueries{RollQueries{CommandQueries{TargetQueries{value}}}},source(producer){orders=&source;external=external_source;}
        bool routes_changed(Actor& actor){return source.changed(human(actor));}
        std::int32_t route_count(Actor& actor){return source.count(human(actor));}
        auto prepare_route(Actor& actor,std::uint32_t index,const std::array<float,4>& from){return source.prepare_route(human(actor),index,from);}
        auto actor_position(Actor& actor){ui.require_actor_borrower(&actor);const auto matrix=ui.d_.scene.frames().actor_world_matrix(actor.base().frame());
            return std::array<float,4>{{matrix[12],matrix[13],matrix[14],matrix[15]}};}
        std::int32_t selected_order_index(){
            auto& auxiliary=ui.world_.owners().hud_panels().auxiliary();if(!auxiliary.info().human)return -1;
            auto* value=ui.find_human_borrower(auxiliary.info().human);OrderQueries queries{ui,*value,source};return auxiliary.selected_order_index(queries);
        }
    };
    struct CommandFrameQueries:CommandRouteQueries{
        GameUiCommandFrameSource& frame_source;
        CommandFrameQueries(GameUiPresentation& value,GameUiCommandFrameSource& producer,GameUiSignalExternalSource* external_source)
            :CommandRouteQueries{value,producer,external_source},frame_source(producer){}
        const void* current_identity(){auto* value=current_player();return value?identity(*value):nullptr;}
        const void* previous_identity(){auto* value=previous_human();return value?identity(*value):nullptr;}
        Actor* null_actor(){return nullptr;}
        bool paused(){return ui.d_.session.words.get<std::uint8_t>(0x64)!=0;}
        std::uint32_t* selected_order_flags(){CommandPolicyQueries queries{ui,source};return queries.selected_order_flags();}
        std::uint32_t human_movement(const void* actor){CommandPolicyQueries queries{ui,source};return queries.human_movement(actor);}
        std::uint32_t human_mind(const void* actor){CommandPolicyQueries queries{ui,source};return queries.human_mind(actor);}
        std::uint32_t human_pose(const void* actor){CommandPolicyQueries queries{ui,source};return queries.human_pose(actor);}
        std::int32_t order_count(const void* actor){return source.count(human(*resolve(actor)));}
        std::uint32_t order_kind(const void* actor,std::uint32_t index){return source.order(human(*resolve(actor)),index).kind;}
        std::int32_t roster_count(){const auto size=ui.d_.players.control().human_roster().size();
            if(size>0x7fffffffu)throw std::length_error("Command roster exceeds native signed range");return static_cast<std::int32_t>(size);}
        Actor* roster_human(std::int32_t index){auto* value=ui.d_.players.control().human_roster().at(index);if(value)(void)human(*value);return value;}
        void present_human(Actor& actor,std::uint32_t index){frame_source.present_command_human(human(actor),index);}
        void advance_detached_camera(std::uint32_t elapsed){frame_source.advance_command_camera(ui.d_.camera.detached(),elapsed);}
        void advance_effects(std::uint32_t elapsed){ui.world_.modules().game_effects().update_loading(elapsed,ui.d_.session,ui.d_.players.control());}
        void publish_soldier_selection(){publish_signal_selection(nullptr,3);}
    };
    struct CommandControlQueries{
        GameUiPresentation& ui;GameUiCommandControls& controls;
        void request_options(){ui.d_.session.status=0xe;} //004B8716 writes SAME game04; no loading activation.
        void request_realtime(bool enabled){controls.request_realtime(ui.world_,enabled);}
    };
    struct CommandInputQueries:RollQueries{
        using Actor=GameSceneActors::Instance;using Human=world_loading::Human;using Loan=GameUiCommandOrderLoan;
        GameUiCommandInputSource& source;
        CommandInputQueries(GameUiPresentation& ui,GameUiCommandInputSource& source,GameUiSignalExternalSource* external)
            :RollQueries{CommandQueries{TargetQueries{ui}}},source(source){this->external=external;}
        struct InfoQueries:OrderQueries{
            GameUiCommandInputSource& input_source;
            void mark_selected(std::uint32_t index,bool selected){validate();input_source.mark_order_selected(human,index,selected);}
            const void* order_identity(std::uint32_t index){validate();const auto loan=input_source.read_order(human,index);if(loan.human!=&human||loan.index!=index||!loan.identity)throw std::logic_error("Command order token does not belong to SAME Human/index");return loan.identity;}
        };
        auto info_queries(){const auto identity=ui.world_.owners().hud_panels().auxiliary().info().human;if(!identity)throw std::logic_error("Command order selection requires actual info Human");
            auto& value=*ui.find_human_borrower(identity);return InfoQueries{OrderQueries{ui,value,source},source};}
        std::int32_t selected_order_index(){auto& aux=ui.world_.owners().hud_panels().auxiliary();if(!aux.info().human)return -1;auto queries=info_queries();return aux.selected_order_index(queries);}
        std::optional<Loan> selected_order(){const auto index=selected_order_index();if(index<0)return std::nullopt;auto queries=info_queries();return order(queries.human,index);}
        void select_order_index(std::int32_t index){auto queries=info_queries();ui.world_.owners().hud_panels().auxiliary().select_order_index(index,queries);}
        void select_order_token(const Loan& loan){if(!loan.human)throw std::logic_error("Command selection lost actual order Human");ui.require_actor_borrower(loan.human);(void)human(*loan.human);auto& aux=ui.world_.owners().hud_panels().auxiliary();
            if(aux.info().human!=loan.human){OrderQueries queries{ui,*loan.human,source};aux.update_order_display(loan.human,queries);}auto queries=info_queries();aux.select_order_token(loan.identity,queries);}
        Loan order(Actor& actor,std::uint32_t index){auto& value=human(actor);const auto loan=source.read_order(value,index);
            if(loan.human!=&value||loan.index!=index||!loan.identity)throw std::logic_error("Command order loan does not belong to actual Human/index");if(loan.target)ui.require_actor_borrower(loan.target);return loan;}
        Loan pick_order(world_loading::Frames::Frame frame){const auto loan=source.pick_order_model(ui.world_,frame);if(!loan.human)throw std::logic_error("Command collision has no actual Human order owner");
            ui.require_actor_borrower(loan.human);(void)human(*loan.human);const auto checked=order(*loan.human,loan.index);if(checked.identity!=loan.identity||checked.model!=frame)throw std::logic_error("Command collision model is absent from actual Human order list");return checked;}
        std::int32_t order_count(Actor& actor){return source.count(human(actor));}
        bool previous_index_valid(){return ui.d_.players.control().previous_index()>=0;}
        Human& receiver(){auto* value=previous_human();if(!value)throw std::logic_error("Command mutation requires actual previous Human");return human(*value);}
        auto actor_position(Actor& actor){ui.require_actor_borrower(&actor);const auto frame=actor.base().frame();if(!frame)throw std::logic_error("Command actor model is retired");const auto m=ui.d_.scene.frames().actor_world_matrix(frame);return game::HudCommandVector{{m[12],m[13],m[14],m[15]}};}
        auto actor_forward(Actor& actor){ui.require_actor_borrower(&actor);const auto frame=actor.base().frame();if(!frame)throw std::logic_error("Command actor model is retired");const auto m=ui.d_.scene.frames().actor_world_matrix(frame);return game::HudCommandVector{{m[8],m[9],m[10],m[11]}};}
        auto actor_base_position(Actor& actor){ui.require_actor_borrower(&actor);return actor.base().world_position();}
        auto order_endpoint(const Loan& loan){if(loan.target)return actor_position(*loan.target);return loan.position;}
        auto order_model_position(const Loan& loan){if(!loan.human)throw std::logic_error("Command order model lost actual Human");ui.require_actor_borrower(loan.human);(void)human(*loan.human);if(!loan.model)throw std::logic_error("Command order model84 is absent");const auto m=ui.d_.scene.frames().actor_world_matrix(loan.model);return game::HudCommandVector{{m[12],m[13],m[14],m[15]}};}
        void select_human(Actor& actor){source.select_command_human(ui.world_,human(actor));}
        void create_order(Actor& receiver,std::uint32_t kind,std::int32_t parameter,const game::HudCommandVector& position,const game::HudCommandVector& direction,const std::array<std::uint32_t,3>& policies,Actor* target,std::uint32_t row,std::int32_t insert){
            if(target)ui.require_actor_borrower(target);source.create_order(human(receiver),kind,parameter,position,direction,policies,target,row,insert);}
        void change_order(std::int32_t index,const game::HudCommandVector& position,const game::HudCommandVector& direction,const std::array<std::uint32_t,3>& policies){source.change_order(receiver(),index,position,direction,policies);}
        void remove_order(std::int32_t index){source.remove_order(receiver(),index);}void clear_orders(){source.clear_orders(receiver());}
        std::uint32_t* selected_order_flags(){CommandPolicyQueries queries{ui,source};return queries.selected_order_flags();}
        std::uint32_t human_movement(const void* actor){CommandPolicyQueries queries{ui,source};return queries.human_movement(actor);}
        std::uint32_t human_mind(const void* actor){CommandPolicyQueries queries{ui,source};return queries.human_mind(actor);}
        std::uint32_t human_pose(const void* actor){CommandPolicyQueries queries{ui,source};return queries.human_pose(actor);}
        struct InputHit{game::HudCommandVector position{},normal{};bool dynamic=false;Actor* actor=nullptr;world_loading::Frames::Frame order_model{};};
        auto screen_ray(std::int32_t x,std::int32_t y){return ui.d_.scene.frames().hud_world_screen_ray(ui.d_.scene.frames().world_scene_frame(),x,y);}
        auto project_world(const game::HudCommandVector& point){return ui.d_.scene.frames().hud_project_world_point(ui.d_.scene.frames().world_scene_frame(),point);}
        std::optional<InputHit> ray(const game::HudCommandVector& start,const game::HudCommandVector& travel,std::uint32_t layers,std::uint32_t flags,bool normal){
            const auto hit=ui.world_.owners().human_services().human_geometry_ray(start,travel,layers,flags,normal);if(!hit)return std::nullopt;InputHit result;result.position=hit->position;result.normal=hit->normal;
            if(const auto* object=hit->dynamic_identity){if(object->flags&0x40u){result.dynamic=true;
                if(object->mask&0x8000u){result.order_model=object->frame;if(!result.order_model)throw std::logic_error("Command order collision lost actual model");}
                else {result.actor=static_cast<Actor*>(object->actor);if(result.actor)ui.require_actor_borrower(result.actor);}}}
            return result;
        }
        std::int32_t display_width(){return game::hud_command_signed(ui.graphics_width());}std::int32_t display_height(){return game::hud_command_signed(ui.graphics_height());}
        void request_options(){ui.d_.session.status=14;}void request_realtime(bool enabled){source.request_realtime(ui.world_,enabled);}
        bool camera_inverted(){return ui.d_.startup.profile_settings().bytes[0x39]!=0;}
        void camera_mode(std::uint32_t kind,const game::HudCommandVector* position){source.camera_mode(ui.world_,ui.d_.camera.detached(),kind,position);}
        void camera_axes(float x,float y){source.camera_axes(ui.d_.camera.detached(),x,y);}
        std::int32_t occupant_count(Actor& actor){if(kind(actor)==20)return source.actor20_occupant_count(actor);return CommandQueries::occupant_count(actor);}
        Actor* occupant(Actor& actor,std::int32_t index){if(index<0||index>=occupant_count(actor))return nullptr;auto* value=kind(actor)==20?source.actor20_occupant(actor,index):TargetQueries::occupant(actor,static_cast<std::size_t>(index));if(value)(void)human(*value);return value;}
        void show_nested_roll(bool enabled,Actor* target){ui.show_command_roll(enabled,target,external);}
        template<class Input>void roll_controls(Input& input){RollControlQueries queries{SignalQueries{RollQueries{CommandQueries{TargetQueries{ui}}}}};queries.external=external;ui.roll_dialog(true).process_roll_controls(input,queries);}
        bool roll_choose(unsigned row){return ui.roll_dialog(true).choose_roll_row(row,*this);}bool roll_row_valid(){return ui.roll_dialog(true).roll_row_valid();}
        std::int32_t roll_row(){return ui.roll_dialog(true).roll().selected84;}std::uint32_t roll_action(unsigned row){return static_cast<std::uint32_t>(ui.roll_dialog(true).roll().rows.at(row).action);}
        Actor* roll_target(){const auto identity=ui.roll_dialog(true).roll().field88;return identity?resolve(identity):nullptr;}
    };
    struct CommandModalQueries:CommandInputQueries{
        GameUiCommandModalSource& modal;
        CommandModalQueries(GameUiPresentation& ui,GameUiCommandModalSource& source,GameUiSignalExternalSource* external):CommandInputQueries{ui,source,external},modal(source){}
        void enable_command_camera(bool enabled){modal.enable_command_camera(ui.world_,ui.d_.camera.detached(),enabled);}
        void prepare_command_sight(){modal.prepare_command_sight(ui.world_);}void retire_command_special_model(){modal.retire_command_special_model(ui.world_);}
        void selection_notifications(bool enabled){ui.d_.players.control().selection().notification_flag()=enabled;}
        void publish_signal_selection(Actor* actor,unsigned mode){SignalQueries queries{RollQueries{CommandQueries{TargetQueries{ui}}}};queries.publish_signal_selection(actor,mode);}
        std::size_t scene_actor_count(){return ui.d_.scene.frames().scene_actor_identity_count();}
        Actor* scene_actor(std::size_t index){auto* value=ui.d_.scene.frames().scene_actor_identity(static_cast<std::int32_t>(index));if(value)ui.require_actor_borrower(value);return value;}
        std::int32_t roster_count(){const auto size=ui.d_.players.control().human_roster().size();if(size>0x7fffffffu)throw std::length_error("Command roster exceeds native signed range");return static_cast<std::int32_t>(size);}
        Actor* roster_human(std::int32_t index){auto* value=ui.d_.players.control().human_roster().at(index);if(value)(void)human(*value);return value;}
        void hide_human(Actor& actor,std::uint32_t duration){ui.world_.owners().human_services().hide_human(human(actor),duration);}
        void show_human(Actor& actor,std::uint32_t duration){ui.world_.owners().human_services().show_human(human(actor),duration);}
        bool script_suspended(Actor& actor){return game::human_script_suspended(human(actor));} //005A5D30, distinct from Human150 visibility.
        Actor* null_actor(){return nullptr;}
    };
    struct CommandPresentationQueries:CommandModalQueries{
        CommandPresentationQueries(GameUiPresentation& ui,GameUiCommandModalSource& source,GameUiSignalExternalSource* external):CommandModalQueries{ui,source,external}{}
        bool command_allowed(){return ui.d_.session.words.get<std::uint8_t>(0x36)!=0;}
        auto& hud_flags(){return ui.parent_.hud()->flags;}auto& saved_flags(){return ui.d_.hud.saved_command_flags();}
        void refresh_layout(){ui.refresh_hud_layout();}void refresh_current_human(){ui.world_.owners().hud_panels().post_bind(ui.d_.startup.application_flags(),hud_flags());}
        bool first_person_enabled(){return ui.d_.first_person.view().enabled()||ui.d_.first_person.view().secondary_enabled();}
        void enable_first_person(bool enabled){ui.d_.first_person.view().enable(ui.d_.startup.application_flags(),enabled);} //0069A4A0→00491FD0.
        void reset_current_player_mode(){auto* actor=current_player();if(!actor)throw std::logic_error("Command FPV transition requires actual current Human");auto& value=human(*actor);
            if(ui.world_.owners().human_services().human_is_player(value)){auto& state=ui.d_.first_players.state(value);auto* player=dynamic_cast<world_loading::Composition::StateFactory::Player*>(&state);
                if(!player)throw std::logic_error("Command Player mode requires actual Player B0");ui.d_.presentation.set_player_mode(*player,value,0,ui.d_.startup.application_flags());}} //005A8190→005469E0.
        void position_timers(const game::HudCommandVector& position){ui.world_.owners().hud_panels().timers().position(position);}
        void show_command_panel(bool enabled){ui.world_.owners().hud_panels().command().show(enabled,*this);}
        bool command_visible(){return ui.world_.owners().hud_panels().command().visible();}
        void show_inventory_panel(bool enabled){ui.show_inventory_panel(enabled);}
        void hide_command_signals(){ui.show_signals(false,nullptr,nullptr);}void hide_command_roll(){ui.world_.owners().hud_panels().dialogs().hide_roll();}
        void hide_command_selector(){ui.show_weapon_selector(false);}void hide_command_info(){ui.world_.owners().hud_panels().info().hide();}
        void hide_command_map(){auto& panels=ui.world_.owners().hud_panels();panels.map().hide(panels.auxiliary());}
        void clear_command_crosshair(){ui.publish_crosshair_target(nullptr,0);}void select_command_pointer(std::int32_t kind){ui.world_.owners().hud_panels().auxiliary().select_pointer(kind);}
    };
    struct SelectorControlQueries{
        GameUiPresentation& ui;GameUiSelectorCommands& commands;using Human=world_loading::Human;using Entry=world_loading::Actors::Items::Entry;
        Human& resolve_human(const void* identity){return *ui.find_human_borrower(identity);}
        bool selector_busy(Human& human){ui.require_actor_borrower(&human);return commands.selector_busy(human);}
        bool action_active(Human& human){return ui.d_.first_players.state(human).words().template get<std::int32_t>(0x60)!=-1;}
        Entry current_weapon(Human& human){return ui.d_.first_players.state(human).item();}
        unsigned item_kind(Entry entry){return ui.d_.world_actors.items().runtime_item(entry).definition().kind();}
        bool focused(Human& human){ui.require_actor_borrower(&human);return human.field174!=0;}
        void use_item(Human& human,Entry entry){(void)ui.d_.world_actors.items().runtime_item(entry);commands.use_selector_item(human,entry);}
        void move_weapon(Human& human,Entry item,unsigned from,unsigned to){(void)ui.d_.world_actors.items().runtime_item(item);commands.move(human,item,human,from,to,0);}
        void swap_weapon(Human& human,Entry current,Entry selected,unsigned slot){auto& items=ui.d_.world_actors.items();(void)items.runtime_item(current);(void)items.runtime_item(selected);commands.swap(human,current,3,human,selected,slot);}
        void hide_selector(){ui.show_weapon_selector(false);}
        void layout_selector(){auto* current=ui.d_.first_players.current_player();if(!current)throw std::logic_error("Selector controls require actual current Human for layout");ui.require_actor_borrower(current);ui.world_.owners().hud_panels().selector().layout(*current,ui.d_.world_actors.items());}
    };
    struct HudControlQueries{
        using Human=world_loading::Human;using Actor=GameSceneActors::Instance;
        GameUiPresentation& ui;const GameUiHudControlSources& sources;std::uint32_t* first;std::uint32_t* second;std::size_t count;
        auto& panels(){return ui.world_.owners().hud_panels();}auto& hud_flags(){return ui.parent_.hud()->flags;}
        template<class T>T& required(T* source,const char* message){if(!source)throw std::runtime_error(message);return *source;}
        auto* orders(){return sources.orders?sources.orders:ui.signal_orders_;}auto* external(){return sources.external?sources.external:ui.signal_external_;}
        auto& players(){return required(sources.players,"HUD controls require actual Players selection/resume/camera producer");}
        bool offline(){return (ui.d_.startup.application_flags()&7u)==7u;}bool paused(){return ui.d_.session.words.get<std::uint8_t>(0x64)!=0;}bool camera_suppressed(){return ui.d_.camera.hud_suppressed();}
        bool visible(world_loading::Frames::Frame frame){if(!frame)throw std::logic_error("HUD controls lost actual required artwork");return ui.d_.scene.frames().hud_frame_visible(frame);}
        bool command_visible(){return panels().command().visible();}bool inventory_visible(){return visible(panels().inventory().root());}
        bool map_visible(){return panels().map().center()&&visible(panels().map().center());}bool info_visible(){return visible(panels().info().root());}bool selector_visible(){return visible(panels().selector().root());}
        bool map_latched(){return ui.d_.hud.control_latches()[1]!=0;}bool info_latched(){return ui.d_.hud.control_latches()[0]!=0;}
        bool signals_active(){return panels().dialogs().signals_active();}bool roll_active(){return panels().dialogs().roll().flag30!=0;}
        std::uint32_t message_mode(){return panels().messages().state().field10;}bool message_editing(){return panels().messages().state().flagd385!=0;}
        void request_realtime(bool enabled){GameUiCommandControls* controls=sources.realtime?sources.realtime:sources.command;required(controls,"HUD controls require actual containing realtime producer").request_realtime(ui.world_,enabled);}
        void set_message_mode(std::uint32_t mode){auto input=ui.backend_.presentation_input();panels().messages().mode(mode,offline(),input,*this);}
        void stop_message_input(){ui.hud_message_input_mode(0);}void say(bool team){ui.hud_message_say(team);}
        void hide_signals(){ui.show_signals(false,nullptr,nullptr,orders(),external());}void hide_roll(){panels().dialogs().hide_roll();}void restore_roll_position(){panels().dialogs().restore_roll_position();}
        void hide_selector(){ui.show_weapon_selector(false);}void hide_info(){panels().info().hide();}void hide_map(){panels().map().hide(panels().auxiliary());}
        void show_info(){ui.show_info(true,sources.info);}void show_map(bool camera){ui.show_map(true,camera,sources.map);}void show_selector(){ui.show_weapon_selector(true);}
        void pointer_kind(std::int32_t kind){panels().auxiliary().select_pointer(kind);}std::uint32_t pointer_hit_tag(){return panels().auxiliary().pointer_hit_tag();}
        void position_timers(bool signals){const auto real=[](std::uint32_t bits){float value;std::memcpy(&value,&bits,4);return value;};panels().timers().position({{real(0x3f7051db),real(signals?0xbe6b851fu:0xbeec29e1u),0,0}});}
        void process_pointer(){ui.process_hud_pointer(first,second,count);}void process_map(){ui.process_map_controls(first,second,count,sources.map);}
        void process_signals(){ui.process_signal_controls(required(sources.signals,"HUD controls require actual signal command producer"),first,second,count,orders(),external());}
        void process_roll(){ui.process_roll_controls(required(sources.roll,"HUD controls require actual roll command producer"),first,second,count,external());}
        void process_selector(){ui.process_weapon_selector(required(sources.selector,"HUD controls require actual Human selector command producer"),first,second,count);}
        void process_inventory(){ui.process_inventory_controls(first,second,count,sources.inventory);}void process_command(){ui.process_command_controls(required(sources.command,"HUD controls require actual command producers"),first,second,count,external());}
        void show_command(bool enabled){ui.show_command(enabled,required(sources.command,"HUD command modal requires actual producers"),external());}
        void show_inventory(bool enabled){ui.show_inventory(enabled,nullptr,sources.roll);}void inventory_panel(bool enabled){ui.show_inventory_panel(enabled);}
        std::int32_t current_index(){return ui.d_.players.control().current_index();}void select_player_index(std::int32_t index){players().select_player_index(ui.world_,index);}
        void select_command_player_index(std::int32_t index){players().select_command_player_index(ui.world_,index);}std::int32_t next_player_index(bool reverse){return players().next_player_index(ui.world_,reverse);}
        void resume_player_control(){players().resume_player_control(ui.world_);}void refresh_current_human(){panels().post_bind(ui.d_.startup.application_flags(),hud_flags());}
        void publish_current_inventory(){auto* current=ui.d_.players.control().current_player();if(current)ui.require_actor_borrower(current);ui.publish_inventory_actors(1,current,nullptr);}
        Human* roster_human(std::int32_t index){auto* human=ui.d_.players.control().human_roster().at(index);if(human)ui.require_actor_borrower(human);return human;}
        Human* selected_human(){auto* value=ui.d_.players.control().selected_player(ui.d_.startup.application_flags());if(value)ui.require_actor_borrower(value);return value;}
        Human* null_human(){return nullptr;}bool is_player(Human& human){ui.require_actor_borrower(&human);return ui.world_.owners().human_services().human_is_player(human);}
        std::int32_t health(Human& human){ui.require_actor_borrower(&human);return game::human_hud_signed(human.properties.stats_ec[0]);}
        Actor* selection_target(){auto* identity=ui.d_.hud.selection_actors()[0];if(!identity)return nullptr;CommandQueries queries{TargetQueries{ui}};return queries.resolve(identity);}
        void show_signals(Human* human){ui.show_signals(true,selection_target(),human,orders(),external());}void show_roll(){ui.show_roll(true,false,external());}
        std::uint32_t ticks(){return timeGetTime();}auto& soldier_click_ticks(){return ui.d_.hud.soldier_click_ticks();}
        void focus_previous_human(){auto* previous=ui.d_.players.control().previous_player();if(previous)ui.require_actor_borrower(previous);players().focus_command_human(ui.world_,ui.d_.camera.detached(),previous);} //004BF210→camera88/00462780.
        void cycle_layout(){ui.cycle_hud_layout();}
    };
    struct ScoreboardQueries{
        GameUiPresentation& ui;
        using Scoreboard=std::remove_reference_t<decltype(std::declval<world_loading::World&>().owners().hud_panels().scoreboard())>;
        auto& configuration(){return ui.d_.startup.game_configuration();}
        std::int32_t remaining_time(){return Scoreboard::remaining_time(configuration().rules228[2],ui.d_.session.words.get<std::uint32_t>(0x30));}
        std::int32_t team_time(){return Scoreboard::team_time(configuration().rules228[5],ui.d_.session.words.get<std::uint32_t>(0x7c));}
        std::int32_t elapsed_time(){return ui.d_.session.words.get<std::int32_t>(0x30);}
        float team_score(std::uint32_t team){return ui.d_.startup.game_control_records().team_scores44c.at(team);}
        world_loading::Human* reserved_human(std::int32_t index){auto* actor=ui.d_.scene.frames().scene_actor_identity(index);if(!actor)return nullptr;
            ui.require_actor_borrower(actor);auto* human=dynamic_cast<world_loading::Human*>(actor);if(!human)throw std::logic_error("Scoreboard reserved slot requires actual Human");return human;}
        std::int32_t health(world_loading::Human& human){return ui.d_.first_players.health(human);}
        bool suppressed(world_loading::Human& human){return ui.d_.presentation.human_selected_suppressed(human);}
        std::int32_t team(world_loading::Human& human){
            ui.require_actor_borrower(&human);return game::human_hud_signed(ui.backend_.hud_human_team(human,ui.d_.startup.application_flags()));
        }
        bool teams_enabled(){return configuration().hud_teams240()!=0;}
        game::HudMissionScore mission_score(){if(configuration().rules228[21]){const auto losses=ui.d_.session.words.get<std::int32_t>(0x84);return {losses,configuration().rules228[18]};}
            return game::hud_local_mission_score(ui.d_.startup.game_control_records(),ui.d_.startup,ui.d_.startup.local_player_identity(),configuration());}
    };
    struct InfoQueries{
        GameUiPresentation& ui;GameUiInfoSource* source;
        auto& network(){if(!source)throw std::runtime_error("HUD info requires actual network peer/mission/address producer");return *source;}
        bool offline(){return (ui.d_.startup.application_flags()&7u)==7u;}
        auto localized(std::uint32_t id){return ui.backend_.hud_localized(id);}
        std::size_t objective_count(){return ui.d_.session.mission?ui.d_.session.mission->related.size():0;}
        frontend::GameDataNode* objective(std::size_t index){if(!ui.d_.session.mission)throw std::logic_error("HUD info lost actual mission descriptor");return ui.d_.session.mission->related.at(index).get();}
        std::uint32_t objective_status(const frontend::GameDataNode& objective){return objective.word10;}
        std::uint32_t objective_caption(const frontend::GameDataNode& objective){return objective.word8;}
        std::uint32_t objective_team_caption(const frontend::GameDataNode& objective,std::int32_t team){return game::hud_objective_team_caption(objective,static_cast<std::uint32_t>(team),[&](const auto& node,std::uint32_t side){return network().objective_team_caption(node,static_cast<std::int32_t>(side));});}
        std::int32_t objective_team(const frontend::GameDataNode& objective){return game::hud_objective_team(objective,[&](const auto& node){return network().objective_team(node);});}
        std::int32_t local_team(){return ui.backend_.hud_local_team();}
        std::string binding_name(std::int32_t action,std::uint32_t mode){const auto index=game::find_hud_help_binding(ui.d_.startup.profile_bindings().settings,action,mode);
            if(!index)throw std::runtime_error("HUD help token requires actual profile binding");return ui.d_.startup.binding_name(*index);}
        auto mission_title(){return game::hud_network_mission_title(ui.d_.session.mission,[&]{return network().mission_title(ui.world_);});}
        std::string network_address(){return network().network_address();}
        std::int32_t time_limit(){return ui.d_.startup.game_configuration().rules228[2];}
        std::int32_t frag_limit(){return ui.d_.startup.game_configuration().rules228[1];}
        std::uint32_t elapsed_bits(){return ui.d_.session.words.get<std::uint32_t>(0x30);}
        std::uint32_t result_status(){return ui.d_.session.words.get<std::uint32_t>(0x88);}
        auto& peers(){return ui.d_.startup.game_control_records();}
        const auto& peer(const void* key){return game::hud_info_peer(peers(),key);}
        void begin_peers(){ui.d_.startup.enter_control_lock(0);}void end_peers()noexcept{ui.d_.startup.leave_control_lock(0);}
        std::size_t peer_count(){return peers().records.size();}
        const void* peer_identity(std::size_t index){return &peers().records.at(index);}
        const void* local_peer_identity(){const auto id=ui.d_.startup.local_player_identity();if(id<0)throw std::out_of_range("HUD info local peer identity");return peer_identity(static_cast<std::size_t>(id));}
        std::string peer_name(const void* key){return peer(key).name();}
        std::int32_t peer_stat(const void* key,std::uint32_t offset){return game::hud_info_peer_stat(peer(key),offset);}
        std::int32_t peer_team(const void* key){return game::human_hud_signed(peer(key).word(0x64));}
        world_loading::Human* peer_human(const void* key){const auto& record=peer(key);if(!record.word(0x3c))return nullptr;
            auto* human=network().peer_human(record);if(!human)throw std::logic_error("Positive peer3C lacks its actual typed Human producer");ui.require_actor_borrower(human);return human;}
        bool peer_unavailable(const void* key){return game::hud_info_peer_unavailable(peer_human(key));}
        std::int32_t health(world_loading::Human& human){ui.require_actor_borrower(&human);return ui.d_.first_players.health(human);}
        std::optional<std::uint32_t> weapon_alignment(world_loading::Human& human){ui.require_actor_borrower(&human);auto& items=ui.d_.world_actors.items();game::FirstPersonHumanItems<std::remove_reference_t<decltype(items)>> view(items);
            const auto entry=view.weapon(human);if(!entry)return std::nullopt;return items.human_entry_alignment(entry);}
        float team_score(std::uint32_t team){return ui.d_.startup.game_control_records().team_scores44c.at(team);}
        std::int32_t team_count(std::uint32_t team){return network().team_count(team);}
    };
    struct MapQueries{
        GameUiPresentation& ui;GameUiMapSource* source;using Human=world_loading::Human;using Actor=GameSceneActors::Instance;using Base=game::ActorBase<world_loading::Frames>;
        auto& external(){if(!source)throw std::runtime_error("HUD map requires actual network/zone/speech producer");return *source;}
        Actor& resolve(Base& base){for(std::size_t n=0;n<ui.d_.scene.frames().scene_actor_identity_count();++n){auto* actor=ui.d_.scene.frames().scene_actor_command_identity(static_cast<std::int32_t>(n));if(actor&&&actor->base()==&base)return *actor;}
            throw std::logic_error("Map actor borrower is absent from SAME physical/pending World registry");}
        void validate(Human& human){for(std::size_t n=0;n<ui.d_.scene.frames().scene_actor_identity_count();++n){auto* actor=ui.d_.scene.frames().scene_actor_command_identity(static_cast<std::int32_t>(n));if(actor&&dynamic_cast<Human*>(actor)==&human)return;}
            throw std::logic_error("Map Human borrower is absent from SAME physical/pending World registry");}
        Human* current_human(){auto* human=ui.d_.players.control().current_player();if(human)validate(*human);return human;}
        std::int32_t roster_count(){const auto size=ui.d_.players.control().human_roster().size();if(size>0x7fffffffu)throw std::length_error("Map roster exceeds native signed count");return static_cast<std::int32_t>(size);}
        Human* roster_human(std::int32_t n){auto* human=ui.d_.players.control().human_roster().at(n);if(human)validate(*human);return human;}
        Human* reserved_human(std::uint32_t n){if(n>0x7fffffffu)throw std::out_of_range("Map Human identity");auto* human=ui.d_.scene.frames().scene_human_identity(static_cast<std::int32_t>(n));if(human)validate(*human);return human;}
        auto position(Human& human){validate(human);return base_position(human.base());}
        game::HudMapVector base_position(Base& base){(void)resolve(base);const auto frame=base.frame();if(!frame)throw std::logic_error("Map actor lost actual model");const auto m=ui.d_.scene.frames().actor_world_matrix(frame);return game::HudMapVector{{m[12],m[13],m[14],m[15]}};}
        auto direction(Human& human){validate(human);return game::human_camera_target_direction(human);}
        std::int32_t health(Human& human){validate(human);return ui.d_.first_players.health(human);}
        bool unavailable(Human& human){validate(human);return ui.world_.owners().human_services().human_unavailable(human);}
        std::int32_t identity(Human& human){validate(human);return human.base().id10();}
        bool player_controlled(Human& human){validate(human);return game::script_human_player_controlled(human,ui.d_.startup.application_flags());}
        bool suppressed(Human& human){validate(human);return ui.d_.presentation.human_selected_suppressed(human);}
        std::int32_t team(Human& human){validate(human);return game::human_hud_signed(ui.backend_.hud_human_team(human,ui.d_.startup.application_flags()));}
        std::vector<Human*> known_enemies(){ //0045ED10 read query over SAME owned selection38 records, reverse.
            std::vector<Human*> result;const auto& records=ui.d_.players.control().selection().records();for(auto n=records.size();n;){const auto& record=*records[--n];
                if(record.references&&record.human&&health(*record.human)>0&&!unavailable(*record.human))result.push_back(record.human);}return result;
        }
        unsigned game_mode(){return ui.d_.startup.selected_game_mode();}
        Human* last_seen_human(){auto* actor=ui.d_.scene.frames().single_selected_human();if(!actor)return nullptr;
            for(std::size_t n=0;n<ui.d_.scene.frames().scene_actor_identity_count();++n)if(ui.d_.scene.frames().scene_actor_command_identity(static_cast<std::int32_t>(n))==actor){
                auto* human=dynamic_cast<Human*>(actor);if(!human)throw std::logic_error("Map single-Human summary resolves to another actor");return human;}
            throw std::logic_error("Map single-Human summary is absent from SAME physical/pending World registry");}
        std::int32_t vip_identity(){ //0069E070, actual game71/78 and command identity (including reserved pending).
            if(!ui.d_.session.words.get<std::uint8_t>(0x71))return -1;const auto id=ui.d_.session.words.get<std::int32_t>(0x78);
            return id!=-1&&ui.d_.scene.frames().scene_actor_command_identity(id)?id:-1;
        }
        auto camera_matrix(){const auto frame=ui.d_.scene.frames().active_game_camera();if(!frame)throw std::logic_error("Map camera requires SAME scene active camera");return ui.d_.scene.frames().actor_world_matrix(frame);}
        auto camera_position(){const auto m=camera_matrix();return game::HudMapVector{{m[12],m[13],m[14],m[15]}};}
        auto camera_direction(){const auto m=camera_matrix();return game::HudMapVector{{m[8],m[9],m[10],m[11]}};}
        std::uint32_t display_width(){return ui.graphics_width();}std::int32_t display_height(){const auto bits=ui.graphics_height();std::int32_t n;std::memcpy(&n,&bits,4);return n;}
        bool has_speech(Human& human){validate(human);return human.activation.flag214;}
        std::string speech(Human& human){validate(human);return external().human_speech(human);}
        std::int32_t speech_timer(Human& human){validate(human);const auto bits=human.activation.field254;std::int32_t n;std::memcpy(&n,&bits,4);return n;}
        auto marked_items(){return ui.d_.world_actors.item_models().kind20_items();} //Snapshot only; SAME actual kind20 index.
        auto marked_item_position(const void* identity){auto& item=ui.d_.world_actors.items().runtime_item_identity(identity);if(item.holder_actor)return base_position(*item.holder_actor);
            const auto frame=item.model(0);if(!frame)throw std::logic_error("Map item has no actual primary model");const auto m=ui.d_.scene.frames().actor_world_matrix(frame);return game::HudMapVector{{m[12],m[13],m[14],m[15]}};}
        auto* spawn_record(){auto& squad=ui.parent_.live_squad();return squad.empty()?nullptr:&squad.front();}
        std::int32_t& spawn_identity(frontend::PreMissionSoldier& record){return record.network_identity;} //Native04, NOT profile Human identity.
        std::int32_t spawn_team(frontend::PreMissionSoldier& record){return record.specialization;} //Native40;2 spectator.
        std::int32_t actor_identity(Base& base){(void)resolve(base);return base.id10();}
        std::int32_t zone_state(Base& base){auto& actor=resolve(base);if(const auto* zone=game::actor_zone_presentation(actor))return zone->hud_zone_state();return external().zone_state(base);}
        void validate_objective(const frontend::GameDataNode& objective){if(ui.d_.session.mission)for(const auto& owner:ui.d_.session.mission->related)if(owner.get()==&objective)return;throw std::logic_error("Map objective borrower is absent from SAME mission");}
        std::uint32_t objective_state(const frontend::GameDataNode& objective){validate_objective(objective);return objective.word10;}
        std::uint32_t objective_caption(const frontend::GameDataNode& objective,std::uint32_t team){validate_objective(objective);return game::hud_objective_team_caption(objective,team,[&](const auto& node,std::uint32_t side){return external().objective_caption(node,side);});}
        std::int32_t objective_team(const frontend::GameDataNode& objective){validate_objective(objective);return game::hud_objective_team(objective,[&](const auto& node){return external().objective_team(node);});}
        std::int32_t local_team(){return ui.backend_.hud_local_team();}
    };
    struct VisibilityQueries{
        GameUiPresentation& ui;using Human=world_loading::Human;using Actor=GameSceneActors::Instance;
        void clear_target(){ui.publish_crosshair_target(nullptr,1);}
        bool command_visible(){return ui.world_.owners().hud_panels().command().visible();}
        bool camera_suppressed(){return ui.d_.camera.hud_suppressed();}
        auto* current_player(){auto* human=ui.d_.first_players.current_player();if(human)(void)ui.d_.first_players.inline_control(*human);return human;}
        Actor* attachment(Human& human){auto* actor=human.field160;if(actor)ui.require_actor_borrower(actor);return actor;}
        std::uint32_t kind(Actor& actor){ui.require_actor_borrower(&actor);return actor.base().type();}
        bool first_person_enabled(){return ui.d_.first_person.view().enabled()||ui.d_.first_person.view().secondary_enabled();} //0069A4B0, actual58 OR59.
        std::uint32_t seat_flags(Human& human,Actor& actor){return ui.d_.first_players.seat_presentation().flags(actor,human.field164);}
        bool seat_hides_cursor(Human& human,Actor& actor){return ui.d_.first_players.seat_presentation().hides_cursor(actor,human.field164);}
        bool weapon_suppresses(Human& human){
            game::FirstPersonHumanItems<world_loading::Actors::Items> held(ui.d_.world_actors.items());
            const auto weapon=held.weapon(human);if(!weapon)return false;
            const auto& definition=held.definition(weapon);
            if(definition.kind()==4){const auto& payload=definition.payloads[1];
                if(payload.type==4){if(payload.bytes.size()<12)throw std::runtime_error("Crosshair cursor payload lacks native0C scalar");
                    const auto kind=frontend::pre_mission_detail::Bytes::integer(frontend::pre_mission_detail::Bytes::word(payload.bytes.data()+8));if(kind<4)return true;
                }
            }
            return definition.kind()==6;
        }
        std::uint32_t player_mode(Human& human){return ui.d_.first_players.player_mode(human,ui.d_.startup.application_flags());}
    };
    struct ItemIcons{world_loading::Actors::Items& items;auto get(const frontend::PreMissionItem& definition){return items.item_icon(definition);}};
    void require_open(){
        if(!parent_.constructed())throw std::logic_error("UI parent SessionOwner is retired");
        (void)world_.owners().dependencies();
    }
    world_loading::World& world_;world_loading::Dependencies& d_;
    world_loading::World::Parent& parent_;GameOwnerBackend& backend_;Graphics& graphics_;GameEffectsFadeRender fade_;
    GameUiHumanOrderSource* signal_orders_=nullptr;GameUiSignalExternalSource* signal_external_=nullptr;
};
//The pre-scene volume/procedural pipeline and debug geometry remain P22-RENDER.
//A caller must supply the actual receiver; no empty successful service is made.
class GameUiRenderStages {
public:
    virtual ~GameUiRenderStages()=default;
    virtual void render_before_scene()=0;
    virtual void render_debug_geometry()=0;
};
class GameUiSceneCallbacks final:public GameSceneCallbacks {
public:
    GameUiSceneCallbacks(GameUiPresentation& ui,GameUiRenderStages& render)
        :ui_(ui),render_(render){}
    std::uint32_t dispatch_game_scene(std::uint32_t stage)override{return game::scene_callback(stage,*this);}
    bool has_game()const{return ui_.has_game();}auto game_status()const{return ui_.game_status();}
    auto game_word24()const{return ui_.game_word24();}
    bool suppress_hud()const{return ui_.suppress_hud();}bool full_viewport()const{return ui_.full_viewport();}
    bool debug_enabled()const{return ui_.debug_enabled();}bool hud_enabled(){return ui_.hud_enabled();}
    void before_hud(){ui_.before_hud();}void render_hud(){ui_.render_hud();}
    auto graphics_viewport(){return ui_.graphics_viewport();}void graphics_viewport(const CursorViewport& value){ui_.graphics_viewport(value);}
    auto graphics_width(){return ui_.graphics_width();}auto graphics_height(){return ui_.graphics_height();}
    void render_mission_text(){ui_.render_mission_text();}void render_game_fade(){ui_.render_game_fade();}
    void render_effects_overlay(){ui_.render_effects_overlay();}
    void control_before_scene(){render_.render_before_scene();}
    void control_after_scene(){ui_.control_after_scene();}void render_debug(){render_.render_debug_geometry();}
    void render_network_overlay()noexcept {} //00724CB0 is an actual RET in the frozen client.
private:
    GameUiPresentation& ui_;GameUiRenderStages& render_;
};
}
#endif
