#ifndef HD2_GAME_DOG_CONTROLLER_TARGETS_H
#define HD2_GAME_DOG_CONTROLLER_TARGETS_H
#include "game/dog_damage.h"
#include "game/dog_controller_vector.h"
#include "game/human_ai_event_queue.h"
#include <cmath>
namespace hd2::game {
template<class State,class Pool>auto make_dog_target_event(State& state,Pool& pool){ //005CC8D0 positive event producer.
    if(!state.target88)throw std::runtime_error("Dog target event lost actual target88");
    const auto position=state.target88->base().world_position();auto event=pool.acquire();
    event->words[0]=0x10000017u;event->words[1]=10000;event->words[2]=0;
    event->words[3]=state.actor10?static_cast<std::uint32_t>(state.actor10->base().id10()):0xffffffffu;
    event->words[4]=state.target88?static_cast<std::uint32_t>(state.target88->base().id10()):0xffffffffu;
    std::memcpy(event->words.data()+8,position.data(),16);event->words[12]=0;return event;
}
inline long double dog_controller_length(float square){
    std::int32_t bits;std::memcpy(&bits,&square,4);
    return bits<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(square));
}
inline float dog_controller_square(const std::array<float,4>& from,const std::array<float,4>& to,bool reverse=false){
    const auto x=static_cast<long double>(to[0])-from[0],y=static_cast<long double>(to[1])-from[1],z=static_cast<long double>(to[2])-from[2];
    return static_cast<float>(reverse?(z*z+y*y)+x*x:(x*x+y*y)+z*z);
}
//005CC7A0 intentionally computes (1000000-distance)+distance. Do not replace
//the original equal-score/first-candidate policy with a nearest-target heuristic.
template<class State,class Actor,class Queries>void consider_dog_target(State& state,Actor* actor,float& score,Queries& queries){
    if(!actor)throw std::runtime_error("Dog target selection reaches native NULL candidate");
    if(queries.health(*actor)<=0||queries.attached(*actor))return;
    if(!state.actor10)throw std::runtime_error("Dog target selection requires actual Actor10");
    const auto from=state.actor10->base().world_position(),to=actor->base().world_position();
    auto distance=dog_controller_length(dog_controller_square(from,to,true));if(distance>1000000.f)distance=1000000.f;
    const auto candidate=(1000000.L-distance)+distance;
    if(candidate>score){score=static_cast<float>(candidate);state.target88=actor;}
}
//005CC8D0. Scans the SAME live spotted574/heard564 lists; damage_source8C is
//an unconditional override before target reset, list reads or event production.
template<class State,class Queries>void select_dog_controller_target(State& state,bool force,Queries& queries){
    if(state.damage_source8c){state.target88=state.damage_source8c;return;}
    float score=0;
    if(force&&state.target88){state.target90.xyz=std::array<float,3>{{-10000,-10000,-10000}};state.target90.w.reset();state.target88=nullptr;}
    else if(state.target88){
        if(!state.actor10)throw std::runtime_error("Dog current target score requires actual Actor10");
        float distance=static_cast<float>(dog_controller_length(dog_controller_square(state.actor10->base().world_position(),state.target88->base().world_position())));
        if(distance>1000000.f)distance=1000000.f;
        score=static_cast<float>((1000000.L-distance)+distance);
    }else score=-10000;
    for(std::size_t n=0;n<queries.dog().spotted574.size();++n)consider_dog_target(state,queries.dog().spotted574[n],score,queries);
    for(std::size_t n=0;n<queries.dog().heard564.size();++n){
        auto* record=queries.dog().heard564[n];if(!record)throw std::runtime_error("Dog target selection reaches native NULL heard record");
        if(!record->strength1c)throw std::runtime_error("Dog target selection requires actual heard1C strength");
        if(*record->strength1c>19){
            if(!record->source14)throw std::runtime_error("Dog target selection requires actual heard14 source");
            consider_dog_target(state,queries.source_actor(*record->source14),score,queries);
        }
    }
    if(state.target88)queries.target_event(); //Selected4/5 virtual34 returns current follow18, never a constant NULL.
}
//005CD120. Native flag clearing precedes optional Script completion callback.
template<class State,class Queries>void clear_dog_controller_target(State& state,Queries& queries){
    state.target90.xyz=std::array<float,3>{{-10000,-10000,-10000}};state.target90.w.reset();
    const auto flags=state.flags20;state.flags20=flags&0xfffcffffu;
    state.followa0.xyz=std::array<float,3>{{-10000,-10000,-10000}};state.followa0.w.reset();state.target88=state.damage_source8c=nullptr;
    if((flags&0x100000u)&&state.kind14==5&&state.scriptc0&&(flags&0x40000u)){queries.target_finished(state.scriptc0);state.flags20&=0xffefffffu;}
}
//005CDFF0: unusual unsigned now+2500000<record.clock admission, and native
//forward increment AFTER swap/pop deliberately skips the replacement element.
template<class Queries>void retire_dog_controller_heard(Queries& queries){
    for(std::size_t n=0;n<queries.dog().heard564.size();++n){
        const auto now=queries.microseconds();auto* record=queries.dog().heard564[n];
        if(!record)throw std::runtime_error("Dog heard retirement reaches native NULL clock18 read");
        if(!record->clock18)throw std::runtime_error("Dog heard retirement requires actual record18 clock");
        if(static_cast<std::uint32_t>(now+2500000u)<*record->clock18){
            queries.release(record);auto& list=queries.dog().heard564;list[n]=list.back();list.pop_back();
        }
    }
}
}
#endif
