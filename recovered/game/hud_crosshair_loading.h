#ifndef HD2_GAME_HUD_CROSSHAIR_LOADING_H
#define HD2_GAME_HUD_CROSSHAIR_LOADING_H
#include "game/hud_text_loading.h"
#include <array>
#include <memory>
#include <stdexcept>
namespace hd2::game {
//004A9740/004A9F00/004A9F10/004AA760. Settings MUST resolve actual006B66D0
//current application24 manager274 or editor application20 profile73; no default
//enabled preference or fake manager is supplied by this source receiver.
template<class Frames,class Factory,class Settings>
class HudCrosshairLoading {
public:
    using Frame=typename Frames::Frame;using Text=typename Factory::DisplayText;
    HudCrosshairLoading(Frames& frames,Factory& factory,Settings& settings):frames_(frames),factory_(factory),settings_(settings) {}
    ~HudCrosshairLoading(){close();}
    void bind(Frame scene) {
        if(text_)throw std::logic_error("HUD crosshair rebound before teardown");
        require(scene);field30_=0;flag34_=0;root_=frames_.hud_find(scene,"cross",0xffff);require(root_);
        frames_.hud_message_position(root_,.5f,-.375f,-.14000000059604644775390625f);frames_.update_scene_object(root_);
        constexpr std::array<const char*,4> names{{"cross_L","cross_R","cross_T","cross_B"}};
        for(unsigned n=0;n!=4;++n)parts_[n]=frames_.actor_find(root_,names[n],1);
        constexpr std::array<float,4> zero{{0,0,0,0}},one{{1,1,1,0}}; //Fourth native vector lanes unwritten; zero guards.
        for(auto part:parts_) {
            require(part);frames_.hud_group18(part);
            frames_.hud_material_flags(indexed_material(part),0x08000000);
            frames_.hud_material_vector(indexed_material(part),0x0c,zero);
            frames_.hud_material_vector(indexed_material(part),0x10,zero);
            frames_.hud_material_vector(indexed_material(part),0x08,one);
            frames_.hud_material14(indexed_material(part),0.f);
            frames_.hud_visual_a4(part,.60000002384185791015625f);
        }
        text_=factory_.make_hud_text();if(!text_)throw std::runtime_error("HUD crosshair text factory returned null");
        name_=frames_.actor_find(root_,"cross_name",1);
        if(name_){frames_.hud_crosshair_name_scale(name_);bind_hud_text_geometry(frames_,*text_,scene,name_);frames_.object_visible(name_,false);}
        else bind_hud_text_geometry(frames_,*text_,scene,root_);
        field14_=0;frames_.update_scene_object(root_);show(1);
        colour20_[0]=colour20_[1]=colour20_[2]=0;set_colour(one);
    }
    void show(std::uint8_t visible) { //Stores request before current preference query, no equality shortcut.
        visible35_=visible;
        if(visible&&!settings_.hud_crosshair_allowed())visible35_=0;
        require(root_);frames_.object_visible(root_,visible35_!=0);frames_.update_scene_object(root_);
    }
    void set_colour(const std::array<float,4>& value) { //Root E0 bit1 (visibility) gates; NaN differs and causes assignment.
        require(root_);if(!frames_.hud_frame_visible(root_))return;
        if(value[0]==colour20_[0]&&value[1]==colour20_[1]&&value[2]==colour20_[2])return;
        colour20_=value; //All FOUR words cached before material calls, even if one later fails.
        for(auto n:{2u,3u,1u,0u})frames_.hud_material_vector(frames_.hud_first_material(parts_[n]),0x0c,value);
        frames_.update_scene_object(root_);
    }
    void close()noexcept{text_.reset();} //004A9F00, no preceding record clear.
    void draw(){ //004AAAB0 checks field30, independently of root visibility.
        if(!field30_)return;
        if(!text_)throw std::logic_error("HUD crosshair draw before text initialization");
        factory_.hud_draw_text(*text_);
    }
    void discard_borrowers()noexcept{root_=name_=Frame{};parts_.fill(Frame{});}
    Frame root()const{return root_;}Frame name()const{return name_;}const auto& parts()const{return parts_;}const auto& text()const{return text_;}
    const auto& colour()const{return colour20_;}std::uint8_t visible()const{return visible35_;}std::uint32_t field14()const{return field14_;}
private:
    static void require(Frame value){if(!value)throw std::runtime_error("Required HUD crosshair artwork is missing");}
    Frame indexed_material(Frame visual){const auto lod=frames_.first_person_material_lod(visual);require(lod);return frames_.first_person_group_material(lod,0);}
    Frames& frames_;Factory& factory_;Settings& settings_;Frame root_{},name_{};std::array<Frame,4> parts_{};
    std::unique_ptr<Text> text_;std::array<float,4> colour20_{};std::uint32_t field14_=0,field30_=0;std::uint8_t flag34_=0,visible35_=0;
};
} //namespace hd2::game
#endif
