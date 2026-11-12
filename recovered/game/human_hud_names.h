#ifndef HD2_GAME_HUMAN_HUD_NAMES_H
#define HD2_GAME_HUMAN_HUD_NAMES_H
#include "game/human_hud_loading.h"
#include "frontend/control_startup.h"
namespace hd2::game {
//005A8380's nonoffline snapshot is NINE words40..60 under the actual process
//lock0; rank is snapshot[5] (native54), NOT the final word60. ASM5A83A3/C4.
template<class Locks>std::uint32_t human_hud_peer_rank(frontend::ControlStartupState& state,Locks& locks,std::int32_t identity){
    std::array<std::uint32_t,9> snapshot;
    struct Lock {Locks& locks;explicit Lock(Locks& value):locks(value){locks.enter_control_lock(0);}~Lock(){locks.leave_control_lock(0);}};
    {Lock lock(locks);if(identity<0)throw std::out_of_range("Human HUD rank requires its actual peer identity");
        const auto& record=state.records.at(static_cast<std::size_t>(identity));
        for(unsigned n=0;n!=9;++n)snapshot[n]=record.word(0x40+n*4);
    }
    return snapshot[5];
}
//005A81B0→0072EBD0: invalid signed/bounds ID is genuinely empty. Copy the
//actual record08 name immediately; no peer buffer survives this call.
inline std::string human_hud_peer_name(const frontend::ControlStartupState& state,std::int32_t identity){
    return identity<0||static_cast<std::size_t>(identity)>=state.records.size()?std::string{}:state.records[static_cast<std::size_t>(identity)].name();
}
//005A82C0: offline/profile fallback is SHORT10, or Actor virtual04/name.
template<class Human>std::string human_hud_short_name(const Human& human,std::uint32_t mode,const frontend::ControlStartupState& peers){
    if((mode&7u)!=7u&&human.properties.field19c==1)return human_hud_peer_name(peers,human.base().id10());
    if(const auto* profile=human.profile_record)return profile->user_name.empty()?profile->short_name:profile->user_name;
    return human.base().name();
}
//005A8820/00730530's read-only UI team query. Uses the existing Human kind
//and process record64; no roster, simulation or peer state is created here.
template<class Human>std::uint32_t human_hud_team(const Human& human,std::uint32_t mode,bool remote_ai,const frontend::ControlStartupState& peers){
    if((mode&7u)==7u||remote_ai){const auto kind=human.properties.field19c;
        if(kind==2||((mode&7u)!=7u&&kind==5))return 0;
        return kind==1||kind==4?1u:2u;
    }
    const auto id=static_cast<std::uint32_t>(human.base().id10());return id<peers.records.size()?peers.records[id].word(0x64):2u;
}
//007304B0: local peer64 has no native bounds fallback. Require the produced
//record rather than substituting the Human's team or a default value.
inline std::int32_t human_hud_local_team(const frontend::ControlStartupState& peers,std::int32_t identity){
    if(identity<0)throw std::out_of_range("HUD local team requires its actual peer identity");
    return human_hud_signed(peers.records.at(static_cast<std::size_t>(identity)).word(0x64));
}
} //namespace hd2::game
#endif
