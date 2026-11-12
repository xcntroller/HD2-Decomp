#ifndef HD2_GAME_SESSION_ENTRY_RECORDS_H
#define HD2_GAME_SESSION_ENTRY_RECORDS_H
#include "frontend/control_startup.h"
namespace hd2::game {
//0072E890, status9's separate entry-reset phase. This clears records, not the
//process manager owner. Received-start0 instead calls00724110/00723000 before
//old-game retirement; do not merge those two containing protocols.
template<class Runtime>void reset_session_entry_records(frontend::ControlStartupState& state,Runtime& runtime){
    runtime.enter_control_lock(0);
    {
        struct Unlock{Runtime& runtime;~Unlock(){runtime.leave_control_lock(0);}}unlock{runtime};
        for(auto& peer:state.records){peer.human_actor(nullptr);for(unsigned n=0;n<9;++n)peer.word(0x40+4*n,0);}
    }
    if(state.service14)runtime.clear_control_input_records(); //After418 unlock; embedded30/defaults/manager survive.
    for(unsigned n=0;n<3;++n){state.team_statistics464[n].fill(0);state.team_scores44c[n]=0;}
}
}
#endif
