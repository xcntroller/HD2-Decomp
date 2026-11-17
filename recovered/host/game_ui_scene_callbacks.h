#ifndef HD2_HOST_GAME_UI_SCENE_CALLBACKS_H
#define HD2_HOST_GAME_UI_SCENE_CALLBACKS_H
#include "game/scene_callback.h"
#include "host/game_scene.h"
#include "host/game_ui_render_binding.h"
namespace hd2::host {
//The existing callback adapter is instantiated with actual GameUiPresentation.
//Its type parameter also permits native boundary audits without a Windows World.
template<class Ui>class GameUiSceneCallbackBinding final:public GameSceneCallbacks {
public:
    GameUiSceneCallbackBinding(Ui& ui,GameUiRenderStages& render)
        :ui_(ui){ui_.bind_render_stages(this,render);}
    ~GameUiSceneCallbackBinding()override{retire();}
    GameUiSceneCallbackBinding(const GameUiSceneCallbackBinding&)=delete;
    GameUiSceneCallbackBinding& operator=(const GameUiSceneCallbackBinding&)=delete;
    void retire()noexcept{ui_.unbind_render_stages(this);}
    std::uint32_t dispatch_game_scene(std::uint32_t stage)override{
        (void)ui_.render_stages(this);return game::scene_callback(stage,*this);
    }
    bool has_game()const{return ui_.has_game();}auto game_status()const{return ui_.game_status();}
    auto game_word24()const{return ui_.game_word24();}
    bool suppress_hud()const{return ui_.suppress_hud();}bool full_viewport()const{return ui_.full_viewport();}
    bool debug_enabled()const{return ui_.debug_enabled();}bool hud_enabled(){return ui_.hud_enabled();}
    void before_hud(){ui_.before_hud();}void render_hud(){ui_.render_hud();}
    auto graphics_viewport(){return ui_.graphics_viewport();}template<class Viewport>void graphics_viewport(const Viewport& value){ui_.graphics_viewport(value);}
    auto graphics_width(){return ui_.graphics_width();}auto graphics_height(){return ui_.graphics_height();}
    void render_mission_text(){ui_.render_mission_text();}void render_game_fade(){ui_.render_game_fade();}
    void render_effects_overlay(){ui_.render_effects_overlay();}
    void control_before_scene(){ui_.render_stages(this).render_before_scene();}
    void control_after_scene(){ui_.control_after_scene();}void render_debug(){ui_.render_stages(this).render_debug_geometry();}
    void render_network_overlay()noexcept {} //00724CB0 is an actual RET in the frozen client.
private:
    Ui& ui_;
};
} //namespace hd2::host
#endif
