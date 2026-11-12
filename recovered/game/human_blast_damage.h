#ifndef HD2_GAME_HUMAN_BLAST_DAMAGE_H
#define HD2_GAME_HUMAN_BLAST_DAMAGE_H
#include "game/human_loading_lifetime.h"
#include <stdexcept>
namespace hd2::game {
//0059C2A0 is genuine constant Human+D8, independent of the hit frame.
inline constexpr std::int32_t human_blast_material=0x42;
//006DDD40 supplies the actual borrowed source1C and a final hit-kind ZERO to
//Human005A1C90. The containing World validates source lifetime before this call.
template<class Human>void apply_human_blast_damage(Human& human,std::uint32_t kind,float amount,
        const HumanVector& point,const HumanVector& impulse,Human* source){
    if(!human.loading_lifetime)throw std::logic_error("Blast Human damage requires actual containing lifetime");
    human.loading_lifetime->damage(kind,amount,point,impulse,source,0);
}
}
#endif
