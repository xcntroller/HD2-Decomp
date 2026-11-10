#ifndef HD2_GAME_CAR_SUBSTEPS_H
#define HD2_GAME_CAR_SUBSTEPS_H

#include "game/car_body_integration.h"
#include <optional>

namespace hd2::game {

template<class Frames>struct CarSubstepState {
    using Body=CarBodyIntegration<Frames>;using Matrix=typename Frames::Matrix;
    std::uint32_t flags=0x8000dff9u; //004E2900, not the separate Car variant flagsF74.
    std::uint16_t count=0,index=0;
    float maximum_delta=.02800000086426735f,previous_delta=.019999999552965164f,field724=0;
    std::optional<Matrix> previous_world,previous_inverse;
    std::optional<typename Body::Candidate> previous_pose;
};

//0050DB90. Step is the mandatory containing0050DE60 implementation. It must
//increment the SAME state.index on entry and produce body.motion().travel;
//this wrapper cannot stand in for that solver or publish a ready Car.
template<class Frames,class Step>void run_car_substeps(const CarPhysics& physics,
        CarBodyIntegration<Frames>& body,std::vector<CarWheelRuntime<Frames>>& wheels,
        CarSubstepState<Frames>& state,float elapsed,float speed,float brake,
        float first_drive,float second_drive,Step&& step){
    if(!std::isfinite(elapsed)||elapsed<0||!std::isfinite(state.maximum_delta)||state.maximum_delta<=0)
        throw std::runtime_error("Invalid Car simulation substep interval");
    std::array<float,3> drag{};
    if(speed>1){
        const float square=speed*speed;const auto& fields=physics.scalars();
        constexpr unsigned offsets[]{0x854,0x728,0x730};
        for(unsigned n=0;n!=3;++n)drag[n]=static_cast<float>(static_cast<long double>(fields.get(offsets[n]+4))*fields.get(offsets[n])*square*.6499999761581421f);
    }
    if(brake==0)state.field724=0;
    body.motion().travel=0;
    if(state.flags&0x10u){
        const auto& pose=body.pose();
        if(!(state.flags&0x400u)){state.previous_world=pose.world;state.previous_inverse=pose.inverse;}
        state.previous_pose=typename CarBodyIntegration<Frames>::Candidate{pose.position,pose.forward,pose.up,pose.right};
    }
    state.index=0;
    if(elapsed<=state.maximum_delta){
        state.count=1;step(elapsed,drag,first_drive,second_drive);
    }else{
        const auto ratio=static_cast<long double>(elapsed)/state.maximum_delta;
        if(ratio>=65535)throw std::runtime_error("Car substep count exceeds defined WORD range");
        state.count=static_cast<std::uint16_t>(ratio)+1;
        for(unsigned n=0;n<state.count;++n)step(elapsed/static_cast<float>(state.count),drag,first_drive,second_drive);
    }
    for(auto wheel=wheels.rbegin();wheel!=wheels.rend();++wheel)wheel->scalars.set(0x26c,body.motion().travel+wheel->scalars.get(0x26c));
    state.previous_delta=elapsed<.00009999999747378752f?.00009999999747378752f:elapsed;
}
} //namespace hd2::game
#endif
