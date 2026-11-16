#ifndef HD2_GAME_CAR_AUTOMATIC_GEARS_H
#define HD2_GAME_CAR_AUTOMATIC_GEARS_H

#include "game/car_gear_control.h"

namespace hd2::game {

//00509AA0's predictive decisions use the PREVIOUS solver's010/020, not
//velocity or the forward axis. X/Y spill before the dot; Z remains extended
//after its initial force*Z spill. These are local arithmetic, not new exports.
template<class Vector>long double car_predicted_drive_dot(float force,float current,
        const Vector& drive,const Vector& net,bool spill_x){
    const float z=force*drive[2],inverse=1.f/current;
    auto x=static_cast<long double>(force)*drive[0]*inverse;
    if(spill_x)x=static_cast<float>(x);
    const float px=static_cast<float>(x+net[0]);
    const float py=static_cast<float>(static_cast<long double>(force)*drive[1]*inverse+net[1]);
    return (static_cast<long double>(px)*drive[0]+static_cast<long double>(py)*drive[1])+
        (static_cast<long double>(z)*inverse+net[2])*drive[2];
}
template<class Vector>float car_controller_net_length(const Vector& net){
    const float square=static_cast<float>((static_cast<long double>(net[2])*net[2]+
        static_cast<long double>(net[1])*net[1])+static_cast<long double>(net[0])*net[0]);
    std::int32_t bits;std::memcpy(&bits,&square,4);
    if(bits<0x322bcc77)return 0;
    if(9.99999993922529e-09f<=std::abs(static_cast<long double>(square)-1))return static_cast<float>(std::sqrt(static_cast<long double>(square)));
    return 1;
}

//00509AA0 automatic gear selection through0050A625. next_index is the
//original local44: direct reversal changes it; ordinary requests do not.
//All request/force/audio calls use their recovered bodies on SAME state.
template<class State,class Receivers>void select_car_automatic_gear(const CarPhysics& physics,
        State& state,Receivers& receivers,float elapsed,float rolling,bool allowed,std::int32_t& next_index){
    auto& drive=state.drive;auto& solver=state.solver;auto& controller=state.controller();
    const auto& fields=physics.scalars();const auto get=[&](int offset){return fields.get(static_cast<unsigned>(offset));};
    const auto ratio=[&](int gear){return car_gear_ratio(physics,gear);};
    const auto request=[&](int gear){return request_car_gear(physics,drive,solver.substeps.flags,state.health.drivetrain,state.reset_fields.field780,gear);};
    const auto kind=static_cast<unsigned>(get(0x550));const bool ordinary=kind==0||kind==2;
    const auto gear=drive.gear;const auto next=gear+1;const auto input=drive.throttle;
    if(!((!(solver.substeps.flags&0x4000u)||state.health.drivetrain>0)&&state.finalizer.engine_transition&&
            state.control_requests.flag229d&&controller.controlled&&!(drive.body_flags&0x1000000u)&&
            !drive.shift_in_progress&&gear!=0&&(drive.flag2293||controller.has_strip_connections)))return;
    if(gear<0){
        if(solver.speed_angle>=1.5707963705062866f||input>=.30000001192092896f||
                (drive.body_flags&6u)||solver.speed<=1){
            if(drive.brake>.30000001192092896f&&solver.speed<1){
                state.reset_fields.field780=static_cast<float>(static_cast<long double>(elapsed)+state.reset_fields.field780);
                if(state.reset_fields.field780>.0020000000949949026f)(void)request(1);
            }
        }else{
            next_index=2;
            const float speed=static_cast<float>((static_cast<long double>(solver.speed)/get(0x8d8))*ratio(gear));
            while(next_index<physics.gear_count()){
                if(speed<get(0x7d0+(next_index-2)*4))break;
                ++next_index;
            }
            drive.gear=drive.requested_gear=next_index-1;
        }
        return;
    }
    if(!(solver.speed_angle<=1.5707963705062866f||input>=.30000001192092896f||
            (drive.body_flags&4u)||!ordinary||solver.speed<=1)){
        next_index=0;drive.gear=drive.requested_gear=-1;return;
    }
    if(!(rolling<=drive.engine_speed||drive.brake>=.0010000000474974513f||state.brake_amount>=.0010000000474974513f)){
        auto selected=next;
        if(selected<=physics.gear_count()){
            const float inverse=1.f/ratio(gear);
            do{
                if(static_cast<long double>(inverse)*ratio(selected-1)*rolling<=get(0x7c8+selected*4))break;
                ++selected;
            }while(selected<=physics.gear_count());
        }
        drive.gear=drive.requested_gear=selected-1;return;
    }
    if(!(drive.engine_speed<=get(0x7c8+next*4)||physics.gear_count()<=gear||!allowed)){
        if(state.body().pose().forward[1]>-.09000000357627869f){
            float low=1,high=1,current=1;
            (void)evaluate_car_gear_force(physics,get(0x810+next*4)*60.f,low,next,false);
            (void)evaluate_car_gear_force(physics,get(0x774)*60.f,high,next,false);
            (void)evaluate_car_gear_force(physics,get(0x7c8+next*4)*60.f,current,gear,false);
            const float average=static_cast<float>((static_cast<long double>(high)+low)*.5f);
            const auto& force=solver.drive_force;
            const auto square=(static_cast<long double>(force[2])*force[2]+static_cast<long double>(force[1])*force[1])+static_cast<long double>(force[0])*force[0];
            if(car_predicted_drive_dot(average,current,force,solver.published_net,true)>0||square<1){
                const float acceleration=static_cast<float>((-car_engine_torque(fields,drive.engine_speed,0)*get(0x8dc))/
                    (static_cast<long double>(ratio(gear))*get(0x738)));
                if(static_cast<long double>(car_controller_net_length(solver.published_net))/get(0x918)<acceleration)
                    (void)request(drive.gear+1);
            }
        }else (void)request(next);
        return;
    }
    //Native downshift arm skips the later reverse timer even when it does not
    //request a gear. A refused request still permits the first-gear prediction.
    bool kickdown=false;
    if(get(0x80c+next*4)<=drive.engine_speed)kickdown=true;
    else if(gear>1){
        if(drive.brake<.30000001192092896f&&state.brake_amount<.30000001192092896f&&allowed){
            int candidate=2;
            if(candidate<next){
                const float inverse=1.f/ratio(gear);
                do{
                    const float speed=static_cast<float>(static_cast<long double>(inverse)*ratio(candidate-1)*drive.engine_speed);
                    if(get(0x814+(candidate-2)*4)<=speed&&speed<get(0x7d0+(candidate-2)*4))break;
                    ++candidate;
                }while(candidate<next);
            }
            if(input>.30000001192092896f||candidate==2){
                (void)request(candidate-1);
                if(candidate==2){
                    float first=1,second=1;
                    (void)evaluate_car_gear_force(physics,static_cast<float>((60.L/ratio(1))*ratio(2)*drive.engine_speed),first,1,false);
                    (void)evaluate_car_gear_force(physics,drive.engine_speed*60.f,second,2,false);
                    if(car_predicted_drive_dot(first,second,solver.drive_force,solver.published_net,false)<0){
                        drive.shift_in_progress=false;drive.clutch=0;drive.gear=1;
                        drive.transmission=(drive.transmission&~3u)|4u;receivers.clutch_changed();
                    }
                }
            }
            return;
        }
        kickdown=true;
    }
    if(kickdown&&gear>1&&drive.engine_speed<static_cast<long double>(get(0x7c8+gear*4))-
            (static_cast<long double>(get(0x7c8+gear*4))-get(0x80c+gear*4))*.25f&&
            input>.30000001192092896f&&controller.previous_first_input<.30000001192092896f&&ordinary&&allowed){
        (void)request(gear-1);return;
    }
    if(ordinary&&drive.brake>.30000001192092896f&&solver.speed<.5f){
        state.reset_fields.field780=static_cast<float>(static_cast<long double>(elapsed)+state.reset_fields.field780);
        if(state.reset_fields.field780>.0020000000949949026f)(void)request(-1);
    }
}

} //namespace hd2::game
#endif
