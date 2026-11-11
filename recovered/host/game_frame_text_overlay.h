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
        if(result<0){OverlayTexture failed(frame,GameCarFrames::ResourceRelease{module_});texture.reset();return false;} //Native failed optional load forcesNULL.
        texture.reset(frame);return true;
    }
    std::uint32_t overlay_driver_state(){return driver_.render_state(3);}
    void overlay_driver_state(std::uint32_t value){if(driver_.set_render_state(3,value)<0)throw std::runtime_error("Cannot set process overlay driver state");}
    void bind_overlay_texture(EngineFrame* texture){if(!graphics_.methods->bind_texture(&graphics_,texture,0))throw std::runtime_error("Cannot bind process overlay texture");}
    std::int32_t overlay_width(){return graphics_.methods->width(&graphics_);}std::int32_t overlay_height(){return graphics_.methods->height(&graphics_);}
    void draw_overlay_vertices(const game::EffectsFadeVertex* vertices,std::size_t count){
        if(graphics_.methods->draw_primitive(&graphics_,3,static_cast<std::uint32_t>(count/3),vertices,1)<0)
            throw std::runtime_error("Actual frame text overlay draw failed");
    }
private:
    MenuDriver& driver_;Graphics& graphics_;HMODULE module_;
};
//ONE process registry and independent font texture. Startup owns this from
//006DBFC0's final006E8590 until006DC3A0's006E85A0, before driver close.
class GameFrameTextOverlayProcess {
public:
    using Registry=game::FrameTextOverlay<GameFrameTextOverlayBackend>;
    template<class Files>GameFrameTextOverlayProcess(MenuDriver& driver,Graphics& graphics,HMODULE module,Files& files)
        :backend_(driver,graphics,module),registry_(backend_){registry_.initialize(files);}
    Registry& registry(){return registry_;}
private:
    GameFrameTextOverlayBackend backend_;Registry registry_;
};
} //namespace hd2::host
#endif
