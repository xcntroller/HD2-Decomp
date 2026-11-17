#ifndef HD2_GAME_DOG_SIGHT_NOTIFICATIONS_H
#define HD2_GAME_DOG_SIGHT_NOTIFICATIONS_H
#include "game/dog_damage.h"
#include "game/mission_objective_status.h"
namespace hd2::game {
//005D5CB0, same native574 spotted list and current534 receiver.
template<class Dog,class Frames,class World>void notify_dog_sight(Dog& dog,bool lost,std::uint32_t index,Frames& frames,World& world){
    if(!dog.field534||(dog.require_controller().dog_control_state().flags20&0x40000000u))return;
    auto* target=world.dog_sight_target(index);
    if(!target)throw std::runtime_error("Dog Sight callback reaches native NULL target record");
    if(target->kind!=0)return;
    auto* actor=target->actor;if(!actor)throw std::runtime_error("Dog Sight callback requires actual target actor");
    if(!lost){
        if(world.dog_sighted_human_health(*actor)<=0)return;
        const auto own_model=dog.base().frame();(void)frames.actor_world_matrix(own_model);
        const auto other=frames.actor_world_matrix(actor->base().frame());
        const auto own=frames.dog_cached_world_matrix(own_model);
        const auto x=static_cast<long double>(other[12])-own[12],y=static_cast<long double>(other[13])-own[13],z=static_cast<long double>(other[14])-own[14];
        const float square=static_cast<float>((x*x+y*y)+z*z);std::int32_t bits;std::memcpy(&bits,&square,4);
        const auto distance=bits<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(square));
        if(distance>dog_signed_word(dog.properties.fields4a0[2]))return; //Native unordered continues.
        if(actor->base().type()!=3||!world.dog_sighted_enemy_kind(*actor))return;
        //005D5C70's containing0069BFD0 is the actual retail RET output.
        retail_objective_status_output();dog.spotted574.push_back(actor);
        if(dog.field534)dog.field534->dog_control_sight_acquired(actor);
    }else if(actor->base().type()==3&&world.dog_sighted_enemy_kind(*actor)){
        for(auto n=dog.spotted574.size();n;)if(dog.spotted574[--n]==actor)dog.spotted574.erase(dog.spotted574.begin()+n);
        retail_objective_status_output();if(dog.field534)dog.field534->dog_control_sight_lost(actor);
    }
}
}
#endif
