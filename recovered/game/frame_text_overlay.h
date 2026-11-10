#ifndef HD2_GAME_FRAME_TEXT_OVERLAY_H
#define HD2_GAME_FRAME_TEXT_OVERLAY_H
#include "game/effects_fade_render.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>
namespace hd2::game {
//Separate process overlay008CA3A8/008CA3B8, NOT the game's text/font module.
//Pages are borrowed, as in the native wrappers. Unregister before a Page dies;
//the graphics device and registered pages must outlive close().
struct FrameTextPage {
    struct Row {std::string text;std::array<std::int32_t,3> rgb{{-1,-1,-1}};};
    std::vector<std::unique_ptr<Row>> rows;
    std::array<std::int32_t,3> rgb{{255,255,255}};
    float x=.1f,y=.1f,font_height=.06f,row_step=.067f;
    bool bottom_up=false,reverse_characters=false,reverse_rows=false;
    std::uint32_t age34=0;
    void clear(){while(!rows.empty())rows.pop_back();age34=0;} //006E7AF0, retains capacity.
};
struct FrameTextGlyph {float u0=0,v0=0,u1=0,v1=0,width=0;};
static_assert(sizeof(FrameTextGlyph)==20);
template<class Backend>class FrameTextOverlay {
public:
    using Texture=typename Backend::OverlayTexture;
    struct Entry {FrameTextPage* page;std::uint32_t flags;std::optional<std::string> name;};
    explicit FrameTextOverlay(Backend& backend):backend_(backend),texture_(backend.empty_overlay_texture()){}
    ~FrameTextOverlay(){close();}
    FrameTextOverlay(const FrameTextOverlay&)=delete;FrameTextOverlay& operator=(const FrameTextOverlay&)=delete;
    //006E8590/006E8530. A failed optional font leaves the old glyph matrix;
    //successful font loading zeros the table before the possibly short read.
    template<class Files>void initialize(Files& files){
        if(initialized_)throw std::logic_error("Text overlay initialization repeated");
        initialized_=true;
        if(!backend_.load_overlay_texture(texture_))return;
        glyphs_.fill({});
        const auto handle=files.open("tables\\fnt_matrix.bin",0);if(handle==0xffffffffu)return;
        struct Close {Files& files;std::uint32_t handle;~Close(){files.close(handle);}} close{files,handle};
        (void)files.read(handle,glyphs_.data(),sizeof(glyphs_)); //Native ignores short reads.
    }
    void register_page(FrameTextPage& page,std::uint32_t flags,std::optional<std::string> name={}){ //006E8160 absent frozen.
        for(auto i=entries_.rbegin();i!=entries_.rend();++i)if(i->page==&page)return;
        entries_.push_back({&page,flags,std::move(name)});
    }
    void unregister_page(FrameTextPage& page){ //006E83B0 calls006E8460 (ALL pages) before wrapper retirement.
        for(std::size_t n=entries_.size();n;){--n;if(entries_[n].page==&page){clear_pages();entries_.erase(entries_.begin()+n);return;}}
    }
    void clear_pages(){for(std::size_t n=entries_.size();n;){--n;entries_[n].page->clear();}} //006E8460.
    void close(){ //006E85A0; page owners themselves survive.
        clear_pages();while(!entries_.empty())entries_.pop_back();texture_.reset();initialized_=false;
    }
    bool enabled=true; //Actual00843F7C starts1.
    void render(){ //006E8490: preserve RAW driver3, including non-boolean bits.
        if(!enabled)return;
        if(!initialized_)throw std::logic_error("Initialize actual text overlay before rendering");
        const auto state=backend_.overlay_driver_state();frontend::CleanupErrors errors;
        try{
            backend_.overlay_driver_state(0);backend_.bind_overlay_texture(texture_.get());
            if(texture_)for(std::size_t n=entries_.size();n;){
                --n;draw_page(*entries_.at(n).page);
                if(entries_.at(n).flags&1u)entries_[n].page->clear(); //Reread wrapper after draw.
            }
        }catch(...){count_=0;errors=frontend::CleanupErrors(std::current_exception());} //Retire incomplete native draw storage on a source failure.
        errors.attempt([&]{backend_.overlay_driver_state(state);});errors.rethrow();
    }
    const auto& glyphs()const{return glyphs_;}
    const auto& entries()const{return entries_;}
private:
    static std::int32_t truncate(long double value){
        value=std::trunc(value);
        if(!std::isfinite(value)||value<-2147483648.L||value>2147483647.L)
            throw std::runtime_error("Text overlay coordinate outside native integer range");
        return static_cast<std::int32_t>(value); //__ftol truncates, independent of FISTP mode.
    }
    static std::int32_t add(std::int32_t a,std::int32_t b){const auto bits=std::uint32_t(a)+std::uint32_t(b);std::int32_t result;std::memcpy(&result,&bits,4);return result;}
    static std::int32_t subtract(std::int32_t a,std::int32_t b){const auto bits=std::uint32_t(a)-std::uint32_t(b);std::int32_t result;std::memcpy(&result,&bits,4);return result;}
    void flush(){if(count_){backend_.draw_overlay_vertices(vertices_.data(),count_);count_=0;}}
    void emit(float x,float y,std::uint32_t colour,float u,float v){
        auto& vertex=vertices_.at(count_++);vertex.x=x;vertex.y=y;vertex.z=0;vertex.rhw=1;vertex.diffuse=colour;vertex.u=u;vertex.v=v;
        //Native does not write specular; same persistent process buffer retains it.
    }
    void draw_page(const FrameTextPage& page){ //006E7CC0.
        const auto start=truncate(static_cast<long double>(backend_.overlay_width())*page.x);
        const float height=static_cast<float>(backend_.overlay_height());
        float y=page.y;if(page.bottom_up)y=static_cast<float>(static_cast<long double>(y)-page.row_step);
        for(std::size_t row=0;row<page.rows.size();++row){
            if(y>=1.f||y<0.f){flush();return;}
            const auto index=page.reverse_rows?page.rows.size()-1-row:row;
            const auto& value=*page.rows[index];const auto length=value.text.find('\0');
            const auto size=length==std::string::npos?value.text.size():length;
            //Native fixed1000-vertex buffer has no oversized-row protection.
            if(size>vertices_.size()/6)throw std::runtime_error("Text overlay row exceeds native vertex storage");
            if(vertices_.size()<count_+size*6)flush();
            auto colour=std::uint32_t(value.rgb[0]==-1?page.rgb[0]:value.rgb[0])-256u; //Native DWORD wrap.
            for(unsigned c=1;c!=3;++c)colour=colour*256u+std::uint32_t(value.rgb[c]==-1?page.rgb[c]:value.rgb[c]);
            const float pixels=static_cast<float>(static_cast<long double>(height)*page.font_height);
            const float top=static_cast<float>(static_cast<long double>(y)*height);
            auto x=start;
            for(std::size_t c=0;c<size;++c){
                const auto ch=static_cast<unsigned char>(value.text[page.reverse_characters?size-1-c:c]);
                if(ch>=128)throw std::runtime_error("Signed native overlay glyph index precedes glyph table");
                const auto& glyph=glyphs_[ch];const auto width=static_cast<long double>(pixels)*glyph.width;
                if(width==0){x=add(x,truncate(static_cast<long double>(pixels)*(page.reverse_characters?-.4:.4)));continue;} //008104C8/D0 doubles.
                const auto step=truncate(width);if(page.reverse_characters)x=subtract(x,step);
                const auto right=add(x,step);const float left=static_cast<float>(x),end=static_cast<float>(right);
                const float bottom=static_cast<float>(static_cast<long double>(top)+pixels);
                emit(left,top,colour,glyph.u0,glyph.v0);emit(end,bottom,colour,glyph.u1,glyph.v1);emit(left,bottom,colour,glyph.u0,glyph.v1);
                emit(left,top,colour,glyph.u0,glyph.v0);emit(end,top,colour,glyph.u1,glyph.v0);emit(end,bottom,colour,glyph.u1,glyph.v1);
                if(!page.reverse_characters)x=right;
            }
            y=static_cast<float>(static_cast<long double>(y)+(page.bottom_up?-static_cast<long double>(page.row_step):page.row_step));
        }
        flush();
    }
    Backend& backend_;Texture texture_;std::array<FrameTextGlyph,256> glyphs_{};
    std::vector<Entry> entries_;std::array<EffectsFadeVertex,1000> vertices_{};std::size_t count_=0;bool initialized_=false;
};
} //namespace hd2::game
#endif
