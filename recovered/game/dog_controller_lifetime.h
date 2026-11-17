#ifndef HD2_GAME_DOG_CONTROLLER_LIFETIME_H
#define HD2_GAME_DOG_CONTROLLER_LIFETIME_H
#include "game/scene_actor.h"
namespace hd2::game {
//The actual Actor534 owner. Unrecovered controller bodies remain required
//virtual receivers; no numeric presence flag or successful idle control exists.
template<class Frames>class DogControllerLifetime {
public:
    using Actor=SceneActor<Frames>;
    virtual ~DogControllerLifetime()=default;
    virtual std::int32_t dog_control_kind()const=0; //14.
    virtual Actor* dog_control_target()=0; //24, actual88 (not follow18).
    virtual long double dog_control_speed()=0; //28, ST0.
    virtual void dog_control_dead()=0; //005CE6E0.
    virtual void dog_control_damage(std::uint32_t,Actor*)=0; //54.
};
}
#endif
