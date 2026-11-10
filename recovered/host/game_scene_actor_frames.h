#ifndef HD2_HOST_GAME_SCENE_ACTOR_FRAMES_H
#define HD2_HOST_GAME_SCENE_ACTOR_FRAMES_H

#include "host/game_scene_actors.h"

namespace hd2::host {

//Actual Scene2 frame facade: SAME registered actors, buckets, proximity and
//Human state owners as loading/activation. Movement must be the real008AADD0
//005647C0 receiver; Derived must provide actual Human/Dog and runtime11 ticks.
//There is no replacement list, successful empty movement or cold Human tick.
template<class Dependencies,class Movement,class Derived>class GameSceneActorFrames {
public:
    using Actor=GameSceneActors::Instance;
    using Human=GameSceneActors::Human;
    GameSceneActorFrames(Dependencies& dependencies,Movement& movement,Derived& derived)
        :d_(dependencies),movement_(movement),derived_(derived){}
    void tick_scene(bool authority,std::uint32_t elapsed){d_.scene.frames().tick_scene_actors(authority,elapsed,*this);}
    void tick_runtime(bool authority,std::uint32_t elapsed){d_.world_actors.runtime().tick(authority,elapsed,derived_);}
    std::uint32_t mode_flags()const{return d_.startup.application_flags();}
    bool human_remote_ai_enabled(){return d_.startup.human_remote_ai_enabled();}
    void flush_retired_scene_actors(std::uint32_t elapsed){
        d_.scene.frames().flush_scene_actor_removals(elapsed,d_.world_actors,d_.scene.buckets());
    }
    void advance_actor_proximity100(){d_.proximity.tick100();}
    void advance_human_movement(std::uint32_t elapsed,const std::vector<Actor*>& active){movement_.advance(elapsed,active);}
    void update_scene_actor(Actor& actor,bool authority,std::uint32_t elapsed){
        game::update_loading_scene_actor(actor,authority,elapsed,d_.world_actors,derived_);
    }
    game::SceneHumanFrameSummary human_frame_summary(Actor& actor){
        auto* human=dynamic_cast<Human*>(&actor);
        if(!human)throw std::runtime_error("Scene2 Human frame summary has a different actual actor family");
        return game::summarize_frame_human(*human,*this);
    }
    bool human_unavailable(const Human& human)const { //005A6380, same actual B0/health/kind gates.
        std::int32_t health;const auto bits=human.properties.stats_ec[0];std::memcpy(&health,&bits,4);
        if(health<1||!human.loading_state||human.properties.field19c==1)return false;
        return (human.loading_state->words().template get<std::uint32_t>(0x50)&0x4000000u)!=0;
    }
private:
    Dependencies& d_;Movement& movement_;Derived& derived_;
};
} //namespace hd2::host
#endif
