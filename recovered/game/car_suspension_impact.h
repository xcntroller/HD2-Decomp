#ifndef HD2_GAME_CAR_SUSPENSION_IMPACT_H
#define HD2_GAME_CAR_SUSPENSION_IMPACT_H

#include "game/car_contact_callbacks.h"
#include <optional>

namespace hd2::game {

struct CarSuspensionImpact {
    CarContactVector impulse{},linear_change{},angular_change{},relative_velocity{};
    float normal_speed=0,magnitude=0;
};

//00512390's suspension hard-stop impulse. The caller applies the original
//compression/slope/flag40 gate, accumulates these changes, then averages across
//all impacted wheels. Damage/sound/passenger notifications remain separate
//actual receivers; returning an impulse does not replace those callbacks.
template<class Frames>std::optional<CarSuspensionImpact> car_suspension_impact(Frames& frames,
        const CarContactState& state,const CarContactVector& up,const CarContactVector& point,
        CarContactVector normal,const CarContactVector& previous_normal){
    const auto dot=[](const auto& a,const auto& b){return (static_cast<long double>(a[0])*b[0]+static_cast<long double>(a[1])*b[1])+static_cast<long double>(a[2])*b[2];};
    if(dot(previous_normal,normal)<.8600000143051147f){
        const float projection=static_cast<float>(dot(up,normal));
        for(unsigned axis=0;axis!=3;++axis)normal[axis]=projection*up[axis];
    }
    CarContactResponse response{};car_other_contact_response(frames,state,point,normal,response);
    const float speed=static_cast<float>(dot(response.velocity,normal));
    if(!(speed<0))return std::nullopt;
    const auto& angular=response.angular_response;const auto& offset=response.offset;
    CarContactVector crossed{};
    for(unsigned axis=0;axis!=3;++axis){const auto next=(axis+1)%3,last=(axis+2)%3;crossed[axis]=static_cast<float>(static_cast<long double>(angular[next])*offset[last]-static_cast<long double>(angular[last])*offset[next]);}
    const auto inverse_mass=1/static_cast<long double>(state.mass918);
    auto denominator=(static_cast<long double>(crossed[1])*normal[1]+static_cast<long double>(crossed[2])*normal[2])+static_cast<long double>(crossed[0])*normal[0]+inverse_mass;
    if(std::abs(denominator)<9.999999747378752e-06)denominator=9.999999747378752e-06f; //DOUBLE00810950, FLOAT00810744.
    const auto scale=std::abs(static_cast<long double>(speed))/denominator;
    CarSuspensionImpact result;result.normal_speed=std::abs(speed);result.relative_velocity=response.velocity;
    for(unsigned axis=0;axis!=3;++axis){result.impulse[axis]=static_cast<float>(normal[axis]*scale);result.linear_change[axis]=static_cast<float>(inverse_mass*result.impulse[axis]);}
    CarContactResponse impulse{};car_other_contact_response(frames,state,point,result.impulse,impulse);
    for(unsigned axis=0;axis!=3;++axis)result.angular_change[axis]=-impulse.angular_response[axis];
    const float square=static_cast<float>((static_cast<long double>(result.impulse[1])*result.impulse[1]+static_cast<long double>(result.impulse[2])*result.impulse[2])+static_cast<long double>(result.impulse[0])*result.impulse[0]);
    std::int32_t bits;std::memcpy(&bits,&square,4);
    result.magnitude=bits<0x322bcc77?0.f:std::abs(square-1.f)<9.99999993922529e-09f?1.f:static_cast<float>(std::sqrt(static_cast<long double>(square)));
    return result;
}
} //namespace hd2::game
#endif
