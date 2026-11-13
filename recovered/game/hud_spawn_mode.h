#ifndef HD2_GAME_HUD_SPAWN_MODE_H
#define HD2_GAME_HUD_SPAWN_MODE_H
#include <cstdint>
#include <stdexcept>
namespace hd2::game {
//0049F250: flags are published before any required panel call. Borrow each
//actual inline component again after callbacks; notably objectives is read twice.
template<class Frames,class Hud,class Panels,class Mode,class Pause>
void apply_hud_spawn_mode(Frames& frames,Hud& hud,Panels& panels,std::uint32_t phase,Mode&& mode,Pause&& pause){
    const auto show=[&](auto frame,bool visible){
        if(!frame)throw std::logic_error("HUD spawn mode requires its actual panel frame");
        frames.object_visible(frame,visible);
    };
    const auto objective=[&](bool visible){if(const auto root=panels.objectives().root())show(root,visible);};
    const auto scoreboard=[&]{const auto root=panels.scoreboard().root();show(root,panels.scoreboard().style()!=-1);};
    if(phase==2){
        hud.flags=(hud.flags&~0x80u)|0x100u;
        panels.inventory().hide();show(panels.artwork().speed()[0],false);
        panels.cursor().set_kind(6);
        panels.dialogs().hide_signals([&]{panels.clear_roster_selection();});
        panels.dialogs().hide_roll();panels.messages().hide(mode,pause);
        show(panels.hands().root(),false);show(panels.timers().root(),false);
        show(panels.small().status().frames[0],false);
        panels.map().hide(panels.auxiliary());
        panels.auxiliary().hide_selected_pointer(); //Native calls004ABAF0 again, even with no map center.
        scoreboard();objective(false);objective(false);
    }else if(phase==1){
        hud.flags=(hud.flags&~0x100u)|0x80u;
        show(panels.small().status().frames[0],false);show(panels.hands().root(),false);
        objective(true);show(panels.timers().root(),false);
    }else if(phase==0){
        hud.flags&=~0x180u;
        show(panels.hands().root(),true);show(panels.small().status().frames[0],true);
        scoreboard();objective(true);show(panels.timers().root(),true);
    }
    panels.post_bind(mode(),hud.flags);
}
} //namespace hd2::game
#endif
