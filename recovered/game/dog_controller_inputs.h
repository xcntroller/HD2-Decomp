#ifndef HD2_GAME_DOG_CONTROLLER_INPUTS_H
#define HD2_GAME_DOG_CONTROLLER_INPUTS_H
#include <stdexcept>
namespace hd2::game {
//005CE110, the actual kind4/5 virtual6C destination. Flags and the two
//buffers belong to the SAME controller/process; untouched scans stay intact.
template<class State>void publish_dog_controller_inputs(State& state){
    const auto first=[&]() -> auto& {if(!state.first60)throw std::runtime_error("Dog input publication requires actual first60 buffer");return *state.first60;};
    const auto second=[&]() -> auto& {if(!state.second64)throw std::runtime_error("Dog input publication requires actual second64 buffer");return *state.second64;};
    if(state.flags20&4u)first()[0]=1.f;
    else if(state.flags20&8u)first()[1]=1.f;
    first()[5]=(state.flags20&0x4000u)&&!(state.flags20&0x20000000u)?1.f:0.f;
    if(state.flags20&3u){second()[9]=1.f;state.flags20&=~3u;}
    else second()[9]=0.f;
    if(state.flags20&0x80000u)first()[4]=1.f;
}
} //namespace hd2::game
#endif
