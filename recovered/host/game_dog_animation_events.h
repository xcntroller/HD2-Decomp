#ifndef HD2_HOST_GAME_DOG_ANIMATION_EVENTS_H
#define HD2_HOST_GAME_DOG_ANIMATION_EVENTS_H
#include "host/game_actor_services.h"
#include "game/dog_animation_events.h"
namespace hd2::host {template<class Dog>using GameDogAnimationEvents=game::DogAnimationEvents<GameActorServices,Dog>;}
#endif
