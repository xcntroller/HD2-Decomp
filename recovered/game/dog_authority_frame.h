#ifndef HD2_GAME_DOG_AUTHORITY_FRAME_H
#define HD2_GAME_DOG_AUTHORITY_FRAME_H
#include "game/dog_body_orders.h"
namespace hd2::game {
//005D5620. Authority mode2 suppresses only history; produced530 and final
//death374 gate the whole frame. Target getters are intentionally repeated and
//reloaded around Body callbacks. No target snapshot substitutes for them.
template<class Dog,class Queries>void advance_dog_authority(Dog& dog,std::uint32_t elapsed,Queries& q){
    if(!dog.suspended530)throw std::runtime_error("Dog authority requires actual005D4DB0 flag530 producer");
    if(*dog.suspended530||dog.body_actions.death374==4)return;
    if(!(q.mode_flags()&2u))q.history(elapsed);
    if(!dog.elapsed55c)throw std::runtime_error("Dog authority requires actual55C elapsed producer");
    dog.elapsed55c=dog_order_signed(static_cast<std::uint32_t>(*dog.elapsed55c)+elapsed);
    if(q.model_y()<-500.f)q.below_world_damage();
    if(dog.require_controller().dog_control_kind()==1)q.manual_inputs();
    dog.require_controller().dog_control_authority(elapsed);
    q.body_actions();q.body_animation();q.head_timer(elapsed);
    if(dog.require_controller().dog_control_target()){
        auto* target=dog.require_controller().dog_control_target();if(!target)throw std::runtime_error("Dog bite enlargement lost native second target");q.target_geometry(*target,true);
    }
    q.body(elapsed);
    if(dog.require_controller().dog_control_target()){
        auto* target=dog.require_controller().dog_control_target();if(!target)throw std::runtime_error("Dog bite restoration lost native second target");q.target_geometry(*target,false);
    }
    const auto action=dog_order_signed(dog.body_actions.current364);
    q.head_request(action>=7&&action<=12?0x18u:4u,action>=7&&action<=12?1u:0u);
    q.nearby_sounds();
}
}
#endif
