#ifndef HD2_GAME_MISSION_SCRIPT_CHECKPOINT_TELEPORT_H
#define HD2_GAME_MISSION_SCRIPT_CHECKPOINT_TELEPORT_H
#include "game/human_joint_math.h"
#include <string>

namespace hd2::game {
//0041AC08, FRM_TeleportNearCheckpoint. The FIRST/SECOND named-point datasets
//and Car/Human owners belong to the containing World; no copied runtime state.
inline HumanVector script_checkpoint_direction(const HumanVector& target,const HumanVector& origin){
    HumanVector direction{};for(unsigned axis=0;axis!=3;++axis)direction[axis]=target[axis]-origin[axis];
    const auto extended=(static_cast<long double>(direction[0])*direction[0]+
        static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[1])*direction[1];
    const float square=static_cast<float>(extended);
    const float difference=static_cast<float>(extended-1); //FST square, then extended FSUB/FST argument0041CF60.
    constexpr float epsilon=9.99999993922529e-09f;
    if(std::abs(difference)>=epsilon){
        if(square>=epsilon){const auto inverse=1.L/std::sqrt(static_cast<long double>(square));
            for(unsigned axis=0;axis!=3;++axis)direction[axis]=static_cast<float>(direction[axis]*inverse);
        }else{const unsigned axis=direction[0]!=0?0u:direction[2]!=0?2u:1u;direction[axis]=direction[axis]<0?-1.f:1.f;}
    }
    return direction; //Fourth lane is native temporary padding, not a navigation coordinate.
}
template<class Frames,class Actor,class World>
void teleport_script_checkpoint(Frames& frames,Actor* actor,typename Frames::Frame frame,
        const std::string& position_name,const std::string& direction_name,World& world){
    if(!frame)return;
    const bool car=actor&&actor->base().type()==4;
    const auto position=world.script_checkpoint_point(position_name,car);if(!position)return;
    if(car)world.script_suspend_teleport_car(*actor); //004E95B0(0) BEFORE changing pose.
    if(actor){
        actor->base().world_position(*position);
        if(car)frames.car_suspension_position(actor->base().frame(),*position); //Inline0041D110, XYZ only.
        else frames.car_camera_position(actor->base().frame(),*position); //Actual frame08.
        if(actor->base().type()==3)world.script_refresh_teleport_human(*actor); //0041D800 BEFORE direction.
    }else frames.car_camera_position(frame,*position);
    if(!direction_name.empty())if(const auto target=world.script_checkpoint_point(direction_name,car)){
        const auto direction=script_checkpoint_direction(*target,*position);
        if(actor){frames.car_direction(actor->base().frame(),direction);actor->base().world_direction(direction);}
        else frames.car_camera_direction(frame,direction,0.f); //Native generic frame0C, not actor1C.
    }
    frames.update_scene_object(actor?actor->base().frame():frame);
    if(car)world.script_reset_teleport_car(*actor); //Save22BF,004E9DB0(0,true),004F2530(saved,false).
}
} //namespace hd2::game
#endif
