#ifndef HD2_GAME_HUMAN_FRAME_STATS_H
#define HD2_GAME_HUMAN_FRAME_STATS_H
#include "game/human_state_frame_timers.h"
#include <limits>
#include <cstdint>
#include <cstring>

namespace hd2::game {
//005A64F0: DWORD wrapping addition, signed upper-base comparison BEFORE lower0.
template<class Human>void adjust_human_frame_stat3(Human& human,std::uint32_t amount){
    auto& value=human.properties.stats_ec[3];value+=amount;std::int32_t current,limit;
    std::memcpy(&current,&value,4);std::memcpy(&limit,&human.properties.stats_11c[3],4);
    if(current>limit)value=human.properties.stats_11c[3];else if(current<0)value=0;
}
//005A93A0/005A94F0: actual HumanC8/D4 FLOAT words, not profile base stats.
//Only the original debug-format calls ending in genuine0069BFE0 RET are omitted.
template<class Human>void increment_human_frame_stat(Human& human,bool endurance,float amount,std::uint32_t mode){
    if((mode&7u)!=7u)return;
    auto& word=human.fieldc0[endurance?2:5];float previous;std::memcpy(&previous,&word,4);
    const auto sum=static_cast<long double>(previous)+amount;float value=static_cast<float>(sum);
    const auto limit=endurance?5.f:2.f;if(sum>limit)value=limit;std::memcpy(&word,&value,4);
}
//00588710. Actor7C is a FLOAT bit pattern. The conditioning ratio uses signed
//integer division before FLOAT conversion, while elapsed is zero-extended.
template<class Human,class State,class Inline>
void advance_human_endurance(Human& human,State& state,Inline& control,std::uint32_t elapsed){
    const auto capacity=human_frame_signed_word(human.properties.stats_ec[1]); //Human virtual10=0059C2B0→005A6600(1).
    const auto carried_word=human.base().field7c();float carried;
    std::memcpy(&carried,&carried_word,4);
    const float load=static_cast<float>(static_cast<long double>(carried)/capacity);
    const auto current=human_frame_signed_word(human.properties.stats_ec[2]);
    const auto baseline=human_frame_signed_word(human.properties.stats_11c[2]);
    if(!baseline||(current==std::numeric_limits<std::int32_t>::min()&&baseline==-1))
        throw std::runtime_error("Human endurance reached the native conditioning division fault");
    const float conditioning=static_cast<float>(current/baseline);
    const auto flags=state.words().template get<std::uint32_t>(0x50);
    std::uint32_t movement=0xffffffffu;
    if(flags&0xf8u){
        movement=control.frame_movement_mode(); //005A5CC0, same effective-mode corrections.
        if(movement==5&&!(flags&8u))movement=flags&0x10u?1u:2u;
        else if(movement==2&&(flags&0x10u))movement=1;
    }
    long double rate;
    switch(movement){
    case 2:rate=-static_cast<long double>(load);break;
    case 5:rate=-20.L*load;break;
    default:{
        long double factor=1;
        if(movement==0)factor=1.5f;
        else if(movement!=1){const auto posture=control.frame_posture_mode();factor=posture==1?4.f:posture==2?6.f:2.f;}
        rate=((1.L-load)+conditioning)*factor;
        break;
    }
    }
    auto& energy=human.activation.field14c;
    energy=static_cast<float>((static_cast<long double>(elapsed)*rate)*.0010000000474974513f+energy);
    if(static_cast<long double>(energy)>current)energy=static_cast<float>(current);
    else if(energy<0)energy=0;
    auto& words=state.words();
    if(energy==0&&words.template get<std::uint32_t>(0x5c)==0x10000u)words.set(0x5c,0x4000u);
}
//005A88F0. Activity mask comes from B0+50&F8; action flags come from+5C.
//Signed wrapping timers use STRICT thresholds, one subtraction per call.
template<class Human,class State>void advance_human_periodic_stats(Human& human,State& state,std::uint32_t elapsed,std::uint32_t mode){
    const auto activity=state.words().template get<std::uint32_t>(0x50)&0xf8u;if(!activity)return;
    const auto flags=state.words().template get<std::uint32_t>(0x5c);
    const auto advance=[&](std::uint32_t& timer,std::uint32_t interval){
        timer+=elapsed;std::int32_t signed_timer;std::memcpy(&signed_timer,&timer,4);
        if(signed_timer<=static_cast<std::int32_t>(interval))return false;
        timer-=interval;return true;
    };
    if((flags&0x10000u)&&(activity&8u)){
        if(advance(human.activation.e8,25000))increment_human_frame_stat(human,true,.1f,mode);
        return; //Endurance selection suppresses stealth even before its timer expires.
    }
    if((flags&0x1000u)&&advance(human.activation.e4,20000))increment_human_frame_stat(human,false,.05f,mode);
}
} //namespace hd2::game
#endif
