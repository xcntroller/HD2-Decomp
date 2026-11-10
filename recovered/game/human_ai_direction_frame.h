#ifndef HD2_GAME_HUMAN_AI_DIRECTION_FRAME_H
#define HD2_GAME_HUMAN_AI_DIRECTION_FRAME_H
#include "game/human_joint_owners.h"
#include <cmath>

namespace hd2::game {
inline long double human_ai_steering_yaw(const HumanVector& current,const HumanVector& desired){
    const auto horizontal=[](const HumanVector& v){
        std::array<float,2> result{{v[0],v[2]}};
        const auto length=std::sqrt(static_cast<long double>(v[0])*v[0]+static_cast<long double>(v[2])*v[2]);
        if(length>9.99999993922529e-09f){const auto scale=1/length;result[0]=static_cast<float>(v[0]*scale);result[1]=static_cast<float>(v[2]*scale);}
        return result;
    };
    const auto a=horizontal(current),b=horizontal(desired);
    auto angle=std::atan2(static_cast<long double>(b[1]),static_cast<long double>(b[0]))-
        std::atan2(static_cast<long double>(a[1]),static_cast<long double>(a[0])); //007885B0: SECOND minus FIRST.
    if(angle< -3.1415927410125732f)angle+=6.2831854820251465f;
    if(angle>3.1415927410125732f)angle-=6.2831854820251465f;
    return angle;
}
//00575940: real124 directions/typed borrowed50 frame, actual Human head260,
//and mandatory00570A10/00570980 look emission. No animation tick substitute.
template<class State,class Frames,class Look>std::uint32_t advance_human_ai_frame_direction(State& state,
        std::uint32_t elapsed,Frames& frames,Look& look){
    auto& direction=state.resources().direction();auto& words=direction.state();
    if(const auto target=direction.target_frame()){
        auto* human=state.human();if(!human||!human->head_marker)throw std::logic_error("AI aim-frame update requires actual Human head260");
        const auto a=frames.actor_world_matrix(human->head_marker),b=frames.actor_world_matrix(target);
        long double x=static_cast<long double>(b[12])-a[12];
        HumanVector delta=words.template get<HumanVector>(0x10); //Native stack W is unwritten; source retains prior target W without credit.
        delta[0]=static_cast<float>(x);delta[1]=static_cast<float>(static_cast<long double>(b[13])-a[13]);delta[2]=static_cast<float>(static_cast<long double>(b[14])-a[14]);
        const auto y=static_cast<long double>(delta[1]),z=static_cast<long double>(delta[2]);
        const auto square=(x*x+y*y)+z*z;const float stored=static_cast<float>(square);
        constexpr float epsilon=9.99999993922529e-09f;
        if(!(std::abs(square-1)<epsilon)){
            if(!(stored<epsilon)){
                const auto scale=1/std::sqrt(static_cast<long double>(stored));x*=scale;delta[0]=static_cast<float>(x);
                delta[1]=static_cast<float>(y*scale);delta[2]=static_cast<float>(z*scale);
            }else{
                const auto axis=x!=0?0u:delta[2]!=0?2u:1u;delta[axis]=delta[axis]<0?-1.f:1.f;if(!axis)x=delta[0];
            }
        }
        const auto current=words.template get<HumanVector>(0);
        if((static_cast<long double>(delta[2])*current[2]+static_cast<long double>(delta[1])*current[1])+x*current[0]>-.1f||direction.current_locked()){
            const auto frame=direction.target_frame();if(!frame)throw std::runtime_error("AI look target lost actual borrowed frame");
            const auto matrix=frames.actor_world_matrix(frame);const HumanVector position{{matrix[12],matrix[13],matrix[14],matrix[15]}};
            look(8,&position);
        }else (void)direction.set_target(delta);
        direction.target_frame(typename std::decay_t<decltype(direction)>::FrameType{});
    }else{
        const auto delta=words.template get<HumanVector>(0x20);
        const auto square=(static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[1])*delta[1])+static_cast<long double>(delta[2])*delta[2];
        if(square>.1f){ //Native unordered comparison takes the interval branch.
            if(!direction.current_locked())(void)direction.set_target(delta);
            else{const auto yaw=human_ai_steering_yaw(words.template get<HumanVector>(0x10),delta);look(yaw<0?5u:6u,nullptr);}
            for(unsigned axis=0;axis!=3;++axis)words.set(0x20+axis*4,0.f); //Native W copies unwritten stack; retain source W.
        }else if(direction.advance_look_interval(elapsed))look(1,nullptr);
    }
    state.words().set(0x20,words.template get<HumanVector>(0));state.words().set(0x10,words.template get<HumanVector>(0x10));
    return 0;
}
} //namespace hd2::game
#endif
