#ifndef HD2_HOST_CREDITS_GEOMETRY_H
#define HD2_HOST_CREDITS_GEOMETRY_H
#include "frontend/credits_text.h"
#include "host/menu_text_geometry.h"

namespace hd2::host {
struct CreditsPiece { std::string text; std::uint8_t style; int x,height; };
struct CreditsRow { std::vector<CreditsPiece> pieces; std::int16_t baseline=0; int height=0; };
struct CreditsSubmission { unsigned font; std::vector<CursorVertex> vertices; };
inline unsigned credits_font_index(unsigned font) {
    switch(font) { case 2:return 0; case 3:return 1; case 5:return 2; case 6:return 3; }
    throw std::logic_error("Unknown Credits font");
}
//00645270/00645F20 nonwrapping branch,006469C0,00646A10/00646DB0.
// Only the actual14 Credits styles are accepted, rather than claiming the
// shared paragraph renderer or original font/text object ABI is reconstructed.
class CreditsLayout {
public:
    CreditsLayout(const frontend::CreditsDocument& document,const std::array<FontMetrics,4>& fonts,int width,int height)
        : fonts_(fonts),width_(width),height_(height) {
        if(width<=0 || height<=0 || width>32767 || height>32767)
            throw std::invalid_argument("Invalid Credits dimensions");
        const float left=static_cast<float>(static_cast<long double>(width)*.02);
        const float right=static_cast<float>(static_cast<long double>(width)*.98);
        const float span=right-left;
        for(const auto& record:document.records) {
            if(record.text.size()>65536) throw std::length_error("Oversized Credits record");
            const auto& style=frontend::credits_styles.at(record.style);
            if(style.new_row || rows_.empty()) rows_.push_back({});
            const int glyph_height=static_cast<int>(static_cast<long double>(height)*style.height+.5f);
            int length=0;
            for(unsigned char ch:record.text) length+=glyph_advance(fonts[credits_font_index(style.font)][ch],glyph_height);
            const int remaining=static_cast<int>(span)-length;
            const auto x=style.column>=0 ? static_cast<int>(static_cast<long double>(static_cast<float>(style.column*span+left))-length*.5f)
                : static_cast<int>(left+(remaining>0?remaining/2:0));
            rows_.back().pieces.push_back({record.text,record.style,x,glyph_height});
            rows_.back().height=std::max(rows_.back().height,glyph_height);
        }
        std::int16_t baseline=0;
        for(auto& row:rows_) { baseline=frontend::credits_short(baseline+row.height); row.baseline=baseline; }
    }
    bool exhausted() const { return first_>=rows_.size(); } //0064BCC0
    const auto& rows() const { return rows_; }
    std::size_t first_visible() const { return first_; }
    //0064B950: only rendering advances the exhaustion cursor. Rows with a
    // negative bottom are retired permanently; future rows stop at height+100.
    std::vector<CreditsSubmission> draw(std::int16_t scroll) {
        std::vector<CreditsSubmission> result;
        for(std::size_t i=first_;i<rows_.size();++i) {
            const auto& row=rows_[i];
            const auto bottom=frontend::credits_short(row.baseline-scroll);
            if(bottom<0) {first_=i+1;continue;}
            if(bottom>height_+100) break;
            for(const auto& piece:row.pieces) {
                const auto font=credits_font_index(frontend::credits_styles[piece.style].font);
                if(result.empty() || result.back().font!=font) result.push_back({font,{}});
                auto& vertices=result.back().vertices;
                int x=piece.x;
                float depth; const std::uint32_t bits=0x3f43acc5;std::memcpy(&depth,&bits,4);
                for(unsigned char ch:piece.text) {
                    const auto& m=fonts_[font][ch];const auto advance=glyph_advance(m,piece.height);
                    if(m.aspect!=0) {
                        const auto vertex=[&](int px,int py,float u,float v) {return CursorVertex{static_cast<float>(px),static_cast<float>(py),depth,1/depth,0xffffffff,0,u,v};};
                        const auto bl=vertex(x,bottom,m.u_left,m.v_bottom),tl=vertex(x,bottom-piece.height,m.u_left,m.v_top);
                        const auto tr=vertex(x+advance,bottom-piece.height,m.u_right,m.v_top),br=vertex(x+advance,bottom,m.u_right,m.v_bottom);
                        vertices.insert(vertices.end(),{bl,tl,tr,br,bl,tr});
                    }
                    x=frontend::credits_short(x+advance);
                }
            }
        }
        return result;
    }
    //0064BA2C: band height scales with WIDTH, not height; original depth/rhw.
    std::array<std::array<CursorVertex,4>,2> fades() const {
        float depth,rhw;const std::uint32_t z=0x3ce8bf06,w=0x420cc9df;
        std::memcpy(&depth,&z,4);std::memcpy(&rhw,&w,4);
        const auto band=static_cast<int>(static_cast<long double>(width_)*(1.0f/1024)*100);
        const auto v=[&](float x,float y,std::uint32_t color,float u,float t){return CursorVertex{x,y,depth,rhw,color,0,u,t};};
        const auto width=static_cast<float>(width_),height=static_cast<float>(height_);
        return {{{v(0,0,0xff000000,0,0),v(width,0,0xff000000,1,0),v(width,static_cast<float>(band),0,0,1),v(0,static_cast<float>(band),0,1,1)},
                 {v(0,height-band,0,0,0),v(width,height-band,0,1,0),v(width,height,0xff000000,0,1),v(0,height,0xff000000,1,1)}}};
    }
private:
    std::array<FontMetrics,4> fonts_;
    std::vector<CreditsRow> rows_;
    int width_,height_;
    std::size_t first_=0;
};
}
#endif
