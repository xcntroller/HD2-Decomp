#ifndef HD2_GAME_CAR_DRIVE_CONTROLLER_H
#define HD2_GAME_CAR_DRIVE_CONTROLLER_H

#include "game/car_automatic_gears.h"
#include "game/car_settling_controller.h"

namespace hd2::game {

//00509AA0, including running input, automatic selection, clutch/traction,
//sleep/hold and contact policy. State is the SAME initialized Car lifetime.
//The mandatory step is0050DE60: production still rejects its unrecovered
//powered branch. No successful solver/finalizer or readiness is manufactured.
template<class Frames,class State,class Receivers,class Step>void update_car_controller(Frames& frames,
        typename Frames::Frame model,const CarPhysics& physics,State& state,Receivers& receivers,
        std::uint32_t milliseconds,Step&& step){
    if(!milliseconds)milliseconds=1;
    auto& drive=state.drive;auto& solver=state.solver;auto& controller=state.controller();
    auto& input=state.control_requests;auto& body=state.body();auto& wheels=state.wheels;
    auto& steering=state.steering;auto& transition=state.finalizer.engine_transition;
    const auto& fields=physics.scalars();const auto get=[&](int offset){return fields.get(static_cast<unsigned>(offset));};
    const auto ratio=[&](int gear){return car_gear_ratio(physics,gear);};
    const auto request=[&](int gear){return request_car_gear(physics,drive,solver.substeps.flags,state.health.drivetrain,state.reset_fields.field780,gear);};
    const auto dot=[](const auto& a,const auto& b){return (static_cast<long double>(a[0])*b[0]+static_cast<long double>(a[1])*b[1])+static_cast<long double>(a[2])*b[2];};
    const auto& forward=body.pose().forward;const auto& velocity=body.motion().velocity;
    const float rolling=static_cast<float>(std::abs((static_cast<long double>(ratio(drive.gear))/get(0x8d8))*dot(forward,velocity)));
    if(state.brake_latch)input.first=input.second=0;
    if(state.reset_fields.field22c1){state.reset_fields.field22c2=transition;transition=false;input.first=input.second=0;}
    if(state.update_hold>0)drive.body_flags|=0x10000000u;else drive.body_flags&=~0x10000000u;
    if(solver.speed_angle>1.5707963705062866f&&drive.gear>0&&input.first>.30000001192092896f&&
            (drive.transmission&4u)&&(drive.transmission&0x20u)&&solver.speed>1){drive.brake=1;drive.body_flags|=0x1000000u;}
    drive.body_flags&=~2u;if(drive.body_flags&0x20000000u)drive.body_flags|=1u;
    if(solver.mass_changed){controller.inactive_milliseconds=0;drive.body_flags|=1u;solver.stable_milliseconds=std::min(solver.stable_milliseconds,std::int32_t{0});}
    const float ticks=static_cast<float>(milliseconds);
    const float elapsed=static_cast<float>(static_cast<long double>(ticks)*.0010000000474974513f);
    const auto kind=static_cast<unsigned>(get(0x550));const bool ordinary=kind==0||kind==2;
    if(kind==2)update_car_steering(frames,model,physics,body,wheels,steering,elapsed,solver.speed);
    if(transition){
        const auto empty=[&]{state.fuel=0;state.health.destroyed=true;input.first=input.second=0;};
        if(state.fuel<=.00009999999747378752f)empty();
        else{
            const float throttle=kind==1?static_cast<float>((std::abs(static_cast<long double>(drive.throttle))+
                std::abs(static_cast<long double>(drive.second_throttle)))*.5f):drive.throttle;
            state.fuel=static_cast<float>(static_cast<long double>(state.fuel)-
                (static_cast<long double>(throttle)*.800000011920929f+
                 (static_cast<long double>(drive.engine_speed)/get(0x221c))*.20000000298023224f)*get(0x784)*elapsed);
            if(state.fuel<.00009999999747378752f)empty();
        }
    }
    drive.throttle=input.first;drive.second_throttle=input.second;
    if(!input.flag229c){
        if(drive.clutch_target<=controller.desired_clutch){
            if(drive.clutch_target<controller.desired_clutch){
                drive.clutch_target=static_cast<float>(static_cast<long double>(elapsed)*get(0x900)+drive.clutch_target);
                if(controller.desired_clutch<drive.clutch_target)drive.clutch_target=controller.desired_clutch;
            }
        }else{
            drive.clutch_target=static_cast<float>(static_cast<long double>(drive.clutch_target)-static_cast<long double>(elapsed)*get(0x900));
            if(drive.clutch_target<controller.desired_clutch)drive.clutch_target=controller.desired_clutch;
        }
    }else drive.clutch_target=controller.desired_clutch;
    bool allowed=true;
    if(get(0x774)<drive.engine_speed){
        const float excess=static_cast<float>(static_cast<long double>(drive.engine_speed)-
            (static_cast<long double>(solver.speed)/get(0x8d8))*ratio(drive.gear));
        if(std::abs(excess)>3.f&&excess>0)allowed=false;
    }
    if(drive.gear&&allowed)for(const auto& wheel:wheels){
        if((wheel.flags&0x102u)&&!(wheel.flags&0x400u)&&(wheel.flags&0x80u)&&
                drive.engine_speed<static_cast<long double>(get(0x221c))-2.f){allowed=false;break;}
    }
    auto next_index=drive.gear+1;
    select_car_automatic_gear(physics,state,receivers,elapsed,rolling,allowed,next_index);
    controller.previous_first_input=drive.throttle;controller.previous_second_input=drive.second_throttle;
    drive.body_flags&=~0x1000000u;
    if(input.flag229d&&drive.shift_in_progress&&!(drive.transmission&0x10u)){
        if(drive.transmission&2u)drive.throttle=drive.second_throttle=0;
        drive.throttle*=drive.clutch;drive.second_throttle*=drive.clutch;
    }
    float acceleration=static_cast<float>((dot(velocity,forward)-dot(forward,controller.previous_velocity))/solver.substeps.previous_delta);
    if(drive.gear<0)acceleration=-acceleration;
    const auto rise=[&](float value,float& filtered,bool direct){
        if(value<=filtered||direct)filtered=value;
        else{filtered=static_cast<float>(static_cast<long double>(elapsed)*get(0x514)+filtered);if(value<filtered)filtered=value;}
    };
    const auto fall=[&](float value,float& filtered){
        if(value<filtered)filtered=value;
        else{filtered=static_cast<float>(static_cast<long double>(filtered)-static_cast<long double>(elapsed)*get(0x514));if(filtered<0)filtered=0;}
    };
    if(acceleration<=get(0x510)||(drive.transmission&4u)){
        rise(drive.throttle,drive.first_filtered,(drive.transmission&4u)!=0);
        rise(drive.second_throttle,drive.second_filtered,false); //Native second side has no bit4 bypass.
    }else{fall(drive.throttle,drive.first_filtered);fall(drive.second_throttle,drive.second_filtered);}
    if(drive.transmission&0x10u){drive.first_filtered=drive.throttle;drive.second_filtered=drive.second_throttle;}
    drive.throttle=drive.first_filtered;drive.second_throttle=drive.second_filtered;
    float first=0,second=0;
    if(ordinary){
        const auto force=car_drive_force(physics,drive,elapsed,drive.throttle,solver.speed,velocity,transition);
        first=second=static_cast<float>(force*.5f*drive.transfer);
        if(input.flag229d&&(drive.transmission&3u)&&!(drive.transmission&4u)){
            float sum=first+second;if(std::abs(sum)<static_cast<float>(.009999999776482582))sum=.009999999776482582f;
            const float capacity=static_cast<float>(static_cast<long double>(ratio(drive.gear))*drive.clutch*get(0x8f8)*drive.transfer);
            if((capacity<sum&&!(drive.transmission&1u))||capacity<static_cast<long double>(sum)*.800000011920929f){
                first=static_cast<float>(static_cast<long double>(first)*(static_cast<long double>(capacity)/sum));
                second=static_cast<float>((static_cast<long double>(capacity)/sum)*second);
            }
        }
    }else if(kind==1){
        first=static_cast<float>(car_drive_force(physics,drive,elapsed,std::abs(drive.throttle),solver.speed,velocity,transition)*.5f);
        if(drive.throttle<0)first=-first;
        auto force=car_drive_force(physics,drive,elapsed,std::abs(drive.second_throttle),solver.speed,velocity,transition)*.5f;
        if(drive.second_throttle<0)force=-force;
        first*=drive.transfer;second=static_cast<float>(force*drive.transfer);
    }
    const auto stable=static_cast<std::uint32_t>(solver.stable_milliseconds)+milliseconds;std::memcpy(&solver.stable_milliseconds,&stable,4);
    if(static_cast<float>(.03500000014901161)<std::abs(static_cast<long double>(steering.current)-controller.previous_steering)){
        controller.previous_steering=steering.current;controller.inactive_milliseconds=0;
        drive.body_flags=(drive.body_flags&~2u)|1u;solver.stable_milliseconds=-500;
    }
    if(!(drive.body_flags&1u)||(drive.body_flags&0x10000000u)){
        if(((get(0x524)<input.first||get(0x524)<input.second)&&drive.gear!=0)||drive.gear==0||controller.requests_update){
            if(!(drive.body_flags&0x10000000u))drive.body_flags=(drive.body_flags&~2u)|1u;
            controller.inactive_milliseconds=0;solver.stable_milliseconds=std::min(solver.stable_milliseconds,std::int32_t{0});
        }else{
            controller.previous_engine_speed=drive.engine_speed;
            update_car_drivetrain(physics,drive,elapsed,first+second,0,solver.speed,solver.speed_angle,forward,velocity,receivers);
            if(static_cast<long double>(get(0x914))+1.f<drive.engine_speed){drive.transmission&=~0x10u;drive.body_flags|=1u;}
            else{
                drive.clutch=0;drive.transmission=(drive.transmission&~1u)|0x10u;
                if(transition&&solver.speed<.699999988079071f){drive.brake=1;drive.body_flags|=0x1000000u;}
                controller.frame_simulated=false;drive.shift_in_progress=false;
            }
            if(!(drive.body_flags&1u)&&(!controller.controlled||(drive.body_flags&0x1000000u))&&solver.stable_milliseconds>1000){drive.body_flags|=2u;return;}
        }
    }
    if(ordinary&&(drive.transmission&0x20u)){
        const auto original=drive.transmission;drive.transmission=original&~0x10u;
        if(drive.gear==0)drive.transmission=(drive.transmission&~0xcu)|1u;
        else{
            const float low=get(0x8ec)+1.f;
            if(low<drive.engine_speed){
                if(drive.engine_speed<=static_cast<long double>(get(0x8f4))+8.f||!(original&8u)){
                    if(!(original&4u)){drive.transmission|=1u;if(!drive.shift_in_progress)drive.clutch=drive.clutch_target;}
                }else{if(!(original&3u))drive.clutch=drive.clutch_target;drive.transmission=original&~0x14u;}
            }else if(low<=rolling){drive.transmission=(original&~0x14u)|1u;if(!drive.shift_in_progress)drive.clutch=drive.clutch_target;}
            else{
                drive.shift_in_progress=false;drive.transmission=original&~0x13u;drive.clutch=0;
                if(controller.previous_first_input<=get(0x524)){
                    drive.transmission=original&~0x17u;
                    if(transition){drive.brake=.10000000149011612f;if(solver.speed<.699999988079071f){drive.brake=1;drive.body_flags|=0x1000000u;}}
                    drive.transmission|=0x10u;
                }else{
                    auto candidate=drive.gear;
                    if(input.flag229d&&rolling<get(0x810+candidate*4)&&candidate>1){
                        if(candidate>2){
                            const float inverse=1.f/ratio(next_index-1);
                            do{
                                const auto candidate_ratio=ratio(candidate-1);
                                if(get(0x80c+candidate*4)<=static_cast<long double>(inverse)*rolling*candidate_ratio&&
                                        static_cast<long double>(inverse)*drive.engine_speed*candidate_ratio<get(0x7c8+candidate*4))break;
                                --candidate;
                            }while(candidate>2);
                        }
                        (void)request(candidate-1);
                    }
                    drive.transmission|=0x14u;
                }
            }
            const auto transmission=drive.transmission;
            if(!(transmission&4u)||(drive.gear>1&&input.flag229d)||drive.gear==0)drive.transmission=(transmission&~0xcu)|1u;
            else{
                drive.shift_in_progress=false;
                if(drive.engine_speed<=get(0x8f4)){drive.clutch=0;drive.transmission=transmission&~8u;}
                else if(!(transmission&8u)){
                    drive.engine_speed=get(0x8f4)+1.f;
                    const float sum=first+second;
                    drive.clutch=static_cast<float>(std::abs(static_cast<long double>(sum))/(static_cast<long double>(ratio(drive.gear))*get(0x8f8)*drive.transfer));
                    if(drive.clutch>1)drive.clutch=1;
                    drive.transmission=(transmission&~3u)|8u;
                }else if(get(0x8f4)<rolling){drive.clutch=drive.clutch_target;drive.transmission=(transmission&~0xcu)|1u;}
            }
        }
        for(auto n=wheels.size();n;){const auto flags=wheels[--n].flags;
            if((flags&2u)&&(flags&0x20u)&&!(flags&0x400u)&&drive.throttle<=0){drive.clutch=0;break;}}
    }
    controller.previous_engine_speed=drive.engine_speed;controller.previous_velocity=body.motion().velocity;
    controller.previous_speed=solver.speed;controller.previous_rolling_speed=solver.rolling_speed;
    if(!transition)drive.body_flags|=0x1000000u;
    if(!(drive.body_flags&0x10000000u))run_car_substeps(physics,body,wheels,solver.substeps,elapsed,solver.speed,drive.brake,first,second,
        [&](float delta,const std::array<float,3>& drag,float left,float right){step(delta,drag,left,right,input.first,input.second);});
    if(state.health.destroyed)state.audio.stop_engine_transition(drive,state.finalizer.inside,transition,state.health.destroyed);
    if(!(drive.body_flags&8u)){
        bool contact=false;for(auto n=wheels.size();n;){const auto flags=wheels[--n].flags;if((flags&8u)&&!(flags&0x400u)){contact=true;break;}}
        if(!contact)drive.body_flags|=8u;
    }else{
        bool all=true;for(auto n=wheels.size();n;){const auto flags=wheels[--n].flags;if((!(flags&8u)&&!(flags&0x400u))||(flags&0x10u)){all=false;break;}}
        if(all)drive.body_flags&=~8u;
    }
    std::array<float,3> displacement{};for(unsigned a=0;a!=3;++a)displacement[a]=body.initial_position()[a]-controller.position_checkpoint[a];
    const auto moved=(static_cast<long double>(displacement[0])*displacement[0]+static_cast<long double>(displacement[1])*displacement[1])+static_cast<long double>(displacement[2])*displacement[2];
    if(.02250000089406967f<=moved||(drive.body_flags&0x20000000u)){controller.position_checkpoint=body.initial_position();controller.inactive_milliseconds=0;}
    else controller.inactive_milliseconds+=milliseconds;
    if(!(drive.body_flags&2u))solver.stable_milliseconds=std::min(solver.stable_milliseconds,std::int32_t{0});
    if(solver.stable_milliseconds>1000||controller.inactive_milliseconds>4000)drive.body_flags&=~1u;
}

} //namespace hd2::game
#endif
