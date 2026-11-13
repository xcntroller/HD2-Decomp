#ifndef HD2_GAME_CAR_CONTROL_REQUESTS_H
#define HD2_GAME_CAR_CONTROL_REQUESTS_H
#include "game/car_drivetrain.h"

namespace hd2::game {
//Actual6FC/700 input words precede00509AA0's filtering into85C/860. Keep
//them separate from the drivetrain's filtered throttle values.004EA780 sets
//229B/C false and229D true; wider occupied input producers use this SAME state.
struct CarControlRequests {float first=0,second=0;bool flag229b=false,flag229c=false,flag229d=true;};

//004F2100 and004F2270. Caller borrows native724 and7C4 from the existing
//substep state,8E4 from drivetrain.brake. Preserve the FLOAT ramp store.
inline float clamp_car_pedal(float value){return !(value<=1.f)?1.f:value<.05000000074505806f?0.f:value;} //Native dead zone0080FAA4.
template<class Fields,class Substeps>
void request_car_drive_pedal(CarControlRequests& input,CarDrivetrainState& drive,const Fields& fields,
        Substeps& steps,float value,bool first){
    value=clamp_car_pedal(value);const auto kind=static_cast<unsigned>(fields.get(0x550));
    const auto brake=[&]{
        if(input.flag229b)drive.brake=value;
        else{steps.field724+=steps.previous_delta;if(fields.get(0x720)<steps.field724)steps.field724=fields.get(0x720);
            drive.brake=static_cast<float>((static_cast<long double>(steps.field724)/fields.get(0x720))*value);}
    };
    if(kind==0||kind==2){
        input.second=0;
        if(!input.flag229d){input.first=drive.running?value:0.f;return;}
        if(drive.gear<0){brake();drive.body_flags&=0xfeffffffu;return;}
        input.first=drive.running?value:0.f;
    }else if(kind==1){
        if(first){input.first=drive.running?value:0.f;drive.brake=0;}
        else{input.second=drive.running?value:0.f;drive.brake=0;}
    }
}
template<class Fields,class Substeps>
void request_car_brake_pedal(CarControlRequests& input,CarDrivetrainState& drive,const Fields& fields,
        Substeps& steps,float value){
    value=clamp_car_pedal(value);if(static_cast<unsigned>(fields.get(0x550))==1)return;
    const auto brake=[&]{
        if(input.flag229b)drive.brake=value;
        else{steps.field724+=steps.previous_delta;if(steps.field724>fields.get(0x720))steps.field724=fields.get(0x720);
            drive.brake=static_cast<float>((static_cast<long double>(steps.field724)/fields.get(0x720))*value);}
    };
    if(!input.flag229d){brake();return;}
    if(drive.gear>=0){brake();drive.body_flags&=0xfeffffffu;return;}
    input.first=input.second=0;if(drive.running)input.first=value;
}
template<class Fields>void request_car_clutch_pedal(CarControlRequests& input,const Fields& fields,float& desired,float value){ //004F23B0,908.
    value=!(value<=1.f)?1.f:value<0.f?0.f:value; //Clutch uses0, not the pedal dead zone.
    desired=input.flag229c?static_cast<float>(((1.L-value)*fields.get(0x5f8)+value)*value):value;
}
template<class Fields,class Substeps>void request_car_emergency_brake(const Fields& fields,const Substeps& steps,
        float& ramp,float& amount,bool& latch,bool enabled,bool held){ //004F26A0,710/740/22C7.
    if(!enabled){ramp=amount=0;latch=false;return;}
    ramp+=steps.previous_delta;if(ramp>fields.get(0x70c))ramp=fields.get(0x70c);
    latch=held;amount=ramp/fields.get(0x70c);
}
} //namespace hd2::game
#endif
