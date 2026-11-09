#ifndef HD2_GAME_HUD_ACTOR_IDENTITY_VIEW_H
#define HD2_GAME_HUD_ACTOR_IDENTITY_VIEW_H
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
namespace hd2::game {
//0089B3B0/B4 is Scene2 global0089B3A0+10/+14 IDENTITY slots, not its
//active list.007D8AB3/00475DD0 prove the receiver. No copied/padded registry:
//fresh actual count/lookup sees reserved32 slots and later holes/changes.
template<class SceneActors>
class HudActorIdentityView {
public:
    explicit HudActorIdentityView(SceneActors& actors):actors_(actors){}
    std::size_t size()const{return actors_.scene_actor_identity_count();}
    auto* operator[](std::size_t index)const {
        if(index>static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))throw std::out_of_range("HUD actor identity index");
        auto* actor=actors_.scene_actor_identity(static_cast<std::int32_t>(index));
        return actor?&actor->base():nullptr;
    }
private:
    SceneActors& actors_;
};
} //namespace hd2::game
#endif
