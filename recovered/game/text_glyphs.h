#ifndef HD2_GAME_TEXT_GLYPHS_H
#define HD2_GAME_TEXT_GLYPHS_H
#include "frontend/text_style_data.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string_view>
namespace hd2::game {
//007E5E00 FISTP QWORD/lowEAX. Invalid/overflow produces integer-indefinite,
//whose LOW32 is0. Preserve32/16 wrapping instead of native C++ signed overflow.
inline std::int32_t text_ftol(long double value){
    std::uint32_t low=0;
    if(std::isfinite(value)&&value>=-9223372036854775808.L&&value<9223372036854775808.L)
        low=static_cast<std::uint32_t>(static_cast<std::uint64_t>(static_cast<std::int64_t>(value)));
    std::int32_t result;std::memcpy(&result,&low,4);return result;
}
inline std::int16_t text_short(std::int32_t value){const auto bits=static_cast<std::uint16_t>(value);std::int16_t out;std::memcpy(&out,&bits,2);return out;}
inline std::int32_t text_add(std::int32_t first,std::int32_t second){const auto bits=static_cast<std::uint32_t>(first)+static_cast<std::uint32_t>(second);std::int32_t out;std::memcpy(&out,&bits,4);return out;}
inline std::int32_t text_sub(std::int32_t first,std::int32_t second){const auto bits=static_cast<std::uint32_t>(first)-static_cast<std::uint32_t>(second);std::int32_t out;std::memcpy(&out,&bits,4);return out;}
inline float text_real(std::uint32_t bits){float out;std::memcpy(&out,&bits,4);return out;}
//00644EB0: signed display HEIGHT word, original heightFLOAT and +FLOAT.5.
inline std::int32_t text_pixel_height(std::uint16_t height,const frontend::TextStyleDescriptor& style){
    std::int16_t signed_height;std::memcpy(&signed_height,&height,2);
    return text_ftol(static_cast<long double>(signed_height)*text_real(style[2])+.5f);
}
//00644E10, metric provider is the SAME selected actual font. Password flag4
//uses '*' without the ordinary zero-product fallback. Masked/ordinary paths
//also differ in the native int→float store before multiplying glyph aspect.
template<class Font>std::int32_t text_glyph_width(Font& fonts,const frontend::TextStyleDescriptor& style,
                                               unsigned char character,std::int32_t height){
    if(style[12]&4u)return text_ftol(static_cast<long double>(height)*fonts.glyph(style[0],'*').aspect+.5f);
    const auto stored=static_cast<float>(height);
    const auto product=static_cast<long double>(stored)*fonts.glyph(style[0],character).aspect;
    if(product==0)return text_ftol(static_cast<long double>(height)*.4+.5);
    return text_ftol(static_cast<long double>(stored)*fonts.glyph(style[0],character).aspect+.5f);
}
//00644D60 accepted BYTE-font branch; use stored COW length, including NUL bytes.
template<class Font>std::int32_t text_byte_width(Font& fonts,const frontend::TextStyleDescriptor& style,
                                              std::string_view bytes,std::int32_t height){
    std::int32_t width=0;for(unsigned char character:bytes)width=text_add(width,text_glyph_width(fonts,style,character,height));return width;
}
//00644EF0 BYTE branch: preserve original string if the FIRST character is too
//wide; otherwise return prefix through last fitting character. No ellipsis.
template<class Font>std::string_view text_crop_bytes(Font& fonts,const frontend::TextStyleDescriptor& style,
                                                   std::string_view bytes,std::int32_t height,float maximum){
    std::int32_t width=0;
    for(std::size_t n=0;n!=bytes.size();++n){width=text_add(width,text_glyph_width(fonts,style,static_cast<unsigned char>(bytes[n]),height));
        if(maximum<static_cast<float>(width))return n?bytes.substr(0,n):bytes;}
    return bytes;
}
struct TextGlyphVertex {float x,y,z,rhw;std::uint32_t color,specular;float u,v;};
static_assert(sizeof(TextGlyphVertex)==32);
//00648860 accepted BYTE/atlas branch and00643FB0 vertex stores. Font backend
//provides actual glyph metrics, atlas bind and Graph88 triangle submission.
//This is word emission, not a layout/window/selection renderer substitute.
template<class Font>bool emit_text_byte_glyphs(Font& fonts,const frontend::TextStyleDescriptor& style,std::string_view bytes,
        std::int32_t height,std::int16_t& x,std::int16_t y,float depth,std::uint32_t color){
    if(!height)return false;
    std::array<TextGlyphVertex,192> vertices{};std::size_t count=0;
    const auto flush=[&]{if(count){fonts.bind(style[0]);fonts.submit(vertices.data(),count);count=0;}};
    const auto vertex=[&](float px,float py,float u,float v){
        //Specular14 is untouched native zero-fill; keep a defined zero guard.
        vertices[count++]={px,py,depth,static_cast<float>(1.L/depth),color,0,u,v};
    };
    for(unsigned char character:bytes){
        const auto& glyph=fonts.glyph(style[0],style[12]&4u?'*':character);
        std::int32_t advance;
        if(glyph.aspect==0)advance=text_ftol(static_cast<long double>(height)*.4+.5);
        else {
            advance=text_ftol(static_cast<long double>(height)*glyph.aspect+.5f);
            const auto left=static_cast<float>(x),right=static_cast<float>(text_add(x,advance));
            const auto bottom=static_cast<float>(y),top=static_cast<float>(text_sub(y,height));
            vertex(left,bottom,glyph.u_left,glyph.v_bottom);vertex(left,top,glyph.u_left,glyph.v_top);
            vertex(right,top,glyph.u_right,glyph.v_top);vertex(right,bottom,glyph.u_right,glyph.v_bottom);
            vertex(left,bottom,glyph.u_left,glyph.v_bottom);vertex(right,top,glyph.u_right,glyph.v_top);
        }
        x=text_short(text_add(x,advance));if(count==vertices.size())flush();
    }
    flush();return true;
}
} //namespace hd2::game
#endif
