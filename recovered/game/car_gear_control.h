#ifndef HD2_GAME_CAR_GEAR_CONTROL_H
#define HD2_GAME_CAR_GEAR_CONTROL_H

#include "game/car_drivetrain.h"

namespace hd2::game {

//004F2030, RET4. Refusal and damaged-drivetrain no-op preserve780/878/910/
//22C6. An already requested gear succeeds BEFORE kind1's reverse restriction.
inline bool request_car_gear(const CarPhysics& physics,CarDrivetrainState& drive,
        std::uint32_t features,float drivetrain_health,float& direction_timer,std::int32_t requested){
    if(requested< -1||requested>physics.gear_count())return false;
    if(requested==drive.requested_gear)return true;
    if(physics.scalars().get(0x550)==1&&requested<0)return false;
    if((features&0x4000u)&&!(drivetrain_health>0))return true;
    if(requested!=-1&&physics.scalars().get(0x221c)<
            (static_cast<long double>(car_gear_ratio(physics,requested))/car_gear_ratio(physics,drive.gear))*drive.engine_speed)return false;
    direction_timer=0;
    if(drive.transmission&0x20u){drive.shift_in_progress=true;drive.transmission=(drive.transmission&~1u)|2u;}
    drive.requested_gear=requested;return true;
}

//004F3840, RET16. RPM bounds are the SAME retained parameter words5/4 at
//coreFF0/FEC, not converted RPS fields914/221C. Refusal leaves the output intact.
inline bool evaluate_car_gear_force(const CarPhysics& physics,float rpm,float& output,
        std::int32_t gear,bool include_drag){
    if(gear< -1||gear>physics.gear_count())return false;
    if(!physics.applied_parameters())throw std::logic_error("Car gear force requires actual applied parameters");
    const auto& parameters=*physics.applied_parameters();
    if(static_cast<long double>(rpm)+.0010000000474974513f<parameters.real(5)||
            static_cast<long double>(rpm)-.0010000000474974513f>parameters.real(4))return false;
    const float speed=static_cast<float>(static_cast<long double>(rpm)*.01666666753590107f);
    auto force=car_engine_torque(physics.scalars(),speed,1.f);
    if(include_drag)force=car_engine_torque(physics.scalars(),speed,0.f)+static_cast<float>(force);
    if(gear)force*=car_gear_ratio(physics,gear);
    output=static_cast<float>(force);return true;
}

} //namespace hd2::game
#endif
