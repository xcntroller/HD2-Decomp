#ifndef HD2_GAME_MISSION_PARTICLE_PUBLICATION_H
#define HD2_GAME_MISSION_PARTICLE_PUBLICATION_H
#include "game/mission_particle_point.h"

namespace hd2::game {
struct MissionParticleDepth {std::uint32_t depth,index;};
struct MissionParticleScratch {std::vector<MissionParticleDepth> order;}; //SAME008998DC/E0/E4, process lifetime.
struct MissionParticleVertex {std::array<float,3> position;std::uint32_t colour;float u,v;};
static_assert(sizeof(MissionParticleVertex)==24);

//006E9C10/006E9D30, reached by00441A00. Keep native hue in degrees and
//FLOAT intermediate stores, including grey's unused undefined hue division.
inline HumanVector particle_rgb_to_hsv(const HumanVector& rgb){
    const float r=rgb[0],g=rgb[1],b=rgb[2];
    const float maximum=(r<g||r<b)?((g<r||g<b)?b:g):r;
    const float minimum=r<=g&&r<=b?r:g<=r&&g<=b?g:b;
    const float difference=maximum-minimum;
    if(maximum==0)return {{-1,0,maximum,0}};
    const float saturation=difference/maximum;float hue;
    if(maximum==r)hue=static_cast<float>((static_cast<long double>(g)-b)/difference);
    else if(maximum==g)hue=static_cast<float>((static_cast<long double>(b)-r)/difference+2.f);
    else hue=static_cast<float>((static_cast<long double>(r)-g)/difference+4.f);
    hue*=60.f;if(hue<0)hue+=360.f;return {{hue,saturation,maximum,0}};
}
inline HumanVector particle_hsv_to_rgb(const HumanVector& hsv){
    const float saturation=hsv[1],value=hsv[2];if(saturation==0)return {{value,value,value,0}};
    const float sector=hsv[0]*.01666666753590107f;
    const auto whole=particle_truncate(std::floor(static_cast<double>(sector)));
    const float fraction=sector-static_cast<float>(whole);
    const float p=static_cast<float>((1.L-saturation)*value);
    const float q=static_cast<float>((1.L-static_cast<long double>(fraction)*saturation)*value);
    const float t=static_cast<float>((1.L-(1.L-fraction)*saturation)*value);
    switch(whole){case 0:return {{value,t,p,0}};case 1:return {{q,value,p,0}};case 2:return {{p,value,t,0}};
        case 3:return {{p,q,value,0}};case 4:return {{t,p,value,0}};default:return {{value,p,q,0}};}
}
inline std::uint32_t particle_colour_component(float value){ //00441B7E..441BE5 FST then FISTP, current rounding mode.
    const float scaled=static_cast<float>(static_cast<long double>(value)*255.f);
    const auto rounded=std::nearbyint(static_cast<long double>(scaled));
    if(!std::isfinite(rounded)||rounded<-2147483648.L||rounded>2147483647.L)return 0x80000000u;
    return static_cast<std::uint32_t>(static_cast<std::int32_t>(rounded));
}
inline std::uint32_t particle_vertex_colour(const MissionParticlePoint& point,float lighting){
    auto hsv=particle_rgb_to_hsv(point.colour);hsv[2]*=lighting;const auto rgb=particle_hsv_to_rgb(hsv);
    auto packed=particle_colour_component(point.alpha);
    for(unsigned axis=0;axis!=3;++axis)packed=(packed<<8)|particle_colour_component(rgb[axis]); //Unmasked OR/spill.
    return packed;
}
} //namespace hd2::game
#endif
