#ifndef HD2_HOST_GAME_FRAME_TEXT_OVERLAY_H
#define HD2_HOST_GAME_FRAME_TEXT_OVERLAY_H
#include "game/frame_text_overlay.h"
#include "host/game_car_frames.h"
namespace hd2::host {
//Process graphics/font bridge; no second menu font or replacement texture.
class GameFrameTextOverlayBackend {
public:
    using OverlayTexture=GameCarFrames::ResourceOwner;
    GameFrameTextOverlayBackend(MenuDriver& driver,Graphics& graphics,HMODULE module):driver_(driver),graphics_(graphics),module_(module){}
    OverlayTexture empty_overlay_texture(){return OverlayTexture(nullptr,GameCarFrames::ResourceRelease{module_});}
    bool load_overlay_texture(OverlayTexture& texture){
        EngineFrame* frame=nullptr;const auto result=graphics_.methods->load_texture(&graphics_,"editfont.bmp",nullptr,8,&frame);
        if(result<0){texture.reset();return false;} //Native failed optional load forcesNULL.
        texture.reset(frame);return true;
    }
    std::uint32_t overlay_driver_state(){return driver_.render_state(3);}
    void overlay_driver_state(std::uint32_t value){(void)driver_.set_render_state(3,value);}
    void bind_overlay_texture(EngineFrame* texture){(void)graphics_.methods->bind_texture(&graphics_,texture,0);}
    std::int32_t overlay_width(){return graphics_.methods->width(&graphics_);}std::int32_t overlay_height(){return graphics_.methods->height(&graphics_);}
    void draw_overlay_vertices(const game::EffectsFadeVertex* vertices,std::size_t count){
        if(graphics_.methods->draw_primitive(&graphics_,3,static_cast<std::uint32_t>(count/3),vertices,1)<0)
            throw std::runtime_error("Actual frame text overlay draw failed");
    }
private:
    MenuDriver& driver_;Graphics& graphics_;HMODULE module_;
};
} //namespace hd2::host
#endif
