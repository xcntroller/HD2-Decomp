#ifndef HD2_HOST_GAME_PHYSICS_FRAMES_H
#define HD2_HOST_GAME_PHYSICS_FRAMES_H

#include "host/game_world_loading.h"
#include "game/collision_ray.h"

namespace hd2::host {
//Module2 frame binding to SAME actual object lists/handles, loaded tree/buckets,
//resource geometry and query owners. Point0/extended-ray1/sphere2/fading4 are supplied. Other
//families remain required when active; no invented success/sleep/delete path.
class GamePhysicsFrames {
public:
    using Vector=game::PhysicsObjectMotion::Vector;
    explicit GamePhysicsFrames(world_loading::Owners& owners):d_(owners.dependencies()){}
    void tick(bool authority,std::uint32_t elapsed){
        require_loaded();d_.world_actors.physics().tick(authority,elapsed,*this);
        d_.scene.frames().rethrow_actor_cleanup_failure();
    }
    std::optional<game::PhysicsObjectMotionHit> physics_point_query(const Vector& start,const Vector& displacement){
        require_loaded();
        game::CollisionRay<EngineFrame*,world_loading::Scene::Descriptor,GameCollisionResources,GameActorServices>
            ray(&d_.scene.collision_tree(),&d_.scene.buckets(),d_.scene.collision_resources(),d_.scene.frames());
        const auto hit=ray.ray(start,displacement,0,0,true);
        return hit?std::optional<game::PhysicsObjectMotionHit>({hit->position,hit->normal,hit_flags(*hit)}):std::nullopt;
    }
    std::optional<game::PhysicsObjectMotionHit> physics_sphere_query(const Vector& start,const Vector& displacement,float radius){
        require_loaded();const auto hit=d_.sweep.sweep(start,displacement,radius,0,0,true);
        return hit?std::optional<game::PhysicsObjectMotionHit>({hit->position,hit->normal,hit_flags(*hit)}):std::nullopt;
    }
    template<class Object>bool advance_other_physics_object(Object& object,float){
        throw std::runtime_error("Required actual Physics object update is unavailable for type "+std::to_string(object.parameters().type));
    }
private:
    template<class Hit>std::uint32_t hit_flags(const Hit& hit)const{
        if(hit.dynamic_identity)return hit.dynamic_identity->flags;
        if(!hit.static_identity)throw std::logic_error("Physics collision lost its actual descriptor/tree identity");
        const auto& id=*hit.static_identity;const auto& tree=d_.scene.collision_tree();
        return id.kind?tree.shapes.at(id.kind-0x80).at(id.index).flags:tree.triangles.at(id.index).flags;
    }
    void require_loaded()const{if(!d_.session.loaded)throw std::logic_error("Physics frame requires actual loaded World/tree");}
    world_loading::Dependencies& d_;
};
} //namespace hd2::host
#endif
