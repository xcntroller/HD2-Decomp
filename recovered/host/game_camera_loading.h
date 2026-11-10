#ifndef HD2_HOST_GAME_CAMERA_LOADING_H
#define HD2_HOST_GAME_CAMERA_LOADING_H
#include "host/game_world_loading.h"
#include "game/camera_collision_queries.h"
namespace hd2::host {
//Concrete00469300 World/input/query bindings. Camera, Players roster/selected/
//current, Human inline98 and FPV all belong to SAME containing allocation.
//Query scratch is temporary; real tree/buckets/resources/exclusions remain
//shared. No synthetic camera, input frame, target or collision success.
class GameCameraLoading {
public:
    explicit GameCameraLoading(world_loading::Owners& owners):d_(owners.dependencies()){}
    void simulate(std::uint32_t elapsed){
        if(!d_.session.loaded)throw std::logic_error("Camera simulation requires actual loaded mission stages");
        game::CameraCollisionQueries<EngineFrame*,world_loading::Scene::Descriptor,GameCollisionResources,GameActorServices>
            queries(d_.scene.collision_tree(),&d_.scene.buckets(),d_.scene.collision_resources(),d_.scene.frames());
        d_.camera.simulate(elapsed,d_.startup.application_flags(),d_.players.control(),d_.startup,*this,
            queries,d_.first_person.view(),d_.startup.camera_breathing_amount());
    }
    bool camera_input_enabled()const{return d_.session.words.get<std::uint8_t>(0x6e)!=0;}
    bool camera_pose_suppressed()const{return d_.session.words.get<std::uint8_t>(0x64)!=0;}
    auto* human_inline(world_loading::Human& human){
        return &d_.world_actors.template human_loading_owner<world_loading::Composition>(human).control();
    }
    auto* camera_target_human(EngineFrame* frame){return d_.scene.frames().camera_target_human(frame);}
    std::int32_t local_player_id()const{return d_.startup.local_player_identity();}
private:
    world_loading::Dependencies& d_;
};
} //namespace hd2::host
#endif
