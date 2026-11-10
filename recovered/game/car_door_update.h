#ifndef HD2_GAME_CAR_DOOR_UPDATE_H
#define HD2_GAME_CAR_DOOR_UPDATE_H

#include "game/car_auxiliary_geometry.h"
#include "game/car_wheel_runtime.h"

namespace hd2::game {

template<class Frames>struct CarDoorRuntime {
    using Auxiliary=CarAuxiliaryGeometry<Frames>;
    float angle=0,target=0,rate=0; //24/28/30, zeroed in004E2E30.
    float window_x=0,window_y=0;
    std::uint16_t flags=0;bool window_moving=false,window_closing=false;
    explicit CarDoorRuntime(const typename Auxiliary::Door& door)
        :window_x(door.window.x),window_y(door.window.y),flags(door.flags){}
};

//00501750, through actual seats' borrowed door indices (reverse seat order).
//0051B660 is a mandatory sound receiver only when a moving door reaches its
//native opening/closing gate. This helper never creates a command or sound.
template<class Frames,class Sounds>void update_car_doors(Frames& frames,
        const CarAuxiliaryGeometry<Frames>& geometry,std::vector<CarDoorRuntime<Frames>>& runtime,
        float elapsed,Sounds& sounds){
    using Auxiliary=CarAuxiliaryGeometry<Frames>;
    for(auto n=geometry.seats().size();n;){const auto seat=--n;const auto index=geometry.seats()[seat].door;
        if(index==Auxiliary::no_door)continue;
        const auto& door=geometry.doors().at(index);auto& state=runtime.at(index);
        if(!(state.flags&2u))continue;
        if(!door.frame)throw std::runtime_error("Moving Car door has no actual frame");
        if(state.target<=state.angle){
            state.angle=static_cast<float>(static_cast<long double>(state.angle)-static_cast<long double>(elapsed)*state.rate);
            if(state.angle<=state.target){state.angle=state.target;state.flags&=0xfffdu;sounds.door(seat,false);}
        }else{
            if(state.angle<9.999999747378752e-05f)sounds.door(seat,true);
            state.angle=static_cast<float>(static_cast<long double>(elapsed)*state.rate+state.angle);
            if(state.target<state.angle){state.angle=state.target;state.flags&=0xfffdu;}
        }
        float angle=door.position[0]<0?-state.angle:state.angle;
        if((state.flags&4u)&&!(state.flags&8u))angle=-angle;
        const auto rotation=car_quaternion_product(frames.car_axis_quaternion({{0,1,0,0}},angle),door.rotation);
        frames.car_steering_rotation(door.frame,rotation);frames.update_scene_object(door.frame);
    }
}

//00501AB0: the .1 lateral offset and strict endpoint comparisons are native.
//Detached glass cancels motion before touching visibility or frame position.
template<class Frames,class Parts>void update_car_windows(Frames& frames,
        const CarAuxiliaryGeometry<Frames>& geometry,std::vector<CarDoorRuntime<Frames>>& runtime,
        const Parts& parts,float elapsed){
    using Auxiliary=CarAuxiliaryGeometry<Frames>;
    for(auto n=geometry.seats().size();n;){const auto index=geometry.seats()[--n].door;
        if(index==Auxiliary::no_door)continue;
        const auto& window=geometry.doors().at(index).window;auto& state=runtime.at(index);
        if(!state.window_moving)continue;
        if(window.part!=0xffff&&(parts.at(window.part).flags&1u)){state.window_moving=false;continue;}
        if(!window.frame)throw std::runtime_error("Moving Car window has no actual frame");
        const float amount=elapsed*window.radius;
        const auto x=[&]{const auto offset=(1.L-2.L*(static_cast<long double>(state.window_y)-window.lower_y))*.10000000149011612f;
            return static_cast<float>(window.x>0?window.x+offset:window.x-offset);};
        if(!state.window_closing){
            state.window_y+=amount;frames.object_visible(window.frame,true);state.window_x=x();
            if(window.y<state.window_y){state.window_x=window.x;state.window_y=window.y;state.window_moving=false;}
        }else{
            state.window_y-=amount;
            if(window.lower_y<=state.window_y)state.window_x=x();
            else{state.window_moving=false;state.window_y=window.lower_y;frames.object_visible(window.frame,false);}
        }
        auto position=window.position;position[0]=state.window_x;position[1]=state.window_y;
        frames.car_suspension_position(window.frame,position);frames.update_scene_object(window.frame);
    }
}
} //namespace hd2::game
#endif
