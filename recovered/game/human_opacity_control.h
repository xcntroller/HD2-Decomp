#ifndef HD2_GAME_HUMAN_OPACITY_CONTROL_H
#define HD2_GAME_HUMAN_OPACITY_CONTROL_H

#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace hd2::game {

//DD0+DA4:00789CA0 reset,00789C80 getter,00789F60 smoothing,
//00789CD0 loading update and00789FC0 actual sector ambient selection.
//The module+54 receiver is mandatory if opacity changes; no default consumer.
class HumanOpacityControl {
public:
    template<class Random>void reset(Random& random) {
        target_=applied_=1;timer_=static_cast<std::int32_t>(random()%1000)+250;
    }
    float opacity() const{return target_>1?1.f:target_;}
    float applied() const{return applied_;}
    void target(float value){target_=value;} //Borrowed producers write native+04.
    void smooth(std::int32_t elapsed) {
        const float step=static_cast<float>(static_cast<long double>(elapsed)*.001f*2);
        const auto value=opacity();
        //Native unordered comparison follows the decreasing branch.
        if(value>applied_) {
            const auto next=static_cast<long double>(applied_)+step;
            applied_=static_cast<float>(next);if(next>=value)applied_=value;
        }else {
            const auto next=static_cast<long double>(applied_)-step;
            applied_=static_cast<float>(next);if(next<=value)applied_=value;
        }
    }
    template<class Frame,class Services>void update(Frame model,std::int32_t elapsed,unsigned mode,Services& services) {
        if(opacity()!=applied_) {smooth(elapsed);services.human_opacity_changed(model,applied_);} //00486510+54.
        const std::int32_t threshold=mode==0?250:mode==1?500:mode==3?10000000:mode==5?1400:1000;
        const auto old=timer_;
        const auto bits=static_cast<std::uint32_t>(timer_)+static_cast<std::uint32_t>(elapsed);
        std::memcpy(&timer_,&bits,4); //Native signed wrapping addition.
        if(old<threshold)return;
        if(services.human_frame_epoch(model)==services.human_scene_epoch()||elapsed>999||mode==0) {
            flag10_=true;timer_%=threshold;
        }else timer_=threshold;
    }
    template<class Frames,class Frame>void bind_sector(Frames& frames,Frame sector) {
        if(!sector)throw std::runtime_error("Human opacity requires its actual scene sector");
        const auto count=frames.human_sector_light_count(sector);
        for(std::uint32_t index=0;index<count;++index) {
            const auto light=frames.human_sector_light(sector,index);
            if(!light)throw std::runtime_error("Human sector contains a null light");
            if(!frames.car_frame_visible(light)||frames.human_light_type(light)!=4)continue;
            const auto rgb=frames.human_light_colour(light);
            //Strict comparisons deliberately fall through to Z on tied maxima.
            const auto component=rgb[0]>rgb[1]&&rgb[0]>rgb[2]?rgb[0]:rgb[1]>rgb[0]&&rgb[1]>rgb[2]?rgb[1]:rgb[2];
            ambient_=static_cast<float>(static_cast<long double>(component)*frames.human_light_multiplier(light));return;
        }
        ambient_=1;
    }
    float ambient() const{return ambient_;}
    std::int32_t timer() const{return timer_;}
    bool flag10() const{return flag10_;}
    void request_refresh(){flag10_=true;} //Native DB4 writer in007646F0/00755500.
    float field0() const{return field0_;}
private:
    float field0_=1,target_=1,applied_=1;
    std::int32_t timer_=0;
    bool flag10_=true;
    float ambient_=1;
};
} //namespace hd2::game
#endif
