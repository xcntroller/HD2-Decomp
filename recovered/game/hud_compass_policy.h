#ifndef HD2_GAME_HUD_COMPASS_POLICY_H
#define HD2_GAME_HUD_COMPASS_POLICY_H

#include "game/human_inventory_lookup.h"

namespace hd2::game {

//0049F1C0:005A78F0(9,nullptr) is an INVENTORY DEFINITION lookup, not
//a player input/action callback. Borrow the current Human and SAME Items owner.
template<class Frames,class Crosshair,class Command,class Players,class Items,class Compass>
void update_hud_compass_visibility(Frames& frames,typename Frames::Frame inventory,Crosshair& crosshair,
                                  Command& command,Players& players,Items& items,Compass& compass,
                                  std::uint32_t mode,std::uint32_t flags) {
    bool visible=false;
    if((mode&7u)==7u&&!(flags&0x1a0u)&&!frames.hud_frame_visible(inventory)&&
            crosshair.field14()!=1&&!command.visible()) {
        if(auto* human=players.current_player())visible=find_human_inventory_definition(*human,items,9)!=nullptr;
    }
    compass.show(visible);
}
} //namespace hd2::game
#endif
