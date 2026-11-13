#ifndef HD2_GAME_HUD_SCOREBOARD_LOADING_H
#define HD2_GAME_HUD_SCOREBOARD_LOADING_H
#include "game/hud_text_loading.h"
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
namespace hd2::game {
struct HudMissionScore {std::int32_t losses,limit;};
//004AE7C0/004AED00, unconditional HUD loading. Configuration is the actual
//application24 manager (224 style/240 teams); it has no successful default.
template<class Frames,class Factory>
class HudScoreboardLoading {
public:
    using Frame=typename Frames::Frame;using Text=typename Factory::DisplayText;
    HudScoreboardLoading(Frames& frames,Factory& factory):frames_(frames),factory_(factory) {}
    ~HudScoreboardLoading(){close();}
    template<class Configuration>void bind(Frame scene,std::uint32_t mode,Configuration& configuration) {
        if(text_)throw std::logic_error("HUD scoreboard rebound before teardown");
        require(scene);root_=frames_.hud_find(scene,"mp_scoretable",0xffff);
        text_=factory_.make_hud_text();if(!text_)throw std::runtime_error("HUD scoreboard text factory returned null");
        text_->text.inset=0;bind_hud_text_geometry(frames_,*text_,scene,root_);
        if((mode&7u)==7u)style_=-1;else {style_=configuration.hud_style224();if(!style_)style_=-1;}
        switch(style_) {
        case 1:append("0",0x43);append("0",0x44);append("00",0x45);append("00:00",0x46);show(true);break;
        case 2:append("0",0x43);append("0",0x44);if(configuration.hud_teams240())append("00",0x45);append("00:00",0x46);break; //No root visibility call.
        case 3: {
            append("99/99",0x44);append("00",0x45);append("00:00",0x46);require(root_);
            const auto allied=frames_.actor_find(root_,"mpallied_score",0xffff);
            const auto axis=frames_.actor_find(root_,"mpaxis_score",0xffff);require(allied);require(axis);
            const auto position=frames_.actor_position(axis);frames_.hud_message_position(allied,position[0],position[1],position[2]);
            frames_.object_visible(axis,false);frames_.update_scene_object(axis);frames_.update_scene_object(allied);break;
        }
        default:show(false);break;
        }
    }
    void close()noexcept{text_.reset();} //004AED00 deletes text without preceding explicit clear.
    template<class Queries>void update(std::uint32_t,Queries& queries){ //004AED20; elapsed and root visibility are unused.
        if(style_!=1&&style_!=2&&style_!=3)return;
        if(!text_)throw std::logic_error("HUD scoreboard update before actual text initialization");
        if(style_==1){
            const auto first=score_integer(queries.team_score(0)),second=score_integer(queries.team_score(1));
            const auto teams=queries.team_time()/1000,remaining=queries.remaining_time()/1000;
            set_number(first,0x43,0);set_number(second,0x44,1);
            set_time(teams,false,0x45,2);set_time(remaining,false,0x46,3);
        }else if(style_==2){
            std::int32_t first=0,second=0;
            for(std::int32_t index=0;index!=32;++index)if(auto* human=queries.reserved_human(index))
                if(queries.health(*human)>0&&!queries.suppressed(*human)){
                    if(queries.team(*human)==0)++first;else ++second; //All nonzero teams count in the second column.
                }
            const auto remaining=queries.remaining_time()/1000;
            set_number(first,0x43,0);set_number(second,0x44,1);
            if(queries.teams_enabled()){
                const auto teams=queries.team_time()/1000;set_time(teams,false,0x45,2);set_time(remaining,true,0x46,3);
            }else set_time(remaining,true,0x46,2);
        }else{
            const auto teams=queries.team_time()/1000,elapsed=queries.elapsed_time()/1000;
            const auto score=queries.mission_score(); //Read losses AND limit before releasing the network peer lock, even for limit-1.
            if(score.limit!=-1)set_fraction(signed_word(static_cast<std::uint32_t>(score.limit)-static_cast<std::uint32_t>(score.losses)),score.limit,0x44,0);
            set_time(teams,false,0x45,1);set_time(elapsed,false,0x46,2); //Keep prior record0 when limit is-1.
        }
    }
    //0069AE80/0069AEB0. Original DWORD multiplication/subtraction wraps before
    //signed interpretation; only remaining-time has the sign-bit clamp.
    static std::int32_t remaining_time(std::int32_t minutes,std::uint32_t elapsed){
        const auto value=signed_word(static_cast<std::uint32_t>(minutes)*60000u-elapsed);return value<0?0:value;
    }
    static std::int32_t team_time(std::int32_t seconds,std::uint32_t elapsed){return signed_word(static_cast<std::uint32_t>(seconds)*1000u-elapsed);}
    void draw(){ //004AF2B0; field08 is this SAME style_ set by004AE7C0.
        if(!root_||style_==-1||!frames_.hud_frame_visible(root_))return;
        if(!text_)throw std::logic_error("HUD scoreboard draw before text initialization");
        factory_.hud_draw_text(*text_);
    }
    void discard_borrowers()noexcept{root_=Frame{};}
    Frame root()const{return root_;}const auto& text()const{return text_;}std::int32_t style()const{return style_;}
private:
    static std::int32_t signed_word(std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;}
    static std::int32_t score_integer(float value){ //007E5E00 low DWORD, not saturating int conversion.
        const long double wide=value;if(!(wide>=-0x1p63L&&wide<0x1p63L))return 0;
        return signed_word(static_cast<std::uint32_t>(static_cast<std::int64_t>(wide)));
    }
    void set_number(std::int32_t value,std::uint8_t style,std::int32_t index){set_formatted("%d",value,0,false,style,index);}
    void set_fraction(std::int32_t value,std::int32_t limit,std::uint8_t style,std::int32_t index){set_formatted("%d/%d",value,limit,true,style,index);}
    void set_time(std::int32_t seconds,bool zero,std::uint8_t style,std::int32_t index){set_formatted(zero?"%02d:%02d":"%2d:%02d",seconds/60,seconds%60,true,style,index);}
    void set_formatted(const char* format,std::int32_t first,std::int32_t second,bool pair,std::uint8_t style,std::int32_t index){
        std::array<char,64> buffer{};const auto size=pair?std::snprintf(buffer.data(),buffer.size(),format,first,second):std::snprintf(buffer.data(),buffer.size(),format,first);
        if(size<0||static_cast<std::size_t>(size)>=buffer.size())throw std::runtime_error("HUD scoreboard formatting failed");
        text_->text.set(std::make_shared<const std::string>(buffer.data()),style,index); //006443C0→006440F0; no clear/rebuild of other records.
    }
    static void require(Frame frame){if(!frame)throw std::runtime_error("Required HUD scoreboard artwork is missing");}
    void append(const char* value,std::uint8_t style){text_->text.append(std::make_shared<const std::string>(value),style);}
    void show(bool visible){require(root_);frames_.object_visible(root_,visible);}
    Frames& frames_;Factory& factory_;Frame root_{};std::unique_ptr<Text> text_;std::int32_t style_=-1;
};
} //namespace hd2::game
#endif
