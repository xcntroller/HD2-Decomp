#ifndef HD2_GAME_ANIMATION_ACTOR_EVENTS_H
#define HD2_GAME_ANIMATION_ACTOR_EVENTS_H
#include "frontend/spatial_sound.h"
#include <cstdlib>
#include <limits>
#include <stdexcept>
namespace hd2::game {
//005B4610 stdcall(frame,event,text,time): differs from Human007759D0.
//Both markers emit relative to the callback frame itself, with source=null.
template<class Frame,class Sound>std::int32_t animation_actor_event(Frame frame,std::uint32_t event,const char* text,Sound& sound) {
    if(!(event&0x1000u)||!text)return 0;
    frontend::SpatialSoundRequest request;
    if(text[0]=='$') {
        const std::string value(text);if(value.size()>=256)throw std::runtime_error("Animation actor sound marker exceeds native buffer");
        const auto first=value.find(' ',1);if(first==std::string::npos)return 0;
        const auto second=value.find(' ',first+1);if(second==std::string::npos)return 0;
        request.filename=value.substr(1,first-1);
        const auto real=[](const std::string& input){char* end;const auto value=std::strtof(input.c_str(),&end);if(end==input.c_str())throw std::runtime_error("Animation actor sound marker leaves native range undefined");return value;};
        request.near_range=real(value.substr(first+1,second-first-1));request.far_range=real(value.substr(second+1));
    }else if(text[0]=='@') {
        const auto value=std::strtoll(text+1,nullptr,10);
        if(value<std::numeric_limits<std::int32_t>::min()||value>std::numeric_limits<std::int32_t>::max())throw std::runtime_error("Animation actor sound entry exceeds native atoi bounds");
        request.group=11;request.entry=static_cast<std::int32_t>(value);
    }else return 0;
    request.relative=frame;request.flags=1;sound.emit_animation_actor_sound(request);return 0;
}
} //namespace hd2::game
#endif
