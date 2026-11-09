#ifndef HD2_GAME_TEXT_RENDER_H
#define HD2_GAME_TEXT_RENDER_H
#include "game/text_layout.h"
#include <stdexcept>
namespace hd2::game {
struct TextViewport {
    std::uint32_t x=0,y=0,width=0,height=0;
    float minimum=0,maximum=1;
};
static_assert(sizeof(TextViewport)==24);
//008AE4F0/4 process renderer controls. The temporary direct-font mode set by
//0063F450 is a separate font-manager branch, outside this byte/atlas consumer.
struct TextRenderControls {bool debug_clip=false;std::int16_t offset=0;};
template<class State>bool text_clip_viewport(const State& state,TextViewport& viewport){ //00647D00.
    const auto clamp=[](long double value,float maximum){if(value>maximum)value=maximum;else if(value<0)value=0;return value;};
    const auto width=static_cast<float>(text_short(state.width())),height=static_cast<float>(text_short(state.height()));
    const auto left=static_cast<float>(clamp(text_real(state.geometry8[0]),width));
    const auto right=static_cast<float>(clamp(text_real(state.geometry8[2]),width));
    const auto bottom=static_cast<float>(clamp(text_real(state.geometry8[3]),height));
    //Native keeps top+FLOAT5 in x87 until the final height conversion.
    const auto top=clamp(static_cast<long double>(text_real(state.geometry8[1]))+5.f,height);
    const float span=static_cast<float>(static_cast<long double>(right)-left);
    const auto h=text_ftol(top-bottom+1.f),w=text_ftol(static_cast<long double>(span)+1.f);
    viewport={static_cast<std::uint32_t>(text_ftol(left)),static_cast<std::uint32_t>(text_ftol(bottom)),
              static_cast<std::uint32_t>(w),static_cast<std::uint32_t>(h),0,1};
    return h>0&&w>0;
}
inline std::array<TextGlyphVertex,4> text_background_quad(float left,float bottom,float right,float top,std::uint32_t color){
    const auto depth=text_real(0x3ce8bf06),rhw=text_real(0x420cc9df);
    return {{{left,bottom,depth,rhw,color,0,0,0},{right,bottom,depth,rhw,color,0,1,0},
             {right,top,depth,rhw,color,0,0,1},{left,top,depth,rhw,color,0,1,1}}};
}
template<class Backend>void emit_text_caret(Backend& backend,const frontend::TextStyleDescriptor& style,
        std::int32_t height,std::int16_t& x,std::int16_t y,float depth,std::uint32_t color){ //00648C80 atlas branch.
    const auto stored_height=static_cast<float>(height);
    const auto baseline=text_ftol(static_cast<long double>(y)-static_cast<long double>(stored_height)*.1f);
    const auto& glyph=backend.glyph(style[0],'_');
    if(glyph.aspect==0){x=text_short(text_add(x,text_ftol(static_cast<long double>(height)*.4+.5)));return;}
    const auto advance=text_ftol(static_cast<long double>(stored_height)*glyph.aspect+.5f);
    const auto left=static_cast<float>(x),right=static_cast<float>(text_add(x,advance));
    const auto bottom=static_cast<float>(baseline),top=static_cast<float>(text_sub(baseline,height));
    const auto vertex=[&](float px,float py,float u,float v){return TextGlyphVertex{px,py,depth,static_cast<float>(1.L/depth),color,0,u,v};};
    const std::array<TextGlyphVertex,6> vertices{{
        vertex(left,bottom,glyph.u_left,glyph.v_bottom),vertex(left,top,glyph.u_left,glyph.v_top),
        vertex(right,top,glyph.u_right,glyph.v_top),vertex(right,bottom,glyph.u_right,glyph.v_bottom),
        vertex(left,bottom,glyph.u_left,glyph.v_bottom),vertex(right,top,glyph.u_right,glyph.v_top)}};
    x=text_short(text_add(x,advance));backend.bind(style[0]);backend.submit(vertices.data(),vertices.size());
}
//00647EE0 ordinary byte-font/atlas branch. The caller owns Begin/EndScene;
//backend must use the SAME selected FontResources and actual graphics/driver.
//The original ignores a failed dirty rebuild and attempts its retained prefix.
template<class State,class Styles,class Backend>void draw_text_byte_atlas(State& state,Styles& styles,Backend& backend,const TextRenderControls& controls){
    if(!state.bytes70[0])return;
    if(state.fonts()!=&backend.fonts())throw std::invalid_argument("Text drawing requires its owning font resources");
    state.synchronize_text_state();
    state.bytes70[4]=0; //This API is explicitly the atlas consumer, not direct/GDI.
    if(state.bytes70[2]||state.text.dirty)(void)state.rebuild_layout(styles);
    TextViewport saved;
    const bool clipped=state.bytes70[3]!=0;
    bool viewport_changed=false;
    if(clipped){
        saved=backend.viewport();backend.query_dimensions(); //00647E70 ignores its supplied dimensions.
        TextViewport viewport;if(!text_clip_viewport(state,viewport))return;
        backend.query_dimensions();backend.viewport(viewport);viewport_changed=true;
    }
    const auto restore_viewport=[&]{if(viewport_changed){backend.query_dimensions();backend.viewport(saved);viewport_changed=false;}};
    try {
        if(state.word98){
            const auto previous=backend.driver_state(3);backend.driver_state(3,0);
            try {
                const auto quad=text_background_quad(text_real(state.geometry8[0]),text_real(state.geometry8[3]),
                    text_real(state.geometry8[2]),text_real(state.geometry8[1]),state.word98);
                backend.bind_none();backend.submit_quad(quad.data());
            }catch(...){backend.driver_state(3,previous!=0);throw;}
            backend.driver_state(3,previous!=0); //647FDA SETNE: restore BOOL, not saved RAW DWORD.
        }
        state.layout.update_offset(state,controls.offset);
        std::size_t line_index=0;
        std::int16_t x=0,y=0;
        std::uint32_t last_color=0;
        const frontend::TextStyleDescriptor* selected=nullptr;
        const auto select=[&](std::uint8_t id){const auto* style=styles.descriptor(id);if(!style||!backend.contains((*style)[0]))return false;selected=style;return true;};
        const auto color=[&]{if(state.word94)return state.word94;if(state.bytes70[1])return ((*selected)[1]&0xffffffu)|0x60000000u;
            if(state.word8c==static_cast<std::int64_t>(line_index))return 0xffffffffu;
            return (*selected)[1];};
        bool failed=false;
        for(;line_index<state.layout.lines.size();++line_index){
            const auto& line=state.layout.lines[line_index];
            const auto baseline=text_short(text_sub(line.baseline,text_short(state.word78)));y=baseline;
            for(const auto& fragment:line.fragments){
                if(!select(fragment.style)){failed=true;break;}
                last_color=color();x=fragment.x;
                const auto height=text_pixel_height(state.height(),*selected);
                if((*selected)[12]&0x80u)y=text_short(text_sub(y,text_sub(line.height,height)/2));
                if(!emit_text_byte_glyphs(backend,*selected,TextLayout::bytes(fragment.text),height,x,y,text_real(state.geometry8[6]),last_color)){failed=true;break;}
                if((*selected)[12]&0x80u)y=baseline;
            }
            if(failed)break;
            if(state.bytes80[1]&&state.word88==static_cast<std::int64_t>(line_index)&&state.bytes80[0]&&selected)
                emit_text_caret(backend,*selected,text_pixel_height(state.height(),*selected),x,y,text_real(state.geometry8[6]),last_color);
        }
        for(const auto& fragment:state.layout.absolute){
            if(!select(fragment.style))continue;
            const auto tint=color();
            x=text_short(text_ftol(static_cast<long double>(fragment.x)+text_real(state.geometry8[0])));
            y=text_short(text_ftol(static_cast<long double>(fragment.y)+text_real(state.geometry8[3])));
            (void)emit_text_byte_glyphs(backend,*selected,TextLayout::bytes(fragment.text),text_pixel_height(state.height(),*selected),x,y,text_real(state.geometry8[6]),tint);
        }
        restore_viewport();
        if(controls.debug_clip&&clipped){
            backend.bind_none();const auto previous=backend.driver_state(9);backend.driver_state(9,0);
            try {
                const auto quad=text_background_quad(static_cast<float>(saved.x),static_cast<float>(saved.y),
                    static_cast<float>(saved.x+saved.width),static_cast<float>(saved.y+saved.height),0x2f005000);
                backend.submit_quad(quad.data());
            }catch(...){backend.driver_state(9,previous);throw;}
            backend.driver_state(9,previous);
        }
    }catch(...){restore_viewport();throw;}
}
} //namespace hd2::game
#endif
