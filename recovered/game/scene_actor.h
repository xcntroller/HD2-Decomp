#ifndef HD2_GAME_SCENE_ACTOR_H
#define HD2_GAME_SCENE_ACTOR_H

#include "game/actor_base.h"
#include "game/serialized_bytes.h"

namespace hd2::game {

//Construction/loading interface only, matching the Scene2 factory's separate
//actor+2C and+34 calls. Simulation/render/script receivers are not represented
//by successful defaults. The containing world cannot activate through this
//interface alone; those actual receivers remain required.
template<class Backend>
class SceneActor {
public:
    using Frame=typename Backend::Frame;
    using Base=ActorBase<Backend>;
    SceneActor(Backend& backend,std::uint32_t type) : backend_(backend),base_(backend,type) {}
    virtual ~SceneActor()=default;
    SceneActor(const SceneActor&)=delete;
    SceneActor& operator=(const SceneActor&)=delete;
    Base& base() {return base_;}
    const Base& base() const {return base_;}
    virtual bool initialize(Frame)=0;
    virtual void load(SerializedBytes&)=0;
    std::uint8_t loading_activation_passes=0; //Source partial-prefix guard, not a native actor flag/readiness/roster.
protected:
    Backend& backend_;
    Base base_;
};

} //namespace hd2::game
#endif
