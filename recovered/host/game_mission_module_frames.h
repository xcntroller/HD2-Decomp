#ifndef HD2_HOST_GAME_MISSION_MODULE_FRAMES_H
#define HD2_HOST_GAME_MISSION_MODULE_FRAMES_H

#include "host/game_camera_loading.h"
#include "host/game_physics_frames.h"
#include "host/game_scene_actor_frames.h"

namespace hd2::host {

//Borrow the loaded World and its registered module owners. The physics,
//movement, Scene2 and camera facades all address the same World allocation.
template<class Runtime>class GameMissionModuleFrames {
public:
    GameMissionModuleFrames(world_loading::World& world,Runtime& runtime)
        :world_(world),d_(world.owners().dependencies()),physics_(world.owners()),camera_(world.owners()),
         actors_(d_,world.owners().human_services(),runtime),runtime_(runtime){}
    void tick(bool authority,std::uint32_t elapsed){
        if(!d_.session.loaded)throw std::logic_error("Mission modules require the actual loaded World");
        world_.modules().dispatch().tick_modules(authority,elapsed,*this);
        d_.scene.frames().rethrow_actor_cleanup_failure();
        d_.hearing.rethrow_callback_failure();
    }
    void tick_physics(bool authority,std::uint32_t elapsed){physics_.tick(authority,elapsed);}
    void tick_runtime_actors(bool authority,std::uint32_t elapsed){actors_.tick_runtime(authority,elapsed);}
    void tick_scene_actors(bool authority,std::uint32_t elapsed){actors_.tick_scene(authority,elapsed);}
    void tick_camera(std::uint32_t elapsed){camera_.simulate(elapsed);}
    using Actor=GameSceneActors::Instance*;
    void sight_exclude_dog(Actor actor){runtime_.sight_exclude_dog(actor);}
    auto sight_dog_origin(Actor actor){return runtime_.sight_dog_origin(actor);}
    void sight_dog_marker_metric(Actor observer,Actor target,std::uint32_t& interval,float& score){
        runtime_.sight_dog_marker_metric(observer,target,interval,score);
    }
    auto sight_dog_head_matrix(Actor actor){return runtime_.sight_dog_head_matrix(actor);}
private:
    world_loading::World& world_;
    world_loading::Dependencies& d_;
    GamePhysicsFrames physics_;
    GameCameraLoading camera_;
    GameSceneActorFrames<world_loading::Dependencies,world_loading::Services,Runtime> actors_;
    Runtime& runtime_;
};

} //namespace hd2::host
#endif
