#ifndef HD2_GAME_DOG_CONTROLLER_ACTIONS_H
#define HD2_GAME_DOG_CONTROLLER_ACTIONS_H
#include "game/dog_controller_vector.h"
#include "game/dog_body_state.h"
namespace hd2::game {
//005C1E60, XYZ equality and LAST stance gate precede direction publication.
//Unproduced native W stays source padding; no new numerical value is claimed.
template<class Body,class Orders>void request_dog_body_direction(Body& body,const Orders& orders,const DogControllerVector& direction){
    const auto& xyz=direction.position();
    if(xyz[0]==body.direction180[0]&&xyz[1]==body.direction180[1]&&xyz[2]==body.direction180[2])return;
    if(orders.next_stance()==1)return;
    for(unsigned n=0;n!=3;++n)body.direction180[n]=xyz[n];
    if(direction.w)body.direction180[3]=*direction.w;
}
//005C6C70, SAME process008AC4A8 BSS counter; native zero remains zero after
//the actual actor-direction rotation and Body direction request.
template<class Dog,class Frames>void turn_dog_controller_body(Dog& dog,std::int32_t& count,Frames& frames){
    if(count){count=dog_order_signed(static_cast<std::uint32_t>(count)-1u);return;}
    auto direction=dog.base().world_direction();
    frames.car_transform_in_place(direction,frames.human_axis_rotation({{0,1,0,0}},2.799999952316284f));
    DogControllerVector value;value.assign(direction);request_dog_body_direction(dog.body,dog.body_orders,value);count=0;
}
//005CEC60, actual buffer slots, priority requests and native call ordering.
template<class State,class Queries>void publish_dog_controller_actions(State& state,Queries& queries){
    state.movement30.xyz=std::array<float,3>{{0,0,0}};state.movement30.w.reset();
    queries.action(3);
    const auto first=[&](unsigned n){if(!state.first60)throw std::runtime_error("Dog action publication requires actual first60 buffer");return state.first60->at(n);};
    const auto second=[&](unsigned n){if(!state.second64)throw std::runtime_error("Dog action publication requires actual second64 buffer");return state.second64->at(n);};
    if(first(0)!=0.f)queries.action(1);
    if(first(1)!=0.f)queries.action(0);
    if(second(6)!=0.f)queries.action(2);
    queries.posture(first(5)!=0.f?1u:0u);
    if(second(59)!=0.f)queries.action(5);
    if(second(60)!=0.f||first(4)!=0.f)queries.action(4);
    if(second(9)!=0.f){
        if(queries.next_stance()==1)queries.stance(0);
        else if(queries.next_stance()==0)queries.stance(1);
    }
    if(second(60)!=0.f)queries.turn_body();
}
}
#endif
