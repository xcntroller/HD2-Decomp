#ifndef HD2_GAME_EFFECTS_FADE_H
#define HD2_GAME_EFFECTS_FADE_H

#include <array>
#include <cstdint>
#include <limits>

namespace hd2::game {

//GEffects004860B0 zeroes2C0..2DC.00486B30 virtual8C sets the actual fade
//target/rate/colour; render/tick consumers remain separate. Native null-colour
//path copies an indeterminate W from its stack; source preserves W and marks
//it unspecified rather than inventing a meaningful fourth colour component.
class EffectsFade {
public:
    void configure(std::int32_t duration,bool enabled,const std::array<float,4>* colour=nullptr) {
        target_=enabled?1.f:0.f;
        rate_=duration?static_cast<float>(1.L/static_cast<long double>(duration)):std::numeric_limits<float>::max();
        if(colour){colour_=*colour;colour_w_defined_=true;}
        else {colour_[0]=colour_[1]=colour_[2]=0;colour_w_defined_=false;}
        if(current_<0)current_=0;
    }
    float current()const{return current_;}float target()const{return target_;}float rate()const{return rate_;}
    const auto& colour()const{return colour_;}bool colour_w_defined()const{return colour_w_defined_;}
private:
    std::array<float,4> colour_{};float current_=0,target_=0,rate_=0;bool colour_w_defined_=true;
};
} //namespace hd2::game
#endif
