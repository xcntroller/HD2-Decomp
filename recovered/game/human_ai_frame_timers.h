#ifndef HD2_GAME_HUMAN_AI_FRAME_TIMERS_H
#define HD2_GAME_HUMAN_AI_FRAME_TIMERS_H
#include "game/human_state_frame_timers.h"
#include "game/human_frame_stats.h"
#include "game/human_ai_frame_metrics.h"

namespace hd2::game {
template<class Words>void decrement_nonzero_human_ai_timer(Words& words,unsigned offset,std::uint32_t elapsed){
    const auto old=words.template get<std::uint32_t>(offset);if(!old)return;
    const auto next=old-elapsed;words.set(offset,human_frame_signed_word(next)<0?0u:next);
}
//00547DA0: metric test may bypass the reverse ENEMY-state aim predicate.
//Both200/400 branches draw independently from SAME CRT, latter overwrites.
template<class State,class Frames,class Random>void refresh_human_guard_reaction_timer(State& state,Frames& frames,Random& random){
    auto* human=state.human();if(!human)throw std::logic_error("Guard reaction requires bound Human");
    auto& words=state.state();
    const auto distance=human_ai_closest_enemy_distance(*human,frames);
    if(!(distance<=human_frame_signed_word(words.template get<std::uint32_t>(0x1a4)))){
        const auto& enemies=human->perception.lists()[0];bool accepts=false;
        for(auto n=enemies.size();n;){
            auto* enemy=dynamic_cast<std::remove_pointer_t<decltype(human)>*>(enemies[--n]);
            if(!enemy||!enemy->loading_state)throw std::runtime_error("Guard reaction requires actual enemy Human B0");
            if(human_ai_aims_at(*enemy,*enemy->loading_state,*human,frames)){accepts=true;break;}
        }
        if(!accepts)return;
    }
    const auto draw=[&]{const auto value=random();if(value>0x7fff)throw std::runtime_error("Guard reaction requires SAME CRT rand");return value;};
    constexpr float scale=0x1.0002p-15f;
    if(state.words().template get<std::uint32_t>(0x50)&0x200u)words.set(0x1a8,static_cast<std::uint32_t>(static_cast<long double>(draw())*scale*5000.f+7000.f));
    if(state.words().template get<std::uint32_t>(0x50)&0x400u)words.set(0x1a8,static_cast<std::uint32_t>(static_cast<long double>(draw())*scale*4000.f+5000.f));
}
//00547E70, called only at the actual Guard authority boundary.
template<class State,class Queries>void advance_human_guard_frame_prefix(State& state,std::uint32_t elapsed,Queries& queries){
    if(auto* target=state.timer_target())if(!queries.human_scene_actor_matches(*target)){
        state.resources().target().bind_actor(nullptr);state.timer_target(nullptr);
    }
    auto& inputs=queries.human_ai_frame_inputs();inputs.clear();state.bind_frame_inputs(inputs.first,inputs.second);
    auto& words=state.state();words.set(0x1ac,words.template get<std::uint32_t>(0x1ac)+elapsed);
    state.advance_frame_timers(elapsed,queries);
    decrement_nonzero_human_ai_timer(words,0x1b0,elapsed);
    const auto enemies=[&]() -> const auto& {
        auto* human=state.human();if(!human)throw std::logic_error("Guard frame requires bound Human");return human->perception.lists()[0];
    };
    if(!(state.words().template get<std::uint32_t>(0x50)&0x100u)){
        decrement_nonzero_human_ai_timer(words,0x1a8,elapsed);
        if(!enemies().empty()&&human_frame_signed_word(words.template get<std::uint32_t>(0x1a8))<1000)state.refresh_reaction_timer();
        decrement_nonzero_human_ai_timer(words,0x1c4,elapsed);
    }
    words.set(0x1d0,state.item()?0u:words.template get<std::uint32_t>(0x1d0)+elapsed);
    const auto cadence=words.template get<std::uint32_t>(0x1c8)-elapsed;words.set(0x1c8,cadence);
    if(human_frame_signed_word(cadence)<=0){
        words.set(0x1c8,5000u);
        const auto count=enemies().size();if(count>0x7fffffffu)throw std::length_error("Guard hostile count exceeds native signed vector");
        adjust_human_frame_stat3(*state.human(),2u-static_cast<std::uint32_t>(count)); //005AE940 is genuine constant2, NOT event count.
    }
    words.set(0x1dc,enemies().empty()?0u:words.template get<std::uint32_t>(0x1dc)+elapsed);
    state.resources().advance_current_action(elapsed);
}
//00559C60: common timers FIRST, target validation, action age, THEN clear/
//publish shared scratch. Guard intentionally clears scratch BEFORE timers.
template<class State,class Queries>void advance_human_area_frame_prefix(State& state,std::uint32_t elapsed,Queries& queries){
    state.advance_frame_timers(elapsed,queries);
    if(auto* target=state.timer_target())if(!queries.human_scene_actor_matches(*target))state.timer_target(nullptr);
    state.resources().advance_current_action(elapsed);
    auto& inputs=queries.human_ai_frame_inputs();inputs.clear();state.bind_frame_inputs(inputs.first,inputs.second);
}
} //namespace hd2::game
#endif
