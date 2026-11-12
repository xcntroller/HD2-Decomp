#ifndef HD2_GAME_HUMAN_AI_TRIGGER_OUTPUT_H
#define HD2_GAME_HUMAN_AI_TRIGGER_OUTPUT_H
#include "game/human_animation_requests.h"
#include "game/human_state_frame_timers.h"
#include <algorithm>
#include <array>
namespace hd2::game {
//00572C00 writes SECOND input34 before clearing D8 bit1.
template<class State>void finish_human_ai_trigger(State& state){
    auto* input=state.second_frame_inputs();if(!input)throw std::logic_error("AI trigger finish requires actual second input");
    input->set(0x34,1.f);
    auto& words=state.resources().state();words.set(0xd8,words.template get<std::uint32_t>(0xd8)&~1u);
}
//00572AF0: input production, not held/mounted firing. SAME item, LS3DF
//RandFloat, Sight feedback,74/78 input buffers and native C4 countdown.
template<class State,class World>
void advance_human_ai_trigger(State& state,std::uint32_t elapsed,World& world){
    auto& words=state.resources().state();
    const auto remaining=words.template get<std::uint32_t>(0xc4)-elapsed;
    words.set(0xc4,human_frame_signed_word(remaining)<0?0u:remaining);
    if(!state.item())throw std::logic_error("AI trigger requires actual item");
    auto& item=world.human_frame_output_item(state.item());
    const auto first=[&]{auto* value=state.first_frame_inputs();if(!value)throw std::logic_error("AI trigger requires actual first input");return value;};
    if(item.amount()==0.f||(item.field60&2u)){
        if(words.template get<std::uint32_t>(0xc4)&&world.human_sound_random()<=.8f){first()->set(0x18,1.f);return;}
        words.set(0xd8,words.template get<std::uint32_t>(0xd8)|1u);return;
    }
    //00572AE0 is an actual RET, no target mutation or random draw.
    if(auto* actor=state.resources().aimed_actor())if(actor->base().type()==3u){
        std::uint32_t previous=0;bool visible=false; //Native previous output is not subsequently consumed.
        const auto feedback=world.human_ai_trigger_feedback(*state.human(),world.human_ai_target_human(*actor),&previous,&visible);
        if(!visible){
            const auto flags=state.words().template get<std::uint32_t>(0x50);
            if(!(flags&0x400u)&&human_frame_signed_word(feedback)<((flags&0x200u)?3:1))return;
        }
    }
    first()->set(0x18,1.f);
    auto* second=state.second_frame_inputs();if(!second)throw std::logic_error("AI trigger requires actual second input");
    second->set(0x18,1.f);
}
//00571A40. Admission, duration and command publication before any weapon
//worker. Near-point item2/feedback11 use the existing inline/B0 receivers.
template<class State,class World>bool begin_human_ai_aim(State& state,World& world){
    if(!state.item())return false;
    auto& resources=state.resources();auto& words=resources.state();
    if(!resources.aimed_actor()&&!(words.template get<std::uint32_t>(0xd8)&2u))return false;
    if(!state.words().template get<std::uint8_t>(0xad)){
        auto* input=state.second_frame_inputs();if(!input)throw std::logic_error("AI aim start requires actual second input");
        input->set(0x18,1.f);
    }
    auto& target=resources.target().state;
    const auto base=target.template get<std::int32_t>(0x3c),span=target.template get<std::int32_t>(0x40);
    const auto random=static_cast<long double>(world.human_sound_random());
    const auto duration=base==-1?random*300.f+100.f:((random-.5f)*span)*2+base;
    words.set(0xc4,human_animation_time_integer(duration));
    if(words.template get<std::uint32_t>(0xd8)&2u)words.set(0xc0,std::uint8_t{0});
    state.words().set(0x50,state.words().template get<std::uint32_t>(0x50)|0x200000u);
    if(state.words().template get<std::uint32_t>(0x50)&0x400000u)world.stop_human_ai_look(state);
    if(!(resources.query().state.template get<std::uint32_t>(0x50)&0x10000000u)&&
       (state.words().template get<std::uint32_t>(0x50)&2u)&&state.point()&&(state.point()->flags&0x4000u)){
        words.set(0xc8,200u);world.begin_human_ai_inline_action(state);
    }
    return true;
}
//00572C20: target44/48 optional stop duration, original action22/command12,
//then clear D0/D8 and sample actual item deadlines with distinct game28 reads.
template<class State,class World>
void stop_human_ai_aim(State& state,std::uint32_t delay,World& world){
    auto& resources=state.resources();auto& target=resources.target().state;
    const auto base=target.template get<std::int32_t>(0x44),span=target.template get<std::int32_t>(0x48);
    if(base!=-1){
        const auto scaled=(static_cast<long double>(world.human_sound_random())-.5f)*span;
        delay=static_cast<std::uint32_t>(human_animation_time_integer(scaled+scaled+base));
    }
    const auto flags=state.words().template get<std::uint32_t>(0x50);
    state.words().set(0x50,flags&~0x200000u);
    if(flags&0x8000000u)world.finish_human_ai_inline_action(state);
    resources.publish_aimed_actor(nullptr);resources.state().set(0xd8,0u);
    if(state.item()){
        const auto deadlines=[&]{
            if(!state.item())throw std::logic_error("AI stop lost actual item between native deadline samples");
            auto& item=world.human_frame_output_item(state.item());
            const auto first_now=world.game_word28();const auto first=human_frame_signed_word(item.field14-first_now);
            const auto second_now=world.game_word28();
            const auto second=human_animation_time_integer(-static_cast<long double>(human_frame_signed_word(second_now-item.field58)));
            return std::array<std::int32_t,2>{{first,second}};
        };
        const auto first=deadlines();
        if((first[0]>0||first[1]>0)&&static_cast<std::uint32_t>(std::max(first[0],first[1]))>delay){
            const auto fresh=deadlines();
            resources.state().set(0xc8,fresh[0]<1&&fresh[1]<1?0u:static_cast<std::uint32_t>(std::max(fresh[0],fresh[1])));return;
        }
    }
    resources.state().set(0xc8,delay);
}
}
#endif
