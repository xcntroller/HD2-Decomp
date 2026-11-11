#ifndef HD2_GAME_HUMAN_AI_SHOT_DIRECTION_H
#define HD2_GAME_HUMAN_AI_SHOT_DIRECTION_H

#include "game/human_joint_owners.h"
#include <cmath>

namespace hd2::game {

//00572120 uses LS3DF RandFloat. The two baseline draws precede the optional
//recent-shot and definition-command draws, independently of the spread vector.
template<class Random>long double human_ai_shot_spread(std::int32_t skill,std::uint32_t posture,
        std::uint32_t stance,std::int32_t since_shot,std::int32_t command,Random& random){
    const float skill_spread=static_cast<float>(1.L-static_cast<long double>(skill)*.01f);
    const auto posture_draw=random();
    float spread=static_cast<float>(2.L*skill_spread+
        (posture==0?static_cast<long double>(posture_draw)*.3f+.7f:
         posture==1?static_cast<long double>(posture_draw)*.6f+.3f:
                    static_cast<long double>(posture_draw)*.4f));
    const auto stance_draw=static_cast<long double>(random());
    switch(stance){
    case 0:spread=static_cast<float>(spread+stance_draw*.3f+.2f);break;
    case 1:spread=static_cast<float>(spread+stance_draw*.2f+.3f);break;
    case 2:spread=static_cast<float>(spread+stance_draw*.1f+.4f);break;
    case 5:spread=static_cast<float>(spread+stance_draw*.3f+.5f);break;
    default:spread=static_cast<float>(spread+stance_draw*.5f);break;
    }
    if(since_shot<2500)spread=static_cast<float>(spread+static_cast<long double>(random())*.6f+.4f);
    if(command==2)return spread+static_cast<long double>(random())*.4f+1.L;
    if(command==1||command==4)return spread+static_cast<long double>(random())*.2f+.3f;
    return spread;
}

inline void normalize_human_ai_shot(HumanVector& vector,long double length){
    constexpr float epsilon=9.99999993922529e-09f;
    if(std::abs(length-1)<epsilon)return;
    const float stored=static_cast<float>(length);
    if(!(stored<epsilon)){ //Native unordered comparison also takes the sqrt branch.
        const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));
        for(unsigned n=0;n!=3;++n)vector[n]=static_cast<float>(vector[n]*inverse);
    }else{
        const auto axis=vector[0]!=0?0u:vector[2]!=0?2u:1u;
        vector[axis]=vector[axis]<0?-1.f:1.f;
    }
}

template<class Random>void spread_human_ai_shot(HumanVector& direction,float spread,
        std::uint32_t state_kind,unsigned difficulty,Random& random){ //005722B0 ordinary path.
    if(state_kind==3)spread*=.4f;
    else if(difficulty==1)spread+=spread;
    else if(difficulty==2)spread*=1.5f;
    for(unsigned n=0;n!=3;++n)direction[n]*=100.f;
    HumanVector side{{-direction[2],0,direction[0],0}};
    const float side_square=static_cast<float>(static_cast<long double>(side[0])*side[0]+
        static_cast<long double>(side[2])*side[2]);
    normalize_human_ai_shot(side,side_square);
    for(unsigned n=0;n!=3;++n){
        const float amplitude=side[n]*spread;
        const auto offset=(static_cast<long double>(random())-.5f)*amplitude;
        direction[n]=static_cast<float>(direction[n]+offset+offset);
    }
    const float square=static_cast<float>((static_cast<long double>(direction[0])*direction[0]+
        static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[2])*direction[2]);
    normalize_human_ai_shot(direction,square);
}

template<class Random>bool aim_human_ai_ballistic_shot(HumanVector& direction,const HumanVector& origin,
        HumanVector target,float speed,std::uint32_t kind,std::uint32_t flags,std::int32_t skill,Random& random){
    if(kind!=3){
        const float spread=static_cast<float>(10.L-static_cast<long double>(skill)*.1f);
        const auto y=(static_cast<long double>(random())-.5f)*2;
        const auto x=(static_cast<long double>(random())-.5f)*2;
        const auto z=(static_cast<long double>(random())-.5f)*2;
        target[0]=static_cast<float>(target[0]+x*spread);
        target[1]=static_cast<float>(target[1]+static_cast<float>(y*spread));
        target[2]=static_cast<float>(target[2]+z*spread);
    }
    float height=origin[1];
    if(flags&4u)height+=1.8f;
    else if(flags&2u)height+=1.1f;
    else if(flags&1u)return false;
    target[0]-=origin[0];target[1]-=height;target[2]-=origin[2];
    constexpr float epsilon=9.99999993922529e-09f;
    const auto distance=[](float square){
        if(square<9.99999993922529e-09f)return 0.f;
        return std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.f:std::sqrt(square);
    };
    const auto horizontal=static_cast<long double>(target[0])*target[0]+static_cast<long double>(target[2])*target[2];
    const float planar_square=static_cast<float>(horizontal);
    const auto planar=distance(planar_square);
    const float flight=static_cast<float>(static_cast<long double>(distance(static_cast<float>(
        static_cast<long double>(target[0])*target[0]+static_cast<long double>(target[1])*target[1]+
        static_cast<long double>(target[2])*target[2])))*1.1f/speed);
    auto x=static_cast<long double>(target[0]),z=static_cast<long double>(target[2]);
    if(std::abs(static_cast<long double>(planar_square)-1)>=epsilon){
        if(planar_square>=epsilon){
            const auto inverse=1.L/std::sqrt(static_cast<long double>(planar_square));
            x*=inverse;z*=static_cast<float>(inverse);
        }else if(x!=0)x=x<0?-1:1;
        else if(z!=0)z=z<0?-1:1;
    }
    const auto angle=std::atan((static_cast<long double>(flight)*flight*4.905f+target[1])/planar);
    const auto cosine=std::cos(angle),sine=std::sin(static_cast<long double>(static_cast<float>(angle)));
    target[0]=static_cast<float>(x*cosine);
    target[1]=static_cast<float>(sine);
    target[2]=static_cast<float>(z*cosine);
    normalize_human_ai_shot(target,static_cast<long double>(target[0])*target[0]+
        static_cast<long double>(target[2])*target[2]+sine*target[1]);
    direction=target;
    return true;
}

} //namespace hd2::game
#endif
