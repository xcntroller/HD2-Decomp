#ifndef HD2_GAME_HUD_SOLDIER_LOADING_H
#define HD2_GAME_HUD_SOLDIER_LOADING_H
#include "game/hud_artwork_loading.h"
#include "game/hud_text_loading.h"
#include "game/human_hud_loading.h"
#include "game/human_inventory_lookup.h"
#include "game/first_person_human_items.h"
#include <memory>
#include <utility>
namespace hd2::game {
//004B30E0/004B3840/004B3E70: one of six HUD soldier panels. Main HUD binds
//indices5..0, sharing the SAME first/second artwork tables. Actual Human/photo/
//caption/initial command binding is supplied; later health/command simulation
//remains a separate receiver, not a successful loading replacement.
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
    template<class Human,class Items>void bind_human(Human& human,Items& items,std::uint32_t mode,std::int32_t local_player) { //004B3C60.
        human74_=&human;
        if(!text_||!visibility_text_)throw std::logic_error("HUD Human binding requires its actual text owners");
        text_->text.clear();visibility_text_->text.clear();
        update_name(human,items,mode);
        if(!alive(human))material_texture(art_.photo_material,death_.get());
        else if(const auto* profile=human.profile_record){
            if(profile->portrait)material_texture(art_.photo_material,frames_.hud_portrait_texture(profile->portrait));
            FirstPersonHumanItems<Items> hand(items);const auto weapon=hand.weapon(human);
            text_->text.set(weapon?factory_.hud_localized(items.runtime_item(weapon).definition().caption):std::make_shared<const std::string>(),0x3b,1);
        }
        require(art_.photo_material);frames_.hud_material_vector(art_.photo_material,4,{{1,1,1,0}}); //Material virtual04, RGB only; native W unwritten.
        frames_.update_scene_object(art_.photo);
        flagb0_=true;fields84_=fields80_=fields78_=1;field7c_=1;flag88_=false;selected89_=0;
        show(art_.selected,false);first_mode_=second_mode_=-1;
        const bool special=find_human_inventory_definition(human,items,0x16)||find_human_inventory_definition(human,items,7)||find_human_inventory_definition(human,items,0x104);
        show(art_.silence,special);
        show(art_.medic,signed_word(human.properties.stats_ec[7])>=41); //005A6600(7), raw signed stat108.
        update_first_mode(human,mode,local_player);update_second_mode(human,mode,local_player);
    }
    template<class Human,class Items>void update_name(Human& human,Items& items,std::uint32_t mode) { //004B3890.
        require_human(human);
        if((mode&7u)!=7u){
            bind_network_hud_human_name(human,items,factory_,*text_,field8c_);return;
        }
        field8c_=offline_human_hud_rank(human);const auto name=offline_human_hud_name(frames_,human);
        auto value=name;
        if(human.properties.field19c!=4&&human.properties.field19c!=5){
            if(field8c_>=8)throw std::runtime_error("HUD Human rank exceeds actual eight localization rows");
            const auto rank=factory_.hud_localized(0x27a6u+field8c_);value=(rank?*rank:std::string{})+" "+name;
        }
        text_->text.set(std::make_shared<const std::string>(std::move(value)),0x3a,0);
    }
    template<class Human>void update_first_mode(Human& human,std::uint32_t mode,std::int32_t local_player) { //004B47D0.
        require_human(human);
        if(!alive(human)){material_texture(art_.command_material,art_.universal_texture);return;}
        const auto value=human_hud_first_mode(human);if(first_mode_==value)return;
        first_mode_=value;
        if(!human_camera_is_player(human,mode,local_player)&&value!=-1){
            if(!first_states_)throw std::logic_error("HUD Human lost actual first-state artwork table");
            const auto index=value==3?2:value==2?3:value;
            material_texture(art_.command_material,first_states_->at(static_cast<std::size_t>(index)));return;
        }
        material_texture(art_.command_material,art_.universal_texture);
    }
    template<class Human>void update_second_mode(Human& human,std::uint32_t mode,std::int32_t local_player) { //004B4860.
        require_human(human);
        if(!alive(human)){material_texture(art_.mind_material,art_.universal_texture);return;}
        const auto value=human_hud_second_mode(human);if(second_mode_==value)return;
        second_mode_=value;
        if(!human_camera_is_player(human,mode,local_player)&&value!=-1){
            if(!second_states_)throw std::logic_error("HUD Human lost actual second-state artwork table");
            material_texture(art_.mind_material,second_states_->at(static_cast<std::size_t>(value)));return;
        }
        material_texture(art_.mind_material,art_.universal_texture);
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
    const void* human_borrower()const{return human74_;}std::int32_t first_mode()const{return first_mode_;}std::int32_t second_mode()const{return second_mode_;}
private:
    static std::int32_t signed_word(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
    template<class Human>static bool alive(const Human& human){return signed_word(human.properties.stats_ec[0])>0;}
    template<class Human>void require_human(const Human& human)const{if(human74_!=&human||!text_)throw std::logic_error("HUD soldier update requires its SAME bound Human/text");}
    void material_texture(Frame material,Frame texture){require(material);frames_.first_person_material_texture(material,texture);} //Actual material virtual1C.
    static void require(Frame frame){if(!frame)throw std::runtime_error("Required HUD soldier artwork is missing");}
    void show(Frame frame,bool visible){require(frame);frames_.object_visible(frame,visible);}
    Frame child(const char* name){const auto result=frames_.actor_find(art_.root,name,0x10001);require(result);frames_.hud_visual_flags(result,0x1000);return result;}
    std::unique_ptr<Text> make_text(){auto result=factory_.make_hud_text();if(!result)throw std::runtime_error("HUD soldier text factory returned null");return result;}
    Frames& frames_;Factory& factory_;Frame scene_{};Artwork art_{};unsigned index_=0;
    const std::array<Frame,20>* first_states_=nullptr;const std::array<Frame,40>* second_states_=nullptr;void* human74_=nullptr;
    TextureOwner death_{};std::unique_ptr<Text> text_,visibility_text_;
    //Defined zero guards for cold/unwritten storage, not recovery credit for a native constructor.
    std::uint8_t selected89_=0;float field98_=0;std::array<float,4> colour_a0_{};std::uint32_t field8c_=0;
    std::int32_t first_mode_=-1,second_mode_=-1;std::uint32_t fields78_=0,fields80_=0,fields84_=0;float field7c_=0;bool flag88_=false,flagb0_=false;
};
} //namespace hd2::game
#endif
