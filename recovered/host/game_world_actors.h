#ifndef HD2_HOST_GAME_WORLD_ACTORS_H
#define HD2_HOST_GAME_WORLD_ACTORS_H

#include "host/game_world_scene.h"
#include "host/game_mission_items.h"
#include "host/game_runtime_actors.h"
#include "host/game_physics_objects.h"

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
        :scene_(scene),consumers_(consumers),models_(scene.frames(),catalogue,suffix,game_word28),
         items_(scene.frames(),models_,catalogue,files,clock,game_word28,consumers),
         runtime_(scene.frames(),*this,navigation,text,scene.buckets(),bodies_,items_),
         physics_(scene.frames(),*this,scene.buckets(),runtime_,scene.effects().frame_fades()){}
    ~GameWorldActors(){close();}
    GameWorldActors(const GameWorldActors&)=delete;GameWorldActors& operator=(const GameWorldActors&)=delete;
    auto& items(){require_open();return items_;}auto& runtime(){require_open();return runtime_;}
    auto& physics(){require_open();return physics_;}auto& item_models(){require_open();return models_;}
    auto& rigid_bodies(){require_open();return bodies_;}
    std::uint32_t mode_flags()const{return consumers_.mode_flags();}
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
        items_.clear();frames.clear_actor_entries();models_.clear_model_cache();
    }
private:
    void require_open()const{if(closed_)throw std::logic_error("World actor owners are closed");}
    Scene& scene_;Consumers& consumers_;typename Body::Registry bodies_;GameRuntimeItemServices models_;
    Items items_;Runtime runtime_;Physics physics_;bool closed_=false;
};
} //namespace hd2::host
#endif
