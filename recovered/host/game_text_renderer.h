#ifndef HD2_HOST_GAME_TEXT_RENDERER_H
#define HD2_HOST_GAME_TEXT_RENDERER_H
#include "game/text_render.h"
#include "host/game_fonts.h"
#include "host/graphics_interface.h"
namespace hd2::host {
//006402A0 atlas bind and Graph44/7C/80/88 bridge, using the text owner's
//existing five fonts. Caller owns scene/device-state setup and presentation.
class GameTextRenderer {
public:
    GameTextRenderer(Graphics* graphics,MenuDriver& driver,const GameFonts& fonts)
        : graphics_(graphics),driver_(driver),fonts_(fonts){if(!graphics_)throw std::invalid_argument("Game text requires graphics");}
    const GameFonts& fonts()const{return fonts_;}
    bool contains(unsigned id)const{return fonts_.contains(id);}
    const GlyphMetrics& glyph(unsigned id,unsigned char character)const{return fonts_.glyph(id,character);}
    void bind(unsigned id){if(!graphics_->methods->bind_texture(graphics_,fonts_.atlas(id),0))throw std::runtime_error("Cannot bind game font atlas");}
    void bind_none(){if(!graphics_->methods->bind_texture(graphics_,nullptr,0))throw std::runtime_error("Cannot clear game text texture");}
    void submit(const game::TextGlyphVertex* vertices,std::size_t count){submit_primitive(3,static_cast<std::uint32_t>(count/3),vertices,count);}
    void submit_quad(const game::TextGlyphVertex* vertices){submit_primitive(4,2,vertices,4);}
    game::TextViewport viewport()const{
        const auto* current=graphics_->methods->viewport(graphics_);if(!current)throw std::runtime_error("Missing game text viewport");
        return {current->x,current->y,current->width,current->height,current->min_z,current->max_z};
    }
    void viewport(const game::TextViewport& value){
        const CursorViewport target{value.x,value.y,value.width,value.height,value.minimum,value.maximum};
        if(graphics_->methods->set_viewport(graphics_,&target)<0)throw std::runtime_error("Cannot set game text viewport");
    }
    void query_dimensions()const{(void)graphics_->methods->width(graphics_);(void)graphics_->methods->height(graphics_);}
    std::uint32_t driver_state(std::uint32_t state)const{return driver_.render_state(state);}
    void driver_state(std::uint32_t state,std::uint32_t value){if(driver_.set_render_state(state,value)<0)throw std::runtime_error("Cannot set game text driver state");}
private:
    void submit_primitive(std::uint32_t type,std::uint32_t primitives,const game::TextGlyphVertex* vertices,std::size_t count){
        //Copy the ABI-compatible fields instead of aliasing unrelated C++ structs.
        if(count>1680)throw std::invalid_argument("Game text batch exceeds native message capacity");
        std::array<CursorVertex,1680> converted;
        static_assert(sizeof(CursorVertex)==sizeof(game::TextGlyphVertex));
        std::memcpy(converted.data(),vertices,count*sizeof(*vertices));
        if(graphics_->methods->draw_primitive(graphics_,type,primitives,converted.data(),1)<0)
            throw std::runtime_error("Cannot submit game text glyphs");
    }
    Graphics* graphics_;MenuDriver& driver_;const GameFonts& fonts_;
};
} //namespace hd2::host
#endif
