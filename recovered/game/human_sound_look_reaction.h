#ifndef HD2_GAME_HUMAN_SOUND_LOOK_REACTION_H
#define HD2_GAME_HUMAN_SOUND_LOOK_REACTION_H

#include "game/human_joint_math.h"
#include <cmath>
#include <stdexcept>

namespace hd2::game {

//00573900(kind2)/00573C50(kind0). The SAME Human owns head260, perception268
//and inline98; callers enqueue the common sound event BEFORE entering here.
template<class AI,class Record,class Frames,class Inline>
void human_sound_look_reaction(AI& ai,const Record& record,bool once,Frames& frames,Inline& control) {
    auto& state=ai.resources().state();
    if(once&&state.template get<std::uint32_t>(0x108))return;
    auto* human=ai.human();if(!human)throw std::logic_error("Human sound look reaction has no bound Human");
    if(!record.actor)throw std::runtime_error("Human sound look reaction requires its actual source actor");
    if(record.actor->base().type()==3) { //005A5E50: observer's hostile membership, not friendship.
        const auto& enemies=human->perception.lists()[0];
        for(auto n=enemies.size();n;)if(enemies[--n]==record.actor)return;
    }
    if(!human->head_marker)throw std::runtime_error("Human sound look reaction requires actual head marker260");
    const auto head=frames.actor_world_matrix(human->head_marker);
    HumanVector direction{{static_cast<float>(static_cast<long double>(record.position[0])-head[12]),
        static_cast<float>(static_cast<long double>(record.position[1])-head[13]),
        static_cast<float>(static_cast<long double>(record.position[2])-head[14]),0}};
    //57395C leaves the Z subtraction extended for its first multiply.
    const auto dz=static_cast<long double>(record.position[2])-head[14];
    auto square=(dz*direction[2]+static_cast<long double>(direction[1])*direction[1])+
        static_cast<long double>(direction[0])*direction[0];
    constexpr float epsilon=9.99999993922529e-09f;
    auto stored=static_cast<float>(square);
    if(!(std::abs(square-1)<epsilon)) {
        if(!(stored<epsilon)) {
            const auto scale=1/std::sqrt(static_cast<long double>(stored));
            for(unsigned n=0;n!=3;++n)direction[n]=static_cast<float>(direction[n]*scale);
        }else {
            const auto axis=direction[0]!=0?0u:direction[2]!=0?2u:1u;
            direction[axis]=direction[axis]<0?-1.f:1.f;
        }
    }
    const auto model=frames.actor_world_matrix(human->base().frame());
    long double x=model[8],y=model[9];float z=model[10];
    square=(y*y+static_cast<long double>(z)*z)+x*x;stored=static_cast<float>(square);
    if(!(std::abs(square-1)<epsilon)) {
        if(!(stored<epsilon)) {
            const auto scale=1/std::sqrt(static_cast<long double>(stored));x*=scale;y*=scale;z=static_cast<float>(z*scale);
        }else if(x!=0)x=x<0?-1:1;
        else if(z!=0)z=z<0?-1.f:1.f;
        else y=y<0?-1:1;
    }
    //Forward X/Y remain extended, Z is stored. Unordered comparisons continue.
    if((static_cast<long double>(z)*direction[2]+y*direction[1])+x*direction[0]<-.4f)return;
    const auto right=frames.actor_world_matrix(human->base().frame());
    const auto side=(static_cast<long double>(direction[2])*right[2]+static_cast<long double>(direction[1])*right[1])+
        static_cast<long double>(direction[0])*right[0];
    if(side>0||side<0) {
        control.look_command(side>0?5:6,nullptr); //00570A10 command5 → SAME inline98.
        ai.words().set(0x50,ai.words().template get<std::uint32_t>(0x50)&~0x400000u);
        state.set(0x108,std::uint32_t{1500});
    }
    state.set(0x110,std::uint32_t(once?1000:2000));
}
} //namespace hd2::game
#endif
