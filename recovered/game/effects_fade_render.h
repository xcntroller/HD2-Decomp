#ifndef HD2_GAME_EFFECTS_FADE_RENDER_H
#define HD2_GAME_EFFECTS_FADE_RENDER_H
#include "game/effects_fade.h"
#include "game/session_activation.h"
#include <cmath>
#include "frontend/cleanup.h"
namespace hd2::game {
//Actual BSS0089B6E8 four32-byte vertices.004867F0 writes ONLY XY/diffuse;
//remaining lanes stay at actual static zero, no invented RHW/Z/UV producer.
struct EffectsFadeVertex {float x=0,y=0,z=0,rhw=0;std::uint32_t diffuse=0,specular=0;float u=0,v=0;};
static_assert(sizeof(EffectsFadeVertex)==32);
inline std::uint32_t effects_fade_component(float component){
    const float value=static_cast<float>(static_cast<long double>(component)*255.f); //FST then FISTP, current rounding mode.
    const auto rounded=std::nearbyint(static_cast<long double>(value));
    if(!std::isfinite(rounded)||rounded<-2147483648.L||rounded>2147483647.L)return 0x80000000u; //Native indefinite DWORD.
    return static_cast<std::uint32_t>(static_cast<std::int32_t>(rounded));
}
//004867F0. Preserve BOOLEANIZED driver3/Graph14, intentional Graph0=1 and
//textureNULL. Backend supplies actual native dimensions/state/draw endpoints.
template<class Backend>void draw_effects_fade(Backend& backend,std::array<EffectsFadeVertex,4>& vertices,
        float alpha,const std::array<float,4>* colour){
    const auto driver=std::uint32_t(backend.fade_driver_state()!=0),graphics=std::uint32_t(backend.fade_graphics_state(14)!=0);
    frontend::CleanupErrors errors;
    try{
        backend.fade_driver_state(0);backend.fade_graphics_state(14,0);
        const auto width=static_cast<float>(static_cast<std::uint32_t>(backend.fade_width()));
        const auto height=static_cast<float>(static_cast<std::uint32_t>(backend.fade_height()));
        const auto rgb=colour?*colour:std::array<float,4>{};
        auto packed=effects_fade_component(alpha);
        for(unsigned component=0;component<3;++component)packed=(packed<<8)|effects_fade_component(rgb[component]); //Native unmasked OR/spill.
        vertices[0].x=vertices[0].y=0;vertices[1].x=width;vertices[1].y=0;
        vertices[2].x=0;vertices[2].y=height;vertices[3].x=width;vertices[3].y=height;
        for(auto& vertex:vertices)vertex.diffuse=packed;
        backend.fade_graphics_state(0,1);backend.fade_bind_null_texture();backend.fade_draw(vertices);
    }catch(...){errors=frontend::CleanupErrors(std::current_exception());}
    errors.attempt([&]{backend.fade_driver_state(driver);});errors.attempt([&]{backend.fade_graphics_state(14,graphics);});errors.rethrow();
}
//00486C10, GEffects virtualA0 AFTER scene rendering. Rendering resolves the
//negative completion sentinel to target BEFORE choosing startup black alpha.
template<class Process,class Draw>void render_loading_effects_fade(Process& process,Draw& draw){
    process.fade.resolve_render_sentinel();
    std::int32_t time;std::memcpy(&time,&process.field224,4);
    const auto current=process.fade.current();
    if(time>3999||!process.active248){if(current!=0.f)draw(current,&process.fade.colour());return;}
    float alpha=1.f;
    if(time>3000)alpha=static_cast<float>(1.L-static_cast<long double>(static_cast<float>(time-3000))*.001000000047497451305389404296875f);
    if(current!=0.f&&alpha<current)alpha=current;
    const std::array<float,4> black{};draw(alpha,&black);
}
//0069AE10. Status5 is an actual early return; called by006C9260 only for5/6
//and word24==0. This fade is distinct from GEffects startup/fade state.
template<class Draw>void render_session_fade(const SessionActivationState& session,Draw& draw){
    if(session.status==5)return;
    const auto bits=session.words.get<std::uint32_t>(0x20)-200u;std::int32_t signed_time;std::memcpy(&signed_time,&bits,4);
    float value=static_cast<float>(signed_time);if(value<0)value=0;
    value=static_cast<float>(1.L-static_cast<long double>(value)*.0033333334140479564666748046875f); //008159E0 FLOAT.
    if(value!=0.f)draw(value,nullptr);
}
} //namespace hd2::game
#endif
