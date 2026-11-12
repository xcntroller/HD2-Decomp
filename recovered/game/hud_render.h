#ifndef HD2_GAME_HUD_RENDER_H
#define HD2_GAME_HUD_RENDER_H
#include <array>
#include <cstdint>
#include <stdexcept>
#include "game/hud_selection_drawing.h"
namespace hd2::game {
//0049B9B0. Caller's actual00698500/00464E90 camera owner is mandatory.
//Consumers must provide real command drawing bodies; no default service exists.
template<class Panels,class Camera>void hud_render_prepass(Panels& panels,Camera& camera){
    if(camera.hud_suppressed())return;
    if(panels.command().visible())panels.command().draw_prepass(); //004B7780 same route owner.
}
//0049B9F0 control flow. Panels are the SAME HudPanelsLoading, camera is the
//actual current mission camera, mode is actual982100, flags20 are SAME HUD20/21.
//The command-before consumer is supplied by the concrete World UI adapter.
//This source orchestration cannot activate a world by itself.
template<class Frames,class Factory,class Hud,class Panels,class Camera,class Actors,class Consumers>
void hud_render(Frames& frames,Factory& factory,Hud& hud,typename Frames::Frame scene,Panels& panels,
                Camera& camera,std::uint32_t mode,const std::array<std::uint8_t,2>& flags20,const Actors& selection_actors,Consumers& consumers){
    if(camera.hud_suppressed())return;
    if(factory.hud_no_gi()!=0)return;
    const auto visible=[&](typename Frames::Frame frame){if(!frame)throw std::runtime_error("Required HUD render artwork is missing");return frames.hud_frame_visible(frame);};
    const auto draw_map=[&]{if(auto map=panels.map().center())if(visible(map))panels.map().draw();};
    const auto draw_info=[&]{if(visible(panels.info().root()))panels.info().draw();};
    if(!(hud.flags&4u)){
        if((mode&7u)!=7u){
            if(flags20[1]||flags20[0]||visible(panels.small().hint().visual)){
                if(!scene)throw std::runtime_error("Initial HUD draw requires its actual scene");
                frames.hud_render_scene(scene);draw_map();draw_info();
                if(visible(panels.small().hint().visual))panels.small().draw_hint();
            }
            panels.messages().draw();
        }
        return;
    }
    draw_hud_selection(frames,factory,selection_actors,panels.dialogs(),panels.command()); //0049E260.
    if(panels.small().server_message().enabled)panels.small().draw_server_message();
    if(panels.small().injury_message().enabled)panels.small().draw_injury_message();
    if((mode&7u)==7u&&panels.command().visible()){
        consumers.command_before_scene(); //004B6720.
        if(scene)frames.hud_render_scene(scene);
        panels.command().draw_after_scene(); //004B7830.
    }else if(scene)frames.hud_render_scene(scene);
    panels.crosshair().draw();
    if(panels.dialogs().signals_active())panels.dialogs().draw_signals(); //004C05F0/004C0880.
    if(panels.dialogs().roll().flag30)panels.dialogs().draw_roll();
    if(!visible(panels.inventory().root()))panels.messages().draw();
    if(hud.flags&0x18u){
        if(visible(panels.hands().root()))panels.hands().draw();
        for(unsigned n=6;n;){auto& soldier=panels.soldiers()[--n];if(visible(soldier.artwork().root))soldier.draw();}
    }
    if(visible(panels.timers().root()))panels.timers().draw();
    if(visible(panels.small().carnage().visual))panels.small().draw_carnage();
    if(visible(panels.inventory().root()))panels.inventory().draw();
    draw_map();draw_info();
    if(visible(panels.selector().root()))panels.selector().draw(); //004AD140 same populated record owner.
    if(visible(panels.small().status().frames[0]))panels.small().draw_status();
    if(visible(panels.auxiliary().progress().background))panels.auxiliary().draw_progress();
    if(auto root=panels.scoreboard().root())if(panels.scoreboard().style()!=-1&&visible(root))panels.scoreboard().draw(); //Callee rereads visibility.
    if(auto root=panels.small().voting().visual)if(visible(root))panels.small().draw_voting(); //Callee rereads visibility.
    if(flags20[1]||flags20[0]){draw_map();draw_info();} //Original deliberately repeats these draws.
    if(visible(panels.small().hint().visual))panels.small().draw_hint();
}
} //namespace hd2::game
#endif
