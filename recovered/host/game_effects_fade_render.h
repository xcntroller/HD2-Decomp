#ifndef HD2_HOST_GAME_EFFECTS_FADE_RENDER_H
#define HD2_HOST_GAME_EFFECTS_FADE_RENDER_H
#include "host/game_world_loading.h"
#include "game/effects_fade_render.h"
namespace hd2::host {
//Actual GEffects004867F0 graphics/driver endpoints, borrowed SAME process quad
//and actual scene frame store. Does not render an absent mission/camera or own
//a replacement fade, graphics state, texture or synthetic framebuffer.
class GameEffectsFadeRender {
public:
    GameEffectsFadeRender(world_loading::Owners& owners,Graphics& graphics):d_(owners.dependencies()),graphics_(graphics){}
    void operator()(float alpha,const std::array<float,4>* colour){
        game::draw_effects_fade(*this,d_.startup.effects_loading_process().fade_vertices,alpha,colour);
    }
    std::uint32_t fade_driver_state(){return d_.scene.frames().hud_driver_state(3);}
    void fade_driver_state(std::uint32_t value){d_.scene.frames().hud_driver_state(3,value);}
    std::uint32_t fade_graphics_state(std::uint32_t kind){return graphics_.methods->state(&graphics_,kind);}
    void fade_graphics_state(std::uint32_t kind,std::uint32_t value){graphics_.methods->set_state(&graphics_,kind,value);}
    std::int32_t fade_width(){return graphics_.methods->width(&graphics_);}std::int32_t fade_height(){return graphics_.methods->height(&graphics_);}
    void fade_bind_null_texture(){(void)graphics_.methods->bind_texture(&graphics_,nullptr,0);} //Native ignores return for unbind.
    void fade_draw(const std::array<game::EffectsFadeVertex,4>& vertices){
        if(graphics_.methods->draw_primitive(&graphics_,5,2,vertices.data(),1)<0)throw std::runtime_error("Actual effects fade draw failed");
    }
private:
    world_loading::Dependencies& d_;Graphics& graphics_;
};
} //namespace hd2::host
#endif
