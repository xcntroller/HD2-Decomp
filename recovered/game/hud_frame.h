#ifndef HD2_GAME_HUD_FRAME_H
#define HD2_GAME_HUD_FRAME_H
#include <cstdint>
#include <stdexcept>
namespace hd2::game {
//0049BD50, over the SAME loaded HUD/panels. Queries bind completed UI receivers
//and required Human/network/console producers, without scheduling a World frame.
template<class Frames,class Hud,class Panels,class Queries>
void advance_hud_frame(Frames& frames,Hud& hud,Panels& panels,std::uint32_t elapsed,Queries& queries){
    if(!hud.flags)return;
    if(!(hud.flags&4u)){queries.messages(elapsed);panels.crosshair().show(0);return;}
    const auto visible=[&](typename Frames::Frame frame){if(!frame)throw std::logic_error("HUD frame requires actual loaded artwork");return frames.hud_frame_visible(frame);};
    if(panels.small().injury_message().enabled)panels.small().update_injury_message(elapsed);
    panels.small().server_message_visible(queries.paused()&&!panels.command().visible()); //6B6BC, not HUD20/21.
    auto* const human=queries.current_human(); //Original cached EDI persists through all later callbacks.
    if(human&&queries.inventory_changed(*human))panels.post_bind(queries.mode(),hud.flags);
    if(visible(panels.injury_movies().visual()))panels.injury_movies().update(elapsed);
    panels.auxiliary().update_pointer(elapsed);
    if(visible(panels.info().root()))queries.info(elapsed);
    if(auto map=panels.map().map())if(visible(map))queries.map(elapsed); //108FC map, NOT108F8 center.
    if(auto camera=panels.small().camera())if(visible(camera))queries.camera_snapshot(elapsed);
    if((queries.mode()&7u)==7u){
        if(!panels.command().visible()){queries.orders(human);queries.crosshair(elapsed);}
        else {auto* previous=queries.previous_human();queries.orders(previous?previous:human);}
        queries.messages(elapsed);queries.selection(elapsed);
    }else {queries.selection(elapsed);queries.crosshair(elapsed);queries.messages(elapsed);}
    if(panels.dialogs().signals_active())queries.signals(elapsed);
    else if(panels.dialogs().roll().flag30)queries.roll(elapsed);
    else if(visible(panels.selector().root()))queries.selector(elapsed);
    queries.progress(human);
    if(hud.flags&0x18u){
        queries.hands(elapsed);
        if(!queries.paused())queries.status(elapsed);
        queries.soldiers(); //Reverse six roots, each visibility tested by its actual receiver.
    }
    queries.timers(elapsed);queries.carnage(elapsed);
    if(hud.flags&0x18u)queries.compass(elapsed);
    if(panels.command().visible())queries.command(elapsed);
    if(visible(panels.inventory().root()))queries.inventory(elapsed);
    queries.vehicle(human); //Uses cached Human; modal gates read fresh flags/visibility.
    queries.scoreboard(elapsed);panels.small().update_voting(); //004AF470 genuine RET.
}
} //namespace hd2::game
#endif
