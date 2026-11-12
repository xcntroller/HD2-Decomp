#ifndef HD2_GAME_HUD_INVENTORY_PRESENTATION_H
#define HD2_GAME_HUD_INVENTORY_PRESENTATION_H
#include <stdexcept>
namespace hd2::game {
//0049DE90: actual modal transition order. Actor/Human producers remain with
//their real owners; no retained callback, new menu or alternate inventory.
template<class Actor,class Queries>void show_hud_inventory(bool enabled,Actor* target,Queries& queries){
    if(!enabled){if(!queries.command_visible())queries.select_inventory_pointer(-1);queries.show_inventory_panel(false);queries.refresh_inventory_compass();return;}
    Actor* primary=nullptr;Actor* secondary=nullptr;
    if(queries.offline()){primary=queries.current_player();secondary=target;}
    else{primary=queries.selected_human();if(!primary){queries.refresh_inventory_compass();return;}
        if(queries.special_game_mode()&&target){queries.request_inventory(*primary,queries.actor_id(*target));secondary=target;}}
    queries.publish_inventory_actors(secondary?3u:1u,primary,secondary);
    queries.hide_inventory_signals();queries.hide_inventory_roll();queries.hide_inventory_selector();queries.hide_inventory_info();queries.hide_inventory_map();
    queries.select_inventory_pointer(0);queries.show_inventory_panel(enabled);queries.refresh_inventory_compass();
}
} //namespace hd2::game
#endif
