#ifndef HD2_GAME_HUMAN_LOADING_OWNER_FACTORY_H
#define HD2_GAME_HUMAN_LOADING_OWNER_FACTORY_H

#include "game/human_loading_composition.h"
#include "game/world_loading_dependency_link.h"
#include <memory>

namespace hd2::game {

//Actual per-Human construction boundary used by WorldActors. All references
//come from the containing world dependency set; no extra process, registry,
//speech cache, input manager, roster or presentation owner is constructed here.
//WorldActors publishes the returned lifetime before invoking either activation
//pass. Dependencies and all borrowed resources must outlive those lifetimes.
template<class Composition,class Dependencies,class WorldServices>
class HumanLoadingOwnerFactory {
public:
    explicit HumanLoadingOwnerFactory(WorldServices& world):world_(world){}
    void bind_dependencies(Dependencies& dependencies){dependencies_.bind(dependencies);}
    void unbind_dependencies(Dependencies& dependencies)noexcept{dependencies_.unbind(dependencies);}
    bool dependencies_bound()const noexcept{return dependencies_.bound();}
    WorldServices& world_services(){return world_;} //Borrow the SAME containing receiver, never a second factory/world.
    HumanLoadingOwnerFactory(Dependencies& dependencies,WorldServices& world)
        :dependencies_(dependencies),world_(world){}
    template<class Frames,class Actors,class Human>
    std::unique_ptr<Composition> make_human_loading_owner(Frames& frames,Actors& actors,Human& human){
        auto& d=dependencies_.get();
        if(&frames!=&d.scene.frames()||&actors!=&d.world_actors)
            throw std::logic_error("Human factory requires the SAME containing scene and WorldActors");
        using Factory=typename Composition::Factory;
        typename Factory::Dependencies inline_dependencies{
            frames,d.scene.buckets(),d.scene.exclusions(),d.startup.human_morph_tables(),d.startup.human_physics_globals(),
            d.audio,d.audio.random_source(),d.animation_events,d.scene.effects(),d.presentation,
            world_,actors.items(),d.animations,d.catalogue,d.scene.collision_resources(),d.sweep,d.scene.collision_tree()};
        return std::make_unique<Composition>(inline_dependencies,human,d.inputs,d.second_tables,&d.navigation,
            d.input_callbacks,d.startup.human_ai_action_pools(),d.contexts,d.profiles,d.audio,d.sight.source(),
            d.hearing,d.proximity,d.camera,d.players.control());
    }
private:
    WorldLoadingDependencyLink<Dependencies> dependencies_;WorldServices& world_;
};

} //namespace hd2::game
#endif
