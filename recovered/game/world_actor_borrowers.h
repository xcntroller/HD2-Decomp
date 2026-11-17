#ifndef HD2_GAME_WORLD_ACTOR_BORROWERS_H
#define HD2_GAME_WORLD_ACTOR_BORROWERS_H
#include <cstddef>
#include <cstdint>
namespace hd2::game {
//UI loans may name Scene2 OR Runtime actors in the SAME containing World.
//Never dereference an unvalidated incoming pointer or combine their numeric IDs.
template<class Scene,class Runtime,class Actor>bool world_owns_actor(Scene& scene,Runtime& runtime,const Actor* actor){
    if(!actor)return false;
    for(std::size_t n=0;n<scene.scene_actor_identity_count();++n)if(scene.scene_actor_identity(static_cast<std::int32_t>(n))==actor)return true;
    return runtime.owns(actor);
}
template<class Scene,class Runtime>auto world_actor_for_base(Scene& scene,Runtime& runtime,const void* identity){
    for(std::size_t n=0;n<scene.scene_actor_identity_count();++n){auto* actor=scene.scene_actor_identity(static_cast<std::int32_t>(n));if(actor&&&actor->base()==identity)return actor;}
    return runtime.first_owned_forward([&](auto& actor){return &actor.base()==identity;});
}
} //namespace hd2::game
#endif
