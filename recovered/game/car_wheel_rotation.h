#ifndef HD2_GAME_CAR_WHEEL_ROTATION_H
#define HD2_GAME_CAR_WHEEL_ROTATION_H

#include "game/car_cold_solver.h"

namespace hd2::game {

//00507A40: actual wheel/strip visual spin, wobble and authored base rotation.
//C0 is the displacement accumulated by00512390 and is cleared in native order.
//The middle-pair/opposite-side strip sampling and its extra radius division
//are preserved from the original, rather than replaced by wheel-speed guesses.
template<class Frames>void update_car_wheel_rotation(Frames& frames,const CarPhysics& physics,
        const CarBodyIntegration<Frames>& body,std::vector<CarWheelRuntime<Frames>>& wheels,
        const CarDrivetrainState& drive,const CarWheelQueryState& queries,bool wheel_spin,float elapsed){
    using Vector=typename Frames::Vector;
    constexpr float tau=6.283185005187988f;
    const float engine_turn=static_cast<float>((static_cast<long double>(tau)/car_gear_ratio(physics,drive.gear))*drive.engine_speed*elapsed);
    const auto dot=[](const Vector& a,const Vector& b){return (static_cast<long double>(a[0])*b[0]+static_cast<long double>(a[1])*b[1])+static_cast<long double>(a[2])*b[2];};
    bool strip_contact=false;std::array<float,2> strip_travel{};
    for(auto n=wheels.size();n;){const auto flags=wheels[--n].flags;if((flags&0x108u)==0x108u&&!(flags&0x400u)){
        const auto middle=wheels.size()/2;
        for(unsigned side=0;side!=2;++side){const auto& wheel=wheels.at(middle+side);strip_travel[side]=static_cast<float>(dot(wheel.vectors.at(0xb0),wheel.vectors.at(0xc0)));}
        strip_contact=true;break;
    }}
    for(auto n=wheels.size();n;){
        const auto index=--n;auto& wheel=wheels[index];const auto flags=wheel.flags;if(flags&0x400u)continue;
        auto& fields=wheel.scalars;
        const bool strip=(flags&0x100u)!=0;
        if(!(flags&0x40000000u)&&(strip||!(flags&0x20u))){
            const auto radius=fields.get(0x1d8);float change=0;bool advance=false;
            if(strip){
                if(!wheel_spin){
                    if(strip_contact)change=strip_travel[(index&1u)?0:1];
                    else{change=(queries.projected_radius/radius)*engine_turn;if(drive.gear<0)change=-change;}
                    change/=radius;advance=true;
                }else if(drive.gear>0||drive.gear==-1){change=(queries.projected_radius/radius)*engine_turn;if(drive.gear==-1)change=-change;advance=true;}
            }else if(!(flags&2u)){change=static_cast<float>(dot(wheel.vectors.at(0xc0),wheel.vectors.at(0xb0))/radius);advance=true;}
            else if(!(flags&0x80u)){
                if(flags&8u){change=static_cast<float>(dot(wheel.vectors.at(0xc0),wheel.vectors.at(0xb0))/radius);advance=true;}
                else if(drive.clutch<.10000000149011612f||!drive.gear){change=static_cast<float>(dot(body.pose().forward,wheel.vectors.at(0xc0))/radius);advance=true;}
                else if(drive.gear>0||drive.gear==-1){
                    //Native divides this no-contact engine temporary once
                    //here and again at00507FE7, unlike its flag80 branch.
                    const float temporary=(queries.projected_radius/radius)*engine_turn;
                    change=(drive.gear==-1?-temporary:temporary)/radius;advance=true;
                }
            }else if(drive.gear>0||drive.gear==-1){change=(queries.projected_radius/radius)*engine_turn;if(drive.gear==-1)change=-change;advance=true;}
            if(advance)fields.set(0x1f0,fields.get(0x1f0)-change);
            auto angle=fields.get(0x1f0);if(angle>tau)angle-=tau;else if(angle<0)angle+=tau;fields.set(0x1f0,angle);
            const auto spin=frames.car_axis_quaternion(Vector{{1,0,0,0}},angle);
            const auto wobble=frames.car_axis_quaternion(Vector{{0,0,1,0}},fields.get(0x22c));
            frames.car_steering_rotation(wheel.frame,car_quaternion_product(car_quaternion_product(wobble,spin),wheel.rotation));
        }
        wheel.vectors[0xc0]=Vector{};
    }
}
} //namespace hd2::game
#endif
