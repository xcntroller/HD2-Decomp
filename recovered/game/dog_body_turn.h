#ifndef HD2_GAME_DOG_BODY_TURN_H
#define HD2_GAME_DOG_BODY_TURN_H
#include "game/human_motion_velocity.h"
namespace hd2::game {
//005C89C0: smooth the supplied Body180 direction toward actual Actor40. This
//does not publish a model or actor direction. Controller-kind read is late.
template<class Actor,class Frames,class Receivers>
void turn_dog_body_direction(HumanVector& value,Actor& actor,Frames& frames,Receivers& receivers){
    const auto supplied=value;const auto actual=actor.base().world_direction();
    const auto angle=human_motion_planar_angle(supplied,actual);const float stored_angle=static_cast<float>(angle);
    const auto magnitude=std::abs(angle);if(magnitude<9.999999747378752e-05)return;
    const auto factor=receivers.dog_control_kind()==1?3.f:actor.body_orders.next_posture()==1?.009999999776482582f:.0024999999441206455f;
    auto step=(magnitude+1.L)*factor;if(stored_angle>0)step=-step;step*=100.f;
    const float stored_step=static_cast<float>(step);
    if(std::abs(step)<magnitude){
        auto result=frames.car_transform(actual,frames.human_axis_rotation({{0,1,0,0}},stored_step));
        result[1]=actual[1];value=result;
    }
}
}
#endif
