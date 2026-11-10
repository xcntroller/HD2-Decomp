#ifndef HD2_GAME_HUMAN_STATE_FRAME_TIMERS_H
#define HD2_GAME_HUMAN_STATE_FRAME_TIMERS_H
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace hd2::game {
inline std::int32_t human_frame_signed_word(std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;}
template<class Words>void decrement_positive_human_state_timer(Words& words,unsigned offset,std::uint32_t elapsed){
    const auto old=words.template get<std::uint32_t>(offset);if(human_frame_signed_word(old)<=0)return;
    const auto value=old-elapsed;words.set(offset,human_frame_signed_word(value)<0?0u:value);
}
//00588010. Queries supplies actual process list0089B4E0, target94/life and
//the CURRENT derived B0+0C completion receiver. Invalid-target false does not
//decrement64; timeout true follows its store. Neither means generic success.
template<class State,class Queries>void advance_human_base_frame_timers(State& state,std::uint32_t elapsed,Queries& queries){
    auto& words=state.words();decrement_positive_human_state_timer(words,0xb8,elapsed);
    const auto duration=words.template get<std::uint32_t>(0x64);if(human_frame_signed_word(duration)<=0)return;
    const auto required_target=[&]() -> auto& {
        auto* target=state.action_target();if(!target)throw std::runtime_error("Human action timer requires actual typed B0+68 target");return *target;
    };
    bool invalid=false;
    switch(words.template get<std::uint32_t>(0x60)){
    case 2:invalid=!queries.human_action_target_list_contains(state.action_target());break; //Native compares identity, including NULL, without dereference.
    case 4:invalid=queries.human_action_target_field94(required_target())!=0;break;
    case 15:case 19:case 35:{
        if(!queries.human_action_target_alive(required_target()))invalid=true;
        else{auto* human=state.human();if(!human)throw std::logic_error("Human action timer requires actual B0+44");invalid=!queries.human_action_target_alive(*human);}
        break;
    }
    case 38:invalid=queries.human_action_target_field94(required_target())!=1;break;
    default:break;
    }
    if(invalid){queries.finish_human_state_action(state,false);return;}
    const auto remaining=duration-elapsed;words.set(0x64,remaining);
    if(human_frame_signed_word(remaining)<=0)queries.finish_human_state_action(state,true);
}
//00571680 prefix: each positive countdown clamps only negative wrapped result,
//then10C accumulates elapsed, THEN the actual B0 base timer receiver runs.
template<class Words>void advance_human_ai_frame_timers(Words& words,std::uint32_t elapsed){
    for(auto offset:{0x120u,0xc8u,0xccu,0x108u,0x110u})decrement_positive_human_state_timer(words,offset,elapsed);
    words.set(0x10c,words.template get<std::uint32_t>(0x10c)+elapsed);
}
} //namespace hd2::game
#endif
