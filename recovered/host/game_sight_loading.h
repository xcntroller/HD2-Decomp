#ifndef HD2_HOST_GAME_SIGHT_LOADING_H
#define HD2_HOST_GAME_SIGHT_LOADING_H

#include "host/game_scene_actors.h"
#include "game/sight_loading.h"
#include "game/human_sight_relations.h"
#include "game/human_perception.h"

namespace hd2::host {

//SAME Scene2 frame-data registry, no parallel actor map. World must provide
//actual module11 roster/005A84F0, non7 friendship and005A5250/005D5CB0
//receivers before registration.
template<class World>class GameSightLoading {
public:
    using Frame=GameSceneActors::Frame;using Actor=GameSceneActors::Instance*;
    using Human=game::HumanSceneActor<GameActorServices,GameSceneActors::Runtime>;
    Actor sight_actor(Frame frame) {
        if(!frame)return nullptr;
        const auto* data=frames_.find_actor_frame_data(frame);
        return data?reinterpret_cast<Actor>(data->words[0]):nullptr;
    }
    std::uint32_t sight_actor_type(Actor actor) const {if(!actor)throw std::logic_error("Sight requires an actual actor");return actor->base().type();}
    Frame sight_actor_model(Actor actor) const {if(!actor)throw std::logic_error("Sight requires an actual model owner");return actor->base().frame();}
    std::uint32_t sight_human_kind(Actor actor) const{return human(actor).properties.field19c;}
    bool human_sight_friendly(Actor observer,Actor target){
        if((world_.mode_flags()&7)!=7)return world_.network_human_sight_friendly(human(observer),*target);
        return game::offline_human_sight_friendly(human(observer),*target,world_.human_roster(),
            [&](Human& value){return world_.human_sight_alignment(value);});
    }
    bool human_sight_hostile(Actor observer,Actor target){return !human_sight_friendly(observer,target);} //005A0980 repeats005A0AB0.
    void human_sight_changed(Actor observer,bool lost,std::uint32_t target){game::notify_human_sight(human(observer),lost,target,source_,*this);}
    auto* human_sight_state(Human& value){return value.loading_state;}
    bool human_sight_friendly(Human& observer,Human& target){return human_sight_friendly(&observer,&target);}
    void human_sight_enemy_acquired(Human& observer,Human& target){if(auto* state=observer.loading_state)state->sight_enemy_acquired(target);}
    void human_sight_enemy_lost(Human& observer,Human& target){if(auto* state=observer.loading_state)state->sight_enemy_lost(target);}
    void human_sight_dead_acquired(Human& observer,Human& target){if(auto* state=observer.loading_state)state->sight_dead_acquired(target);}
    std::int32_t sight_dog_health(GameSceneActors::Instance& actor) const {
        const auto* dog=dynamic_cast<const game::DogSceneActor<GameActorServices>*>(&actor);
        if(!dog)throw std::runtime_error("Sight Dog target has a different actual actor family");
        const auto bits=dog->properties.fields4a0[0];std::int32_t health;std::memcpy(&health,&bits,4);return health;
    }
    void player_sight_dog(Human& observer,GameSceneActors::Instance& target){world_.player_sight_dog(observer,target);}
    void dog_sight_changed(Actor observer,bool lost,std::uint32_t target){world_.dog_sight_changed(*observer,lost,target);}
    using Source=game::SightLoading<GameSceneActors,GameSightLoading,Actor>;
    GameSightLoading(GameSceneActors& frames,World& world):frames_(frames),world_(world),source_(frames,*this){}
    Source& source(){return source_;}
private:
    static Human& human(Actor actor) {
        auto* result=dynamic_cast<Human*>(actor);
        if(!result)throw std::runtime_error("Sight Human receiver has a different actual actor family");
        return *result;
    }
    GameSceneActors& frames_;World& world_;Source source_;
};
} //namespace hd2::host
#endif
