#ifndef HD2_GAME_CAR_STEERING_H
#define HD2_GAME_CAR_STEERING_H

#include "game/car_body_integration.h"

namespace hd2::game {

struct CarSteeringState {
    float requested=0,current=0,maximum=0,difference=0; //948/94C/934/958.
    float response_curve=.5f; //004E2900 initializes core5F4, separate from serialized physics coefficients.
    bool snap=false; //2299, initialized by004EA780.
};

//004F2420 maps the signed input through the actual steering response curve.
inline void request_car_steering(CarSteeringState& state,float requested){
    long double input=requested;
    if(input>1)input=1;
    else if(input< -1)input=-1;
    const auto curve=state.response_curve;
    const auto response=!(input>0)?-(input-(1.L+input)*curve)*input:((1.L-input)*curve+input)*input;
    state.requested=static_cast<float>(response*state.maximum);
}

//00507400. The original's inline local-matrix cache is supplied by the actual
//004C8140 endpoint; quaternion publication normalizes through the real engine.
//ASM50775E..5078BC preserves the current-angle sign cases which Ghidra folds.
template<class Frames>void update_car_steering(Frames& frames,typename Frames::Frame model,
        const CarPhysics& physics,const CarBodyIntegration<Frames>& body,
        std::vector<CarWheelRuntime<Frames>>& wheels,CarSteeringState& state,float elapsed,float speed){
    using Vector=typename Frames::Vector;const auto& fields=physics.scalars();
    auto factor=1-static_cast<long double>(speed)/fields.get(0x6f0);if(factor<0)factor=0;
    if(speed>1){
        const auto inverse=1/static_cast<long double>(speed);const auto& v=body.motion().velocity;const auto& right=body.pose().right;
        const auto lateral=((inverse*v[0])*right[0]+(inverse*v[1])*right[1])+(inverse*v[2])*right[2];
        if((lateral>.34&&state.requested>0)||(lateral<-.34&&state.requested<0))factor=1; //DOUBLE00810970/968.
    }
    state.maximum=static_cast<float>((static_cast<long double>(fields.get(0x92c))-fields.get(0x930))*factor+fields.get(0x930));
    if(!state.snap){
        const bool returning=static_cast<long double>(state.requested)*state.current<0||std::abs(state.requested)<std::abs(state.current);
        const unsigned first=returning?0x93c:0x938,last=returning?0x944:0x940;
        const float amount=static_cast<float>(((static_cast<long double>(fields.get(first))-fields.get(last))*factor+fields.get(last))*elapsed);
        if(std::abs(static_cast<long double>(state.requested)-state.current)<=amount)state.current=state.requested;
        else if(state.current<state.requested)state.current+=amount;
        else state.current-=amount;
    }else state.current=state.requested;
    state.difference=static_cast<float>((std::abs(static_cast<long double>(state.current))/fields.get(0x92c))*fields.get(0x954));
    for(auto n=wheels.size();n;){
        const auto index=--n;auto& wheel=wheels[index];if((wheel.flags&0x400u)||frames.object_parent(wheel.parent)!=model)continue;
        const auto local=frames.car_local_matrix(wheel.parent);const Vector axis{{local[4],local[5],local[6],local[7]}};
        const bool odd=(index&1u)!=0;const auto toe=static_cast<long double>(wheel.scalars.get(0x178));long double angle=odd?-toe:toe;
        if(wheel.flags&1u){
            auto current=static_cast<long double>(state.current);
            if(state.current>0&&!odd)current-=state.difference;
            else if(state.current<=0&&odd)current+=state.difference;
            const auto turn=current*wheel.scalars.get(0x1f8);
            angle+=fields.get(0x1f8)<=wheel.vectors.at(0x30)[2]?-turn:turn;
        }
        const auto turn=frames.car_axis_quaternion(axis,static_cast<float>(angle));const auto& alignment=wheel.alignment;
        //Native alignment CROSS turn.5078F9..5079D1 has different FLOAT
        //spills for X/Y/Z, unlike004EA780's other product sequence.
        std::array<float,4> rotation{};
        rotation[3]=static_cast<float>(static_cast<long double>(turn[3])*alignment[3]-((static_cast<long double>(turn[0])*alignment[0]+static_cast<long double>(turn[2])*alignment[2])+static_cast<long double>(turn[1])*alignment[1]));
        for(unsigned a=0;a!=3;++a){const auto b=(a+1)%3,c=(a+2)%3;
            auto first=static_cast<long double>(turn[a])*alignment[3];
            auto second=static_cast<long double>(turn[3])*alignment[a];
            if(a)first=static_cast<float>(first);
            if(a==2)second=static_cast<float>(second);
            auto sum=first+second;if(a!=2)sum=static_cast<float>(sum);
            rotation[a]=static_cast<float>(sum+(static_cast<long double>(alignment[b])*turn[c]-static_cast<long double>(alignment[c])*turn[b]));
        }
        frames.car_steering_rotation(wheel.parent,rotation);
    }
}

//0050B180's kind2 visual pass. It rebuilds the parent's local cache after
//publishing the base wheel quaternion; it does not issue a frame24 update.
template<class Frames>void reset_car_tracked_steering(Frames& frames,
        const std::vector<CarWheelRuntime<Frames>>& wheels){
    for(auto n=wheels.size();n;){const auto& wheel=wheels[--n];if(!(wheel.flags&1u)||(wheel.flags&0x400u))continue;
        const auto matrix=frames.car_local_matrix(wheel.parent);
        const auto turn=frames.car_axis_quaternion({{matrix[4],matrix[5],matrix[6],matrix[7]}},0.f);
        frames.car_steering_rotation(wheel.parent,car_quaternion_product(turn,wheel.rotation));
        (void)frames.car_local_matrix(wheel.parent);
    }
}

//0050CF20..0050CF40. The third frame1C argument is a FLOAT roll; earlier
//decompilation lost the current/max-angle division and504 multiplier.
template<class Frames,class Auxiliary>void update_car_steering_wheel(Frames& frames,const Auxiliary& geometry,
        const CarPhysics& physics,const CarSteeringState& steering){
    const auto wheel=geometry.steering_wheel();if(!wheel)return;
    const auto roll=static_cast<float>((static_cast<long double>(steering.current)/steering.maximum)*physics.scalars().get(0x504));
    if(!std::isfinite(roll))throw std::runtime_error("Car steering wheel has no valid current steering limit");
    frames.car_steering_wheel_direction(wheel,geometry.steering_direction(),roll);
}
} //namespace hd2::game
#endif
