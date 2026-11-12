#ifndef HD2_GAME_HUD_MAP_PRESENTATION_H
#define HD2_GAME_HUD_MAP_PRESENTATION_H
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
namespace hd2::game {
using HudMapVector=std::array<float,4>;
//004A5B50's two-dimensional normalizer preserves a zero X/Z pair. It is
//different from the three-dimensional fallback used for the scene camera.
inline HudMapVector hud_map_direction(const HudMapVector& source){
    float x=source[0],y=source[2];const auto square=static_cast<long double>(y)*y+static_cast<long double>(x)*x;
    const float stored=static_cast<float>(square);constexpr float epsilon=9.99999993922529e-09f;
    if(epsilon<=std::abs(square-1.f)){
        if(epsilon<=stored){const auto scale=1/std::sqrt(static_cast<long double>(stored));x=static_cast<float>(x*scale);y=static_cast<float>(y*scale);}
        else if(x!=0)x=x>=0?1.f:-1.f;else if(y!=0)y=y>=0?1.f:-1.f;
    }
    return {{x,y,0,0}};
}
inline HudMapVector hud_map_camera_direction(HudMapVector value){
    const auto square=(static_cast<long double>(value[2])*value[2]+static_cast<long double>(value[1])*value[1])+static_cast<long double>(value[0])*value[0];
    const float stored=static_cast<float>(square);constexpr float epsilon=9.99999993922529e-09f;
    if(epsilon<=std::abs(square-1.f)){
        if(epsilon<=stored){const auto scale=1/std::sqrt(static_cast<long double>(stored));for(unsigned n=0;n!=3;++n)value[n]=static_cast<float>(value[n]*scale);}
        else{const auto n=value[0]!=0?0u:value[2]!=0?2u:1u;value[n]=value[n]>=0?1.f:-1.f;}
    }
    return hud_map_direction(value);
}
template<class Text>void hud_map_label_geometry(Text& text,const HudMapVector& first,const HudMapVector& second,float width,std::int32_t height,float depth){ //004A65B0→00644B80.
    std::array<float,7> geometry{};
    geometry[0]=static_cast<float>((static_cast<long double>(second[0])+.01f)*width);
    geometry[1]=static_cast<float>((static_cast<long double>(second[1])-.01f)*height*-1.3333333730697632f);
    //Original stores the first WORLD-position additions to FLOAT before scaling.
    const float right=static_cast<float>(static_cast<long double>(first[0])+.4f),top=static_cast<float>(static_cast<long double>(first[1])+.01f);
    geometry[2]=static_cast<float>(static_cast<long double>(right)*width);
    geometry[3]=static_cast<float>(static_cast<long double>(top)*height*-1.3333333730697632f);
    geometry[4]=geometry[2]-geometry[0];geometry[5]=geometry[1]-geometry[3];geometry[6]=depth;
    for(unsigned n=0;n!=7;++n)std::memcpy(&text.geometry8[n],&geometry[n],4);
    text.bytes70[2]=text.bytes70[0]=1;text.text.dirty=true;
}
} //namespace hd2::game
#endif
