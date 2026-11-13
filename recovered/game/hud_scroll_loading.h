#ifndef HD2_GAME_HUD_SCROLL_LOADING_H
#define HD2_GAME_HUD_SCROLL_LOADING_H
#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <cmath>
#include "game/text_glyphs.h"
namespace hd2::game {
//004AE2E0 inventory scrollbar loading and required button/slider bind/state/
//geometry callees. Frame/callback identities are borrowed; only slider20's
//seven-float allocation is owned. Runtime callbacks borrow the same inventory
//only during the call; no second input/scroll/slot owner is constructed.
template<class Frames>
class HudScrollLoading {
public:
    using Frame=typename Frames::Frame;
    struct Button {Frame root{},normal{},active{},inactive{};void* callback=nullptr;std::uint32_t tag=0;std::uint8_t disabled0c=0;};
    struct Slider {Frame root{},normal{},active{},base{};void* callback=nullptr;std::uint32_t tag=0;std::uint8_t disabled0c=0;
        std::unique_ptr<std::array<float,7>> geometry;bool horizontal24=false,vertical25=false;float percentage28=0;};
    explicit HudScrollLoading(Frames& frames):frames_(frames) {}
    ~HudScrollLoading(){close_geometry();}
    void bind(void* inventory,Frame root,std::uint32_t tag) { //RET12: actual inventory identity, frame, tag.
        if(slider_.geometry)throw std::logic_error("HUD scrollbar rebound before retirement");
        require(root);root_=root;tag_=tag;frames_.hud_frame_tag(root,tag);callback_=inventory;
        if(const auto up=frames_.actor_find(root,"barrow up*",0x10020))bind_button(up_,this,up,tag|1u);
        if(const auto down=frames_.actor_find(root,"barrow down*",0x10020))bind_button(down_,this,down,tag|2u);
        if(const auto slider=frames_.actor_find(root,"slider*",0x10020))bind_slider(this,slider,tag|3u);
        set_disabled(1);words80_={{0,0,0,0xffffffffu,1}};
    }
    bool bind_button(Button& button,void* parent,Frame root,std::uint32_t tag) { //004ADA70, inner false result ignored by containing binder.
        require(root);button.root=root;button.tag=tag;frames_.hud_frame_tag(root,tag);button.callback=parent;
        button.normal=frames_.actor_find(root,"normal*",0x10001);button.active=frames_.actor_find(root,"actived*",0x10001);
        button.inactive=frames_.actor_find(root,"inactived*",0x10001);
        if(!button.normal||!button.active||!button.inactive)return false;
        copy_position(button.active,button.normal);copy_position(button.inactive,button.normal);
        for(auto frame:{button.normal,button.active,button.inactive})frames_.hud_frame_tag(frame,tag);
        button_state(button,true,false,false);return true;
    }
    bool bind_slider(void* parent,Frame root,std::uint32_t tag) { //004ADC90.
        require(root);slider_.root=root;slider_.tag=tag;frames_.hud_frame_tag(root,tag);slider_.callback=parent;
        slider_.normal=frames_.actor_find(root,"normal*",0x10001);slider_.active=frames_.actor_find(root,"actived*",0x10001);
        slider_.base=frames_.actor_find(root,"sliderbase*",0x10001);
        if(!slider_.normal||!slider_.active||!slider_.base)return false;
        copy_position(slider_.active,slider_.normal);frames_.object_visible(slider_.base,false);frames_.update_scene_object(slider_.base);
        frames_.hud_frame_tag(slider_.normal,tag);frames_.hud_frame_tag(slider_.active,tag);slider_state(true,false);
        slider_.geometry=std::make_unique<std::array<float,7>>();rebuild_slider_geometry();return true;
    }
    void set_disabled(std::uint8_t disabled) { //004AE790 calls004ADBA0 twice, then004AE290. Store flags BEFORE frame calls.
        disabled0c_=disabled;
        up_.disabled0c=disabled;button_state(up_,disabled==0,false,disabled!=0);
        down_.disabled0c=disabled;button_state(down_,disabled==0,false,disabled!=0);
        slider_.disabled0c=disabled;slider_state(disabled==0,false);
    }
    void rebuild_slider_geometry() { //004ADDD0, two frame10 bounds reads and float-store order.
        require(slider_.normal);
        const auto normal=frames_.hud_scroll_bounds(slider_.normal);
        const auto half_x=static_cast<float>((static_cast<long double>(normal[4])-normal[0])*.5f);
        const auto half_y=static_cast<float>((static_cast<long double>(normal[1])-normal[5])*.5f);
        require(slider_.base);const auto base=frames_.hud_scroll_bounds(slider_.base);auto& g=geometry();
        g[0]=static_cast<float>(static_cast<long double>(base[0])+half_x);g[2]=static_cast<float>(static_cast<long double>(base[4])-half_x);
        g[1]=static_cast<float>(static_cast<long double>(base[1])-half_y);g[3]=static_cast<float>(static_cast<long double>(base[5])+half_y);
        const auto position=frames_.actor_position(slider_.normal);
        g[0]=static_cast<float>(static_cast<long double>(g[0])-position[0]);g[2]=static_cast<float>(static_cast<long double>(g[2])-position[0]);
        g[1]=static_cast<float>(static_cast<long double>(g[1])-position[2]);g[3]=static_cast<float>(static_cast<long double>(g[3])-position[2]);
        g[5]=static_cast<float>(static_cast<long double>(g[3])-g[1]);g[4]=static_cast<float>(static_cast<long double>(g[2])-g[0]);
        slider_.vertical25=g[5]>g[4];slider_.horizontal24=!slider_.vertical25;set_percentage(50.f);
    }
    void set_percentage(float percentage) { //004AE0F0 reads root world position BEFORE equality comparison.
        require(slider_.root);const auto world=frames_.actor_world_matrix(slider_.root);
        std::array<float,4> position{{world[12],world[13],world[14],world[15]}};
        if(slider_.percentage28==percentage)return;
        slider_.percentage28=percentage;if(percentage<0)slider_.percentage28=0;if(slider_.percentage28>100)slider_.percentage28=100;
        constexpr float percent=.00999999977648258209228515625f;
        if(slider_.vertical25) {
            auto& g=geometry();auto value=static_cast<long double>(g[3])-static_cast<long double>(g[5])*slider_.percentage28*percent;
            if(value<g[1])value=g[1];
            if(value>g[3])value=g[3];
            position[1]=static_cast<float>(value);
        } else if(slider_.horizontal24) {
            auto& g=geometry();auto value=static_cast<long double>(g[4])*slider_.percentage28*percent+g[0];
            if(value<g[0])value=g[0];
            if(value>g[2])value=g[2];
            position[0]=static_cast<float>(value);
        } else return;
        frames_.hud_scroll_position(slider_.root,position);frames_.update_scene_object(slider_.root);
    }
    void set_range(std::int32_t count){ //004AE4D0; no synthetic enable/percentage when actual geometry is absent.
        if(count<2){words80_[1]=words80_[2]=0;words80_[0]=0x42c80000u;set_disabled(1);}
        else{if(!words80_[1])set_disabled(0);words80_[1]=static_cast<std::uint32_t>(count);
            const float step=static_cast<float>(100.L/(count-1));std::memcpy(&words80_[0],&step,4);
            if(count<=index())words80_[2]=static_cast<std::uint32_t>(count-1);}
        float step;std::memcpy(&step,&words80_[0],4);set_percentage(static_cast<float>(static_cast<long double>(index())*step));
    }
    std::int32_t index()const{std::int32_t value;std::memcpy(&value,&words80_[2],4);return value;} //004AE570.
    bool leave(){ //004AE640; button004ADBE0/slider004AE250.
        if(words80_[3]==1||words80_[3]==2){auto& b=words80_[3]==1?up_:down_;button_state(b,!b.disabled0c,false,b.disabled0c!=0);}
        else if(words80_[3]==3)slider_state(true,false);
        words80_[3]=0xffffffffu;return true;
    }
    bool enter(std::uint32_t tag){ //004AE5B0/004AE2D0/004AE620, AL is the only original result consumed.
        if(!leave())return false;words80_[3]=tag&15u;
        if(words80_[3]==1||words80_[3]==2){auto& b=words80_[3]==1?up_:down_;if(b.disabled0c)return false;button_state(b,false,true,false);return true;}
        if(words80_[3]==3){if(!slider_.disabled0c)slider_state(false,true);return true;}return false;
    }
    bool hover(std::uint32_t tag){ //004AE6A0.
        const auto part=tag&15u;if(part<1||part>3)return leave();
        if(words80_[3]!=part&&leave())return enter(tag);return true;
    }
    template<class Changed>bool click(Changed&& changed){ //004AE6F0→004ADC10→004AE3B0.
        if(disabled0c_)return true;if(words80_[3]!=1&&words80_[3]!=2)return false;
        auto& button=words80_[3]==1?up_:down_;if(button.disabled0c)return false;
        if(words80_[3]==1){if(index()<=0)return true;--words80_[2];if(index()<0)words80_[2]=0;}
        else {const auto count=signed_word(words80_[1]);if(index()>=static_cast<std::int64_t>(count)-1)return true;++words80_[2];if(count<index())words80_[2]=words80_[1];}
        set_percentage(static_cast<float>(static_cast<long double>(index())*step()));changed(tag_,words80_[2]);return true;
    }
    template<class Changed>bool drag(const std::array<float,4>& position,const std::array<float,4>& delta,Changed&& changed){ //004AE730→004AE260→004ADF40.
        if(disabled0c_)return true;if(words80_[3]!=3)return false;
        if(move_slider(position,delta))publish_percentage(slider_.percentage28,changed);return true;
    }
    bool release(){ //004AE760 restores the actual index position, not an input action consumer.
        if(!disabled0c_&&words80_[3]==3)set_percentage(static_cast<float>(static_cast<long double>(index())*step()));return true;
    }
    void close_geometry()noexcept{slider_.geometry.reset();} //004AE2B0: no frame release or fabricated frame owner.
    void discard_borrowers()noexcept{root_=Frame{};callback_=nullptr;up_.root=up_.normal=up_.active=up_.inactive=Frame{};up_.callback=nullptr;
        down_.root=down_.normal=down_.active=down_.inactive=Frame{};down_.callback=nullptr;slider_.root=slider_.normal=slider_.active=slider_.base=Frame{};slider_.callback=nullptr;}
    const auto& up()const{return up_;}const auto& down()const{return down_;}const auto& slider()const{return slider_;}
    const auto& words80()const{return words80_;}Frame root()const{return root_;}void* callback()const{return callback_;}
private:
    static std::int32_t signed_word(std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;}
    float step()const{float value;std::memcpy(&value,&words80_[0],4);return value;}
    bool move_slider(const std::array<float,4>& position,const std::array<float,4>& delta){ //004ADF40/004AE0B0; reads WORLD position even for zero movement.
        require(slider_.root);const auto world=frames_.actor_world_matrix(slider_.root);std::array<float,4> value{{world[12],world[13],world[14],world[15]}};
        const unsigned axis=slider_.vertical25?1u:0u;if((!slider_.vertical25&&!slider_.horizontal24)||delta[axis]==0)return false;
        require(slider_.normal);const auto local=frames_.actor_position(slider_.normal);auto& g=geometry();
        value[axis]=static_cast<float>(static_cast<long double>(position[axis])-local[slider_.vertical25?2u:0u]);
        const auto low=slider_.vertical25?g[1]:g[0],high=slider_.vertical25?g[3]:g[2];if(value[axis]<low)value[axis]=low;if(high<value[axis])value[axis]=high;
        frames_.hud_scroll_position(slider_.root,value);frames_.update_scene_object(slider_.root);
        const auto percent=(slider_.vertical25?(static_cast<long double>(g[3])-value[1])/g[5]:(static_cast<long double>(value[0])-g[0])/g[4])*100.f;
        if(percent==slider_.percentage28)return false;slider_.percentage28=static_cast<float>(percent);return true;
    }
    template<class Changed>void publish_percentage(float percentage,Changed& changed){ //004AE470: floor(DOUBLE(extended division+.5)), then unsigned cap.
        const auto rounded=std::floor(static_cast<double>(static_cast<long double>(percentage)/step()+.5));
        auto value=static_cast<std::uint32_t>(text_ftol(rounded));const auto last=words80_[1]-1u;if(last<value)value=last;
        if(words80_[2]!=value){words80_[2]=value;changed(tag_,value);}
    }
    static void require(Frame frame){if(!frame)throw std::runtime_error("Required HUD scrollbar artwork is missing");}
    std::array<float,7>& geometry(){if(!slider_.geometry)throw std::logic_error("HUD slider geometry is not allocated");return *slider_.geometry;}
    void copy_position(Frame destination,Frame source){const auto p=frames_.actor_position(source);frames_.hud_message_position(destination,p[0],p[1],p[2]);}
    void button_state(Button& button,bool normal,bool active,bool inactive) { //004ADA20 RET12, all visibility calls THEN all updates.
        require(button.normal);frames_.object_visible(button.normal,normal);require(button.active);frames_.object_visible(button.active,active);
        require(button.inactive);frames_.object_visible(button.inactive,inactive);
        for(auto frame:{button.normal,button.active,button.inactive})frames_.update_scene_object(frame);
    }
    void slider_state(bool normal,bool active) { //004ADC50 RET8.
        require(slider_.normal);frames_.object_visible(slider_.normal,normal);require(slider_.active);frames_.object_visible(slider_.active,active);
        frames_.update_scene_object(slider_.normal);frames_.update_scene_object(slider_.active);
    }
    Frames& frames_;Frame root_{};void* callback_=nullptr;std::uint32_t tag_=0;std::uint8_t disabled0c_=0;
    Button up_,down_;Slider slider_;std::array<std::uint32_t,5> words80_{};
};
} //namespace hd2::game
#endif
