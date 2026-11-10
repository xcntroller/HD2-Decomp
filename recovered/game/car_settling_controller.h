#ifndef HD2_GAME_CAR_SETTLING_CONTROLLER_H
#define HD2_GAME_CAR_SETTLING_CONTROLLER_H

#include "game/car_cold_solver.h"
#include "game/car_steering.h"

namespace hd2::game {

template<class Frames>struct CarSettlingControllerState {
    using Vector=typename Frames::Vector;
    Vector previous_velocity{},position_checkpoint{};
    float previous_engine_speed=0,previous_speed=0,previous_rolling_speed=0;
    float first_filtered=1,second_filtered=1,desired_clutch=1,previous_steering=0;
    bool controlled=false,requests_update=false,frame_simulated=false; //2291/229E/229F.
    std::uint32_t inactive_milliseconds=0;
    explicit CarSettlingControllerState(const CarBodyIntegration<Frames>& body):position_checkpoint(body.pose().position){}
};

//00509AA0's actual engine-off/unoccupied branch, including ordinary sleeping
//and held updates after0050B180 clears22C0. Running, occupied and shift-requested
//control requires the wider original controller. BODYCOL uses the SAME owner
//passed by the containing Car, even during settling when its10 gate is clear.
template<class Frames,class Queries,class Receivers,class Collision>void update_car_cold(Frames& frames,
        typename Frames::Frame model,const CarPhysics& physics,const CarTyres& tyres,
        CarBodyIntegration<Frames>& body,std::vector<CarWheelRuntime<Frames>>& wheels,
        CarDrivetrainState& drive,CarColdSolverState<Frames>& state,CarSteeringState& steering,
        CarSettlingControllerState<Frames>& controller,Queries& queries,Receivers& receivers,
        std::int32_t update_hold,std::uint32_t milliseconds,std::uint32_t layers,std::uint32_t flags,Collision& collision){
    if(drive.running||drive.gear!=1||drive.changing_gear||steering.requested!=0||steering.current!=0)
        throw std::runtime_error("Car cold controller requires its actual unoccupied zero-input state");
    if(!milliseconds)milliseconds=1;
    const float elapsed=static_cast<float>(static_cast<long double>(milliseconds)*.0010000000474974513f);
    const auto& fields=physics.scalars();
    const auto& forward=body.pose().forward;const auto& velocity=body.motion().velocity;
    const float rolling=static_cast<float>(std::abs((static_cast<long double>(car_gear_ratio(physics,drive.gear))/fields.get(0x8d8))*
        ((static_cast<long double>(forward[0])*velocity[0]+static_cast<long double>(forward[1])*velocity[1])+static_cast<long double>(forward[2])*velocity[2])));
    drive.body_flags&=0xeffffffdu;
    if(update_hold>0)drive.body_flags|=0x10000000u;
    if(drive.body_flags&0x20000000u)drive.body_flags|=1u;
    if(state.mass_changed){controller.inactive_milliseconds=0;drive.body_flags|=1u;
        state.stable_milliseconds=std::min(state.stable_milliseconds,std::int32_t{0});}
    const auto kind=static_cast<unsigned>(fields.get(0x550));
    if(kind==2)update_car_steering(frames,model,physics,body,wheels,steering,elapsed,state.speed);
    if(drive.clutch_target<controller.desired_clutch)drive.clutch_target=std::min(controller.desired_clutch,static_cast<float>(static_cast<long double>(elapsed)*fields.get(0x900)+drive.clutch_target));
    else if(controller.desired_clutch<drive.clutch_target)drive.clutch_target=std::max(controller.desired_clutch,static_cast<float>(static_cast<long double>(drive.clutch_target)-static_cast<long double>(elapsed)*fields.get(0x900)));
    //With both genuine input fields6FC/700 zero, either native acceleration
    //filter arm produces zero from their nonnegative508/50C values.
    drive.body_flags&=~0x1000000u;controller.first_filtered=controller.second_filtered=0;
    drive.throttle=drive.second_throttle=0;
    float first_drive=0,second_drive=0;
    if(kind==0||kind==2){const auto force=car_drive_force(physics,drive,elapsed,0,state.speed,velocity);first_drive=second_drive=static_cast<float>(force*.5f*drive.transfer);}
    else if(kind==1){first_drive=static_cast<float>(car_drive_force(physics,drive,elapsed,0,state.speed,velocity)*.5f*drive.transfer);second_drive=static_cast<float>(car_drive_force(physics,drive,elapsed,0,state.speed,velocity)*.5f*drive.transfer);}
    const auto ticks=static_cast<std::uint32_t>(state.stable_milliseconds)+milliseconds;std::memcpy(&state.stable_milliseconds,&ticks,4);
    if(std::abs(static_cast<long double>(steering.current)-controller.previous_steering)>static_cast<float>(.03500000014901161)){
        controller.previous_steering=steering.current;controller.inactive_milliseconds=0;
        drive.body_flags=(drive.body_flags&~2u)|1u;state.stable_milliseconds=-500;
    }
    if(!(drive.body_flags&1u)||(drive.body_flags&0x10000000u)){
        if(fields.get(0x524)<0||controller.requests_update){
            if(!(drive.body_flags&0x10000000u))drive.body_flags=(drive.body_flags&~2u)|1u;
            controller.inactive_milliseconds=0;state.stable_milliseconds=std::min(state.stable_milliseconds,std::int32_t{0});
        }else{
            controller.previous_engine_speed=drive.engine_speed;
            update_car_drivetrain(physics,drive,elapsed,first_drive+second_drive,0,state.speed,state.speed_angle,body.pose().forward,body.motion().velocity,receivers);
            if(fields.get(0x914)+1.f<drive.engine_speed){drive.transmission&=~0x10u;drive.body_flags|=1u;}
            else{drive.clutch=0;drive.transmission=(drive.transmission&~1u)|0x10u;controller.frame_simulated=false;}
            if(!(drive.body_flags&1u)&&(!controller.controlled||(drive.body_flags&0x1000000u))&&state.stable_milliseconds>1000){
                drive.body_flags|=2u;return;
            }
        }
    }
    if((kind==0||kind==2)&&(drive.transmission&0x20u)){
        const auto original=drive.transmission;auto transmission=original&0xffffffefu;drive.transmission=transmission;
        const float low=fields.get(0x8ec)+1.f;
        bool engage=false;
        if(low<drive.engine_speed){
            if(drive.engine_speed<=fields.get(0x8f4)+8.f||!(original&8u)){if(!(original&4u))engage=true;}
            else{if(!(original&3u))drive.clutch=drive.clutch_target;drive.transmission=original&0xffffffebu;}
        }else if(low<=rolling){transmission=original&0xffffffebu;engage=true;}
        else{
            drive.changing_gear=false;drive.clutch=0;
            //Input714=0 <=524 on the applied loading parameter sets. Other
            //input thresholds must not silently select the driving shift arm.
            if(fields.get(0x524)<0)throw std::runtime_error("Car cold input selects unresolved automatic driving control");
            drive.transmission=(original&0xffffffe8u)|0x10u;
        }
        if(engage){drive.transmission=transmission|1u;drive.clutch=drive.clutch_target;}
        transmission=drive.transmission;
        if(!(transmission&4u))drive.transmission=(transmission&0xfffffff3u)|1u;
        else{
            drive.changing_gear=false;
            if(drive.engine_speed<=fields.get(0x8f4)){drive.clutch=0;drive.transmission=transmission&0xfffffff7u;}
            else if(!(transmission&8u)){
                drive.engine_speed=fields.get(0x8f4)+1.f;
                drive.clutch=std::min(1.f,static_cast<float>(std::abs(static_cast<long double>(first_drive)+second_drive)/(car_gear_ratio(physics,drive.gear)*fields.get(0x8f8)*drive.transfer)));
                drive.transmission=(transmission&0xfffffffcu)|8u;
            }else if(fields.get(0x8f4)<rolling){drive.clutch=drive.clutch_target;drive.transmission=(transmission&0xfffffff3u)|1u;}
        }
        for(auto n=wheels.size();n;){const auto wheel_flags=wheels[--n].flags;if((wheel_flags&2u)&&(wheel_flags&0x20u)&&!(wheel_flags&0x400u)){drive.clutch=0;break;}}
    }
    controller.previous_engine_speed=drive.engine_speed;controller.previous_velocity=body.motion().velocity;
    controller.previous_speed=state.speed;controller.previous_rolling_speed=state.rolling_speed;
    drive.body_flags|=0x1000000u;
    if(!(drive.body_flags&0x10000000u))run_car_substeps(physics,body,wheels,state.substeps,elapsed,state.speed,drive.brake,first_drive,second_drive,
        [&](float delta,const std::array<float,3>& drag,float first,float second){car_cold_solver_step(frames,model,physics,tyres,body,wheels,drive,state,queries,receivers,delta,drag,first,second,0,0,layers,flags,collision);});
    if(!(drive.body_flags&8u)){
        bool contact=false;for(auto n=wheels.size();n;){const auto f=wheels[--n].flags;if((f&8u)&&!(f&0x400u)){contact=true;break;}}
        if(!contact)drive.body_flags|=8u;
    }else{
        bool all=true;for(auto n=wheels.size();n;){const auto f=wheels[--n].flags;if((!(f&8u)&&!(f&0x400u))||(f&0x10u)){all=false;break;}}
        if(all)drive.body_flags&=~8u;
    }
    long double moved=0;for(unsigned axis=0;axis!=3;++axis){const float difference=body.pose().position[axis]-controller.position_checkpoint[axis];moved+=static_cast<long double>(difference)*difference;}
    if(moved>=.02250000089406967f||(drive.body_flags&0x20000000u)){controller.position_checkpoint=body.pose().position;controller.inactive_milliseconds=0;}
    else controller.inactive_milliseconds+=milliseconds;
    if(!(drive.body_flags&2u))state.stable_milliseconds=std::min(state.stable_milliseconds,std::int32_t{0});
    if(state.stable_milliseconds>1000||controller.inactive_milliseconds>4000)drive.body_flags&=~1u;
}

//004F67B0's restricted entry is retained for callers which have not yet bound
//the ordinary BODYCOL owner. The real activation must use update_car_cold above.
template<class Frames,class Queries,class Receivers>void update_car_settling(Frames& frames,
        typename Frames::Frame model,const CarPhysics& physics,const CarTyres& tyres,
        CarBodyIntegration<Frames>& body,std::vector<CarWheelRuntime<Frames>>& wheels,
        CarDrivetrainState& drive,CarColdSolverState<Frames>& state,CarSteeringState& steering,
        CarSettlingControllerState<Frames>& controller,Queries& queries,Receivers& receivers,
        std::int32_t update_hold,std::uint32_t milliseconds,std::uint32_t layers,std::uint32_t flags){
    if(!state.mass_changed||update_hold)throw std::runtime_error("Car settling entry requires its actual initial state");
    if(state.substeps.flags&0x10u)throw std::runtime_error("Car ordinary update requires its actual BODYCOL controller");
    CarLoadingWithoutBodyCollision collision;
    update_car_cold(frames,model,physics,tyres,body,wheels,drive,state,steering,controller,queries,receivers,
        update_hold,milliseconds,layers,flags,collision);
}
} //namespace hd2::game
#endif
