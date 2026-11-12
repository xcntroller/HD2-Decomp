#ifndef HD2_GAME_HUD_CONTROLS_H
#define HD2_GAME_HUD_CONTROLS_H
#include "game/hud_command_frame.h"
#include <array>
#include <cstdint>
namespace hd2::game {
//0049C200: actual Players0045DC20 is a required producer; this is the UI
//selection/modal/refresh policy around it, never a scalar-index substitute.
template<class Queries>void select_hud_player(std::int32_t index,Queries& q){
    if(!q.hud_flags()||index==q.current_index())return;q.select_player_index(index);q.hide_roll();q.hide_signals();
    if(q.inventory_visible()){q.inventory_panel(false);q.publish_current_inventory();q.inventory_panel(true);}q.refresh_current_human();
}
template<class Queries>void click_hud_soldier(std::uint32_t tag,bool right,Queries& q){ //0049DCE0/0049DD60; ONE process0089BB80 timer.
    if(tag<100||tag>=106)return;if(right)q.select_command_player_index(static_cast<std::int32_t>(tag-100));else select_hud_player(static_cast<std::int32_t>(tag-100),q);
    if(q.command_visible()){const auto now=q.ticks();if(hud_command_signed(now-q.soldier_click_ticks())<300){q.soldier_click_ticks()=0;q.focus_previous_human();}else q.soldier_click_ticks()=now;}
}
template<class Queries>bool cancel_hud_controls(Queries& q){ //0049C2B0; all reached UI cleanup still executes when nothing was open.
    const bool consumed=q.message_mode()==1||q.signals_active()||q.roll_active()||q.selector_visible()||q.inventory_visible();
    if(q.message_mode()==1)q.set_message_mode(0);if(q.message_editing())q.stop_message_input();
    q.hide_signals();q.hide_roll();q.hide_selector();q.show_inventory(false);q.pointer_kind(q.command_visible()?0:-1);q.position_timers(false);return consumed;
}
//0049C3C0 and0049C110/1A0/1D0. Snapshot/control dispatch only, with actual
//panel workers and required Human/Players/realtime producers supplied in-call.
template<class Input,class Queries>class HudControls {
public:
    HudControls(Input& input,Queries& queries):i(input),q(queries){}
    void run(){
        if(!q.hud_flags())return;
        if(i.second(46)!=0){if(cancel_hud_controls(q))i.second(46,0);return;}
        if(i.second(31)!=0&&!q.command_visible()&&!q.message_editing()&&q.message_mode()!=1)q.request_realtime(!q.paused());
        if(q.paused()&&!q.command_visible()&&q.message_mode()!=1)return;if(q.camera_suppressed())return;
        if(!q.command_visible()&&i.second(27)!=0){
            if(q.message_mode()==1)q.set_message_mode(0);
            else{q.set_message_mode(1);q.position_timers(false);q.hide_signals();q.hide_roll();q.hide_selector();q.inventory_panel(false);q.hide_info();q.hide_map();q.pointer_kind(-1);}
        }else if(q.message_mode()==1||q.message_editing()){clear_pair();consume_digit_scans();return;}
        if(!q.map_visible()){
            if(i.second(28)!=0){q.inventory_panel(false);q.pointer_kind(-1);q.say(false);}
            else if(i.second(29)!=0){q.inventory_panel(false);q.pointer_kind(-1);q.say(true);}
        }
        q.process_pointer();
        if(i.second(24)!=0){q.show_map(q.command_visible());q.position_timers(false);q.hide_signals();q.hide_roll();q.hide_selector();}
        if(q.map_visible()){if(q.map_latched()||i.first(24)!=0){q.process_map();return;}q.hide_map();}
        if(i.second(21)!=0){q.show_info();q.position_timers(false);q.hide_signals();q.hide_roll();q.hide_selector();}
        else if(!q.info_latched()&&i.first(21)==0&&q.info_visible())q.hide_info();
        if(!(q.hud_flags()&4u))return;
        if(q.offline()&&!q.command_visible()&&!q.selected_human())resume_keyboard();
        if(q.signals_active()){q.process_signals();if(!q.signals_active())q.position_timers(false);return;}
        if(q.roll_active()){q.process_roll();return;}
        if(q.offline()&&i.second(30)!=0&&!i.key_down(0x38)&&!i.key_down(0xb8)){const bool reverse=i.key_down(0x2a)||i.key_down(0x36);select_hud_player(q.next_player_index(reverse),q);}
        if(i.second(23)!=0&&!q.command_visible())q.cycle_layout();
        if(q.selector_visible()){q.process_selector();return;}
        if((i.second(32)!=0||i.second(33)!=0)&&!q.command_visible()&&!q.inventory_visible()&&q.selected_human()){
            q.position_timers(false);q.show_selector();q.hide_signals();q.hide_roll();q.hide_info();q.hide_map();
        }
        if(i.second(25)!=0)q.show_inventory(!q.inventory_visible());
        if(q.offline()&&i.second(22)!=0){q.show_command(!q.command_visible()||q.message_mode()==1);return;}
        if(q.command_visible()||q.inventory_visible()){
            if(q.inventory_visible()){if(i.second(19)!=0)q.show_inventory(false);else q.process_inventory();}
            else if(q.command_visible())q.process_command();
            if(i.second(55)!=0||i.second(56)!=0)if(const auto tag=q.pointer_hit_tag())click_hud_soldier(tag,i.second(55)==0,q);
            clear_pair();consume_digit_scans();return;
        }
        if(i.second(19)!=0){q.restore_roll_position();q.show_roll();if(!q.roll_active())return;q.position_timers(false);q.hide_signals();}
        else{
            std::int32_t recipient=-1;for(unsigned n=39;n<=44;++n)if(i.second(n)!=0){recipient=static_cast<std::int32_t>(n-39);break;}
            if(recipient>=0){if(auto* human=q.roster_human(recipient))if(!q.is_player(*human))q.show_signals(human);}
            else if(i.second(45)!=0)q.show_signals(q.null_human());
            else{for(unsigned n=35;n<=38;++n)if(i.second(n)!=0){const auto index=static_cast<std::int32_t>(n-35);auto* human=q.roster_human(index);if(!human||q.health(*human)<1||q.is_player(*human))return;select_hud_player(index,q);q.resume_player_control();return;}return;}
            if(!q.signals_active())return;q.position_timers(true);q.hide_roll();
        }
        q.hide_selector();q.hide_info();q.hide_map();
    }
private:
    void clear_pair(){for(unsigned n=64;n;){--n;i.second(n,0);i.first(n,0);}} //0049C1D0, reverse64 writes.
    void consume_digit_scans(){for(unsigned n=0;n!=10;++n)i.consume_scan(n);} //0049C1A0, scans0..9, not digit characters.
    void resume_keyboard(){ //0049C110, descending64 with only native-listed actions admitted.
        for(unsigned n=64;n;){--n;if((n<18||n==19||(n>=39&&n<=45))&&i.second(n)!=0)q.resume_player_control();}
    }
    Input& i;Queries& q;
};
} //namespace hd2::game
#endif
