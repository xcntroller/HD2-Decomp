#ifndef HD2_GAME_HUD_SESSION_INPUT_H
#define HD2_GAME_HUD_SESSION_INPUT_H
#include <cstdint>
namespace hd2::game {
//0049FC10: actual ordinary roll2F00+30, then signals2E80 visibility.
//Defer input resolution until the gate admits;006C71C0 selects the manager's
//OWN pair when either caller buffer is absent. The ordered scan list is native.
template<class Queries,class Consume>void filter_hud_session_input(Queries& q,Consume consume){
    if(!q.roll_active()&&!q.signals_active())return;
    constexpr std::uint32_t scans[]={0x52,0x4f,0x50,0x51,0x4b,0x4c,0x4d,0x47,0x48,0x49};
    for(const auto scan:scans)consume(scan);
}
//0049F090 uses actual map108FC, NOT its center108F8. Null map is admitted.
template<class Queries>bool hud_nonoffline_input_blocked(Queries& q){
    return (q.hud_flags()&1u)&&q.map_present()&&q.map_visible();
}
//0049D300: command10B20 root visibility precedes inventory34 visibility,
//then message10315 == messages2F90+flagD385; no game pause/selection test.
template<class Queries>bool hud_player_input_blocked(Queries& q){
    return q.command_visible()||q.inventory_visible()||q.message_editing();
}
}
#endif
