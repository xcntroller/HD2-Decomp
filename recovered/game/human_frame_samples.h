#ifndef HD2_GAME_HUMAN_FRAME_SAMPLES_H
#define HD2_GAME_HUMAN_FRAME_SAMPLES_H

#include "game/human_activation_fields.h"
#include <cstring>

namespace hd2::game {
//005A0420. DWORD elapsed is zero-extended, not signed; comparison uses the
//extended result after its FLOAT store. NaN remains unordered, not clamped.
inline void advance_human_float_timer(HumanActivationFields& fields,std::uint32_t elapsed){
    if(fields.field154!=0){
        const auto value=static_cast<long double>(fields.field154)-static_cast<long double>(elapsed)*.0010000000474974513f;
        fields.field154=static_cast<float>(value);if(value<0)fields.field154=0;
    }
}
//0059FE30 prefix, before the registered14 predicate. These countdowns run
//even for an inactive/unregistered Human;254 also clears214 on signed<=0.
inline void advance_human_presentation_timers(HumanActivationFields& fields,std::uint32_t elapsed){
    const auto negative=[](std::uint32_t value){return (value&0x80000000u)!=0;};
    if(fields.field17c){fields.field17c-=elapsed;if(negative(fields.field17c))fields.field17c=0;}
    if(fields.field254){fields.field254-=elapsed;if(!fields.field254||negative(fields.field254)){fields.field254=0;fields.flag214=false;}}
}
//005A0480: SAME154 timer/158 cadence, engine RandFloat (not CRT rand), and
//required actual0044F4F0 blood receiver. Native writes XYZ only in its stack
//vectors; source retains model W/defines direction W0 without credit.
template<class Human,class Frames,class Random,class Drops>
void advance_human_frame_bleeding(Human& human,Frames& frames,Random& random,Drops& drops,
        std::uint32_t elapsed,std::uint32_t mode){
    auto& fields=human.activation;if(fields.field154==0)return;
    if(!(mode&2u))advance_human_float_timer(fields,elapsed);
    fields.field158+=elapsed;
    const auto& velocity=fields.vector1b0;
    const auto square=(static_cast<long double>(velocity[2])*velocity[2]+static_cast<long double>(velocity[1])*velocity[1])+static_cast<long double>(velocity[0])*velocity[0];
    const bool moving=square>=1.0000000168623835e-16f;
    std::int32_t cadence;std::memcpy(&cadence,&fields.field158,4);
    if(cadence<=(moving?1500:2500))return;
    fields.field158=0;
    const auto model=human.base().frame();if(!model)throw std::logic_error("Human blood emission requires actual model20");
    const auto matrix=frames.actor_world_matrix(model);
    HumanVector position{{matrix[12],static_cast<float>(static_cast<long double>(matrix[13])+1),matrix[14],matrix[15]}};
    const auto x=static_cast<long double>(random())-.5f,z=static_cast<long double>(random())-.5f;
    HumanVector direction{{static_cast<float>(static_cast<long double>(static_cast<float>(x+x))*(moving?.1f:.25f)),
        -.9900000095367432f,static_cast<float>(z*(moving?.2f:.5f)),0}};
    drops(position,direction); //No success or skipped receiver for a positive emission.
}
//005A0660 samples the SAME model pose and native1B0/1C0/1D0 storage. The
//native difference is old-new and the factor is NEGATIVE1000/unsigned elapsed.
//No elapsed clamp or synthetic motion/pose is supplied. Attached W lanes are
//unwritten stack values in the original; source retains them without credit.
template<class Human,class Frames>void sample_human_frame_motion(Human& human,Frames& frames,std::uint32_t elapsed){
    auto& fields=human.activation;
    if(human.field160){
        for(unsigned n=0;n!=3;++n)fields.vector1b0[n]=fields.vector1c0[n]=0;
        return;
    }
    fields.vector1c0=fields.vector1b0;fields.vector1b0=fields.position1d0;
    const auto model=human.base().frame();if(!model)throw std::logic_error("Human motion sample requires actual model20");
    const auto matrix=frames.actor_world_matrix(model);fields.position1d0={{matrix[12],matrix[13],matrix[14],matrix[15]}};
    for(unsigned n=0;n!=3;++n)fields.vector1b0[n]=static_cast<float>(static_cast<long double>(fields.vector1b0[n])-fields.position1d0[n]);
    const auto& delta=fields.vector1b0;
    const auto square=(static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[1])*delta[1])+static_cast<long double>(delta[2])*delta[2];
    if(square>=1.0000000168623835e-16f){
        const auto scale=-1000.L/elapsed;
        for(unsigned n=0;n!=3;++n)fields.vector1b0[n]=static_cast<float>(fields.vector1b0[n]*scale);
    }
}
} //namespace hd2::game
#endif
