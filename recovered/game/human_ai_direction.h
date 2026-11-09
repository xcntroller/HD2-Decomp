#ifndef HD2_GAME_HUMAN_AI_DIRECTION_H
#define HD2_GAME_HUMAN_AI_DIRECTION_H

#include "game/human_joint_owners.h"

namespace hd2::game {

//0055D860/0055D8B0 and0055D630/0055D2E0, the SAME60-byte owner at
//AI+124. Separate from005607D0 at128. No steering/simulation producer is
//invented; the actual B0 bind supplies its two original direction vectors.
class HumanAIDirection {
public:
    HumanAIDirection() {
        state_.set(0x40,std::uint16_t{0});state_.set(0x44,.7f);
        state_.set(0x48,std::int32_t{-1});state_.set(0x4c,std::int32_t{-1});
        for(auto offset:{0x20u,0x24u,0x28u,0x10u,0x14u,0x18u,0u,4u,8u,0x30u,0x34u,0x38u,0x3cu,0x50u})
            state_.set(offset,std::uint32_t{0});
    }
    //0055D8B0 is a genuine RET. Scalars have no additional owning lifetime.
    int set_current(const HumanVector& input) { //0055D630.
        const auto value=input,horizontal=horizontal_direction(value);
        const auto dot=static_cast<long double>(horizontal[0])*state_.get<float>(0x38)+
            static_cast<long double>(horizontal[2])*state_.get<float>(0x3c);
        if(dot<state_.get<float>(0x44)) {
            if(state_.get<std::uint16_t>(0x40)&2u)return 2;
            state_.set(0x10,value);state_.set(0x38,horizontal[0]);state_.set(0x3c,horizontal[2]);
        }
        state_.set(0,value);state_.set(0x30,horizontal[0]);state_.set(0x34,horizontal[2]);
        state_.set(0,normalize(value));return 0;
    }
    int set_target(const HumanVector& input) { //0055D2E0; reset copies raw current before normalizing target ONLY.
        const auto value=input,horizontal=horizontal_direction(value);
        const auto dot=static_cast<long double>(horizontal[0])*state_.get<float>(0x30)+
            static_cast<long double>(horizontal[2])*state_.get<float>(0x34);
        if(state_.get<float>(0x44)<=dot) {
            state_.set(0x10,value);state_.set(0x38,horizontal[0]);state_.set(0x3c,horizontal[2]);
            state_.set(0x10,normalize(value));return 0;
        }
        if(state_.get<std::uint16_t>(0x40)&1u)return 1;
        state_.set(0x10,value);state_.set(0,value);state_.set(0x10,normalize(value));
        state_.set(0x38,horizontal[0]);state_.set(0x3c,horizontal[2]);
        state_.set(0x30,horizontal[0]);state_.set(0x34,horizontal[2]);return 0;
    }
    auto& state(){return state_;}const auto& state()const{return state_;}
private:
    static HumanVector horizontal_direction(const HumanVector& value) {
        const auto x=static_cast<long double>(value[0]),z=static_cast<long double>(value[2]);
        const auto length=std::sqrt(z*z+x*x);
        if(!std::isfinite(length)||!std::isfinite(value[1]))throw std::runtime_error("Non-finite AI direction");
        auto result=value;
        if(length>9.99999993922529e-09f) {
            const auto reciprocal=1/length;
            result[0]=static_cast<float>(x*reciprocal);result[2]=static_cast<float>(z*reciprocal);
        }
        return result;
    }
    static HumanVector normalize(HumanVector value) {
        //These routines store (X²+Y²)+Z² to float BEFORE their unit comparison.
        const auto x=static_cast<long double>(value[0]),y=static_cast<long double>(value[1]),z=static_cast<long double>(value[2]);
        const float square=static_cast<float>((x*x+y*y)+z*z);
        constexpr float epsilon=9.99999993922529e-09f;
        if(!std::isfinite(square))throw std::runtime_error("AI direction exceeds finite normalization range");
        if(std::abs(static_cast<long double>(square)-1)>=epsilon) {
            if(square>=epsilon) {
                const auto reciprocal=1/std::sqrt(static_cast<long double>(square));
                for(unsigned axis=0;axis!=3;++axis)value[axis]=static_cast<float>(value[axis]*reciprocal);
            }else {const auto axis=value[0]!=0?0u:value[2]!=0?2u:1u;value[axis]=value[axis]<0?-1.f:1.f;}
        }
        return value;
    }
    HumanJointWords<0x60> state_; //Only the original stores above earn recovery; padding remains unclaimed.
};
} //namespace hd2::game
#endif
