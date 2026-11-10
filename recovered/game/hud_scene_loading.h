#ifndef HD2_GAME_HUD_SCENE_LOADING_H
#define HD2_GAME_HUD_SCENE_LOADING_H
#include "game/hud_text_loading.h"
#include "game/hud_render.h"
#include "frontend/cleanup.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
namespace hd2::game {
//0089BB7C: ONE process click-sound owner shared across HUD lifetimes. Signal/
//compass/style owners are passed separately to the SAME panel composition.
template<class Owner>struct HudClickSound {std::optional<Owner> owner;};
//0049A940/0049B5E0/0049B1B0 source. Actual scene13/cache/model/camera/light/
//click-sound ownership, SAME77 inline TextStates and concrete HudPanelsLoading.
//No world-load/readiness or rendering success can be inferred from this owner.
template<class Frames,class Factory,class Hud,class Panels>
class HudSceneLoading {
public:
    using Frame=typename Frames::Frame;using Text=typename Factory::DisplayText;
    using Owner=decltype(std::declval<Frames&>().create_hud_info_model());
    HudSceneLoading(Frames& frames,Factory& factory,Hud& hud,Panels& panels,HudClickSound<Owner>& click)
        :frames_(frames),factory_(factory),hud_(hud),panels_(panels),click_(click){}
    ~HudSceneLoading(){try{retire();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    HudSceneLoading(const HudSceneLoading&)=delete;HudSceneLoading& operator=(const HudSceneLoading&)=delete;
    bool bind(const std::string& mission,std::uint32_t mode,bool editor_kind3,const typename Panels::Borrowers& borrowers) {
        if(hud_.flags&1u)return true; //Actual idempotent guard before any allocation.
        if(scene_||model_||camera_)throw std::logic_error("HUD loading prefix was not retired");
        if(click_.owner)throw std::logic_error("Another HUD owns the shared click sound");
        try {
            scene_.emplace(frames_.create_hud_scene());if(!scene_->get()){scene_.reset();return false;}
            const auto scene=scene_->get();frames_.hud_fit_viewport(scene,factory_.hud_display_width(),factory_.hud_display_height());
            model_.emplace(frames_.create_hud_info_model());if(!model_->get())throw std::runtime_error("HUD artwork model factory returned null");
            const auto first=std::uint32_t(frames_.hud_driver_state(0x15)!=0),second=std::uint32_t(frames_.hud_driver_state(0x19)!=0);
            frames_.hud_driver_state(0x15,0);frames_.hud_driver_state(0x19,1);
            std::int32_t status;
            try{status=frames_.load_hud_info_model(model_->get(),"gameinterface",0);}
            catch(...){frames_.hud_driver_state(0x15,first);frames_.hud_driver_state(0x19,second);throw;}
            frames_.hud_driver_state(0x15,first);frames_.hud_driver_state(0x19,second);
            if(status<0){retire();return false;} //No successful required-artwork fallback.
            frames_.hud_inventory_link_root(model_->get(),scene);frames_.hud_position(model_->get(),{{0,0,0}});frames_.hud_inventory_add(scene,model_->get());
            camera_.emplace(frames_.create_game_camera());if(!camera_->get())throw std::runtime_error("HUD camera factory returned null");
            frames_.hud_active_camera(scene,camera_->get());frames_.hud_camera_direction(camera_->get());
            frames_.hud_message_position(camera_->get(),.5f,-.375f,-15.5f);frames_.game_camera_range(camera_->get(),.2f,500.f);
            frames_.hud_camera88(camera_->get(),1);frames_.hud_camera84(camera_->get(),2.f);frames_.update_scene_object(camera_->get());
            {
                auto light=frames_.create_hud_light();if(!light)throw std::runtime_error("HUD light factory returned null");
                frames_.human_shadow_name(light.get(),"GI_Light");frames_.light_type(light.get(),4);
                frames_.hud_light_colour(light.get(),{{1,1,1,0}});frames_.hud_light_power(light.get(),1.f);frames_.light_flags(light.get(),0x11);
                frames_.hud_inventory_add(scene,light.get());frames_.hud_root_add_light(scene,light.get());
            } //Scene retains the light before its local release.
            click_.owner.emplace(frames_.create_human_sound());if(!click_.owner->get())throw std::runtime_error("HUD click sound factory returned null");
            click_owned_=true;const auto sound=click_.owner->get();
            if(frames_.human_sound_available())(void)frames_.load_human_sound(sound,"button.wav"); //Original ignores sound cache status.
            frames_.human_sound_parameter7c(sound,3);frames_.object_visible(sound,false);frames_.human_sound_volume(sound,1.f);
            frames_.human_shadow_name(sound,"sound_click");frames_.hud_inventory_link_root(sound,scene);
            panels_started_=true;panels_.bind(scene,mission,mode,editor_kind3,borrowers);
            down_panel_=frames_.hud_find(scene,"down_panel",0xffff);fields6b8f8_.fill(nullptr);
            hud_.flags=9;fields10_.fill(0);flags20_.fill(0);
            const auto border=frames_.hud_find(scene,"Border",0xffff);
            bind_hud_text_geometry(frames_,debug_text(),scene,border);
            debug_text().text.append(std::make_shared<const std::string>("Camera pos: 0,0,0"),0x4e);
            panels_.post_bind(mode,hud_.flags);return true;
        }catch(...){frontend::CleanupErrors errors(std::current_exception());errors.attempt([&]{retire();});errors.rethrow();}
        return false;
    }
    bool close(){if(!hud_.flags)return false;retire();return true;} //Native0049B1B0 zero-flags gate; destructor also reclaims failed prefix.
    template<class Human>bool add_human(Human* human,std::int32_t index,std::uint32_t mode,std::uint32_t difficulty,std::int32_t local_player){
        return panels_.add_human(human,index,mode,difficulty,local_player);
    }
    template<class Human>bool remove_human(Human* human,std::int32_t index){return panels_.remove_human(human,index);}
    template<class Human>void select_human(Human* human,bool selected,std::uint32_t mode,std::uint32_t difficulty){panels_.select_human(human,selected,mode,difficulty);}
    void inventory_changed(){panels_.inventory_changed();}
    template<class Direction>void hud_sound_direction(const Direction& direction,std::uint32_t kind){panels_.hud_sound_direction(direction,kind);} //0049DE80 has no flags gate.
    void append_message(const char* text,const std::array<float,3>& colour){if(hud_.flags)panels_.messages().append(text,colour);} //0049D2E0→00494A40, any nonzero flags.
    template<class Camera>void render_prepass(Camera& camera){hud_render_prepass(panels_,camera);}
    template<class Camera,class Consumers>void render(Camera& camera,std::uint32_t mode,Consumers& consumers){
        hud_render(frames_,factory_,hud_,scene(),panels_,camera,mode,flags20_,fields6b8f8_,consumers);
    }
    auto& selection_actors(){return fields6b8f8_;} //SAME original6B8F8/FC borrowed actor pair.
    Frame scene()const{return scene_?scene_->get():Frame{};}Frame camera()const{return camera_?camera_->get():Frame{};}
    Frame model()const{return model_?model_->get():Frame{};}Frame down_panel()const{return down_panel_;}
private:
    Text& debug_text(){for(const auto& owner:hud_.text_owners())if(owner.original_offset==0x6b85c)return *owner.text;throw std::logic_error("HUD lost SAME inline debug text");}
    void retire(){
        if(!scene_&&!model_&&!camera_&&!panels_started_&&!click_owned_)return;
        frontend::CleanupErrors errors;
        if(hud_.flags){errors.attempt([&]{panels_.hide_cold_soldiers();});hud_.flags&=0xfffffff9u;}
        fields6b8f8_.fill(nullptr);debug_text().text.clear();fields6b8f8_.fill(nullptr); //0049B610 writes the SAME pair again.
        if(panels_started_){errors.attempt([&]{panels_.close();});panels_started_=false;}
        if(click_owned_){click_.owner.reset();click_owned_=false;}
        //On null-factory rejection there may be an empty optional published.
        if(click_.owner&&!click_.owner->get())click_.owner.reset();
        camera_.reset();model_.reset();
        if(scene_&&scene_->get())errors.attempt([&]{frames_.hud_active_camera(scene_->get(),Frame{});});
        panels_.discard_borrowers();scene_.reset();hud_.flags=0;down_panel_=Frame{};errors.rethrow();
    }
    Frames& frames_;Factory& factory_;Hud& hud_;Panels& panels_;HudClickSound<Owner>& click_;
    std::optional<Owner> scene_,model_,camera_;Frame down_panel_{};
    std::array<typename Panels::Actor*,2> fields6b8f8_{};std::array<std::uint32_t,3> fields10_{};std::array<std::uint8_t,2> flags20_{};
    bool panels_started_=false,click_owned_=false;
};
} //namespace hd2::game
#endif
