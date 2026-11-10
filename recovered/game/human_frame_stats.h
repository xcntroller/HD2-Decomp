#ifndef HD2_GAME_HUMAN_FRAME_STATS_H
#define HD2_GAME_HUMAN_FRAME_STATS_H
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
