#ifndef HD2_GAME_DOG_FRAME_HISTORY_H
#define HD2_GAME_DOG_FRAME_HISTORY_H
#include "game/dog_body_orders.h"
namespace hd2::game {
//005D4DB0 produces these SAME actual Actor580/590/5A0 fields, not clip movement.
class DogFrameHistory {
public:
    using Vector=std::array<float,4>;
    Vector velocity580{};std::optional<Vector> position590;std::optional<std::int32_t> elapsed5a0;
    void initialize(const Vector& position){velocity580.fill(0);position590=position;elapsed5a0=0;}
    template<class Actor,class Frames>void advance(Actor& actor,std::int32_t elapsed,Frames& frames){ //005D64E0.
        if(!position590)throw std::runtime_error("Dog frame history requires actual005D4DB0 position590");
        velocity580=*position590; //Copied BEFORE actual world-matrix callback, including undefined/unconsumed W.
        const auto model=actor.base().frame();const auto matrix=frames.actor_world_matrix(model);
        position590=Vector{{matrix[12],matrix[13],matrix[14],matrix[15]}};
        for(unsigned n=0;n!=3;++n)velocity580[n]=static_cast<float>(static_cast<long double>(velocity580[n])-(*position590)[n]);
        const auto square=(static_cast<long double>(velocity580[0])*velocity580[0]+static_cast<long double>(velocity580[1])*velocity580[1])+static_cast<long double>(velocity580[2])*velocity580[2];
        if(!(square<1.0000000168623835e-16f)){
            if(!elapsed5a0)throw std::runtime_error("Dog velocity requires actual previous5A0 elapsed producer");
            const auto factor=-1000.L/ *elapsed5a0; //Exact signed int and extended quotient. Zero deliberately retains native NaN/Inf.
            for(unsigned n=0;n!=3;++n)velocity580[n]=static_cast<float>(factor*velocity580[n]);
        }
        elapsed5a0=elapsed;
    }
};
}
#endif
