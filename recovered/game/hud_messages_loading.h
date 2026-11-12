#ifndef HD2_GAME_HUD_MESSAGES_LOADING_H
#define HD2_GAME_HUD_MESSAGES_LOADING_H
#include <array>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>
#include "game/hud_message_state.h"
#include "game/text_glyphs.h"
#include <string>
namespace hd2::game {
//00495F40/004960F0 and00495900: message artwork/font and TWO screen-bound
//layouts. Queue/render is separate from TextState and borrows SAME HUD+2F90 state.
template<class Frames,class Factory>
class HudMessagesLoading {
public:
    using Frame=typename Frames::Frame;
    using FontOwner=decltype(std::declval<Factory&>().make_hud_fonts());
    using State=HudMessageState;
    struct Record {std::unique_ptr<char[]> bytes;std::array<std::uint32_t,5> words{};};
    HudMessagesLoading(Frames& frames,Factory& factory,State& state):frames_(frames),factory_(factory),state_(state) {}
    ~HudMessagesLoading(){close();}
    void bind(Frame scene,std::uint32_t value) {
        if(fonts_)throw std::logic_error("HUD messages rebound before teardown");
        if(!scene)throw std::runtime_error("HUD messages require actual scene");
        fonts_=factory_.make_hud_fonts();if(!fonts_)throw std::runtime_error("HUD messages font acquisition returned null");
        visual_=frames_.hud_find(scene,"Text",1);state_.value2c=value;state_.fieldd37c=0;state_.flagd384=1;
        state_.fieldd36c=0;state_.input68[0]=0;state_.field168=0;state_.words30={{0,0,0,150}};
        state_.height44=static_cast<float>(factory_.hud_display_height()); //Original signed32 dimensions, height BEFORE width.
        state_.width40=static_cast<float>(factory_.hud_display_width());
        require(visual_);const auto world=frames_.actor_world_matrix(visual_);
        frames_.hud_message_position(visual_,world[12],.180000007152557373046875f,world[14]);
        frames_.update_scene_object(visual_);project_bounds(state_.first48);
        frames_.hud_message_position(visual_,world[12],-.300000011920928955078125f,world[14]);
        frames_.update_scene_object(visual_);project_bounds(state_.second58);
        frames_.object_visible(visual_,false);frames_.update_scene_object(visual_);
        state_.field10=0;state_.flagd385=state_.flagd387=state_.flagd386=0;
    }
    //00494A70/00494A40/00494A60: direct owned C-string insertion, no console
    //command parsing or message formatter substitute. Empty input is a true no-op.
    void append(const char* text,const std::array<float,3>& colour){
        if(!text)throw std::invalid_argument("HUD message requires an actual string");
        const auto length=std::strlen(text);if(!length)return;
        advance_line_indices(); //00495150 BEFORE allocation.
        Record record;
        for(unsigned n=0;n!=3;++n)record.words[n]=static_cast<std::uint32_t>(text_ftol(colour[n]));
        record.words[3]=0;record.words[4]=3;
        record.bytes=std::make_unique<char[]>(length+1);std::memcpy(record.bytes.get(),text,length+1);
        if(records_.size()>=state_.value2c){
            if(records_.empty())throw std::logic_error("HUD message capacity is zero"); //Native invalid underflow guard.
            records_.erase(records_.begin()); //Release oldest payload before shifting records.
        }
        records_.push_back(std::move(record));
    }
    void append(const char* text){append(text,state_.colours);}
    void update(std::uint32_t elapsed){ //00496230, SAME actual message/input state.
        if(state_.flagd385){
            state_.caret_d380+=elapsed;
            throw std::runtime_error("Active HUD message input requires actual00494D20 edit/clipboard receiver");
        }
        state_.fieldd37c+=elapsed;
        std::int32_t time;std::memcpy(&time,&state_.fieldd37c,4);
        if(time>2000){state_.fieldd37c=0;advance_line_indices();} //ONE step, discard overshoot, signed compare.
    }
    void draw(){ //00495F00 wraps00495A00 in RAW driver3 save→1→restore.
        if(!fonts_)throw std::logic_error("HUD messages draw before actual font initialization");
        const auto previous=frames_.hud_driver_state(3);frames_.hud_driver_state(3,1);
        try{factory_.hud_draw_messages(*this);}
        catch(...){frames_.hud_driver_state(3,previous);throw;}
        frames_.hud_driver_state(3,previous);
    }
    void project_bounds(std::array<float,4>& output) { //00495900 null visual leaves ALL caller outputs unchanged.
        if(!visual_)return;
        const auto raw=frames_.hud_message_bounds(visual_); //Native E0&100/virtual58 then12 DWORDs at10.
        constexpr float inset=.004999999888241291046142578125f,yscale=-1.33333337306976318359375f;
        const auto min_x=static_cast<float>(static_cast<long double>(raw[0])+inset);
        const auto min_y=static_cast<float>(static_cast<long double>(raw[1])+inset);
        output[2]=static_cast<float>(static_cast<long double>(raw[4])-inset);
        output[3]=static_cast<float>(static_cast<long double>(raw[5])-inset);
        output[0]=min_x;output[1]=min_y;
        output[0]=static_cast<float>(static_cast<long double>(state_.width40)*output[0]);
        output[1]=static_cast<float>(static_cast<long double>(output[1])*state_.height44*yscale);
        output[2]=static_cast<float>(static_cast<long double>(state_.width40)*output[2]);
        output[3]=static_cast<float>(static_cast<long double>(output[3])*state_.height44*yscale);
        if(output[2]<0)output[2]=state_.width40; //Native wraps negative right edge to WIDTH, no upper clamp.
        if(output[3]>state_.height44)output[3]=state_.height44;
        else if(output[3]<0)output[3]=0; //NaN/zero survive both comparisons.
    }
    template<class Mode,class Pause>void hide(Mode&& mode,Pause&& pause){ //00496120(0) then004961E0(false).
        state_.field10=0;state_.words30[0]=0;
        if((mode()&7u)==7u)pause(false); //Required0069B8B0 before visibility, not an optional notification.
        require(visual_);frames_.object_visible(visual_,false);
        state_.input68[0]=0;state_.flagd387=0;state_.flagd386=0;state_.flagd385=0;
    }
    void clear_records()noexcept { //004950C0 destroys owned raw payloads in reverse, retaining vector capacity.
        while(!records_.empty())records_.pop_back();
        state_.fieldd37c=0;fonts_.reset();
    }
    void close()noexcept { //004960F0 releases font BEFORE queued strings;004950C0 then sees null font.
        fonts_.reset();clear_records();visual_=Frame{};
    }
    const auto& state()const{return state_;}Frame visual()const{return visual_;}const auto& fonts()const{return fonts_;}
    const auto& records()const{return records_;}
    //00495A00/00495360 BYTE-font consumers; backend is actual SAME font owner.
    template<class Backend>void draw_bytes(Backend& backend){
        if(!fonts_||fonts_.get()!=&backend.fonts())throw std::invalid_argument("HUD message drawing requires its actual fonts");
        const auto& bounds=state_.field10==1?state_.second58:state_.first48;
        const auto height=text_ftol(static_cast<long double>(text_real(state_.heightd370))*state_.height44);
        const auto stored_height=static_cast<float>(height);
        auto y=text_ftol(static_cast<long double>(bounds[1])-stored_height);
        if(state_.flagd385)draw_input(backend,text_ftol(bounds[0]),y,height,text_ftol(bounds[2]));
        y=text_sub(y,height);state_.fieldd36c=0;
        std::int32_t line=0;
        for(auto n=records_.size();n;){const auto& record=records_[--n];
            if(signed_word(state_.words30[0])<=line){
                const auto target=signed_word(record.words[3]);
                while(line<target&&!state_.field10){y=text_sub(y,height);line=text_add(line,1);}
                const auto stored_y=static_cast<float>(y);
                if(stored_y<bounds[3])return;
                const auto font=record.words[4];
                if(backend.contains(font)){
                    auto x=text_ftol(bounds[0]);
                    const auto colour=((record.words[0]-0x100u)*0x100u+record.words[1])*0x100u+record.words[2];
                    for(const auto* p=record.bytes.get();*p;++p){
                        const auto glyph=backend.glyph(font,static_cast<unsigned char>(*p));
                        const auto product=static_cast<long double>(stored_height)*glyph.aspect;
                        std::int32_t advance;
                        if(product==0)advance=text_ftol(static_cast<long double>(height)*.4);
                        else{advance=text_ftol(product);emit_message_glyph(glyph,x,y,height,advance,colour);}
                        x=text_add(x,advance);if(bounds[2]<static_cast<float>(x))break;
                    }
                    flush(backend,font);y=text_sub(y,height);
                }
            }
            line=text_add(line,1);
        }
    }
private:
    void advance_line_indices(){for(auto& record:records_)++record.words[3];} //00495150 native DWORD wrapping.
    static void require(Frame value){if(!value)throw std::runtime_error("Required HUD Text artwork is missing");}
    static std::int32_t signed_word(std::uint32_t bits){std::int32_t out;std::memcpy(&out,&bits,4);return out;}
    template<class Glyph>void emit_message_glyph(const Glyph& glyph,std::int32_t x,std::int32_t y,std::int32_t height,std::int32_t advance,std::uint32_t colour){
        //004952C0 writes0.02 depth and1/depth; native fixed buffer holds1680
        //vertices. Reject native buffer overflow instead of corrupting memory.
        if(state_.fieldd36c>vertices_.size()-6)throw std::runtime_error("HUD message exceeds native glyph buffer");
        const auto left=static_cast<float>(x),right=static_cast<float>(text_add(x,advance));
        const auto top=static_cast<float>(y),bottom=static_cast<float>(text_add(y,height));
        const auto vertex=[&](float px,float py,float u,float v){vertices_[state_.fieldd36c++]={px,py,.02f,static_cast<float>(1.L/.02f),colour,0,u,v};};
        vertex(left,top,glyph.u_left,glyph.v_top);vertex(right,bottom,glyph.u_right,glyph.v_bottom);
        vertex(left,bottom,glyph.u_left,glyph.v_bottom);vertex(left,top,glyph.u_left,glyph.v_top);
        vertex(right,top,glyph.u_right,glyph.v_top);vertex(right,bottom,glyph.u_right,glyph.v_bottom);
    }
    template<class Backend>void flush(Backend& backend,unsigned font){
        if(state_.fieldd36c){backend.bind(font);backend.submit(vertices_.data(),state_.fieldd36c);state_.fieldd36c=0;}
    }
    template<class Backend>void draw_input(Backend& backend,std::int32_t x,std::int32_t y,std::int32_t height,std::int32_t right){ //00495360 BYTE.
        state_.fieldd36c=0;if(!backend.contains(3))return;
        std::string text;
        if(state_.flagd386)text="Say: ";else if(state_.flagd387)text="SayTeam: ";
        const auto end=std::find(state_.input68.begin(),state_.input68.end(),'\0');
        if(end==state_.input68.end())throw std::runtime_error("HUD message input is not terminated");
        text.append(state_.input68.begin(),end);
        for(std::size_t n=0;n<text.size();++n){if(signed_word(state_.field168)>static_cast<std::int64_t>(n))continue;
            const auto glyph=backend.glyph(3,static_cast<unsigned char>(text[n]));
            const auto product=static_cast<long double>(height)*glyph.aspect;
            std::int32_t advance;
            if(product==0)advance=text_ftol(static_cast<long double>(height)*.4);
            else{advance=text_ftol(product);emit_message_glyph(glyph,x,y,height,advance,0xffffff00u);}
            x=text_add(x,advance);if(right<x)break;
        }
        const auto ticks=signed_word(state_.caret_d380);
        if(ticks>=150&&ticks<301){const auto glyph=backend.glyph(3,'_');
            const auto advance=text_ftol(static_cast<long double>(height)*glyph.aspect);emit_message_glyph(glyph,x,y,height,advance,0xffffff00u);
        }else if(ticks>=300)state_.caret_d380=0;
        flush(backend,3);
    }
    Frames& frames_;Factory& factory_;Frame visual_{};FontOwner fonts_{};State& state_;
    std::vector<Record> records_;
    std::array<TextGlyphVertex,1680> vertices_{}; //Native16C..D36B, untouched specular guarded0.
};
} //namespace hd2::game
#endif
