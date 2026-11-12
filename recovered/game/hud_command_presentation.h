#ifndef HD2_GAME_HUD_COMMAND_PRESENTATION_H
#define HD2_GAME_HUD_COMMAND_PRESENTATION_H
#include <array>
#include <cstdint>
#include <cstring>
namespace hd2::game {
//0049F410. Flags18 belong to the containing HUD, not its inventory subobject.
//The actual FPV enable receiver and Player mode producer run before panel show.
template<class Queries>void show_hud_command(bool enabled,Queries& q){
    const auto real=[](std::uint32_t bits){float value;std::memcpy(&value,&bits,4);return value;};
    if(enabled){
        if(!q.command_allowed())return;
        q.saved_flags()=q.hud_flags()&0x38u;q.hud_flags()=(q.hud_flags()&0xffffffcfu)|8u;q.refresh_layout();
        if(q.first_person_enabled()){q.saved_flags()|=0x40u;q.reset_current_player_mode();q.enable_first_person(false);}
        q.position_timers({{real(0x3f5eb852),real(0xbecccccd),0,0}});q.show_command_panel(true);q.show_inventory_panel(false);
        q.hide_command_signals();q.hide_command_roll();q.hide_command_selector();q.hide_command_info();q.hide_command_map();q.clear_command_crosshair();q.select_command_pointer(0);
    }else if(q.command_visible()){
        q.hud_flags()^=(q.hud_flags()^q.saved_flags())&0x38u;q.refresh_layout();
        if(q.saved_flags()&0x40u)q.enable_first_person(true);
        q.position_timers({{real(0x3f7051db),real(0xbeec29e1),0,0}});q.show_command_panel(false);q.show_inventory_panel(false);q.select_command_pointer(-1);
    }
    q.refresh_current_human();
}
} //namespace hd2::game
#endif
