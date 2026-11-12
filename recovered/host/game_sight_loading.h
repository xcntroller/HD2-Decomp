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
    void adjust_human_sight_alignment(Human& human,float amount){world_.adjust_human_sight_alignment(human,amount);}
    void refresh_human_perception(Human& human){game::refresh_human_perception(human,source_,*this);}
    using Source=game::SightLoading<GameSceneActors,GameSightLoading,Actor>;
    std::optional<float> sight_human_metric(Actor observer,Actor target){return game::human_sight_human_metric(human(observer),human(target),frames_,world_).score;}
    template<class DogQueries>std::optional<float> sight_dog_metric(Actor observer,Actor target,DogQueries& dogs){return game::human_sight_dog_metric(human(observer),target,frames_,world_,dogs).score;}
    GameSightLoading(GameSceneActors& frames,World& world,float& work_clock):frames_(frames),world_(world),work_clock_(work_clock),source_(frames,*this){}
    template<class Queries>void tick(std::uint32_t elapsed,Queries& queries){
        struct Pass {
            GameSightLoading& owner;Source& source;Queries& queries;
            void sight_observe(typename Source::Observer& observer,std::uint32_t mask,std::uint32_t target,std::uint32_t elapsed){source.observe(observer,mask,target,elapsed,*this);}
            std::int32_t sight_human_health(Actor actor){const auto bits=human(actor).properties.stats_ec[0];std::int32_t health;std::memcpy(&health,&bits,4);return health;}
            std::optional<float> sight_human_metric(Actor observer,Actor target){return owner.sight_human_metric(observer,target);}
            std::optional<float> sight_dog_metric(Actor observer,Actor target){return owner.sight_dog_metric(observer,target,queries);}
            typename Source::HumanProbe sight_human_probe(typename Source::Observer& observer,std::uint32_t target,std::uint32_t elapsed){return source.probe_human(observer,target,elapsed,*this);}
            bool sight_dog_probe(typename Source::Observer& observer,std::uint32_t target,std::uint32_t elapsed){return source.probe_dog(observer,target,elapsed,*this);}
            std::uint32_t sight_time(){return owner.world_.sight_time();}
            typename Source::Vector sight_position(Frame frame){return owner.world_.sight_position(frame);}
            typename Source::Vector sight_human_origin(Actor actor){return owner.world_.sight_human_origin(actor);}
            Actor sight_human_attachment(Actor actor){return owner.world_.sight_human_attachment(actor);}
            void sight_exclude_human(Actor actor){owner.world_.sight_exclude_human(actor);}
            void sight_restore_human(Actor actor){owner.world_.sight_restore_human(actor);}
            std::optional<typename World::SightRayHit> sight_ray(const typename Source::Vector& origin,const typename Source::Vector& delta,std::uint32_t layers,std::uint32_t flags){return owner.world_.sight_ray(origin,delta,layers,flags);}
            void sight_exclude_dog(Actor actor){queries.sight_exclude_dog(actor);}
            void sight_restore_dog(){owner.world_.sight_restore_dog();}
            std::optional<typename Source::Vector> sight_dog_origin(Actor actor){return queries.sight_dog_origin(actor);}
            void sight_dog_marker_metric(Actor observer,Actor target,std::uint32_t& interval,float& score){queries.sight_dog_marker_metric(observer,target,interval,score);}
        } pass{*this,source_,queries};
        source_.tick(elapsed,work_clock_,pass);
    }
    bool force_human_target(Human& observer,Human& target){
        struct Pass {
            GameSightLoading& owner;
            auto sight_human_origin(Actor actor){return owner.world_.sight_human_origin(actor);}
            auto sight_human_attachment(Actor actor){return owner.world_.sight_human_attachment(actor);}
            void sight_exclude_human(Actor actor){owner.world_.sight_exclude_human(actor);}
            void sight_restore_human(Actor actor){owner.world_.sight_restore_human(actor);}
            auto sight_human_metric(Actor first,Actor second){return owner.sight_human_metric(first,second);}
            auto sight_position(Frame frame){return owner.world_.sight_position(frame);}
            auto sight_ray(const typename Source::Vector& origin,const typename Source::Vector& delta,std::uint32_t layers,std::uint32_t flags){return owner.world_.sight_ray(origin,delta,layers,flags);}
        }pass{*this};
        return source_.force_human_target(static_cast<std::int32_t>(observer.field258),target.field25c,pass);
    }
    Source& source(){return source_;}
private:
    static Human& human(Actor actor) {
        auto* result=dynamic_cast<Human*>(actor);
        if(!result)throw std::runtime_error("Sight Human receiver has a different actual actor family");
        return *result;
    }
    GameSceneActors& frames_;World& world_;float& work_clock_;Source source_;
};
} //namespace hd2::host
#endif
