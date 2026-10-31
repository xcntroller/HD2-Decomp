#ifndef HD2_HOST_CREDITS_TEXT_H
#define HD2_HOST_CREDITS_TEXT_H
#include "host/opening_text.h"
#include "host/credits_geometry.h"

namespace hd2::host {
// Actual four-atlas renderer; the outer movie-state pass owns Begin/EndScene.
class CreditsText {
public:
    CreditsText(Graphics* graphics,HMODULE module,const frontend::CreditsDocument& document)
        : graphics_(graphics),fonts_{{
            decode_font_metrics(read_menu_text_file("MenuText\\BIG.tbl")),
            decode_font_metrics(read_menu_text_file("MenuText\\SMALL.tbl")),
            decode_font_metrics(read_menu_text_file("Text\\Fonts\\CE\\Set2\\BIG.tbl")),
            decode_font_metrics(read_menu_text_file("Text\\Fonts\\CE\\Set2\\SMALL.tbl"))}},
          layout_(document,fonts_,graphics->methods->width(graphics),graphics->methods->height(graphics)) {
        const auto base=reinterpret_cast<std::uintptr_t>(module);
        unsigned char anchor[]{0xa1,0,0,0,0};const auto address=static_cast<std::uint32_t>(base+0xd216c);
        std::memcpy(anchor+1,&address,4);
        if(std::memcmp(reinterpret_cast<const void*>(base+0x6b389),anchor,5)!=0)
            throw std::runtime_error("Unsupported Credits device-state bridge");
        std::memcpy(&device_,reinterpret_cast<const void*>(base+0xd216c),sizeof(device_));
        if(!device_)throw std::runtime_error("Missing Credits device");
        const std::array<const char*,4> names{{"HD2_MENU_BIG.TGA","HD2_MENU_SMALL.TGA","HD2_CREDITS_CE_BIG.TGA","HD2_CREDITS_CE_SMALL.TGA"}};
        for(std::size_t i=0;i<names.size();++i) {
            EngineFrame* texture=nullptr;
            const auto status=graphics->methods->load_texture(graphics,names[i],nullptr,0x402,&texture);
            if(texture) atlases_[i]=MenuDriver::Owner(texture,scene_method<MenuDriver::Release>(texture,module,0,0x73760));
            check_overlay_status(status,"Load Credits atlas");
            if(!atlases_[i])throw std::runtime_error("Null Credits atlas");
        }
        std::printf("Credits text ready: records=%zu rows=%zu speed=%.9g; no IShow/voice\n",document.records.size(),layout_.rows().size(),static_cast<double>(document.speed));
    }
    bool exhausted() const { return layout_.exhausted(); }
    void draw(std::int16_t scroll) {
        const auto submissions=layout_.draw(scroll);
        TextOverlayState overlay(device_);
        const auto* previous=graphics_->methods->viewport(graphics_);
        if(!previous)throw std::runtime_error("Missing Credits viewport");
        const auto saved=*previous;
        const CursorViewport full{0,0,static_cast<std::uint32_t>(graphics_->methods->width(graphics_)),static_cast<std::uint32_t>(graphics_->methods->height(graphics_)),0,1};
        try {
            check_overlay_status(graphics_->methods->set_viewport(graphics_,&full),"Set Credits viewport");
            for(const auto& submission:submissions) {
                if(submission.vertices.empty())continue;
                if(!graphics_->methods->bind_texture(graphics_,atlases_[submission.font].get(),0))
                    throw std::runtime_error("Cannot bind Credits atlas");
                check_overlay_status(graphics_->methods->draw_primitive(graphics_,3,static_cast<std::uint32_t>(submission.vertices.size()/3),submission.vertices.data(),1),"Submit Credits glyphs");
            }
            graphics_->methods->bind_texture(graphics_,nullptr,0);
            check_overlay_status(device_->SetTexture(0,nullptr),"Unbind Credits atlas");
            check_overlay_status(device_->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG2),"Set Credits fade color");
            check_overlay_status(device_->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG2),"Set Credits fade alpha");
            for(const auto& strip:layout_.fades())
                check_overlay_status(graphics_->methods->draw_primitive(graphics_,4,2,strip.data(),1),"Submit Credits fade");
        } catch(...) { graphics_->methods->set_viewport(graphics_,&saved);throw; }
        check_overlay_status(graphics_->methods->set_viewport(graphics_,&saved),"Restore Credits viewport");
        if(!overlay.restore())throw std::runtime_error("Cannot restore Credits device state");
    }
private:
    Graphics* graphics_;
    std::array<FontMetrics,4> fonts_;
    CreditsLayout layout_;
    std::array<MenuDriver::Owner,4> atlases_{{{nullptr,nullptr},{nullptr,nullptr},{nullptr,nullptr},{nullptr,nullptr}}};
    IDirect3DDevice8* device_=nullptr;
};
}
#endif
