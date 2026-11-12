#ifndef HD2_HOST_GAME_WORLD_LOADING_H
#define HD2_HOST_GAME_WORLD_LOADING_H

#include "host/game_hud_loading.h"
#include "game/hud_world_loading.h"
#include "host/game_car_callbacks.h"
#include "game/offline_spawn_presentation.h"
#include "game/world_actor_loading_consumers.h"
#include "game/scene_actor_loading_runtime.h"
#include "game/car_definition_tables.h"
#include "host/game_loading_modules.h"
#include "game/mission_auxiliary_module_loading.h"
#include "host/game_owner.h"
#include "host/game_loading_queries.h"
#include "game/shot_manager_loading.h"
#include "game/mission_effects_loading.h"
#include "game/particle_module_loading.h"
#include "game/version_module_loading.h"
#include "game/version_debris.h"
#include "game/effects_module_loading.h"
#include "host/game_world_actors.h"
#include "host/game_human_animation_events.h"
#include "host/game_human_sound_loading.h"
#include "host/game_actor_proximity_loading.h"
#include "game/human_loading_owner_factory.h"
#include "game/players_module_loading.h"
#include "game/human_presentation_shared.h"
#include "game/mission_subtitles_loading.h"
#include "game/human_world_loading_services.h"
#include "game/human_loading_presentation.h"
#include "game/human_script_context_services.h"
#include "game/human_dubbing_loading.h"
#include "game/human_animation_world_bindings.h"
#include "game/world_sound_loading.h"
#include "game/first_person_loading_composition.h"
#include "game/first_person_players_loading.h"
#include "game/human_player_mode_loading.h"
#include "game/game_music_references.h"
#include "game/mission_script_loading_runtime.h"
#include "game/mission_script_world_loading.h"
#include "game/mission_script_actor_bindings.h"
#include "game/mission_script_player_predicates.h"
#include "game/mission_script_model_visibility.h"
#include "game/mission_scene_activation.h"
#include "game/human_objective_experience.h"
#include "game/collision_sweep.h"
#include "game/human_input_default_receiver.h"
#include "host/game_occupied_actor_presentation.h"

namespace hd2::host::world_loading {
struct Dependencies;struct PresentationDependencies;
using Services=game::HumanWorldLoadingServices<Dependencies>;
using Presentation=game::HumanLoadingPresentation<PresentationDependencies>;
using Scene=GameWorldScene<GameOwnerBackend>;using Frames=GameActorServices;
using Human=GameSceneActors::Human;
using Control=game::HumanPlayerControlLoading<GameSceneActors,Human,Presentation>;
using Camera=game::MissionCamera<Frames,Human>;
using Consumers=game::WorldActorLoadingConsumers<Dependencies>;
using Actors=GameWorldActors<GameOwnerBackend,Consumers>;
using Inputs=StartupSession::HumanInputs;
struct Sound;
using Subtitles=game::MissionSubtitlesLoading<GameOwnerBackend::DisplayText>;
using Dubbing=game::HumanDubbingLoading<Human,Control,Subtitles>;
using AnimationActors=game::HumanAnimationWorldBindings<GameSceneActors,Services>;
using AnimationSound=game::HumanAnimationWorldSound<Sound,StartupSession,Dubbing>;
using Events=GameHumanAnimationEvents<AnimationActors,AnimationSound>;
using Sweep=game::CollisionSweep<EngineFrame*,Scene::Descriptor,GameCollisionResources,Frames>;
using InlineFactory=game::HumanInlineLoadingFactory<Frames,Human,Scene::Buckets,GameHumanAudioLoading,GameHumanAudioLoading::Random,Events,game::EffectsShadowModels<Frames>,Presentation,Services,Actors::Items,GameAnimations,GameCollisionResources,Sweep>;
using ScriptLink=game::MissionScriptWorldLink<Frames>;
using Contexts=game::HumanScriptContextServices<Frames,Human,ScriptLink,ScriptLink,GameHumanAudioLoading::Random,Services>;

using Group=game::PlayersSubgroup;
using Active=game::HumanActiveAIStateLoading<Frames,Human,Inputs,Services>;
using Players=game::PlayersModuleLoading<GameSceneActors,Human,Presentation,Active,Services,Group>;
using Sight=GameSightLoading<Services>;
struct Hearing;
using Composition=game::HumanLoadingComposition<InlineFactory,Inputs,Contexts,frontend::HumanStore<HumanPortraits>,GameHumanAudioLoading,Sight::Source,Hearing,GameActorProximityLoading,Camera,Control>;
using Shared=game::HumanPresentationShared<Frames,Services>;
using HudWorld=game::HudWorldLoading<Scene::Frames,game::MissionTables<GameOwnerBackend>,StartupSession>;
using Panels=game::HudPanelsLoading<GameSceneActors,GameOwnerBackend,GameOwnerBackend::Hud,Control,Actors::Items,frontend::OpeningTextStyles,Camera,HudWorld>;
using Hud=game::HudSceneLoading<GameSceneActors,GameOwnerBackend,GameOwnerBackend::Hud,Panels>;
using FirstPlayers=game::FirstPersonPlayersLoading<Human,Control,Actors,Composition,GameSceneActors>;
using SeatPresentation=GameOccupiedActorPresentation<Dependencies,Frames,Actors>;
using Cursor=game::HudCursorLoading<GameSceneActors,Control,Actors::Items>;
struct FirstPerson:game::FirstPersonLoadingComposition<GameSceneActors,game::SessionOwner<GameOwnerBackend>::Overlay,Actors::Items,FirstPlayers,Camera,GameAnimationResources,AnimationSound,Cursor,game::EffectsShadowModels<Frames>>{using FirstPersonLoadingComposition::FirstPersonLoadingComposition;};
using PlayerModes=game::HumanPlayerModeLoading<Actors::Items,FirstPerson::View,FirstPlayers>;
struct PresentationDependencies{Shared& shared;const std::uint32_t& mode;StartupSession& startup;const std::int32_t& local_player;unsigned human_difficulty(){return startup.human_difficulty();}PlayerModes& player_modes;FirstPerson& fpv;Camera& camera;Players& players;FirstPlayers& first_person_players;Hud& hud;};
using Music=game::GameMusicReferences;
struct Sound:game::WorldSoundLoading<GameSceneActors,Actors,Composition,Camera,StartupSession>{using WorldSoundLoading::WorldSoundLoading;};
using Router=game::HumanSoundStateRouter<Frames,Human,Composition::StateFactory,Services>;
struct Hearing:GameHumanSoundLoading<Services,Router,StartupSession>{using GameHumanSoundLoading::GameHumanSoundLoading;};

using OwnerFactory=game::HumanLoadingOwnerFactory<Composition,Dependencies,Services>;
using SpawnPresentation=game::SessionSpawnPresentation<Human,Services>;
using SpawnRecords=game::PreparedSpawnPackets<StartupSession,GameSceneActors,SpawnPresentation>;
using SpawnDecoder=game::SessionSpawnPacketDecoder<SpawnRecords,SpawnPresentation>;
struct Dependencies{using Frame=EngineFrame*;using ActorFrames=Frames;using Composition=Composition;StartupSession& startup;const game::MissionRequest& request;game::SessionActivationState& session;Scene& scene;Actors& world_actors;Players& players;FirstPerson& first_person;game::HumanFaceResources<Frames>& faces;GameAnimations& animations;Shared& presentation_shared;Music& music;Sound& sound;Dubbing& dubbing;game::MissionSubtitlesLoading<GameOwnerBackend::DisplayText>& subtitles;Hud& hud;FirstPlayers& first_players;
 GameHumanAudioLoading& audio;Events& animation_events;Presentation& presentation;const frontend::PreMissionItemCatalogue& catalogue;Sweep& sweep;Inputs& inputs;game::MissionTableSet& second_tables;game::MissionNavigation& navigation;game::HumanInputCallbacks& input_callbacks;Contexts& contexts;frontend::HumanStore<HumanPortraits>& profiles;Sight& sight;Hearing& hearing;GameActorProximityLoading& proximity;Camera& camera;OwnerFactory& human_factory;ScriptLink& script_link;game::CarLoadingResources<Frames>& car_resources;game::CarDefinitionTables& car_definitions;game::SessionBroadcastPackets& broadcast_packets;SpawnRecords& spawn_records;SpawnDecoder& spawn_decoder;FrameClockPlatform& clock;game::MissionWorldBounds& bounds;};
using Scripts=game::MissionScriptWorldLoading<Composition,ChunkFiles,Services,Players,FirstPlayers,Sight::Source,Hearing,FirstPerson::View,Camera,game::EffectsShadowModels<Frames>,GameHumanAudioLoading::Random,Hud,Scene::Buckets,FrameClockPlatform>;


//Cold Scene2 callbacks exist before the collision tree/Actors. Only the actual
//process mode is callable then; all later dependencies remain checked-unbound.
//This owner must outlive Scene2 AND Owners, including failed parser teardown.
class Bindings {
    StartupSession& startup_;
public:
    explicit Bindings(StartupSession& startup)
        :startup_(startup),consumers(startup.application_flags_owner()),runtime(consumers),human_factory(services){}
    Bindings(const Bindings&)=delete;Bindings& operator=(const Bindings&)=delete;
    Bindings& require_unbound(Scene& scene,game::SessionOwner<GameOwnerBackend>& parent,StartupSession& startup){
        if(&startup_!=&startup)throw std::logic_error("Containing world changed its cold process owner");
        if(consumers.dependencies_bound()||services.dependencies_bound()||presentation.dependencies_bound()||human_factory.dependencies_bound())
            throw std::logic_error("Containing world bindings already belong to a completed owner");
        if(&scene.frames().actor_runtime()!=&runtime||scene.frames().world_scene_frame()!=parent.scene()->frame()||&scene.frames().mission_tables()!=parent.tables())
            throw std::logic_error("Containing world must borrow SAME cold Scene2 runtime and parent engine scene");
        return *this;
    }
    Consumers consumers;game::SceneActorLoadingRuntime<Frames,Consumers> runtime;
    Services services;Presentation presentation;OwnerFactory human_factory;
};

//Real mutually borrowing cold owner graph before Scene2/collision loading. No reference
//targets unconstructed storage: the aggregates are built LAST and bound only
//after every target exists. This is construction, not module/session activation.
//The containing ordered loader retains the parent SessionOwner/Scene/Bindings,
//vertices until this owner has retired. Items read SAME session.words28 live.
class Owners {
public:
    using Parent=game::SessionOwner<GameOwnerBackend>;
    Owners(Bindings& bindings,Scene& scene,Parent& parent,GameOwnerBackend& backend,
           StartupSession& startup,game::SessionActivationState& session,
           Actors::Vertices& vertices,
           FrameClockPlatform& clock,
           const frontend::PreMissionItemCatalogue& catalogue,const std::string& suffix,
           game::CarDefinitionTables& definitions,const game::CarMaterialRegistry& materials)
        :bindings_(bindings.require_unbound(scene,parent,startup)),startup_(startup),scene_(scene),mode_(startup.application_flags_owner()),navigation_(parent.tables()->sets()[0]),
         actors_(scene,bindings.consumers,navigation_,vertices,startup.text(),catalogue,suffix,startup.pre_mission_files(),clock,game::SessionTimeWord(session)),
         camera_(scene.frames()),inputs_(startup.prepare_game_human_inputs()),input_callbacks_(startup.game_human_input_callbacks()),audio_(startup,scene.frames()),
         faces_(scene.frames()),animation_resources_(startup.loaded_resources()),shared_(scene.frames(),bindings.services,startup.human_projection_process()),
         music_(Music::LiveTracks{scene.documents().fields().field_c8,scene.documents().fields().field_cc}),
         players_(startup.game_module_registry(),scene.frames(),bindings.presentation,startup.players_process(),bindings.services,startup.game_module_registration(11)),
         subtitles_(*parent.text()),dubbing_(players_.control(),subtitles_),sound_(scene.frames(),actors_,camera_,startup),
         animation_actors_(scene.frames(),bindings.services,startup.human_animation_time_scale()),
         animation_sound_(sound_,startup,dubbing_),events_(scene.frames(),animation_actors_,animation_sound_),
         sweep_(scene.collision_resources(),scene.frames()),
         script_link_(scene.frames()),contexts_(scene.frames(),script_link_,script_link_,audio_.random_source(),bindings.services),
         sight_(scene.frames(),bindings.services,startup.game_sight_work_clock()),router_(scene.frames(),bindings.services),
         hearing_(scene.frames(),sight_,bindings.services,router_,startup),proximity_(scene.frames(),startup),
         first_players_(players_.control(),actors_,scene.frames(),session,startup.local_player_identity()),
         hud_world_(scene.frames(),session,*parent.tables(),startup),
         hud_(scene.frames(),backend,startup,*parent.hud(),players_.control(),actors_.items(),camera_,hud_world_),
         first_person_(scene.frames(),*parent.overlays(),actors_.items(),first_players_,camera_,animation_resources_,animation_sound_,hud_.panels().cursor(),scene.effects(),catalogue,scene.documents().fields().field_of_view),
         player_modes_(actors_.items(),first_person_.view(),first_players_,session,startup.mission_integrity_tables(),startup.first_person_preference(),startup.game_configuration(),scene.documents().fields().field_of_view,startup.local_player_identity()),
         car_random_([&startup]{return static_cast<int>(startup.random_word());}),
         car_resources_{scene.car_callbacks().car_tyres(),materials,startup.car_sound_receivers(),startup.car_light_receivers(),scene.car_callbacks().car_particle_pool(),startup.car_seat_camera_shared(),car_random_},
         packets_(startup.game_broadcast_packets()),
         spawn_presentation_(bindings.services),
         spawn_records_(startup.game_control_records(),packets_,startup,scene.frames(),spawn_presentation_,mode_,session),
         spawn_decoder_(spawn_records_,spawn_presentation_,mode_),
         presentation_dependencies_{shared_,mode_,startup,startup.local_player_identity(),player_modes_,first_person_,camera_,players_,first_players_,hud_.scene()},
         dependencies_{startup,parent.request(),session,scene,actors_,players_,first_person_,faces_,startup.game_animations(),shared_,music_,sound_,dubbing_,subtitles_,hud_.scene(),first_players_,
            audio_,events_,bindings.presentation,catalogue,sweep_,inputs_,parent.tables()->sets()[1],navigation_,input_callbacks_,contexts_,startup.pre_mission_humans(),sight_,hearing_,proximity_,camera_,bindings.human_factory,script_link_,car_resources_,definitions,packets_,spawn_records_,spawn_decoder_,clock,bounds_} {
        try {
            bindings_.services.bind_dependencies(dependencies_);
            bindings_.presentation.bind_dependencies(presentation_dependencies_);
            bindings_.human_factory.bind_dependencies(dependencies_);
            bindings_.consumers.bind_dependencies(dependencies_);
            occupied_presentation_.bind_dependencies(dependencies_);
            first_players_.bind_seat_presentation(occupied_presentation_);
            auto* callbacks=dynamic_cast<GameCarCallbacks*>(&scene.car_callbacks());
            if(!callbacks)throw std::logic_error("Containing world requires its actual Car callback owner");
            actors_.bind_car_physics(*callbacks);
            scripts_=std::make_unique<Scripts>(startup.game_module_registry(),startup.game_module_registration(4),
                scene.frames(),startup.pre_mission_files(),bindings.services,players_,first_players_,sight_.source(),hearing_,
                first_person_.view(),camera_,scene.effects(),audio_.random_source(),hud_.scene(),scene.buckets(),clock,session,
                startup.mission_integrity_tables(),startup.mission_script_variables(),packets_,mode_,startup.script_advance_override(),
                parent.request().preparation.mission_resource,script_link_);
        }catch(...){unbind();throw;}
    }
    ~Owners(){close();}
    Owners(const Owners&)=delete;Owners& operator=(const Owners&)=delete;
    void bind_loaded_collisions(){require_open();sweep_.bind(scene_.collision_tree(),scene_.buckets());}
    void unbind_loaded_collisions(){require_open();sweep_.unbind(scene_.collision_tree());}
    bool session_loaded()const noexcept{return dependencies_.session.loaded;}
    Dependencies& dependencies(){require_open();return dependencies_;}
    auto& presentation_dependencies(){require_open();return presentation_dependencies_;}
    Services& human_services(){require_open();return bindings_.services;}
    Scripts& scripts(){require_open();return *scripts_;}
    auto& spawn_records(){require_open();return spawn_records_;}
    auto& input_profiles(){require_open();return inputs_.profiles();}
    auto& hud_panels(){require_open();return hud_.panels();} //SAME inline panel owners; no second HUD.
    void close()noexcept {
        if(closed_)return;closed_=true;
        //The ordered loader must first run native module finish passes and VM
        //finish. Remaining camera/FPV/audio/Human borrowers retire while their
        //actual dependency aggregates and process/scene owners still live.
        frontend::CleanupErrors errors;
        scripts_.reset(); //Checked frame/EC guards cover Humans already retired by native module finish.
        errors.attempt([&]{first_person_.close(mode_);});
        errors.attempt([&]{camera_.clear();});
        errors.attempt([&]{startup_.clear_game_dubbing();});
        actors_.close(); //Retracts Human lifetimes BEFORE binding aggregates.
        errors.attempt([&]{hud_.close();});
        shared_.clear(); //Input records/embedded30 belong to process14; actual B0 retirement detaches its own borrowers.
        if(errors.failed())try{errors.rethrow();}catch(...){scene_.frames().record_actor_cleanup_failure(std::current_exception());}
        unbind();
    }
private:
    void require_open()const{if(closed_)throw std::logic_error("Containing world owners are retired");}
    void unbind()noexcept{
        first_players_.unbind_seat_presentation(occupied_presentation_);
        occupied_presentation_.unbind_dependencies(dependencies_);
        bindings_.consumers.unbind_dependencies(dependencies_);
        bindings_.human_factory.unbind_dependencies(dependencies_);
        bindings_.presentation.unbind_dependencies(presentation_dependencies_);
        bindings_.services.unbind_dependencies(dependencies_);
    }
    Bindings& bindings_;StartupSession& startup_;Scene& scene_;std::uint32_t& mode_;
    game::MissionNavigation navigation_;Actors actors_;Camera camera_;Inputs& inputs_;game::HumanInputCallbacks& input_callbacks_;
    GameHumanAudioLoading audio_;game::HumanFaceResources<Frames> faces_;GameAnimationResources animation_resources_;
    Shared shared_;Music music_;Players players_;Subtitles subtitles_;Dubbing dubbing_;Sound sound_;
    AnimationActors animation_actors_;AnimationSound animation_sound_;Events events_;Sweep sweep_;
    ScriptLink script_link_;Contexts contexts_;Sight sight_;Router router_;Hearing hearing_;GameActorProximityLoading proximity_;
    SeatPresentation occupied_presentation_;FirstPlayers first_players_;HudWorld hud_world_;GameHudLoading<Control,Actors::Items,Camera,HudWorld> hud_;
    FirstPerson first_person_;PlayerModes player_modes_;std::function<int()> car_random_;game::CarLoadingResources<Frames> car_resources_;
    game::SessionBroadcastPackets& packets_;SpawnPresentation spawn_presentation_;SpawnRecords spawn_records_;SpawnDecoder spawn_decoder_;
    game::MissionWorldBounds bounds_;
    PresentationDependencies presentation_dependencies_;Dependencies dependencies_;std::unique_ptr<Scripts> scripts_;bool closed_=false;
};

using Effects=game::EffectsModuleLoading<GameSceneActors,ChunkFiles,GameHumanAudioLoading::Random,Camera,FirstPerson::View,game::EffectsShadowModels<Frames>,StartupSession>;
using Versions=game::VersionModuleLoading<Frames,Scene::Buckets>;
using MissionEffects=game::MissionEffectsLoading<ChunkFiles,Frames>;
using Particles=game::ParticleModuleLoading<ChunkFiles,Frames>;
struct ModuleDependencies {
    StartupSession& startup;const game::MissionRequest& request;Scene& scene;Actors& world_actors;
    game::SessionActivationState& session;Services& human_services;Shared& presentation_shared;
    Scripts& scripts;Sight& sight;Players& players;Camera& camera;
    game::MissionTracksLoading& tracks;Effects& game_effects;Versions& versions;
    game::MissionLightmapGroupsLoading<Frames>& lightmap_groups;game::MissionTextureShiftLoading& texture_shifts;
    Actors::Vertices& vertices;game::ShotManagerLoading<Frames>& shots;game::HeavyFireModuleLoading& heavy_fire;Particles& particles;
};

//All real registered module facades for this same world. Retire native module
//passes and this graph BEFORE Owners; it borrows Actors/camera/FPV/script owners.
//Cold construction permits the real loading tail even before a tree exists;
//required geometry queries still reject until actual tree/grid binding.
class ModuleOwners {
public:
    ModuleOwners(Owners& owners,Actors::Vertices& vertices)
        :d_(owners.dependencies()),mission_effects_(d_.startup.pre_mission_files(),d_.scene.frames(),d_.startup.mission_effects_process()),
         tracks_(registry(),registration(0)),
         effects_(registry(),registration(1),d_.scene.frames(),d_.startup.pre_mission_files(),d_.scene.effects(),d_.startup.effects_loading_process(),
             d_.audio.random_source(),d_.camera,d_.first_person.view(),d_.startup,d_.startup.application_flags_owner(),d_.request.preparation.mission_resource,d_.scene.documents().fields().far_clip),
         versions_(registry(),registration(7),d_.scene.frames(),d_.scene.buckets(),d_.startup.application_flags_owner()),
         lightmaps_(registry(),registration(8),d_.scene.frames()),shifts_(registry(),registration(10)),
         shots_(registry(),registration(14),d_.scene.frames(),d_.startup.shot_manager_process(),d_.startup.application_flags_owner()),
         heavy_fire_(registry(),registration(16),d_.startup.heavy_fire_process()),particles_(registry(),registration(17),mission_effects_,
             [this]{return d_.session.words.template get<std::uint8_t>(0x64)!=0;},
             [&owners](const game::HumanVector& position){return static_cast<float>(owners.human_services().human_scene_light(position,Frames::Frame{},true));}),
         dependencies_{d_.startup,d_.request,d_.scene,d_.world_actors,d_.session,owners.human_services(),d_.presentation_shared,owners.scripts(),d_.sight,d_.players,d_.camera,
             tracks_,effects_,versions_,lightmaps_,shifts_,vertices,shots_,heavy_fire_,particles_},dispatch_(dependencies_){
        typename Versions::TransitionServices services;
        services.script=[this](Frames::Frame frame,bool hit){
            const auto* data=d_.scene.frames().find_actor_frame_data(frame);if(!data||!data->words[1])return;
            auto* context=dependencies_.scripts.module().contexts().context_for_frame(frame);
            if(!context)throw std::logic_error("Version script metadata has no actual containing context");
            if(hit)context->hit((d_.startup.application_flags()&1u)!=0);
            else context->death(dependencies_.scripts.runtime()); //0041D920; no gameplay-save suppression producer is exposed here.
        };
        services.particle=[this](Frames::Frame frame,std::int32_t id){
            if(frame)(void)d_.startup.mission_effects_process().emit(static_cast<std::uint32_t>(id),d_.scene.frames().actor_world_matrix(frame),Frames::Frame{},true,false);
        };
        services.sound=[this](Frames::Frame frame,std::int32_t id){
            if(id==-1||!frame)return;
            frontend::SpatialSoundRequest request;const auto matrix=d_.scene.frames().actor_world_matrix(frame);
            request.position={{matrix[12],matrix[13],matrix[14],matrix[15]}};request.group=7;request.flags=1;request.entry=id;(void)d_.sound.emit(request);
        };
        services.debris=[this](Frames::Frame source,Frames::Frame group,std::uint32_t material,Frames::Frame master,bool effects){
            struct Random {StartupSession& startup;std::int32_t fowl_random_word(){return static_cast<std::int32_t>(startup.random_word());}} random{d_.startup};
            game::emit_version_debris(d_.scene.frames(),d_.world_actors.physics(),random,source,group,material,master,effects);
        };
        services.publish=[this](std::uint32_t master,std::uint32_t part,std::uint32_t stage){
            auto& packet=d_.broadcast_packets.begin(7,0);packet.write(master,16);packet.write(part,8);packet.write(stage,8);packet.finish_broadcast();
        };
        versions_.bind_transition_services(std::move(services));
        car_versions_=dynamic_cast<GameCarCallbacks*>(&d_.scene.car_callbacks());
        if(!car_versions_)throw std::logic_error("Versions binding requires actual scene Car callback owner");
        try{d_.world_actors.bind_versions(versions_);car_versions_->bind_versions(versions_);}
        catch(...){d_.world_actors.unbind_versions(versions_);car_versions_->unbind_versions(versions_);throw;}
    }
    ~ModuleOwners(){d_.world_actors.unbind_versions(versions_);if(car_versions_)car_versions_->unbind_versions(versions_);}
    ModuleOwners(const ModuleOwners&)=delete;ModuleOwners& operator=(const ModuleOwners&)=delete;
    auto& dispatch(){return dispatch_;}auto& mission_effects(){return mission_effects_;}
    auto& game_effects(){return effects_;}
    void finish_file_borrowers(){effects_.finish_file_borrowers();}
    void discard_file_scene_borrowers(){particles_.discard_scene_borrower();mission_effects_.close();}
private:
    frontend::ClientModuleRegistry& registry(){return d_.startup.game_module_registry();}
    frontend::ClientModuleRegistration& registration(unsigned kind){return d_.startup.game_module_registration(kind);}
    Dependencies& d_;MissionEffects mission_effects_;game::MissionTracksLoading tracks_;Effects effects_;Versions versions_;
    game::MissionLightmapGroupsLoading<Frames> lightmaps_;game::MissionTextureShiftLoading shifts_;
    game::ShotManagerLoading<Frames> shots_;game::HeavyFireModuleLoading heavy_fire_;Particles particles_;
    ModuleDependencies dependencies_;GameLoadingModules<ModuleDependencies> dispatch_;GameCarCallbacks* car_versions_=nullptr;
};

//One actual cold world/session dependency allocation. Original loader stages
//operate on these SAME owners; collision data is bound only at006EE570, with no
//dummy grid and no reference to an unconstructed service. Keep this graph until
//native module passes/Human/VM borrowers finish, then reset parent engine scene.
class World final:public game::SessionPacketWorld {
public:
    using Parent=Owners::Parent;
    World(Parent& parent,GameOwnerBackend& backend,StartupSession& startup,
          MenuDriver& driver,HMODULE module,Graphics& graphics,FrameClockPlatform& clock,
          const frontend::PreMissionItemCatalogue& catalogue,const std::string& suffix,
          std::function<void(unsigned,float)> progress,game::SessionActivationState& session)
        :startup_(startup),bindings_(startup),
         scene_(*parent.scene(),driver,module,graphics,startup.pre_mission_files(),startup.loaded_resources(),std::move(progress),startup.text(),
             bindings_.runtime,*parent.tables(),backend,startup.effects_loading_process(),make_game_car_callback_factory(startup,startup.car_definitions(),startup.car_tyres()),
             [&startup,&backend]{(void)startup.car_tyres().load(startup.pre_mission_files());startup.car_materials().rebuild(startup.car_definitions().materials(),backend);}),
         vertices_(scene_.frames(),startup.game_vertex_definitions(),startup.application_flags_owner(),startup.game_vertex_clock()),
         owners_(bindings_,scene_,parent,backend,startup,session,vertices_,clock,catalogue,suffix,startup.car_definitions(),startup.car_materials()),
         modules_(owners_,vertices_){
        scene_.frames().bind_vertex_animation_preference([&startup]{return startup.profile_settings().quality_byte(1)!=0;});
        startup_.bind_game_packet_world(*this);
        std::printf("P2.2 event=world_constructed result=success loaded14=%u\n",session.loaded?1u:0u);
    }
    ~World()override{
        startup_.blast_wave_process().clear(); //Source failure guard: retained frames cannot outlive World/Scene retirement.
        owners_.close(); //SAME current-game/default input getter remains published through Human/B0 retirement.
        startup_.unbind_game_packet_world(*this);
    }
    bool world_loaded()const override{return owners_.session_loaded();}
    bool dispatch_packet(std::uint32_t kind,game::SessionPacketStream& packet,bool authority)override{
        return modules_.dispatch().dispatch_packet(kind,packet,authority);
    }
    void receive_application_objective(std::uint32_t identity,std::uint32_t status)override{
        owners_.scripts().runtime().receive_human_objective_status(static_cast<std::int32_t>(identity),static_cast<std::int32_t>(status));
    }
    void receive_application_peer_departure(std::uint32_t transport)override{owners_.human_services().retire_session_peer(transport);}
    void receive_application_local_removal(std::uint32_t reason)override{
        game::receive_local_session_removal(owners_.dependencies().session,reason,owners_.dependencies().hud);
    }
    void receive_application_spawn_rejection(std::uint32_t reason)override{
        auto& d=owners_.dependencies();auto& services=owners_.human_services();
        game::receive_session_spawn_rejection(d.session,d.startup.pre_mission().squad(),reason,[&]{
            d.hud.show_map([&]{return services.mode_flags();},[&](bool enabled){services.spawn_pause(enabled);});
        });
    }
    void append_application_message(const std::string& message)override{owners_.dependencies().hud.append_message(message.c_str(),{{255,255,255}});}
    game::HumanInputCallbacks* input_default_receiver(std::int32_t identity,bool secondary)override{
        return game::human_input_default_receiver(scene_.frames(),startup_.game_human_input_callbacks(),identity,secondary);
    }
    World(const World&)=delete;World& operator=(const World&)=delete;
    Scene& scene(){return scene_;}Owners& owners(){return owners_;}ModuleOwners& modules(){return modules_;}
    auto& vertices(){return vertices_;}
    void load_collision_tree(const std::string& resource){scene_.load_collision_tree(resource);owners_.bind_loaded_collisions();}
    void release_collision_tree(){
        if(!scene_.collision_tree_if_loaded()){
            if(scene_.buckets().bound())throw std::logic_error("Collision buckets outlive their actual tree");
            return; //Native already-cleared tree branch, including file unload after00698F80.
        }
        if(!scene_.buckets().empty())throw std::logic_error("Retire actual collision descriptors before unbinding the Human sweep");
        owners_.unbind_loaded_collisions();scene_.release_collision_tree();
    }
private:
    StartupSession& startup_;Bindings bindings_;Scene scene_;Actors::Vertices vertices_;Owners owners_;ModuleOwners modules_;
};

} //namespace hd2::host::world_loading
#endif
