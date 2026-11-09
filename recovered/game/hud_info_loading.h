#ifndef HD2_GAME_HUD_INFO_LOADING_H
#define HD2_GAME_HUD_INFO_LOADING_H
#include "game/hud_info_style_data.h"
#include "game/hud_text_loading.h"
#include <memory>
#include <stdexcept>
#include <vector>
namespace hd2::game {
//00496450/00496DC0: category1 cached ingame_info model, scene-owned after
//publication;73 style handles in the SAME process catalogue as opening movies.
//00496E40(false) only hides the root. Its true/population path is not exposed.
template<class Frames,class Factory,class Styles>
class HudInfoLoading {
public:
    using Frame=typename Frames::Frame;using Text=typename Factory::DisplayText;
    HudInfoLoading(Frames& frames,Factory& factory,Styles& styles):frames_(frames),factory_(factory),styles_(styles){ids_.fill(-1);}
    ~HudInfoLoading(){close();}
    HudInfoLoading(const HudInfoLoading&)=delete;
    HudInfoLoading& operator=(const HudInfoLoading&)=delete;
    template<class Configuration>void bind(Frame scene,std::uint32_t mode,Configuration& configuration) {
        if(bound_)throw std::logic_error("HUD info rebound before borrower retirement");
        require(scene);bound_=true;records_.clear();word140_=0;
        const auto first=std::uint32_t(frames_.hud_driver_state(0x15)!=0);
        const auto second=std::uint32_t(frames_.hud_driver_state(0x19)!=0);
        frames_.hud_driver_state(0x15,0);frames_.hud_driver_state(0x19,0);
        //Create inside the protected range: native C++ allocation/factory
        //failure must not leave render states disabled for the containing HUD.
        auto model=[&] {
          try {
            auto owner=frames_.create_hud_info_model();
            const auto frame=owner.get();
            if(!owner)throw std::runtime_error("HUD info model factory returned null");
            const auto status=frames_.load_hud_info_model(frame,"ingame_info",0);
            //Original continues after a cache error and faults on missing root;
            //native guard refuses to publish a successful incomplete panel.
            if(status!=0)throw std::runtime_error("Required HUD info model failed to load");
            frames_.human_shadow_name(frame,"ingame_info");
            frames_.hud_inventory_link_root(frame,scene);
            frames_.hud_inventory_add(scene,frame);
            root_=frames_.actor_find(frame,"game_info",1);require(root_);
            frames_.hud_position(root_,{{.5f,-.375f,-.5f}});
            return owner;
          } catch(...) {
            frames_.hud_driver_state(0x15,first);frames_.hud_driver_state(0x19,second);throw;
          }
        }();
        frames_.hud_driver_state(0x15,first);frames_.hud_driver_state(0x19,second);
        if((mode&7u)==7u)kind_=0;
        else {const auto style=configuration.hud_style224();if(style>=0&&style<=3)kind_=style+1;}
        frames_.hud_info_scale(root_,kind_==1||kind_==4?.7f:1.f,kind_==4?.7f:1.f,1.f);
        frames_.update_scene_object(root_);
        for(const auto* name:{"header","text"}) {
            const auto frame=frames_.actor_find(model.get(),name,1);
            if(frame)frames_.object_visible(frame,false);
        }
        text_=factory_.make_hud_text();if(!text_)throw std::runtime_error("HUD info text factory returned null");
        bind_hud_text_geometry(frames_,*text_,scene,root_); //Default inset10.
        for(std::size_t n=0;n!=ids_.size();++n)ids_[n]=styles_.add(hud_info_styles[n]);
        frames_.object_visible(root_,false);
        //Local model releases AFTER all73 styles/hide; actual scene owns it.
    }
    void close()noexcept {
        for(auto& id:ids_){if(id>=0)styles_.remove(id);id=-1;}
        text_.reset(); //00496DC0 deletes text AFTER unregistering every style.
    }
    void draw(){ //00496E00 changes Graph74/78 state4, not I3D driver state4.
        const auto previous=factory_.hud_graphics_state(4);factory_.hud_graphics_state(4,4);
        try{if(text_)factory_.hud_draw_text(*text_);}
        catch(...){factory_.hud_graphics_state(4,previous);throw;}
        factory_.hud_graphics_state(4,previous);
    }
    void discard_borrowers()noexcept{root_=Frame{};records_.clear();bound_=false;}
    Frame root()const{return root_;}const auto& text()const{return text_;}
    const auto& style_ids()const{return ids_;}std::int32_t kind()const{return kind_;}
    std::uint32_t word140()const{return word140_;}
private:
    static void require(Frame frame){if(!frame)throw std::runtime_error("Required HUD info artwork is missing");}
    Frames& frames_;Factory& factory_;Styles& styles_;
    Frame root_{};std::vector<Frame> records_;std::unique_ptr<Text> text_;
    std::array<std::int32_t,73> ids_;std::int32_t kind_=0;std::uint32_t word140_=0;bool bound_=false;
};
} //namespace hd2::game
#endif
