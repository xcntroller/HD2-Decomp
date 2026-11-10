#ifndef HD2_HOST_GAME_PHYSICS_FRAMES_H
#define HD2_HOST_GAME_PHYSICS_FRAMES_H

#include "host/game_world_loading.h"
#include "game/collision_ray.h"

namespace hd2::host {
//Module2 frame binding to SAME actual object lists/handles, loaded tree/buckets,
//resource geometry and query owners. Types0..5 supply actual frame bodies;
//type5 requires real contact-seeded state. Tree replacement/pickup notification
//remain required positive contact branches; no invented success/sleep/delete.
class GamePhysicsFrames {
public:
    using Vector=game::PhysicsObjectMotion::Vector;
    explicit GamePhysicsFrames(world_loading::Owners& owners):d_(owners.dependencies()){}
    void tick(bool authority,std::uint32_t elapsed){
        require_loaded();d_.world_actors.physics().tick(authority,elapsed,*this);
        d_.scene.frames().rethrow_actor_cleanup_failure();
    }
    std::optional<game::PhysicsObjectMotionHit> physics_point_query(const Vector& start,const Vector& displacement,std::uint32_t layers=0,std::uint32_t flags=0,bool normal=true){
        require_loaded();
        game::CollisionRay<EngineFrame*,world_loading::Scene::Descriptor,GameCollisionResources,GameActorServices>
            ray(&d_.scene.collision_tree(),&d_.scene.buckets(),d_.scene.collision_resources(),d_.scene.frames());
        const auto hit=ray.ray(start,displacement,layers,flags,normal);
        return hit?std::optional<game::PhysicsObjectMotionHit>({hit->position,hit->normal,hit_flags(*hit)}):std::nullopt;
    }
    auto physics_pivot_query(const Vector& start,const Vector& displacement){return physics_point_query(start,displacement,0xffffffffu);}
    auto physics_tipping_query(const Vector& start,const Vector& displacement){return physics_point_query(start,displacement,0xffffffffu,0x200000u,false);}
    void physics_exclude(world_loading::Scene::Descriptor& object){require_loaded();d_.scene.exclusions().single(&object);}
    void physics_reset_exclusions(){d_.scene.exclusions().clear();}
    std::optional<game::PhysicsObjectMotionHit> physics_sphere_query(const Vector& start,const Vector& displacement,float radius){
        require_loaded();const auto hit=d_.sweep.sweep(start,displacement,radius,0,0,true);
        return hit?std::optional<game::PhysicsObjectMotionHit>({hit->position,hit->normal,hit_flags(*hit)}):std::nullopt;
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
