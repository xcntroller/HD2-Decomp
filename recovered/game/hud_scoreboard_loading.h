#ifndef HD2_GAME_HUD_SCOREBOARD_LOADING_H
#define HD2_GAME_HUD_SCOREBOARD_LOADING_H
#include "game/hud_text_loading.h"
#include <memory>
#include <stdexcept>
#include <string>
namespace hd2::game {
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
    void draw(){ //004AF2B0; field08 is this SAME style_ set by004AE7C0.
        if(!root_||style_==-1||!frames_.hud_frame_visible(root_))return;
        if(!text_)throw std::logic_error("HUD scoreboard draw before text initialization");
        factory_.hud_draw_text(*text_);
    }
    void discard_borrowers()noexcept{root_=Frame{};}
    Frame root()const{return root_;}const auto& text()const{return text_;}std::int32_t style()const{return style_;}
private:
    static void require(Frame frame){if(!frame)throw std::runtime_error("Required HUD scoreboard artwork is missing");}
    void append(const char* value,std::uint8_t style){text_->text.append(std::make_shared<const std::string>(value),style);}
    void show(bool visible){require(root_);frames_.object_visible(root_,visible);}
    Frames& frames_;Factory& factory_;Frame root_{};std::unique_ptr<Text> text_;std::int32_t style_=-1;
};
} //namespace hd2::game
#endif
