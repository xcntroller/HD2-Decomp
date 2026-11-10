#ifndef HD2_GAME_CAR_DRIVETRAIN_H
#define HD2_GAME_CAR_DRIVETRAIN_H

#include "game/car_physics.h"

namespace hd2::game {

//Dynamic values initialized by004EA780 (audible by004E2900), separate from004ED270's immutable
//coefficients. The containing solver supplies current body speed/velocity and
//forward direction before these functions run. No standalone simulation here.
struct CarDrivetrainState {
    float engine_speed=0,engine_load=0,clutch=1,clutch_target=1,torque_amount=0;
    float governor=1000,health=1,transfer=1,throttle=0,second_throttle=0,brake=0;
    std::int32_t gear=1,requested_gear=1;
    std::uint32_t transmission=0x20,body_flags=1;
    bool running=false,audible=true,changing_gear=false;
};

inline float car_gear_ratio(const CarPhysics& physics,std::int32_t gear){
    if(gear < -1||gear>physics.gear_count())throw std::runtime_error("Car drivetrain gear exceeds actual parameter table");
    return physics.scalars().get(static_cast<std::size_t>(0x888+gear*4));
}

//0050D5C0, original piecewise torque curve. Interpolation is evaluated in
//extended precision; descending spans retain native subtraction order.
inline long double car_engine_torque(const CarPhysicsScalars& fields,float speed,float throttle){
    const auto get=[&](unsigned offset){return fields.get(offset);};
    const long double at=std::max(speed,get(0x914));long double torque;
    const auto interpolate=[&](unsigned first_speed,unsigned last_speed,unsigned first_torque,unsigned last_torque){
        const auto amount=(at-get(first_speed))/(static_cast<long double>(get(last_speed))-get(first_speed));
        const long double first=get(first_torque),last=get(last_torque);
        return last<first?first-(first-last)*amount:(last-first)*amount+first;
    };
    if(at<=get(0x774))torque=interpolate(0x914,0x774,0x764,0x768);
    else if(at<=get(0x778))torque=get(0x768);
    else if(at<=get(0x77c))torque=interpolate(0x778,0x77c,0x768,0x76c);
    else torque=interpolate(0x77c,0x221c,0x76c,0x770);
    const auto loss=(static_cast<long double>(speed)/get(0x221c))*get(0x8e8);
    return (loss+torque)*throttle-loss;
}

//0050D2C0, including the genuine engine-off return. Body speed/velocity are
//current solver outputs, not constructor defaults or actor-local estimates.
inline long double car_drive_force(const CarPhysics& physics,CarDrivetrainState& state,
        float elapsed,float throttle,float speed,const std::array<float,4>& velocity){
    const auto get=[&](unsigned offset){return physics.scalars().get(offset);};
    if(throttle<get(0x524)&&state.engine_speed<get(0x914)+2.f)throttle=get(0x524);
    if(get(0x221c)<state.engine_speed||state.governor<speed)throttle=0;
    state.body_flags&=0xfffffffbu;
    if(!state.running){
        const float square=static_cast<float>((static_cast<long double>(velocity[2])*velocity[2]+static_cast<long double>(velocity[1])*velocity[1])+static_cast<long double>(velocity[0])*velocity[0]);
        std::int32_t bits;std::memcpy(&bits,&square,4);
        if(bits>0x322bcc76&&(std::abs(square-1.f)<9.99999993922529e-09f||std::sqrt(static_cast<long double>(square))>=.5f))return 0;
        if(state.gear)state.body_flags|=4u;
        return 0;
    }
    const auto torque=car_engine_torque(physics.scalars(),state.engine_speed,throttle*state.health);
    long double amount=0;
    if(get(0x96c)<torque){
        if(torque<=get(0x970))amount=(torque-get(0x96c))/((static_cast<long double>(get(0x970))-get(0x96c))+.5f);
        else amount=1;
    }
    const auto rise=static_cast<long double>(elapsed)*get(0x974);
    if(amount>rise+state.torque_amount)amount=static_cast<float>(rise)+static_cast<long double>(state.torque_amount);
    else{
        elapsed*=get(0x978);
        if(amount<static_cast<long double>(state.torque_amount)-elapsed)amount=static_cast<long double>(state.torque_amount)-elapsed;
    }
    state.torque_amount=std::clamp(static_cast<float>(amount),0.f,1.f);
    auto force=torque*car_gear_ratio(physics,state.gear);
    if(state.governor-1.f<speed&&force>=0)force=-1;
    if(force<=0&&state.clutch>.8999999761581421f&&state.gear&&speed<.5f)state.body_flags|=4u;
    if(speed>get(0x6f0)+2.f)state.brake=.5f;
    return force;
}

//0050D760, final engine/clutch integration used even during cold settling.
//Audio is mandatory when a real audible shift selects0051A480; no default
//successful sound receiver is provided. Caller supplies actual body vectors.
template<class Audio>void update_car_drivetrain(const CarPhysics& physics,CarDrivetrainState& state,
        float elapsed,float drive,float reaction,float speed,float angle,
        const std::array<float,4>& forward,const std::array<float,4>& velocity,Audio& audio){
    const auto get=[&](unsigned offset){return physics.scalars().get(offset);};
    const float capacity=static_cast<float>(static_cast<long double>(get(0x8f8))*state.transfer*state.clutch);
    const float inverse=1.f/car_gear_ratio(physics,state.gear);
    float drag=inverse*state.engine_load;
    const float rolling=static_cast<float>(std::abs(((static_cast<long double>(forward[0])*velocity[0]+static_cast<long double>(forward[1])*velocity[1])+static_cast<long double>(forward[2])*velocity[2])*car_gear_ratio(physics,state.gear)/get(0x8d8)));
    float resistance=reaction*inverse;if(capacity<resistance)resistance=capacity;
    float force=inverse*drive-resistance;if(capacity<drag)drag=capacity;
    bool lock=std::abs(force)<drag;
    const auto kind=static_cast<unsigned>(get(0x550));
    if(kind==0||kind==2){
        if((angle>1.5707963705062866f&&state.gear>0)||(angle<1.5707963705062866f&&state.gear<0&&speed>.0001f))force-=drag;
        else if(state.gear){if(rolling<state.engine_speed)force-=drag;if(state.engine_speed<rolling){lock=true;force+=drag;}}
    }else if(kind==1){if(rolling<state.engine_speed)force-=drag;if(state.engine_speed<rolling)force+=drag;}
    state.engine_speed=static_cast<float>((static_cast<long double>(elapsed)*force)/(static_cast<long double>(get(0x738))*6.283185005187988f)+state.engine_speed);
    const auto flags=state.transmission;
    if(!(flags&0x16u)&&lock&&state.gear){
        if((state.engine_speed<rolling&&force<0)||(rolling<state.engine_speed&&force>0))state.engine_speed=rolling;
    }
    if(get(0x221c)<state.engine_speed)state.engine_speed=get(0x221c)+.0010000000474974513f;
    if(state.engine_speed<0)state.engine_speed=0;
    if(flags&2u){
        state.clutch=static_cast<float>(static_cast<long double>(state.clutch)-static_cast<long double>(elapsed)*get(0x900));
        if(state.clutch>0)return;
        state.clutch=0;state.transmission=(flags&0xfffffffdu)|1u;
        if(state.audible)audio.clutch_changed();
        state.gear=state.requested_gear;return;
    }
    if(!(flags&1u)){
        if(!(flags&0x24u)||state.clutch>state.clutch_target)state.clutch=state.clutch_target;
        return;
    }
    state.gear=state.requested_gear;
    state.clutch=static_cast<float>(static_cast<long double>(elapsed)*get(0x8fc)+state.clutch);
    if(state.clutch>state.clutch_target){
        state.clutch=state.clutch_target;state.transmission=flags&0xfffffffcu;
        if(!state.changing_gear)return;
        state.changing_gear=false;state.throttle=state.second_throttle=.800000011920929f;
    }else if(state.clutch>1){
        state.clutch=1;state.transmission=flags&0xfffffffcu;
        if(!state.changing_gear)return;
        state.changing_gear=false;state.throttle=state.second_throttle=.800000011920929f;
    }
    if(state.changing_gear&&rolling<(get(static_cast<unsigned>(0x7cc+state.gear*4))-get(static_cast<unsigned>(0x810+state.gear*4)))*.15000000596046448f+get(static_cast<unsigned>(0x810+state.gear*4))){
        state.throttle=state.second_throttle=1;state.clutch=state.clutch_target;
        state.transmission&=0xfffffffcu;state.changing_gear=false;
    }
}
} //namespace hd2::game
#endif
