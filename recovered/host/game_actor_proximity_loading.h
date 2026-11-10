#ifndef HD2_HOST_GAME_ACTOR_PROXIMITY_LOADING_H
#define HD2_HOST_GAME_ACTOR_PROXIMITY_LOADING_H

#include "host/startup_session.h"
#include "game/actor_proximity_loading.h"

namespace hd2::host {
//Actual process008AED54 owner/Frames/005A83F0 composition. Startup state/timers
//outlive this world wrapper; clear registrations after actor retirement.
class GameActorProximityLoading {
public:
    using Actor=GameSceneActors::Instance*;
    using Human=game::HumanSceneActor<GameActorServices,GameSceneActors::Runtime>;
    using Source=game::ActorProximityLoading<GameSceneActors,GameActorProximityLoading,Actor>;
    GameActorProximityLoading(GameSceneActors& frames,StartupSession& startup)
        :startup_(startup),source_(startup.actor_proximity_state(),frames,*this){}
    std::int32_t add_human_proximity(Human& human,unsigned kind){return add_actor(&human,kind);}
    std::int32_t add_actor(Actor actor,unsigned kind){return source_.add(actor,kind,startup_.application_flags());}
    void remove_actor(std::int32_t index){(void)source_.remove(index,startup_.application_flags());}
    void remove_human_proximity(std::int32_t index){remove_actor(index);}
    void release_human_proximity(std::int32_t index){remove_human_proximity(index);} //0059E3E0's same006DFE50 endpoint.
    bool speed_kind(std::int32_t index,unsigned kind){return source_.speed_kind(index,kind,startup_.application_flags());}
    void tick100(){source_.tick100();}void clear()noexcept{source_.clear();}Source& source(){return source_;}
    void human_proximity_changed(Actor actor,Actor other,float distance,std::uint32_t next,std::uint32_t previous) {
        auto* human=dynamic_cast<Human*>(actor);
        if(!human)throw std::runtime_error("Proximity range5 feedback requires an actual Human");
        game::notify_human_proximity(*human,other,distance,next,previous);
    }
private:
    StartupSession& startup_;Source source_;
};
} //namespace hd2::host
#endif
