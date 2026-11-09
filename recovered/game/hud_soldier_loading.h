#ifndef HD2_GAME_HUD_SOLDIER_LOADING_H
#define HD2_GAME_HUD_SOLDIER_LOADING_H
#include "game/hud_artwork_loading.h"
#include "game/hud_text_loading.h"
#include <memory>
#include <utility>
namespace hd2::game {
//004B30E0/004B3840/004B3E70: one of six HUD soldier panels. Main HUD binds
//indices5..0, sharing the SAME first/second artwork tables. Runtime Human/photo/
//health/command updates004B3890+ remain required consumers, not invented here.
template<class Frames,class Factory>
class HudSoldierLoading {
public:
    using Frame=typename Frames::Frame;using Text=typename Factory::DisplayText;
    using TextureOwner=typename Frames::ResourceOwner;
    struct Artwork {
        Frame root{},selected{},player{},photo{},health_bar{},health_cross{},health_zero{},stamina_bar{},disguise_bar{},visibility{},ai{},ear{},silence{},medic{},vip{};
        Frame photo_material{},command_material{},mind_material{},universal_texture{};
    };
    HudSoldierLoading(Frames& frames,Factory& factory):frames_(frames),factory_(factory) {}
    ~HudSoldierLoading(){close();}
    void bind(Frame scene,unsigned index,const HudArtworkLoading<Frames>& tables,std::uint32_t mode) {
        if(death_||text_||visibility_text_)throw std::logic_error("HUD soldier rebound before teardown");
        if(index>=6)throw std::out_of_range("HUD soldier panel index");
        require(scene);
        first_states_=&tables.first_states();second_states_=&tables.second_states();human74_=nullptr;scene_=scene;index_=index;
        std::array<char,32> name{};std::snprintf(name.data(),name.size(),"Soldier%02u",index);
        art_.root=frames_.hud_find(scene,name.data(),0xffff);require(art_.root);frames_.hud_frame_tag(art_.root,index+100);
        art_.player=child("player0*");show(art_.player,false);art_.ear=child("ear*");show(art_.ear,false);
        art_.selected=child("select*");show(art_.selected,false);art_.photo=child("photo*");
        frames_.hud_injury_prepare(art_.photo);frames_.hud_group18(art_.photo);art_.photo_material=frames_.hud_first_material(art_.photo);
        art_.health_bar=child("healthbar*");art_.health_cross=child("healthcr*");art_.health_zero=child("health0*");
        art_.stamina_bar=child("stamina0*");art_.stamina_bar=child("staminabar*"); //Original overwrites slot24, but flags BOTH visuals.
        art_.disguise_bar=child("disguise0*");art_.disguise_bar=child("disguisebar*");
        if((mode&7u)!=7u){frames_.hud_unit_scale(art_.disguise_bar);frames_.update_scene_object(art_.disguise_bar);}
        const auto command=child("command*");frames_.hud_group18(command);art_.command_material=frames_.hud_first_material(command);
        const auto mind=child("mind*");frames_.hud_group18(mind);art_.mind_material=frames_.hud_first_material(mind);
        const auto universal=frames_.hud_find(scene,"univerz",0xffff);show(universal,false);art_.universal_texture=frames_.hud_first_texture(universal);
        death_=frames_.load_hud_death_texture();if(!death_)throw std::runtime_error("Required HUD soldier death texture is missing");
        text_=make_text();text_->text.inset=0;bind_hud_text_geometry(frames_,*text_,scene,art_.root);
        art_.visibility=child("visibility*");visibility_text_=make_text();visibility_text_->text.inset=0;
        bind_hud_text_geometry(frames_,*visibility_text_,scene,art_.visibility);show(art_.visibility,false);
        set_selected(0);show(art_.root,false);
        art_.ai=frames_.actor_find(art_.root,"AImode*",0x10001);if(art_.ai)frames_.hud_visual_flags(art_.ai,0x1000);
        field98_=-1;colour_a0_[0]=colour_a0_[1]=colour_a0_[2]=1; //Fourth native stack lane is unwritten; preserve known lane.
        art_.silence=frames_.actor_find(art_.root,"silence*",0x10001);art_.medic=frames_.actor_find(art_.root,"medic*",0x10001);
        art_.vip=frames_.actor_find(art_.root,"mp_vip*",0x10001);if(art_.vip)show(art_.vip,false);field8c_=0;
    }
    void set_selected(std::uint8_t selected) { //004B3E70 stores BYTE BEFORE visibility endpoint, equality returns.
        if(selected89_==selected)return;
        selected89_=selected;show(art_.selected,selected!=0);
    }
    void close()noexcept { //004B3840: death FIRST, then primary/visibility text; no preceding record clear.
        death_.reset();text_.reset();visibility_text_.reset();art_.root=Frame{};
    }
    void draw(){ //004B4760 rereads ROOT visibility before EACH text, not visibility*.
        require(art_.root);
        if(frames_.hud_frame_visible(art_.root)){
            if(!text_)throw std::logic_error("HUD soldier draw before primary text initialization");
            factory_.hud_draw_text(*text_);
        }
        if(frames_.hud_frame_visible(art_.root)){
            if(!visibility_text_)throw std::logic_error("HUD soldier draw before visibility text initialization");
            factory_.hud_draw_text(*visibility_text_);
        }
    }
    void discard_borrowers()noexcept{scene_=Frame{};art_=Artwork{};first_states_=nullptr;second_states_=nullptr;human74_=nullptr;}
    const auto& artwork()const{return art_;}const auto& text()const{return text_;}const auto& visibility_text()const{return visibility_text_;}
    const auto* first_states()const{return first_states_;}const auto* second_states()const{return second_states_;}
    Frame death_texture()const{return death_.get();}unsigned index()const{return index_;}std::uint8_t selected()const{return selected89_;}
    float field98()const{return field98_;}const auto& colour_a0()const{return colour_a0_;}std::uint32_t field8c()const{return field8c_;}
private:
    static void require(Frame frame){if(!frame)throw std::runtime_error("Required HUD soldier artwork is missing");}
    void show(Frame frame,bool visible){require(frame);frames_.object_visible(frame,visible);}
    Frame child(const char* name){const auto result=frames_.actor_find(art_.root,name,0x10001);require(result);frames_.hud_visual_flags(result,0x1000);return result;}
    std::unique_ptr<Text> make_text(){auto result=factory_.make_hud_text();if(!result)throw std::runtime_error("HUD soldier text factory returned null");return result;}
    Frames& frames_;Factory& factory_;Frame scene_{};Artwork art_{};unsigned index_=0;
    const std::array<Frame,20>* first_states_=nullptr;const std::array<Frame,40>* second_states_=nullptr;void* human74_=nullptr;
    TextureOwner death_{};std::unique_ptr<Text> text_,visibility_text_;
    //Defined zero guards for cold/unwritten storage, not recovery credit for a native constructor.
    std::uint8_t selected89_=0;float field98_=0;std::array<float,4> colour_a0_{};std::uint32_t field8c_=0;
};
} //namespace hd2::game
#endif
