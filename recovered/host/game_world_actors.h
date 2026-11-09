#ifndef HD2_HOST_GAME_WORLD_ACTORS_H
#define HD2_HOST_GAME_WORLD_ACTORS_H

#include "host/game_world_scene.h"
#include "host/game_mission_items.h"
#include "host/game_runtime_actors.h"
#include "host/game_physics_objects.h"
#include "host/game_fowl_sound_loading.h"
#include "game/fowl_generator_loading.h"
#include "game/collision_ray.h"

namespace hd2::host {

//Actual shared loading composition, constructed AFTER the scene's collision
//tree and BEFORE module activation. Consumers supplies real item-transfer,
//sound/network and presentation-reference receivers; none are defaulted here.
//Containing Human/state/script owners must finish and retire before close().
//The scene/session/catalogue/clock/Consumers outlive this owner. Module event
//order remains with the containing session; construction is not activation.
template<class TablesBackend,class Consumers>class GameWorldActors {
public:
    using Scene=GameWorldScene<TablesBackend>;
    using Collisions=typename Scene::Buckets;
    using Items=GameMissionItems<Consumers>;
    using Runtime=GameRuntimeActors<GameWorldActors,Collisions,Items>;
    using Physics=GamePhysicsObjects<GameWorldActors,Collisions,Runtime>;
    using Body=game::RigidBody<GameActorServices,Collisions>;
    GameWorldActors(Scene& scene,Consumers& consumers,game::MissionNavigation& navigation,
                    const frontend::LocalizedTextCache& text,const frontend::PreMissionItemCatalogue& catalogue,
                    const std::string& suffix,ChunkFiles& files,FrameClockPlatform& clock,const std::uint32_t& game_word28)
        :scene_(scene),consumers_(consumers),text_(text),catalogue_(catalogue),models_(scene.frames(),catalogue,suffix,game_word28),
         items_(scene.frames(),models_,catalogue,files,clock,game_word28,consumers),
         runtime_(scene.frames(),*this,navigation,text,scene.buckets(),bodies_,items_),
         physics_(scene.frames(),*this,scene.buckets(),runtime_,scene.effects().frame_fades()),fowl_sound_(scene.frames(),consumers){}
    ~GameWorldActors(){close();}
    GameWorldActors(const GameWorldActors&)=delete;GameWorldActors& operator=(const GameWorldActors&)=delete;
    auto& items(){require_open();return items_;}auto& runtime(){require_open();return runtime_;}
    auto& physics(){require_open();return physics_;}auto& item_models(){require_open();return models_;}
    auto& rigid_bodies(){require_open();return bodies_;}
    std::uint32_t mode_flags()const{return consumers_.mode_flags();}
    std::string actor_localized(std::uint32_t id){return frontend::lookup_localized_text(text_,static_cast<std::int32_t>(id));}
    std::string actor_item_caption(std::uint32_t id) {
        if(id>static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max()))throw std::runtime_error("ExplosivePlace item definition is unavailable");
        const auto* item=catalogue_.item(static_cast<std::int32_t>(id));
        if(!item)throw std::runtime_error("ExplosivePlace requires its actual item definition for the armed caption");
        return actor_localized(item->caption);
    }
    void stop_actor_sound_voice(std::int32_t& handle){consumers_.stop_actor_sound_voice(handle);}
    std::uint32_t session_mission_ordinal()const{return consumers_.session_mission_ordinal();}
    auto& session_broadcast_packets(){return consumers_.session_broadcast_packets();}
    void door_script_use(GameActorServices::Frame frame,std::uint32_t command){consumers_.door_script_use(frame,command);}
    auto door_current_player_frame(){return consumers_.door_current_player_frame();}
    std::int32_t emit_door_sound(const frontend::SpatialSoundRequest& request){return consumers_.emit_door_sound(request);}
    void resume_door_sound(std::int32_t handle){consumers_.resume_door_sound(handle);}
    void pause_door_sound(std::int32_t handle){consumers_.pause_door_sound(handle);}
    auto door_sound_cursor(std::int32_t handle){return consumers_.door_sound_cursor(handle);}
    void door_sound_cursor(std::int32_t handle,std::uint32_t cursor){consumers_.door_sound_cursor(handle,cursor);}
    void update_door(game::DoorSceneActor<GameActorServices>& door,std::uint32_t elapsed) {
        require_open();auto queries=scene_.door_queries();door.frame_update(elapsed,*this,scene_.buckets(),queries,scene_.exclusions());
    }
    using Fowl=game::FowlGeneratorSceneActor<GameActorServices>;
    void activate_fowl(Fowl& actor,bool authority){
        require_open();if(!actor.loading)actor.loading=std::make_unique<game::FowlGeneratorLoading<GameActorServices,GameWorldActors>>(scene_.frames(),*this,actor);
        actor.loading->activate(authority);
    }
    void deactivate_fowl(Fowl& actor,bool authority){if(actor.loading)actor.loading->deactivate(authority);}
    std::int32_t fowl_random_word(){return consumers_.fowl_random_word();}
    auto fowl_camera_position(){const auto world=scene_.frames().actor_world_matrix(scene_.frames().active_game_camera());return GameActorServices::Vector{{world[12],world[13],world[14],world[15]}};}
    float fowl_cull_distance(){return consumers_.fowl_cull_distance();} //Actual containing session+C4, not a guessed camera far plane.
    auto fowl_scene_actor(std::size_t index)const{return index>0x7fffffffu?nullptr:scene_.frames().scene_actor_identity(static_cast<std::int32_t>(index));}
    std::size_t fowl_scene_actor_count()const{return scene_.frames().scene_actor_identity_count();}
    bool fowl_human_proximity_eligible(const game::SceneActor<GameActorServices>& actor)const {
        const auto* human=dynamic_cast<const typename Scene::Frames::Human*>(&actor);
        if(!human)throw std::runtime_error("Fowl Human proximity requires its actual typed Human owner");
        return human->properties.field19c==1;
    }
    std::optional<GameActorServices::Vector> fowl_ground(const GameActorServices::Vector& position) {
        game::CollisionRay<GameActorServices::Frame,typename Scene::Descriptor,GameCollisionResources,GameActorServices>
            query(&scene_.collision_tree(),&scene_.buckets(),scene_.collision_resources(),scene_.frames());
        const auto hit=query.ray(position,{{0,-100,0,0}},0,0,false);if(!hit)return std::nullopt;return hit->position;
    }
    std::int32_t add_fowl_listener(GameActorServices::Frame frame){return fowl_sound_.add_listener(frame);}
    void remove_fowl_listener(std::int32_t slot){fowl_sound_.remove_listener(slot);}
    void emit_fowl_sound(const frontend::SpatialSoundRequest& request){consumers_.emit_fowl_sound(request);}
    void rethrow_fowl_sound_failure(){fowl_sound_.rethrow_callback_failure();}
    void receive_actor_packet(typename Runtime::Instance& actor,game::SessionPacketStream& packet) {
        require_open();
        if(actor.base().type()==6)static_cast<game::DoorSceneActor<GameActorServices>&>(actor).receive_loading_packet(packet,*this);
        else if(actor.base().type()==10)consumers_.spawn_packet_decoder().receive(static_cast<game::SpawnPointSceneActor<GameActorServices>&>(actor),&packet);
        else if(actor.base().type()==29){auto& fowl=static_cast<Fowl&>(actor);if(!fowl.loading)throw std::runtime_error("Fowl packet requires its actual generated-bird owner");fowl.loading->launch(false,0);}
        else throw std::runtime_error("Required Scene2 actor packet receiver is unavailable");
    }
    void set_scene_actor_active(GameActorServices::Actor& actor,bool value){scene_.frames().set_scene_actor_active(actor,value);}
    void set_runtime_actor_active(GameActorServices::Actor& actor,bool value){runtime_.set_active(actor,value);}
    void remove_actor_presentation_references(typename Runtime::Instance& actor){consumers_.remove_actor_presentation_references(actor);}
    void close()noexcept {
        if(closed_)return;
        closed_=true;
        //Runtime35 borrows Physics descriptors. Runtime11 owns a body and an
        //Items entry. Retire both before either producer, then ordinary Scene2
        //actors before the remaining entry registry and cached template models.
        runtime_.clear();physics_.clear();
        auto& frames=scene_.frames();frames.clear_scene_actors();
        fowl_sound_.clear();
        items_.clear();frames.clear_actor_entries();models_.clear_model_cache();
    }
private:
    void require_open()const{if(closed_)throw std::logic_error("World actor owners are closed");}
    Scene& scene_;Consumers& consumers_;const frontend::LocalizedTextCache& text_;const frontend::PreMissionItemCatalogue& catalogue_;
    typename Body::Registry bodies_;GameRuntimeItemServices models_;
    Items items_;Runtime runtime_;Physics physics_;GameFowlSoundLoading<Consumers> fowl_sound_;bool closed_=false;
};
} //namespace hd2::host
#endif
