#ifndef HD2_GAME_HUD_SCENE_LOADING_H
#define HD2_GAME_HUD_SCENE_LOADING_H
#include "game/hud_actor_retirement.h"
#include "game/hud_target_selection.h"
#include "game/hud_text_loading.h"
#include "game/hud_render.h"
#include "game/hud_frame.h"
#include "frontend/cleanup.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
namespace hd2::game {
//0089BB7C: ONE process click-sound owner shared across HUD lifetimes. Signal/
//compass/style owners are passed separately to the SAME panel composition.
template<class Owner>struct HudClickSound {std::optional<Owner> owner;std::uint32_t soldier_click_ticks=0;}; //Adjacent process0089BB80 persists across HUD lifetimes.
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
    bool bind(const std::string& mission,std::uint32_t mode,bool editor_kind3) {
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
            panels_started_=true;panels_.bind(scene,mission,mode,editor_kind3);
            down_panel_=frames_.hud_find(scene,"down_panel",0xffff);fields6b8f8_.fill(nullptr);
            hud_.flags=9;fields10_.fill(0);flags20_.fill(0);
            const auto border=frames_.hud_find(scene,"Border",0xffff);
            bind_hud_text_geometry(frames_,debug_text(),scene,border);
            debug_text().text.append(std::make_shared<const std::string>("Camera pos: 0,0,0"),0x4e);
            panels_.post_bind(mode,hud_.flags);return true;
        }catch(...){frontend::CleanupErrors errors(std::current_exception());errors.attempt([&]{retire();});errors.rethrow();}
        return false;
    }
    //0049BD50 requires the concrete UI producers even though zero/warming gates
    //may return before using them. No positive rejection or no-op frame receiver.
    template<class Queries>void update_loading(std::uint32_t elapsed,Queries& queries){advance_hud_frame(frames_,hud_,panels_,elapsed,queries);}
    bool close(){if(!hud_.flags)return false;retire();return true;} //Native0049B1B0 zero-flags gate; destructor also reclaims failed prefix.
    template<class Human>bool add_human(Human* human,std::int32_t index,std::uint32_t mode,std::uint32_t difficulty,std::int32_t local_player){
        return panels_.add_human(human,index,mode,difficulty,local_player);
    }
    template<class Human>bool remove_human(Human* human,std::int32_t index){return panels_.remove_human(human,index);}
    template<class Human>void select_human(Human* human,bool selected,std::uint32_t mode,std::uint32_t difficulty){panels_.select_human(human,selected,mode,difficulty);}
    template<class Human>void retire_dead_human_marker(Human* human){panels_.compass().remove_human(human);} //0049B960, no flags gate.
    void inventory_changed(){panels_.inventory_changed();}
    template<class Mode,class Pause>void spawn_mode(std::uint32_t phase,Mode&& mode,Pause&& pause){panels_.spawn_mode(phase,mode,pause);}
    void hide_map(){flags20_[1]=static_cast<std::uint8_t>(panels_.hide_map());} //0049F040(false), after map receiver returns.
    template<class Mode,class Pause>void show_map(Mode&& mode,Pause&& pause){ //0049F040(true): map then byte21 then message mode0.
        flags20_[1]=static_cast<std::uint8_t>(panels_.show_map());panels_.messages().hide(mode,pause);
    }
    template<class Random>void show_human_damage(Random& random){panels_.show_human_damage(random);}
    void set_hint(bool enabled,std::uint32_t identity){panels_.set_hint(enabled,identity);} //SAME inline small-panel owner.
    void select_objective(unsigned index,std::int32_t side){if(hud_.flags)panels_.select_objective(index,side);} //0049F0C0: any nonzero flags.
    void set_hint(std::int32_t identity){set_hint(identity>=0,static_cast<std::uint32_t>(identity));} //0049DE30, signed ID gate; no flags gate.
    void set_injury_message(std::uint32_t identity,std::uint32_t time){panels_.set_injury_message(identity,time);} //0049DE50, no HUD-flags gate.
    template<class Direction>void hud_sound_direction(const Direction& direction,std::uint32_t kind){panels_.hud_sound_direction(direction,kind);} //0049DE80 has no flags gate.
    void append_message(const char* text,const std::array<float,3>& colour){if(hud_.flags)panels_.messages().append(text,colour);} //0049D2E0→00494A40, any nonzero flags.
    template<class Camera>void render_prepass(Camera& camera){hud_render_prepass(panels_,camera);}
    template<class Camera,class Consumers>void render(Camera& camera,std::uint32_t mode,Consumers& consumers){
        hud_render(frames_,factory_,hud_,scene(),panels_,camera,mode,flags20_,fields6b8f8_,consumers);
    }
    auto& selection_actors(){return fields6b8f8_;} //SAME original6B8F8/FC borrowed actor pair.
    auto& saved_command_flags(){return fields10_[2];} //Actual HUD18, distinct from inventory-relative18.
    auto& control_latches(){return flags20_;}auto& soldier_click_ticks(){return click_.soldier_click_ticks;}
    void refresh_layout(std::uint32_t mode){ //0049E1B0, descending six soldiers then hands/signals/ordinary roll/current-Human refresh.
        if(!down_panel_)throw std::logic_error("HUD layout requires actual down_panel");const auto flags=hud_.flags;
        const auto real=[](std::uint32_t bits){float value;std::memcpy(&value,&bits,4);return value;};
        if(flags&0x38u)frames_.hud_scroll_position(down_panel_,{{real(0x3ee71fbc),real(flags&0x10u?0xbfa91687u:flags&0x20u?0xbfb00000u:0xbfa5158cu),0,0}});
        frames_.update_scene_object(down_panel_);refresh_panel_geometry();panels_.post_bind(mode,hud_.flags);
    }
    void cycle_layout(std::uint32_t mode,unsigned game_mode){ //0049DFE0, position BEFORE flags, native8→10→20→8 priority.
        if(!down_panel_)throw std::logic_error("HUD layout cycle requires actual down_panel");
        const auto real=[](std::uint32_t bits){float value;std::memcpy(&value,&bits,4);return value;};bool show=false,changed=false;
        if(hud_.flags&8u){frames_.hud_scroll_position(down_panel_,{{real(0x3ee71fbc),real(0xbfa91687),0,0}});hud_.flags=(hud_.flags&~8u)|0x10u;changed=true;}
        else if(hud_.flags&0x10u){frames_.hud_scroll_position(down_panel_,{{real(0x3ee71fbc),real(0xbfb00000),0,0}});hud_.flags=(hud_.flags&~0x10u)|0x20u;}
        else if(hud_.flags&0x20u){frames_.hud_scroll_position(down_panel_,{{real(0x3ee71fbc),real(0xbfa5158c),0,0}});hud_.flags=(hud_.flags&~0x20u)|8u;show=changed=true;}
        if(changed){if((mode&7u)==7u){if(game_mode==3){const auto root=panels_.small().carnage().visual;if(!root)throw std::logic_error("HUD layout cycle lost carnage artwork");frames_.object_visible(root,show);}}
            else {const auto root=panels_.scoreboard().root();if(!root)throw std::logic_error("HUD layout cycle lost scoreboard artwork");frames_.object_visible(root,show&&panels_.scoreboard().style()!=-1);}}
        frames_.update_scene_object(down_panel_);refresh_panel_geometry();panels_.small().refresh_status_geometry();panels_.post_bind(mode,hud_.flags);
    }
    template<class Queries>void update_selection(std::uint32_t elapsed,Queries& queries){advance_hud_target_selection(fields10_[0],fields10_[1],fields6b8f8_,elapsed,queries);}
    const void* peer_human(){return hud_.flags?panels_.soldiers()[0].human_borrower():nullptr;} //0049F0E0: HUD10394, NOT6B8F8/FC.
    template<class Human>void refresh_peer_human(Human* human,std::uint32_t mode,std::int32_t local){panels_.refresh_peer_human(human,mode,local);} //0049F100, no flags gate.
    template<class Actor>void remove_actor_references(std::uint32_t mode,unsigned kind,Actor* actor) { //0049EF70.
        if(kind==1&&actor)panels_.auxiliary().retire_order_actor(actor); //Actual order-info Human loan retracts before its B0/order list can retire.
        if(kind==1&&actor)panels_.selector().retire_actor(actor); //Selector controls never borrow the retiring Human/Items on their next input call.
        if(kind==1&&actor)panels_.inventory().retire_actor(actor); //Defined source lifetime guard before Actor/Items retirement.
        if(kind==1&&actor)panels_.map().retire_actor(actor); //Map zone records must not retain the retiring ActorBase.
        if(kind==1&&actor)panels_.crosshair().retire_target(actor); //Actual caller supplies &Actor::base(); retract target identity before actor lifetime ends.
        retire_hud_actor(hud_,fields6b8f8_,panels_.dialogs(),panels_.command(),mode,kind,actor);
    }
    Frame scene()const{return scene_?scene_->get():Frame{};}Frame camera()const{return camera_?camera_->get():Frame{};}
    Frame model()const{return model_?model_->get():Frame{};}Frame down_panel()const{return down_panel_;}
    auto& command_owner(){return panels_.command();} //Borrow SAME actual command pool for0058C5A0/0058D6C0.
private:
    void refresh_panel_geometry(){for(unsigned n=6;n;)panels_.soldiers()[--n].refresh_geometry();panels_.hands().refresh_geometry();panels_.dialogs().refresh_signal_geometry();panels_.dialogs().refresh_roll_geometry();}
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
