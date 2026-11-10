#ifndef HD2_GAME_EFFECTS_FADE_H
#define HD2_GAME_EFFECTS_FADE_H

#include <array>
#include <cstdint>
#include <cstring>
#include <limits>

namespace hd2::game {

//GEffects004860B0 zeroes2C0..2DC.00486B30 virtual8C sets the actual fade
//target/rate/colour;00487240 advances that SAME fade state. Rendering stays
//separate. Native null-colour
//path copies an indeterminate W from its stack; source preserves W and marks
//it unspecified rather than inventing a meaningful fourth colour component.
class EffectsFade {
public:
    void configure(std::int32_t duration,bool enabled,const std::array<float,4>* colour=nullptr) {
        target_=enabled?1.f:0.f;
        rate_=duration?static_cast<float>(1000.L/static_cast<long double>(duration)):std::numeric_limits<float>::max();
        if(colour){colour_=*colour;colour_w_defined_=true;}
        else {colour_[0]=colour_[1]=colour_[2]=0;colour_w_defined_=false;}
        if(current_<0)current_=0;
    }
    void reset_loading(bool finished)noexcept{target_=0;current_=finished?-1000.f:0.f;} //004885D0/004888C0 write SAME2D4/2D0; preserve rate/colour.
    void update(std::uint32_t elapsed) { //00487240/004874BE..00487555.
        if(current_<0){target_=0;current_=0;} //Completed sentinel consumed on NEXT tick.
        if(target_==current_)return;
        std::int32_t signed_elapsed;std::memcpy(&signed_elapsed,&elapsed,4);
        //0080F514 is FLOAT .001, not exact decimal1/1000. Native FSTS writes
        //current but compares the still-extended sum against target afterwards.
        const auto step=static_cast<long double>(signed_elapsed)*rate_*.001000000047497451305389404296875f;
        const bool rising=current_<target_;
        const auto next=rising?step+current_:static_cast<long double>(current_)-step;
        current_=static_cast<float>(next);
        if(rising?next>target_:next<target_)current_=-1.f;
    }
    float current()const{return current_;}float target()const{return target_;}float rate()const{return rate_;}
    const auto& colour()const{return colour_;}bool colour_w_defined()const{return colour_w_defined_;}
private:
    std::array<float,4> colour_{};float current_=0,target_=0,rate_=0;bool colour_w_defined_=true;
};
} //namespace hd2::game
#endif
