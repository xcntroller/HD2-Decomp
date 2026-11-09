#ifndef HD2_GAME_TEXT_LAYOUT_H
#define HD2_GAME_TEXT_LAYOUT_H
#include "game/text_glyphs.h"
#include "frontend/text_object.h"
#include <algorithm>
#include <memory>
#include <vector>
namespace hd2::game {
//00645270/00645F20/00646820 byte-font cache. Native fragment/line SHORTs
//remain SHORTs; these are native containers, not copies of the packed ABI.
class TextLayout {
public:
    struct Fragment {
        frontend::SharedText text;
        std::uint8_t style=0;
        std::int16_t width=-1,height=-1,x=-1,y=-1;
    };
    struct Line {
        std::vector<Fragment> fragments;
        std::int16_t baseline=-1,after=-1,before=-1,height=-1,remaining=-1;
        Line()=default;Line(Line&&)=default;Line& operator=(Line&&)=default;
        Line(const Line&)=delete;Line& operator=(const Line&)=delete;
        ~Line(){while(!fragments.empty())fragments.pop_back();} //006456F0/00644C50 reverse fragments.
    };
    TextLayout()=default;
    TextLayout(const TextLayout&)=delete;
    TextLayout& operator=(const TextLayout&)=delete;
    ~TextLayout(){clear();}
    //00644BA0: reverse line/absolute owners; keep container capacity.
    void clear(){while(!lines.empty())lines.pop_back();while(!absolute.empty())absolute.pop_back();}
    static std::string_view bytes(const frontend::SharedText& text){return text?std::string_view(*text):std::string_view{};}
    template<class State,class Styles,class Fonts>bool rebuild(State& state,Styles& styles,Fonts& fonts){
        clear();
        const auto& records=state.text.records();
        if(records.empty())return false; //Native leaves dirty72 set on this return.
        bool joining=false;
        for(const auto& record:records){
            const auto* style=styles.descriptor(record.style);
            if(!style||!fonts.contains((*style)[0]))return false; //00644CF0.
            if(text_real((*style)[4])<0||text_real((*style)[5])<0){
                if(!((*style)[11]&0xffu)){joining=false;new_line(state);}
                else if(((*style)[12]&8u)||!joining){joining=true;new_line(state);}
                wrap(state,fonts,*style,record);
            }else{
                Fragment fragment{record.text,record.style};
                const auto height=text_pixel_height(state.height(),*style);
                fragment.height=text_short(height);
                fragment.width=text_short(text_byte_width(fonts,*style,bytes(fragment.text),height));
                if(text_real((*style)[3])!=0){
                    const float maximum=static_cast<float>(static_cast<long double>(text_short(state.width()))*text_real((*style)[3]));
                    if(maximum<static_cast<float>(fragment.width))crop(fragment,fonts,*style,fragment.height,maximum,height);
                }
                absolute.push_back(std::move(fragment));
            }
        }
        state.bytes70[3]=0;
        if(!lines.empty()){
            const auto* first=styles.descriptor(records.front().style);
            long double offset=0;
            if((*first)[10]==2)offset=(static_cast<long double>(geometry(state,5))-total_height())*.5f;
            else if((*first)[10]==3)offset=static_cast<long double>(geometry(state,5))-total_height();
            if(!(offset>=0))offset=0;
            auto baseline=text_short(text_ftol(offset+geometry(state,3)));
            for(auto& line:lines){
                baseline=text_short(baseline+line.before+line.height);line.baseline=baseline;
                baseline=text_short(baseline+line.after);
            }
            for(std::size_t n=0;n!=lines.size();++n){
                auto& line=lines[n];if(line.fragments.empty())continue;
                positioned(state,styles,line);
                if(line.remaining<0)state.bytes70[3]=1;
                bool final=true;
                if(n+1<lines.size()&&!lines[n+1].fragments.empty())
                    final=!(styles.descriptor(lines[n+1].fragments.front().style)->at(9)&4u);
                horizontal(state,styles,fonts,line,final,n);
                if(line.remaining<0)state.bytes70[3]=1;
            }
        }
        for(auto& fragment:absolute){
            const auto& style=*styles.descriptor(fragment.style);
            const float x=static_cast<float>(static_cast<long double>(text_real(style[4]))*geometry(state,4)+.5f);
            const float y=static_cast<float>(static_cast<long double>(text_real(style[5]))*geometry(state,5)+.5f);
            if(style[9]==1)fragment.x=text_short(text_ftol(x));
            else if(style[9]==2)fragment.x=text_short(text_ftol(static_cast<long double>(x)-fragment.width*.5f));
            else if(style[9]==3)fragment.x=text_short(text_ftol(static_cast<long double>(x)-fragment.width));
            if(style[10]==1)fragment.y=text_short(text_ftol(y));
            else if(style[10]==2)fragment.y=text_short(text_ftol(static_cast<long double>(y)+fragment.height*.5f));
            else if(style[10]==3)fragment.y=text_short(text_ftol(static_cast<long double>(y)+fragment.height));
            //006467B0 unordered comparisons do not set the clip flag.
            if(fragment.x<0||fragment.y<0||fragment.y+fragment.height>geometry(state,5)||fragment.x+fragment.width>geometry(state,4))state.bytes70[3]=1;
        }
        recompute_scroll(state);
        state.bytes70[2]=0;state.text.dirty=false;
        return true;
    }
    //00646940. Leading/trailing paragraph margins are excluded from height.
    std::int32_t total_height()const{
        if(lines.empty())return 0;
        std::int32_t result=-lines.front().before;
        for(const auto& line:lines)result=text_add(result,line.after+line.height+line.before);
        return text_sub(result,lines.back().after);
    }
    template<class State>void recompute_scroll(State& state)const{ //00643E80.
        auto maximum=lines.empty()?0:text_ftol(static_cast<long double>(lines.back().baseline)-geometry(state,1)+1.f);
        if(maximum<0)maximum=0;
        if(maximum>0)maximum=text_add(maximum,1);
        state.word7c=static_cast<std::uint32_t>(maximum);
        auto position=text_signed(state.word78);
        if(maximum<position)position=maximum;
        if(position<0)position=0;
        state.word78=static_cast<std::uint32_t>(position);
        if(maximum)state.bytes70[3]=1;
    }
    template<class State>void update_offset(State& state,std::int16_t increment)const{ //00648EB0.
        auto offset=text_short(state.word76+increment);
        const auto minimum=lines.empty()?0:text_ftol(static_cast<long double>(geometry(state,1))-lines.back().baseline);
        if(minimum>0){state.word76=0;return;}
        if(offset>0)offset=0;
        if(offset<minimum)offset=text_short(text_sub(minimum,2));
        state.word76=static_cast<std::uint16_t>(offset);
    }
    std::vector<Line> lines;
    std::vector<Fragment> absolute;
private:
    static std::int32_t text_signed(std::uint32_t bits){std::int32_t out;std::memcpy(&out,&bits,4);return out;}
    template<class State>static float geometry(const State& state,unsigned index){return text_real(state.geometry8[index]);}
    template<class State>void new_line(const State& state){Line line;line.remaining=text_short(text_ftol(geometry(state,4)));lines.push_back(std::move(line));}
    static frontend::SharedText slice(std::string_view text){return std::make_shared<const std::string>(text);}
    static bool space(unsigned char c){return c==' '||(c>=9&&c<=13);} //007E6636 CRT bit8; BYTE<80 only.
    static std::string_view c_tail(std::string_view text,std::size_t start){const auto tail=text.substr(start);return tail.substr(0,tail.find('\0'));}
    static void append(Line& line,frontend::SharedText text,std::uint8_t style,std::int32_t width,
                       std::int32_t height,std::int32_t after,std::int32_t before){
        line.fragments.push_back(Fragment{std::move(text),style,text_short(width)});
        line.height=text_short(std::max<std::int32_t>(line.height,height));
        line.after=text_short(std::max<std::int32_t>(line.after,after));
        line.before=text_short(std::max<std::int32_t>(line.before,before));
        line.remaining=text_short(text_sub(line.remaining,width));
    }
    template<class Fonts>static void crop(Fragment& fragment,Fonts& fonts,const frontend::TextStyleDescriptor& style,
                                         std::int32_t crop_height,float maximum,std::int32_t measure_height){
        fragment.text=slice(text_crop_bytes(fonts,style,bytes(fragment.text),crop_height,maximum));
        fragment.width=text_short(text_byte_width(fonts,style,bytes(fragment.text),measure_height));
    }
    template<class State,class Fonts>void wrap(State& state,Fonts& fonts,const frontend::TextStyleDescriptor& style,const frontend::TextRecord& record){
        const auto screen_height=text_short(state.height());
        const auto height=text_pixel_height(state.height(),style);
        const auto maximum=text_ftol(static_cast<long double>(text_short(state.width()))*text_real(style[3])+.5f);
        const auto after=text_ftol(static_cast<long double>(screen_height)*text_real(style[7])+.5f);
        const auto before=text_ftol(static_cast<long double>(screen_height)*text_real(style[6])+.5f);
        auto available=lines.back().remaining;
        std::int32_t limit=maximum>0&&maximum<available?maximum:available;
        if(!(style[11]&0xff00u)){
            const auto width=text_byte_width(fonts,style,bytes(record.text),height);
            append(lines.back(),record.text,record.style,width,height,after,before);
            auto& fragment=lines.back().fragments.back();
            if(text_real(style[3])!=0&&maximum<fragment.width)
                crop(fragment,fonts,style,text_ftol(static_cast<float>(height)),static_cast<float>(maximum),height);
            return; //Remaining uses the ORIGINAL width even after cropping.
        }
        auto text=bytes(record.text);bool original=true;
        std::size_t index=0;
        std::int64_t last_space=0; //Native initial0, subsequent new lines use-1.
        std::int32_t width=0,space_width=0;
        while(index<text.size()){
            const auto c=static_cast<unsigned char>(text[index]);
            const auto next=text_add(width,text_glyph_width(fonts,style,c,height));
            if(limit<next){
                if(last_space){
                    if(last_space==-1){append(lines.back(),slice(text.substr(0,index)),record.style,space_width,height,after,before);return;}
                    const auto position=static_cast<std::size_t>(last_space);
                    append(lines.back(),slice(text.substr(0,position)),record.style,space_width,height,after,before);
                    text=c_tail(text,position+1);original=false;
                }
                new_line(state);available=lines.back().remaining;
                limit=maximum>0&&maximum<available?maximum:available;
                last_space=-1;space_width=width=0;index=0;
            }else{
                if(c<0x80&&space(c)){space_width=width;last_space=static_cast<std::int64_t>(index);}
                ++index;width=next;
            }
        }
        append(lines.back(),original?record.text:slice(text),record.style,width,height,after,before);
    }
    template<class State,class Styles>static void positioned(const State& state,Styles& styles,Line& line){ //00646DB0.
        for(auto& fragment:line.fragments){const auto& style=*styles.descriptor(fragment.style);
            if(!(style[12]&0x10u))continue;
            const float x=static_cast<float>(static_cast<long double>(text_real(style[4]))*geometry(state,4)+geometry(state,0));
            if(style[9]==1)fragment.x=text_short(text_ftol(x));
            else if(style[9]==2)fragment.x=text_short(text_ftol(static_cast<long double>(x)-fragment.width*.5f));
            else if(style[9]==3)fragment.x=text_short(text_ftol(static_cast<long double>(x)-fragment.width));
        }
    }
    template<class State,class Styles,class Fonts>static void split_words(const State& state,Styles& styles,Fonts& fonts,Line& line){ //006472E0→00647830.
        const auto original_count=line.fragments.size();
        for(std::size_t n=0;n<original_count;++n){
            const auto record=line.fragments[n];const auto& style=*styles.descriptor(record.style);
            const auto height=text_pixel_height(state.height(),style);
            auto text=bytes(record.text);std::size_t index=0;std::int32_t width=0;
            while(index<text.size()){
                const auto c=static_cast<unsigned char>(text[index]);
                const auto advance=text_glyph_width(fonts,style,c,height);
                if(c<0x80&&space(c)){
                    line.fragments.push_back(Fragment{slice(text.substr(0,index)),record.style,text_short(width)});
                    text=c_tail(text,index+1);index=0;width=0;
                }else {width=text_add(width,advance);++index;}
            }
            if(!text.empty()&&width>0)line.fragments.push_back(Fragment{slice(text),record.style,text_short(width)});
        }
        line.fragments.erase(line.fragments.begin(),line.fragments.begin()+static_cast<std::ptrdiff_t>(original_count));
    }
    template<class State,class Styles,class Fonts>static void horizontal(const State& state,Styles& styles,Fonts& fonts,Line& line,bool final,std::size_t index){
        const auto& style=*styles.descriptor(line.fragments.front().style);
        if(style[12]&0x10u)return;
        long double x=geometry(state,0);
        bool float_step=false;
        long double gap=0;
        if(state.bytes80[1]==1&&state.word88==static_cast<std::int64_t>(index)){
            const auto delta=text_sub(line.remaining,text_glyph_width(fonts,style,'_',text_pixel_height(state.height(),style)));
            if(delta<0){x+=delta;for(auto& fragment:line.fragments){fragment.x=text_short(text_ftol(x));x+=fragment.width;}return;}
        }
        if(style[9]==1||line.remaining<=0)float_step=true;
        else if(style[9]==2)x+=line.remaining/2;
        else if(style[9]==3)x+=line.remaining;
        else if(style[9]==4){
            if(!final)split_words(state,styles,fonts,line);
            if(!final&&line.fragments.size()>1){
                std::int32_t width=0;for(const auto& fragment:line.fragments)width=text_add(width,fragment.width);
                gap=(static_cast<long double>(geometry(state,4))-width)/(line.fragments.size()-1);
            }
            float_step=true;
        }else return;
        for(auto& fragment:line.fragments){fragment.x=text_short(text_ftol(x));x+=static_cast<long double>(fragment.width)+gap;if(float_step)x=static_cast<float>(x);}
    }
};
} //namespace hd2::game
#endif
