#ifndef HD2_GAME_HUMAN_ANIMATION_EVENTS_H
#define HD2_GAME_HUMAN_ANIMATION_EVENTS_H

#include "frontend/spatial_sound.h"
#include <cstdlib>
#include <limits>

namespace hd2::game {

//00774D80's stdcall(frame,event,text,time), RET16, and007759D0's marker
//dispatch. Actors resolves actual parent frame data and DD0 tracks; Sound owns
//the shared SOUND speech(+50) and spatial(+54) consumers. No fallback callbacks.
template<class Frames,class Actors,class Sound>
class HumanAnimationEvents {
public:
    using Frame=typename Frames::Frame;
    using Actor=typename Actors::Actor;
    HumanAnimationEvents(Frames& frames,Actors& actors,Sound& sound):frames_(frames),actors_(actors),sound_(sound) {}
    std::int32_t invoke(Frame marker,std::uint32_t,const char* text,std::int32_t time) { //00774D80.
        const auto parent=frames_.object_parent(marker);
        if(parent)if(auto* actor=actors_.animation_frame_actor(parent))actors_.queue_human_animation_event(*actor,text,time);
        dispatch_sound(marker,text);return 0; //Sound helper is unconditional, even without a parent actor.
    }
    void dispatch_sound(Frame marker,const char* text) { //007759D0; its event/time DWORDs are unused.
        const auto parent=frames_.object_parent(marker);
        auto* actor=parent?actors_.animation_frame_actor(parent):nullptr;
        if(!text)return;
        if(text[0]=='!') {
            if(std::char_traits<char>::length(text)<3)throw std::runtime_error("Truncated Human speech animation marker");
            (void)number(text+1);const auto id=number(text+3); //First parsed integer is discarded by the original.
            sound_.play_animation_speech(actor&&actors_.animation_actor_type(*actor)==3?actor:nullptr,parent,id,0.f,20.f,true);
            return;
        }
        frontend::SpatialSoundRequest request;
        if(text[0]=='$') {
            const std::string marker_text(text);
            if(marker_text.size()>=256)throw std::runtime_error("Human animation sound marker exceeds original buffer");
            const auto first=marker_text.find(' ',1);if(first==std::string::npos)return;
            const auto second=marker_text.find(' ',first+1);if(second==std::string::npos)return;
            request.filename=marker_text.substr(1,first-1);
            request.near_range=real(marker_text.substr(first+1,second-first-1));
            request.far_range=real(marker_text.substr(second+1));
            request.relative=frames_.actor_find(marker,"a",0xffff); //Native $ searches the callback frame itself.
        }else if(text[0]=='@') {
            if(!parent)throw std::runtime_error("Human grouped animation sound marker has no actual parent");
            request.group=11;request.entry=number(text+1);
            request.relative=frames_.actor_find(parent,"a",0xffff); //Native @ searches its parent.
        }else return;
        request.source=actor;request.flags=1;
        if(actor&&actors_.animation_view_actor(*actor)&&actors_.animation_view_offset_enabled()&&request.relative) {
            //Same frame-local forward operation as004C7FD0; it normalizes the
            //quaternion/builds local rotation only when flag10 is absent.
            const auto direction=frames_.car_forward(static_cast<Frame>(request.relative));
            for(unsigned axis=0;axis!=3;++axis)request.position[axis]=static_cast<float>(static_cast<long double>(direction[axis])*.666f); //0081681C.
        }
        (void)sound_.emit_animation_sound(request);
    }
private:
    static std::int32_t number(const char* text) {
        const auto value=std::strtoll(text,nullptr,10);
        if(value<std::numeric_limits<std::int32_t>::min()||value>std::numeric_limits<std::int32_t>::max())
            throw std::runtime_error("Human animation sound marker integer overflow");
        return static_cast<std::int32_t>(value);
    }
    static float real(const std::string& text) {
        char* end=nullptr;const auto value=std::strtof(text.c_str(),&end);
        if(end==text.c_str())throw std::runtime_error("Human animation sound marker leaves an undefined float");
        return value; //Native sscanf(%f) accepts a valid prefix and ignores its suffix.
    }
    Frames& frames_;Actors& actors_;Sound& sound_;
};
} //namespace hd2::game
#endif
