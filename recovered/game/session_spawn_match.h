#ifndef HD2_GAME_SESSION_SPAWN_MATCH_H
#define HD2_GAME_SESSION_SPAWN_MATCH_H
#include "game/session_activation.h"
namespace hd2::game {
template<class Runtime>void set_session_spawn_match_phase(SessionActivationState& game,Runtime& r,std::uint32_t phase){ //0069AA50.
    game.words.set(0xb8,phase);
    if(phase<2){r.spawn_hud_mode(1);game.words.set(0xb4,0xffffffffu);}
    else if(phase==2){r.spawn_hud_mode(2);r.spawn_camera_command(2,true,1);}
}
template<class Human,class Runtime>void present_session_local_spawn(SessionActivationState& game,Runtime& r,std::uint32_t peer,Human& human){ //0069E290.
    if(peer!=static_cast<std::uint32_t>(r.human_current_player_id()))return;
    if(game.words.template get<std::uint32_t>(0x88)==3){
        r.spawn_selection_suppressed(human,1);r.set_spawn_match_phase(2);
    }else{r.spawn_hud_mode(0);r.refresh_peer_hud_human();}
    game.words.set(0x90,0u);r.spawn_hide_map();
    if(r.human_score_style()==2&&r.spawn_selected_suppressed(human))r.hud_human_spawn_protection(0x4f57,5000);
}
} //namespace hd2::game
#endif
