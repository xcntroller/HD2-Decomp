#ifndef HD2_GAME_DOG_SIGHT_GEOMETRY_H
#define HD2_GAME_DOG_SIGHT_GEOMETRY_H
#include "game/human_joint_owners.h"
#include <optional>
#include <stdexcept>
namespace hd2::game {
template<class Dog,class Frames>HumanMatrix dog_sight_head_matrix(Dog& dog,Frames& frames){ //0056FC80 target544.
    if(!dog.sight_marker544)throw std::runtime_error("Dog Sight requires actual005D4DB0 marker544 producer");
    const auto marker=*dog.sight_marker544;if(!marker)throw std::runtime_error("Dog Sight reaches native NULL544 marker");
    return frames.actor_world_matrix(marker);
}
template<class Dog,class Frames>std::optional<HumanVector> dog_sight_origin(Dog& dog,Frames& frames){ //0046E3B0.
    if(!dog.sight_marker544)throw std::runtime_error("Dog Sight requires actual005D4DB0 marker544 producer");
    if(!*dog.sight_marker544){
        const auto model=dog.base().frame();if(!model)return {};
        (void)frames.actor_world_matrix(model); //Native null-marker fallback still continues to dereference fresh544.
    }
    const auto matrix=dog_sight_head_matrix(dog,frames);
    return HumanVector{{matrix[12],matrix[13],matrix[14],matrix[15]}};
}
}
#endif
