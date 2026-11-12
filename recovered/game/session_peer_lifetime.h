#ifndef HD2_GAME_SESSION_PEER_LIFETIME_H
#define HD2_GAME_SESSION_PEER_LIFETIME_H
#include "game/session_peer_records.h"
#include "game/session_activation.h"
namespace hd2::game {
//0069E720 consumes transport00, reverse-finds its stable peer slot under418,
//then schedules actual Scene2 retirement. Borrowers are reread after callbacks;
//no peer erase, private Human owner, eager delete or invented rollback occurs.
template<class Runtime>void retire_session_peer(frontend::ControlStartupState& peers,
        SessionActivationState& session,std::uint32_t transport,Runtime& runtime){
    runtime.enter_control_lock(0);
    struct Unlock{Runtime& runtime;~Unlock(){runtime.leave_control_lock(0);}}unlock{runtime};
    std::size_t slot=peers.records.size();bool found=false;
    while(slot){--slot;if(peers.records[slot].word(0)==transport){
        peers.records[slot].word(0,0);peers.records.at(slot).byte(8,0);found=true;break;
    }}
    if(!found||!peers.records.at(slot).human_actor())return;
    const auto human=[&]{return peers.records.at(slot).human_actor();};
    peers.records.at(slot).byte(0xa2,1);
    if(session.words.template get<std::uint32_t>(0x78)==slot)session.words.set(0x78,0xffffffffu);
    if((runtime.mode_flags()&7u)!=7u&&(runtime.mode_flags()&2u)){
        if(runtime.peer_hud_human()==human())runtime.refresh_peer_hud_human(); //0049F0E0/0049F100(NULL), NOT HUD selection pair.
        if(runtime.peer_override_human()==human())runtime.clear_peer_override_human(); //0045E8F0(NULL), includes camera/FPV refresh.
        if(runtime.local_peer_human()&&runtime.peer_human_has_state(runtime.local_peer_human())&&
           session.words.template get<std::uint32_t>(0xb4)==slot)session.words.set(0xb4,0xffffffffu);
    }
    if((runtime.mode_flags()&1u)&&runtime.human_remote_ai_enabled())runtime.remove_peer_auxiliary_human(human());
    runtime.remove_peer_runtime_links(human()); //00479700, original forward scan/removal policy.
    runtime.retire_peer_scene_human(human()); //00475C50(false)→00476680→005A7FE0, SAME retained Human.
    peers.records.at(slot).human_actor(nullptr);
    for(unsigned n=0;n!=9;++n)peers.records.at(slot).word(0x40+4*n,0);
}
}
#endif
