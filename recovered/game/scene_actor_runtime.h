#ifndef HD2_GAME_SCENE_ACTOR_RUNTIME_H
#define HD2_GAME_SCENE_ACTOR_RUNTIME_H

#include <cstdint>

namespace hd2::game {

//Required live consumers borrowed by concrete loading owners. No default
//implementations: the containing world must bind its actual mode, script,
//sound/effect and player-record owners before constructing these actors.
template<class Frame,class Vector>
class SceneActorRuntime {
public:
    virtual ~SceneActorRuntime()=default;
    virtual std::uint32_t mode_flags() const=0;
    virtual bool switch_has_context(Frame)=0;
    virtual void notify_switch_state(Frame,std::uint32_t)=0;
    virtual void play_switch_sound(std::uint32_t,const Vector&)=0;
    virtual void release_human_proximity(std::int32_t)=0; //006DFE50,008AED54; distinct from GEffects.
    virtual void clear_human_player_slot(std::uint32_t)=0;
};

} //namespace hd2::game
#endif
