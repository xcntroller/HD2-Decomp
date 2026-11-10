#ifndef HD2_HOST_GAME_SCENE_ACTORS_H
#define HD2_HOST_GAME_SCENE_ACTORS_H

#include "host/game_actor_services.h"
#include "host/game_mesh_movie.h"
#include "game/scene_actor_registry.h"
#include "game/simple_scene_actors.h"
#include "game/geometry_scene_actors.h"
#include "game/model_scene_actors.h"
#include "game/static_weapon_actor.h"
#include "game/human_scene_actor.h"
#include "game/scene_actor_runtime.h"
#include "game/car_scene_actor.h"
#include "frontend/localized_text.h"
#include <cstring>

namespace hd2::host {

//Concrete recovered cold actor constructors and their matching owned registry.
//Other required families reject until their initialization/activation dependencies
//are supplied. Bounded derived activation does not construct a containing world.
class GameSceneActors : public GameMeshMovieServices {
public:
    using Runtime=game::SceneActorRuntime<Frame,Vector>;
    GameSceneActors(GameScene& scene,MenuDriver& driver,HMODULE module,Graphics& graphics,
                    ChunkFiles& files,LoadedResources& resources,std::function<void(unsigned,float)> progress,
                    const frontend::LocalizedTextCache& text,Runtime& runtime,game::CarCallbackServices<GameActorServices>& cars)
        :GameSceneActors(scene,driver,module,graphics,files,resources,std::move(progress),text,runtime){bind_car_callbacks(cars);}
    GameSceneActors(GameScene& scene,MenuDriver& driver,HMODULE module,Graphics& graphics,
                    ChunkFiles& files,LoadedResources& resources,std::function<void(unsigned,float)> progress,
                    const frontend::LocalizedTextCache& text,Runtime& runtime)
        :GameMeshMovieServices(scene,driver,module,graphics,files,resources,std::move(progress)),text_(text),runtime_(runtime){}
    //The actual Car callback owner borrows these SAME services. Construct them
    //first, then bind the callback owner before parsing a Car. Never manufacture
    //another frame-data/entry registry to break that composition dependency.
    void bind_car_callbacks(game::CarCallbackServices<GameActorServices>& cars) {
        if(cars_)throw std::logic_error("Car callbacks are already bound to the scene actor services");
        cars_=&cars;
    }
    using Instance=game::SceneActor<GameActorServices>;
    using ActorOwner=std::unique_ptr<Instance>;
    using Actor=Instance*;
    using Spawn=game::SpawnPointSceneActor<GameActorServices>;
    using Human=game::HumanSceneActor<GameActorServices,Runtime>;
    Human* camera_target_human(Frame target)const { //00466970 model100/kind9/EC Scene2 binding.
        const auto model=frame_model_owner(target);
        if(!model||frame_kind(model)!=9)return nullptr;
        const auto* data=find_actor_frame_data(model);
        if(!data||!data->words[0])return nullptr;
        auto* actor=reinterpret_cast<Instance*>(data->words[0]);
        if(actor->base().type()!=3)return nullptr;
        auto* human=dynamic_cast<Human*>(actor);
        if(!human||human->base().frame()!=model)throw std::runtime_error("Camera target Human binding disagrees with actual model owner");
        return human;
    }
    bool has_scene_actor_factory(std::uint32_t type) const {
        switch(type){case 3:case 4:case 6:case 8:case 9:case 10:case 13:case 21:case 22:case 25:case 28:case 29:case 30:case 33:return true;default:return false;}
    }
    ActorOwner make_scene_actor(std::uint32_t type) {
        switch(type) {
        case 3:return std::make_unique<game::HumanSceneActor<GameActorServices,Runtime>>(*this,runtime_);
        case 4:
            if(!cars_)throw std::runtime_error("Car construction requires the actual shared callback owner");
            return std::make_unique<game::CarSceneActor<GameActorServices>>(*this,*cars_,frontend::lookup_localized_text(text_,0x1bf0));
        case 6:return std::make_unique<game::DoorSceneActor<GameActorServices>>(*this);
        case 8:return std::make_unique<game::LadderSceneActor<GameActorServices>>(*this,frontend::lookup_localized_text(text_,0x1bf2));
        case 9:return std::make_unique<game::SupplyBoxSceneActor<GameActorServices>>(*this);
        case 10:return std::make_unique<game::SpawnPointSceneActor<GameActorServices>>(*this);
        case 13:return std::make_unique<game::SwitchSceneActor<GameActorServices,Runtime>>(*this,runtime_,frontend::lookup_localized_text(text_,0x1bf3));
        case 21:return std::make_unique<game::StaticWeaponSceneActor<GameActorServices>>(*this);
        case 22:return std::make_unique<game::ExplosivePlaceSceneActor<GameActorServices>>(*this);
        case 25:return std::make_unique<game::AnimationSceneActor<GameActorServices>>(*this);
        case 28:return std::make_unique<game::PikeSceneActor<GameActorServices>>(*this);
        case 29:return std::make_unique<game::FowlGeneratorSceneActor<GameActorServices>>(*this);
        case 30:return std::make_unique<game::DogSceneActor<GameActorServices>>(*this,frontend::lookup_localized_text(text_,0x1bf5));
        case 33:return std::make_unique<game::NullSceneActor<GameActorServices>>(*this);
        default:throw std::runtime_error("Required scene actor constructor is not reconstructed: "+std::to_string(type));
        }
    }
    Actor create_actor(std::uint32_t type,Frame frame) {
        if(!has_scene_actor_factory(type))
            throw std::runtime_error("Required scene actor family is not reconstructed: "+std::to_string(type));
        return actors_.create(type,frame);
    }
    bool initialize_actor(Actor actor,Frame frame) {
        if(!actor)return false;
        try {
            if(actor->initialize(frame))return true;
        } catch(...) {actors_.remove(actor);throw;}
        actors_.remove(actor);return false;
    }
    void read_actor_properties(Actor actor,game::SerializedBytes& bytes) {
        if(!actor)throw std::logic_error("Actor property read requires an owner");
        actor->load(bytes);
    }
    void clear_scene_actors() noexcept {actors_.clear();}
    Instance* scene_actor_identity(std::int32_t id)const { //00475DD0, Scene2 identity slots, not runtime IDs.
        return id<0?nullptr:actors_.identity(static_cast<std::size_t>(id));
    }
    std::size_t scene_actor_identity_count()const{return actors_.identity_count();}
    Human* scene_human_identity(std::int32_t identity)const{ //00475E00, including the SAME pending-retirement list.
        auto* actor=actors_.human_identity(identity);if(!actor)return nullptr;
        auto* human=dynamic_cast<Human*>(actor);if(!human)throw std::runtime_error("Human identity resolves to another actual actor family");
        return human;
    }
    std::int32_t player_generation(std::int32_t identity)const { //004754F0 sign-extends native CHAR.
        return identity<0||identity>31?-1:static_cast<std::int8_t>(actors_.generation(static_cast<std::size_t>(identity)));
    }
    std::int32_t first_free_player_identity()const { //004759A0, physical reserved slots0..31 ascending.
        for(std::size_t n=0;n!=32;++n)if(!actors_.identity(n))return static_cast<std::int32_t>(n);
        return -1;
    }
    game::HumanSceneActor<GameActorServices,Runtime>* create_prepared_human(Frame frame,std::int32_t identity) {
        auto* actor=actors_.create(3,frame,true,identity); //004759C0, actual reserved replacement or ordinary append for-1.
        if(!actor)return nullptr;
        if(!initialize_actor(actor,frame))return nullptr;
        return static_cast<game::HumanSceneActor<GameActorServices,Runtime>*>(actor);
    }
    game::SpawnPointSceneActor<GameActorServices>* loading_spawn_point(std::uint32_t team)const { //00475940: active list, ascending, first match.
        for(auto* actor:actors_.active_actors())if(actor&&actor->base().type()==10) {
            auto* spawn=static_cast<game::SpawnPointSceneActor<GameActorServices>*>(actor);
            if(spawn->properties.field90==team&&spawn->flag9c)return spawn;
        }
        return nullptr;
    }
    void set_scene_actor_active(GameActorServices::Actor& base,bool value) {
        auto* actor=actors_.actor_for_base(base);
        if(!actor)throw std::runtime_error("Scene actor active-list receiver has no matching owner");
        actors_.active(actor,value);
    }
    template<class Activation,class Collisions>void activate_scene_actors(bool authority,Activation& activation,Collisions& collisions) {
        actors_.activate([&](Instance& actor) {
            switch(actor.base().type()) {
            case 3:activation.activate_human(static_cast<game::HumanSceneActor<GameActorServices,Runtime>&>(actor),authority);break;
            case 4:activation.activate_car(static_cast<game::CarSceneActor<GameActorServices>&>(actor),authority);break;
            case 6:static_cast<game::DoorSceneActor<GameActorServices>&>(actor).activate(authority,activation,collisions);break;
            case 8:static_cast<game::LadderSceneActor<GameActorServices>&>(actor).activate(authority,activation,collisions);break;
            case 10:static_cast<game::SpawnPointSceneActor<GameActorServices>&>(actor).activate(authority,activation);break;
            case 13:static_cast<game::SwitchSceneActor<GameActorServices,Runtime>&>(actor).activate(authority,activation,collisions);break;
            case 22:static_cast<game::ExplosivePlaceSceneActor<GameActorServices>&>(actor).activate(authority,activation,collisions);break;
            case 9:activation.activate_supply(static_cast<game::SupplyBoxSceneActor<GameActorServices>&>(actor),authority);break;
            case 21:activation.activate_static_weapon(static_cast<game::StaticWeaponSceneActor<GameActorServices>&>(actor),authority);break;
            case 25:activation.activate_animation(static_cast<game::AnimationSceneActor<GameActorServices>&>(actor),authority);break;
            case 28:static_cast<game::PikeSceneActor<GameActorServices>&>(actor).activate(authority,activation,collisions);break;
            case 29:activation.activate_fowl(static_cast<game::FowlGeneratorSceneActor<GameActorServices>&>(actor),authority);break;
            case 33:static_cast<game::NullSceneActor<GameActorServices>&>(actor).activate(authority,activation);break;
            default:throw std::runtime_error("Required Scene2 actor activation is unavailable: "+std::to_string(actor.base().type()));
            }
        });
    }
    template<class Activation,class Collisions>void deactivate_scene_actors(bool authority,Activation& activation,Collisions& collisions) {
        frontend::CleanupErrors errors;
        errors.attempt([&]{if((activation.mode_flags()&7)!=7)flush_scene_actor_removals(100000,activation,collisions);});
        errors.attempt([&]{actors_.deactivate([&](Instance& actor){deactivate_scene_actor(actor,authority,activation,collisions);},
                                            [&]{return authority||!(activation.mode_flags()&1);});});
        errors.rethrow();
    }
    template<class Activation,class Collisions>void deactivate_scene_actor(Instance& actor,bool authority,Activation& activation,Collisions& collisions) {
        switch(actor.base().type()) {
        case 3:activation.deactivate_human(static_cast<game::HumanSceneActor<GameActorServices,Runtime>&>(actor),authority);break;
        case 4:activation.deactivate_car(static_cast<game::CarSceneActor<GameActorServices>&>(actor),authority);break;
        case 6:static_cast<game::DoorSceneActor<GameActorServices>&>(actor).deactivate(authority,activation,collisions);break;
        case 8:static_cast<game::LadderSceneActor<GameActorServices>&>(actor).deactivate(authority,activation,collisions);break;
        case 10:static_cast<game::SpawnPointSceneActor<GameActorServices>&>(actor).deactivate(authority,activation);break;
        case 13:static_cast<game::SwitchSceneActor<GameActorServices,Runtime>&>(actor).deactivate(authority,activation,collisions);break;
        case 22:static_cast<game::ExplosivePlaceSceneActor<GameActorServices>&>(actor).deactivate(authority,activation,collisions);break;
        case 9:activation.deactivate_supply(static_cast<game::SupplyBoxSceneActor<GameActorServices>&>(actor),authority);break;
        case 21:activation.deactivate_static_weapon(static_cast<game::StaticWeaponSceneActor<GameActorServices>&>(actor),authority);break;
        case 25:activation.deactivate_animation(static_cast<game::AnimationSceneActor<GameActorServices>&>(actor),authority);break;
        case 28:static_cast<game::PikeSceneActor<GameActorServices>&>(actor).deactivate(authority,activation,collisions);break;
        case 29:activation.deactivate_fowl(static_cast<game::FowlGeneratorSceneActor<GameActorServices>&>(actor),authority);break;
        case 33:static_cast<game::NullSceneActor<GameActorServices>&>(actor).deactivate(authority,activation);break;
        default:throw std::runtime_error("Required Scene2 actor deactivation is unavailable: "+std::to_string(actor.base().type()));
        }
    }
    template<class RetireHuman>void retire_scene_actor(Actor actor,RetireHuman&& retire_human) {
        if(!actor)return;
        if(actor->base().type()!=3)throw std::runtime_error("Delayed retirement requires an actual Human receiver");
        actors_.retire(actor,std::forward<RetireHuman>(retire_human)); //Mandatory actual005A7FE0 receiver.
    }
    template<class Activation,class Collisions>void flush_scene_actor_removals(std::uint32_t elapsed,Activation& activation,Collisions& collisions) {
        actors_.flush_retired(elapsed,[&]{return activation.mode_flags();},
            [&](Instance& actor){activation.clear_retired_actor_record(actor.base().id10());},
            [&](Instance& actor,bool authority){deactivate_scene_actor(actor,authority,activation,collisions);},
            [&](Instance& actor){activation.remove_actor_presentation_references(actor);});
    }
    std::int32_t actor_inventory_capacity(const GameActorServices::Actor& base) const {
        switch(base.type()) {
        case 4:return 500; //004D0E20.
        case 21:return 100; //00602F60.
        case 3: {
            const auto* data=find_actor_frame_data(base.frame());
            if(!data||!data->words[0])throw std::runtime_error("Human inventory has no concrete actor");
            auto* actor=reinterpret_cast<Instance*>(data->words[0]);
            if(&actor->base()!=&base)throw std::runtime_error("Human inventory actor binding disagrees");
            const auto* human=dynamic_cast<const game::HumanSceneActor<GameActorServices,Runtime>*>(actor);
            if(!human)throw std::runtime_error("Human inventory owner has the wrong concrete family");
            const auto bits=human->properties.stats_ec[1];
            std::int32_t capacity;std::memcpy(&capacity,&bits,sizeof(capacity));
            return capacity; //0059C2B0→005A6600(1), nativeF0.
        }
        default:throw std::runtime_error("Required actor inventory capacity receiver is unavailable");
        }
    }
private:
    const frontend::LocalizedTextCache& text_;
    Runtime& runtime_;
    game::CarCallbackServices<GameActorServices>* cars_=nullptr;
    game::SceneActorRegistry<GameSceneActors> actors_{*this};
};

} //namespace hd2::host
#endif
