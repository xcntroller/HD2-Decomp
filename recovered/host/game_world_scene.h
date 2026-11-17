#ifndef HD2_HOST_GAME_WORLD_SCENE_H
#define HD2_HOST_GAME_WORLD_SCENE_H

#include "host/game_mission_scene.h"
#include "host/game_collision_resources.h"
#include "game/mission_scene_documents.h"
#include "game/effects_shadow_models.h"
#include "game/effects_loading_process.h"
#include "game/dynamic_collision_grid.h"
#include "game/dynamic_collision_exclusions.h"
#include "game/door_collision_queries.h"
#include "game/car_collision_queries.h"

namespace hd2::host {

//Containing source owner for the actual Scene2/name-map/table/environment/
//effects/collision dependency set. SessionOwner retains the engine scene and
//two table datasets. All actor, script, Human and item consumers must borrow
//frames() here, and all collision descriptors must borrow buckets() here.
//This owner alone cannot activate a session, publish audio or draw a first frame.
//External loading/state/runtime/physics owners must retire before close().
template<class TablesBackend>class GameWorldScene {
public:
    using Frames=GameMissionScene<TablesBackend>;
    using CarServices=game::CarCallbackServices<GameActorServices>;
    using CarFactory=std::function<std::unique_ptr<CarServices>(GameActorServices&)>;
    using Tree=game::CollisionTree<EngineFrame*>;
    using Descriptor=game::RigidBodyCollisionDescriptor<EngineFrame*>;
    using Buckets=game::DynamicCollisionBuckets<Descriptor,typename Tree::DynamicGrid>;
    using Documents=game::MissionSceneDocuments<ChunkFiles,GameScene,Frames>;
    using Scratch=typename Documents::Scratch;
    GameWorldScene(GameScene& scene,MenuDriver& driver,HMODULE module,Graphics& graphics,
                   ChunkFiles& files,LoadedResources& resources,std::function<void(unsigned,float)> progress,
                   const frontend::LocalizedTextCache& text,typename Frames::Runtime& runtime,
                   game::MissionTables<TablesBackend>& tables,TablesBackend& table_storage,game::EffectsLoadingProcess<EngineFrame*>& effects_process,
                   CarFactory make_cars,std::function<void()> prepare_cars)
        :files_(files),environment_(effects_process.environment),collision_resources_(scene,module),
         frames_(scene,driver,module,graphics,files,resources,std::move(progress),text,runtime,tables,table_storage,environment_,aliases_),
         effects_(frames_,environment_,effects_process.fade),documents_(files,scene,frames_,std::move(prepare_cars)) {
        if(!make_cars)throw std::logic_error("World scene requires its actual Car callback factory");
        cars_=make_cars(frames_);
        if(!cars_)throw std::runtime_error("Actual Car callback factory returned no owner");
        frames_.bind_car_callbacks(*cars_);
    }
    ~GameWorldScene(){close();}
    GameWorldScene(const GameWorldScene&)=delete;GameWorldScene& operator=(const GameWorldScene&)=delete;
    Frames& frames(){require_open();return frames_;}auto& effects(){require_open();return effects_;}auto& environment(){require_open();return environment_;}
    CarServices& car_callbacks(){require_open();return *cars_;}
    auto& lightmap_aliases(){require_open();return aliases_;}auto& documents(){require_open();return documents_;}
    auto& collision_resources(){require_open();return collision_resources_;}auto& exclusions(){require_open();return exclusions_;}
    Tree& collision_tree(){require_open();if(!tree_)throw std::logic_error("World collision tree is not loaded");return *tree_;}
    const Tree* collision_tree_if_loaded()const noexcept{return tree_.get();}
    Buckets& buckets(){require_open();return buckets_;}
    auto door_queries(){return game::DoorCollisionQueries<EngineFrame*,Descriptor,GameActorServices>(collision_tree(),buckets(),frames_);}
    auto car_queries(){
        return game::CarCollisionQueries<EngineFrame*,Descriptor,GameCollisionResources,GameActorServices,game::SceneActor<GameActorServices>*>(
            collision_tree(),buckets(),collision_resources_,frames_);
    }
    void load_collision_tree(const std::string& resource) {
        require_open();
        if(tree_||buckets_.bound())throw std::logic_error("Retire actual world collision owners before loading another tree");
        const auto path="missions\\"+resource+"\\tree.klz";
        const auto handle=files_.open(path.c_str(),0);
        if(handle==0xffffffffu)throw std::runtime_error("Required mission collision tree is unavailable");
        struct Close {ChunkFiles& files;std::uint32_t handle;~Close(){files.close(handle);}}close{files_,handle};
        const auto size=files_.seek(handle,0,2);
        if(size==0xffffffffu||size>128u*1024u*1024u)throw std::runtime_error("Mission collision tree exceeds bounded size");
        if(files_.seek(handle,0,0)!=0)throw std::runtime_error("Cannot rewind mission collision tree");
        std::vector<unsigned char> bytes(size);
        if(size&&files_.read(handle,bytes.data(),size)!=size)throw std::runtime_error("Short mission collision tree read");
        auto tree=std::make_unique<Tree>(game::read_collision_tree(game::SerializedBytes(bytes),collision_resources_,true));
        //Construct against its final stable allocation, never a local/moved grid.
        buckets_.bind(tree->dynamic);
        tree_=std::move(tree);
    }
    void release_collision_tree(){
        require_open();
        if(!buckets_.empty())throw std::logic_error("Retire actual collision descriptors before releasing the mission tree");
        exclusions_.clear();buckets_.detach();tree_.reset();collision_resources_.clear_views();aliases_.clear();
    }
    void close()noexcept {
        if(closed_)return;
        closed_=true;
        try{effects_.configure_special_model(false,nullptr,true);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
        exclusions_.clear(); //Must stop querying while its actual descriptors still live.
        effects_.human_fades().clear(); //Borrowed Human228 records before Scene2 Human deletion.
        frames_.clear_scene_actors(); //Car borrowers retire while cars_ still lives.
        effects_.frame_fades().clear();
        cars_.reset();
        buckets_.detach();tree_.reset();collision_resources_.clear_views();
        aliases_.clear();
        //Scene reset/release belongs to the containing SessionOwner, after all
        //other runtime actor/entry/physics owners have retired as required above.
    }
private:
    void require_open()const{if(closed_)throw std::logic_error("World scene owner is closed");}
    ChunkFiles& files_;game::EffectsEnvironment& environment_;game::LightmapAliases aliases_;
    GameCollisionResources collision_resources_;std::unique_ptr<Tree> tree_;Buckets buckets_;
    game::DynamicCollisionExclusions<Descriptor> exclusions_;
    std::unique_ptr<CarServices> cars_; //Must outlive every Car held by frames_.
    Frames frames_;game::EffectsShadowModels<GameActorServices> effects_;Documents documents_;bool closed_=false;
};
} //namespace hd2::host
#endif
