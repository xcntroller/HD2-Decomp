#ifndef HD2_GAME_HUMAN_AI_SCENE_STEERING_H
#define HD2_GAME_HUMAN_AI_SCENE_STEERING_H

#include "game/human_joint_owners.h"
#include "game/human_state_frame_timers.h"
#include <cmath>

namespace hd2::game {
inline long double human_scene_steering_square(const std::array<float,3>& velocity){
    const long double x=velocity[0],y=velocity[1],z=velocity[2];
    return (x*x+y*y)+z*z;
}

//0057EF30 normalizes horizontal vectors with a stored FLOAT square. The aim
//vector keeps X/Z extended through the dot and cross, unlike the movement vector.
inline std::array<long double,3> normalize_ai_scene_horizontal(float x,float z){
    const long double square=static_cast<long double>(x)*x+static_cast<long double>(z)*z;
    constexpr float epsilon=9.99999993922529e-09f;
    if(std::abs(square-1)<epsilon)return {{x,0,z}};
    const auto stored=static_cast<float>(square);
    if(!(stored<epsilon)){
        const auto scale=1/std::sqrt(static_cast<long double>(stored));
        return {{x*scale,static_cast<float>(0.L*scale),z*scale}};
    }
    if(x!=0)return {{x<0?-1.L:1.L,0,z}};
    if(z!=0)return {{x,0,z<0?-1.L:1.L}};
    return {{0,1,0}};
}

//0057FDA0/0057FD30, shared by the free-steering and active-path outputs.
template<class AI,class Movement>
bool human_ai_restrict_run(AI& ai,const Movement& movement){
    if(const auto* point=ai.point();point&&((point->flags&0x800u)||movement.flag109))return true;
    const auto& jobs=ai.resources().navigation_jobs();
    if(jobs.empty())return false;
    const auto& first=jobs.front()->words;
    const auto kind=first.template get<std::uint8_t>(0x26);
    return (first.template get<std::uint16_t>(0x24)&0x800u)||kind==3||
        (jobs.size()>1&&kind==1&&jobs[1]->words.template get<std::uint8_t>(0x26)==1);
}
template<class AI,class Movement,class Input>
void apply_human_ai_navigation_stance(AI& ai,const Movement& movement,const Input& input,bool slow_strafe){
    auto& words=ai.words();
    auto& resources=ai.resources();
    const auto request=resources.query().state.template get<std::uint32_t>(0x54);
    const auto restrict_run=[&]{return human_ai_restrict_run(ai,movement);};
    const auto restricted_stance=[&]{
        return restrict_run()||resources.flag().state.template get<std::uint32_t>(0)!=1||
            !(words.template get<std::uint32_t>(0x50)&4u)||
            (words.template get<std::uint8_t>(0xad)&&ai.item())||
            input.template get<float>(0xc)!=0||input.template get<float>(8)!=0||input.template get<float>(4)!=0;
    };
    std::uint32_t stance=0x2000u;
    if(request&0x8000u)stance=0x8000u;
    else if(request&0x1000u)stance=0x1000u;
    else if((request&0x4000u)&&!restrict_run()&&!slow_strafe)stance=0x4000u;
    else if(request&0x10000u){
        if(!restricted_stance())stance=0x10000u;
        else if(!restrict_run())stance=0x4000u;
    }
    words.set(0x5c,stance);
}

//0057EF30 over the actual Human+2C4 steering record and the B0/AI owners.
//The movement W lane is uninitialized in the proven cold writer. Keep the
//existing direction W when the original reads that lane.
template<class AI,class Movement>void apply_human_ai_scene_steering(AI& ai,const Movement& movement){
    auto* input=ai.first_frame_inputs();
    if(!input)throw std::logic_error("AI scene steering requires the actual first frame input buffer");
    auto& words=ai.words();
    auto& resources=ai.resources();
    input->set(0x30,0.f);
    const float square=static_cast<float>(human_scene_steering_square(movement.steering));
    std::uint32_t bits;
    std::memcpy(&bits,&square,sizeof bits);
    const auto speed=human_frame_signed_word(bits)<0x322bcc77?0.L:
        std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(square));
    words.set(0x58,static_cast<float>(speed));

    const auto& direction=resources.direction().state();
    const auto aim=direction.template get<HumanVector>(0x10);
    const auto m=normalize_ai_scene_horizontal(movement.steering[0],movement.steering[2]);
    const auto a=normalize_ai_scene_horizontal(aim[0],aim[2]);
    const float mx=static_cast<float>(m[0]),my=static_cast<float>(m[1]),mz=static_cast<float>(m[2]);
    const auto dot_extended=(a[2]*mz+a[0]*mx)+a[1]*my;
    const auto dot=static_cast<float>(dot_extended);
    const auto absolute=static_cast<float>(std::abs(dot_extended));
    const bool right=a[0]*mz-a[2]*mx>0;
    const bool forward=dot>0;
    const bool side_only=absolute<.38268300890922546f;
    const bool diagonal=!side_only&&absolute<.9238799810409546f;
    HumanVector desired{};
    if(side_only||diagonal){
        input->set(0xc,right?0.f:1.f);
        input->set(8,right?1.f:0.f);
        desired={{right?mz:-mz,0,right?-mx:mx,aim[3]}};
    }
    if(!side_only){
        input->set(0,forward?1.f:0.f);
        input->set(4,forward?0.f:1.f);
        if(diagonal){
            desired[0]=static_cast<float>(static_cast<long double>(desired[0])+(forward?mx:-mx));
            desired[2]=static_cast<float>(static_cast<long double>(desired[2])+(forward?mz:-mz));
        }else desired={{forward?mx:-mx,0,forward?mz:-mz,
            forward?direction.template get<float>(0xc):aim[3]}};
    }
    const auto facing=normalize_ai_scene_horizontal(desired[0],desired[2]);
    for(unsigned axis=0;axis!=3;++axis)desired[axis]=static_cast<float>(facing[axis]);
    (void)resources.direction().set_current(desired);

    apply_human_ai_navigation_stance(ai,movement,*input,
        side_only&&(words.template get<std::uint32_t>(0x50)&1u));
}

template<class AI>void advance_human_ai_scene_steering(AI& ai){ //0057F610.
    const auto* human=ai.human();
    if(!human||!human->scene_movement||human->scene_movement->owner!=human)
        throw std::logic_error("AI scene steering requires the bound Human+2C4 movement owner");
    const auto& movement=*human->scene_movement;
    auto& query=ai.resources().query().state;
    constexpr std::uint32_t steering=0x100000u;
    const auto moving=[&]{return human_scene_steering_square(movement.steering)>.009999999776482582f;}; //0057EEC0.
    auto flags=query.template get<std::uint32_t>(0x50);
    if(!(flags&steering)&&moving()){
        flags|=steering;
        query.set(0x50,flags);
    }
    if(!(flags&steering))return;
    if(moving()&&!(ai.words().template get<std::uint32_t>(0x50)&1u)&&!(flags&0x10000f30u)){ //0057EF00.
        apply_human_ai_scene_steering(ai,movement);
        return;
    }
    query.set(0x50,flags&~steering);
}
} //namespace hd2::game
#endif
