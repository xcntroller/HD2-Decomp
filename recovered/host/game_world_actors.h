#ifndef HD2_HOST_GAME_WORLD_ACTORS_H
#define HD2_HOST_GAME_WORLD_ACTORS_H

#include "game/session_time_word.h"
#include "host/game_world_scene.h"
#include "host/game_mission_items.h"
#include "host/game_runtime_actors.h"
#include "host/game_physics_objects.h"
#include "host/game_fowl_sound_loading.h"
#include "game/fowl_generator_loading.h"
#include "game/collision_ray.h"
#include "host/game_animation_actor_events.h"
#include "game/animation_actor_loading.h"
#include "game/supply_box_loading.h"
#include "game/static_weapon_loading.h"
#include "host/game_static_weapon_events.h"
#include "game/car_loading_activation.h"
#include "game/human_loading_composition.h"
#include <map>

namespace hd2::host {

//Actual shared cold loading composition, constructed BEFORE module activation.
//The SAME bucket owner may await its real tree; registration/queries require it. Consumers supplies real item-transfer,
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
    using Vertices=game::MissionVertexInstances<GameActorServices>;
    using Body=game::RigidBody<GameActorServices,Collisions>;
    GameWorldActors(Scene& scene,Consumers& consumers,game::MissionNavigation& navigation,Vertices& vertices,
                    const frontend::LocalizedTextCache& text,const frontend::PreMissionItemCatalogue& catalogue,
                    const std::string& suffix,ChunkFiles& files,FrameClockPlatform& clock,game::SessionTimeWord game_word28)
        :scene_(scene),consumers_(consumers),vertices_(vertices),navigation_(navigation),text_(text),catalogue_(catalogue),models_(scene.frames(),catalogue,suffix,game_word28),
         items_(scene.frames(),models_,catalogue,files,clock,game_word28,consumers),
         runtime_(scene.frames(),*this,navigation,text,scene.buckets(),bodies_,items_),
         physics_(scene.frames(),*this,scene.buckets(),runtime_,scene.effects().frame_fades()),fowl_sound_(scene.frames(),consumers),animation_events_(scene.frames(),consumers),static_weapon_events_(scene.frames()){}
    ~GameWorldActors(){close();}
    GameWorldActors(const GameWorldActors&)=delete;GameWorldActors& operator=(const GameWorldActors&)=delete;
    auto& items(){require_open();return items_;}auto& runtime(){require_open();return runtime_;}
    auto& physics(){require_open();return physics_;}auto& item_models(){require_open();return models_;}
    template<class Callbacks>void bind_car_physics(Callbacks& callbacks){
        require_open();
        static_assert(noexcept(callbacks.unbind_physics(physics_)),"Car physics retirement must not throw");
        if(car_physics_unbind_||static_cast<typename Scene::CarServices*>(&callbacks)!=&scene_.car_callbacks())
            throw std::logic_error("Car physics must bind the containing scene's actual callback owner once");
        std::function<void()> unbind=[this,&callbacks]()noexcept{callbacks.unbind_physics(physics_);};
        callbacks.bind_physics(physics_);car_physics_unbind_=std::move(unbind);
    }
    auto& rigid_bodies(){require_open();return bodies_;}
    using Human=game::HumanSceneActor<GameActorServices,GameSceneActors::Runtime>;
    template<class Owner>Owner& human_loading_owner(Human& actor){
        require_open();const auto found=human_loading_.find(&actor);
        if(found==human_loading_.end()||actor.loading_lifetime!=found->second.get())
            throw std::runtime_error("Human callback requires its actual containing loading owner");
        auto* owner=dynamic_cast<Owner*>(found->second.get());
        if(!owner)throw std::runtime_error("Human callback loading owner has a different concrete type");
        return *owner;
    }
    bool has_human_loading_owners()const noexcept{return !human_loading_.empty();}
    game::HumanLoadingLifetime& prepare_human_loading(Human& actor){
        require_open();if(closing_)throw std::logic_error("Human activation during world retirement");
        if(scene_.frames().scene_actor_identity(actor.base().id10())!=&actor)
            throw std::logic_error("Human loading requires SAME registered Scene2 identity");
        auto found=human_loading_.find(&actor);
        if(found==human_loading_.end()){
            if(actor.loading_lifetime)throw std::logic_error("Human loading already belongs to another containing world");
            auto owner=consumers_.make_human_loading_owner(scene_.frames(),*this,actor);
            if(!owner||actor.loading_lifetime!=owner.get())throw std::runtime_error("Human factory must publish its actual composed loading owner");
            found=human_loading_.emplace(&actor,std::move(owner)).first;
        }
        return *found->second;
    }
    void activate_human(Human& actor,bool authority){prepare_human_loading(actor).activate(authority);}
    void deactivate_human(Human& actor,bool authority){
        const auto found=human_loading_.find(&actor);
        if(found==human_loading_.end())throw std::runtime_error("Human retirement requires its actual containing loading owner");
        frontend::CleanupErrors errors;errors.attempt([&]{found->second->deactivate(authority);});
        //Scene2 removes reserved Humans immediately after the authority call,
        //including when a receiver fails. Retract all borrowers while it lives.
        if(authority)human_loading_.erase(found);
        errors.rethrow();
    }
    std::uint32_t mode_flags()const{return consumers_.mode_flags();}
    std::int32_t human_current_player_id(){return consumers_.human_current_player_id();}
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
    using Car=game::CarSceneActor<GameActorServices>;
    void activate_car(Car& actor,bool authority){
        require_open();
        if(!actor.loading){
            auto queries=scene_.car_queries();
            actor.loading=std::make_unique<game::CarLoadingActivation<GameActorServices,GameWorldActors,Collisions,decltype(queries)>>(
                scene_.frames(),*this,scene_.buckets(),std::move(queries),scene_.exclusions(),navigation_,consumers_.car_loading_resources(),actor);
        }
        actor.loading->activate(authority);
    }
    void deactivate_car(Car& actor,bool authority){if(actor.loading)actor.loading->deactivate(authority);}
    void update_car(Car& actor,bool authority,std::uint32_t elapsed){
        require_open();if(!actor.loading)throw std::runtime_error("Car update requires its actual loading owner");
        actor.loading->update(authority,elapsed);
    }
    bool car_camera_setting264(){return consumers_.car_camera_setting264();}
    std::int32_t add_car_proximity(Car& actor,unsigned kind){return consumers_.actor_proximity().add_actor(&actor,kind);}
    void remove_car_proximity(std::int32_t index){consumers_.actor_proximity().remove_actor(index);}
    const Car* car_current_attachment(){return consumers_.car_current_attachment();}
    unsigned car_game_kind(){return consumers_.car_game_kind();}
    using StaticWeapon=game::StaticWeaponSceneActor<GameActorServices>;
    void activate_static_weapon(StaticWeapon& actor,bool authority){
        require_open();if(!actor.loading)actor.loading=std::make_unique<game::StaticWeaponLoading<GameActorServices,GameWorldActors,Collisions,Items>>(scene_.frames(),*this,scene_.buckets(),items_,actor);
        actor.loading->activate(authority);
    }
    void deactivate_static_weapon(StaticWeapon& actor,bool authority){if(actor.loading)actor.loading->deactivate(authority);}
    void update_static_weapon(StaticWeapon& actor,bool authority,std::uint32_t elapsed){require_open();if(!actor.loading)throw std::runtime_error("StaticWeapon requires actual loading owner");actor.loading->update(authority,elapsed);}
    void install_static_weapon_events(GameActorServices::Frame frame,game::StaticWeaponLoadingLifetime& control){static_weapon_events_.install(frame,control);}
    void remove_static_weapon_events(GameActorServices::Frame frame,game::StaticWeaponLoadingLifetime& control){static_weapon_events_.remove(frame,control);}
    void rethrow_static_weapon_event_failure(){static_weapon_events_.rethrow_failure();}
    GameActorServices::Vector static_weapon_navigation_position(StaticWeapon& actor){
        const auto references=navigation_.actor_references(&actor);if(!references.empty()){
            const auto position=navigation_.position(references.front());return {{position[0],position[1],position[2],0}};
        }
        const auto world=scene_.frames().actor_world_matrix(actor.base().frame());GameActorServices::Vector position{};
        for(unsigned a=0;a!=3;++a){const float offset=static_cast<float>(static_cast<long double>(world[8+a])*-.5f);position[a]=static_cast<float>(static_cast<long double>(offset)+world[12+a]);}
        return position;
    }
    using Supply=game::SupplyBoxSceneActor<GameActorServices>;
    void activate_supply(Supply& actor,bool authority){
        require_open();if(!actor.loading)actor.loading=std::make_unique<game::SupplyBoxLoading<GameActorServices,GameWorldActors,Collisions,Vertices,game::EffectsFrameFades<GameActorServices>>>(scene_.frames(),*this,scene_.buckets(),vertices_,scene_.effects().frame_fades(),actor);
        actor.loading->activate(authority);
    }
    void deactivate_supply(Supply& actor,bool authority){if(actor.loading)actor.loading->deactivate(authority);}
    void update_supply(Supply& actor,bool authority,std::uint32_t elapsed){
        require_open();if(authority&&(mode_flags()&2u))return; //005FBD20.
        if(!actor.loading)throw std::runtime_error("SupplyBox update requires its actual loading owner");
        actor.loading->update(elapsed);
    }
    void emit_supply_sound(const frontend::SpatialSoundRequest& request){consumers_.emit_supply_sound(request);}
    using Fowl=game::FowlGeneratorSceneActor<GameActorServices>;
    using Animation=game::AnimationSceneActor<GameActorServices>;
    void activate_animation(Animation& actor,bool authority){
        require_open();if(!actor.loading)actor.loading=std::make_unique<game::AnimationActorLoading<GameActorServices,GameWorldActors,Collisions>>(scene_.frames(),*this,scene_.buckets(),actor);
        actor.loading->activate(authority);
    }
    void deactivate_animation(Animation& actor,bool authority){if(actor.loading)actor.loading->deactivate(authority);}
    void update_animation(Animation& actor,bool authority,std::uint32_t elapsed){
        require_open();if(authority&&(mode_flags()&2u))return; //005B54A0; presentation005B5490 is a thunk.
        if(!actor.loading)throw std::runtime_error("Animation update requires its actual loading owner");
        actor.loading->update(elapsed);
    }
    void install_animation_actor_events(GameActorServices::Frame frame,GameActorServices::Frame group){animation_events_.install(frame,group);}
    void remove_animation_actor_events(GameActorServices::Frame group){animation_events_.remove(group);}
    void rethrow_animation_actor_event_failure(){animation_events_.rethrow_failure();}
    std::uint32_t animation_actor_material_word(std::uint32_t id,unsigned offset){
        const auto& records=consumers_.car_definitions().materials();if(id>=records.size())throw std::runtime_error("Animation actor requires its actual HD material definition");
        return records[id].word(offset);
    }
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
    void clear_retired_actor_record(std::int32_t identity){consumers_.clear_retired_actor_record(identity);}
    void close()noexcept {
        if(closed_||closing_)return;
        closing_=true;
        //Human B0/inline/VM presentation borrowers retire while Scene2 actor
        //identities, Items and the shared world dependencies still exist.
        frontend::CleanupErrors human_errors;
        for(auto& record:human_loading_){
            human_errors.attempt([&]{record.second->deactivate(false);});
            human_errors.attempt([&]{record.second->deactivate(true);});
        }
        human_loading_.clear();
        if(human_errors.failed())try{human_errors.rethrow();}catch(...){scene_.frames().record_actor_cleanup_failure(std::current_exception());}
        closed_=true;
        //Runtime35 borrows Physics descriptors. Runtime11 owns a body and an
        //Items entry. Retire both before either producer, then ordinary Scene2
        //actors before the remaining entry registry and cached template models.
        runtime_.clear();
        if(car_physics_unbind_){car_physics_unbind_();car_physics_unbind_={};}
        physics_.clear();
        auto& frames=scene_.frames();frames.clear_scene_actors();
        fowl_sound_.clear();
        animation_events_.clear();
        static_weapon_events_.clear();
        items_.clear();frames.clear_actor_entries();models_.clear_model_cache();closing_=false;
    }
private:
    void require_open()const{if(closed_)throw std::logic_error("World actor owners are closed");}
    Scene& scene_;Consumers& consumers_;Vertices& vertices_;game::MissionNavigation& navigation_;const frontend::LocalizedTextCache& text_;const frontend::PreMissionItemCatalogue& catalogue_;
    typename Body::Registry bodies_;GameRuntimeItemServices models_;
    std::map<Human*,std::unique_ptr<game::HumanLoadingLifetime>> human_loading_;
    Items items_;Runtime runtime_;Physics physics_;std::function<void()> car_physics_unbind_;GameFowlSoundLoading<Consumers> fowl_sound_;GameAnimationActorEvents<Consumers> animation_events_;GameStaticWeaponEvents static_weapon_events_;bool closed_=false,closing_=false;
};
} //namespace hd2::host
#endif
