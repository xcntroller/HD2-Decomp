#ifndef HD2_GAME_MISSION_SCENE_ACTIVATION_H
#define HD2_GAME_MISSION_SCENE_ACTIVATION_H

#include "game/human_joint_owners.h"
#include <cmath>
#include <vector>

namespace hd2::game {

//0069BFF0/0069C080: borrowed LIGHT frames, not owned engine references. Restore
//before scene reset. Native destruction only frees the pointer buffer; it does
//not invent visibility callbacks or extend the scene's lifetime.
template<class Frames>class MissionLightVisibility {
public:
    using Frame=typename Frames::Frame;
    explicit MissionLightVisibility(Frames& frames):frames_(frames){}
    void restore() { //0069BFF0(true), reverse SetOn only, no frame update.
        for(auto n=hidden_.size();n;)frames_.object_visible(hidden_[--n],true);
        hidden_.clear();
    }
    void collect() { //0069BFF0(false), restore old list BEFORE scene enumeration.
        restore();frames_.enumerate_activation_scene(false,2,[&](Frame frame){
            const auto light=frames_.mission_light_properties(frame);
            if(!light.visible||light.type<=0||(light.type>4&&light.type!=8)||(light.flags&9u))return;
            frames_.object_visible(frame,false);frames_.update_scene_object(frame);
            hidden_.push_back(frame); //Publish after visibility/update; preserve writes if allocation fails.
        });
    }
    const auto& hidden()const{return hidden_;}
private:
    Frames& frames_;std::vector<Frame> hidden_;
};

//0072D2B0 mode3/0072D5F0: actual scene-root VISUAL world boxes produce network
//world bounds. Quantizer binding/reset below must use the SAME session manager;
//this producer is not an independent successful input or packet implementation.
class MissionWorldBounds {
public:
    template<class Frames,class Inputs>void rebuild(Frames& frames,Inputs& inputs) {
        HumanVector minimum{{1.e16f,1.e16f,1.e16f,0}},maximum{{-1.e16f,-1.e16f,-1.e16f,0}};
        frames.enumerate_activation_scene(true,1,[&](typename Frames::Frame frame){
            const auto bounds=frames.mission_frame_world_bounds(frame); //Native+58 lazy update, then frame10/20.
            for(unsigned axis=0;axis!=3;++axis) {
                if(bounds.minimum[axis]<minimum[axis])minimum[axis]=bounds.minimum[axis];
                if(maximum[axis]<bounds.maximum[axis])maximum[axis]=bounds.maximum[axis];
            }
        });
        for(unsigned axis=0;axis!=3;++axis)minimum_[axis]=static_cast<float>(static_cast<long double>(minimum[axis])-.05000000074505806f);
        //Z difference is stored before +.1; X/Y remain extended until final store.
        const float z=static_cast<float>(static_cast<long double>(maximum[2])-minimum[2]);
        range_[0]=static_cast<float>((static_cast<long double>(maximum[0])-minimum[0])+.10000000149011612f);
        range_[1]=static_cast<float>((static_cast<long double>(maximum[1])-minimum[1])+.10000000149011612f);
        range_[2]=static_cast<float>(static_cast<long double>(z)+.10000000149011612f);
        inputs.bind_world_quantizer(minimum_,range_); //0072F500, before bit counts and queue resets.
        for(unsigned axis=0;axis!=3;++axis)bits_[axis]=encode_range(range_[axis],counts_[axis]);
        inputs.reset_world_packet_state(); //505C=0, offline00982104=0, three00724EC0 buffers.
    }
    const auto& minimum()const{return minimum_;}const auto& range()const{return range_;}
    const auto& bits()const{return bits_;}const auto& counts()const{return counts_;}
    static std::uint32_t encode_range(float range,std::uint32_t& count) { //0072D900, __ftol truncates range*100 and keeps EAX.
        const auto scaled=static_cast<long double>(range)*100.f;
        if(!std::isfinite(scaled)||scaled<0||scaled>=2147483649.L)
            throw std::runtime_error("Mission world range exceeds native finite encoding");
        count=static_cast<std::uint32_t>(std::trunc(scaled));
        if(count>0x80000000u)throw std::runtime_error("Mission world range would overflow the native power-of-two loop");
        std::uint32_t power=2,bits=1;while(power<count){power*=2;++bits;}return bits;
    }
private:
    HumanVector minimum_{},range_{};std::array<std::uint32_t,3> bits_{},counts_{};
};
} //namespace hd2::game
#endif
