#ifndef HD2_GAME_HUMAN_SCORE_PRESENTATION_H
#define HD2_GAME_HUMAN_SCORE_PRESENTATION_H
#include "game/human_loading_stats.h"
#include "game/human_state_frame_timers.h"
#include "frontend/control_startup.h"

namespace hd2::game {
//006A1CB0, actual HumanStore48..68 FLOATs. Rank factor stays on the x87
//stack through the eight FTOL calls. Missing profile preserves native defaults.
inline std::array<std::uint32_t,12> human_rank_statistics(const frontend::HumanRecord* profile,std::uint32_t rank){
    std::array<std::uint32_t,12> values{{800,80,80,80,80,80,50,30,20,50,0,80}};
    if(!profile)return values;
    const auto stat=[&](unsigned n){float value;std::memcpy(&value,&profile->stats[n+1],4);return value;};
    const auto factor=static_cast<long double>(human_frame_signed_word(rank))*.029999999329447746f+1.f;
    values[0]=human_loading_stat_word(stat(0));values[1]=human_loading_stat_word(stat(1));
    for(auto n:{2u,4u,5u,6u,7u,8u}){
        values[n]=human_loading_stat_word(factor*stat(n));if(human_frame_signed_word(values[n])>100)values[n]=100;
    }
    return values;
}
template<class Human>void publish_human_rank_statistics(Human& human,const std::array<std::uint32_t,12>& values){ //005A96A0.
    for(auto n:{2u,1u,4u,6u,7u,8u})human.properties.stats_ec[n]=values[n];
    for(auto n:{2u,1u,4u,6u,7u,8u})human.properties.stats_11c[n]=values[n];
}
//0072FAC0, lock418 held through localization/HUD. Decoder's preceding54 store
//is deliberately not suppressed: it affects this native old/new comparison.
template<class Services>void present_human_score_rank(frontend::ControlStartupState& state,std::uint32_t rank,
        std::uint32_t identity,Services& services){
    services.enter_control_lock(0);
    struct Unlock{Services& services;~Unlock(){services.leave_control_lock(0);}}unlock{services};
    if(identity>=state.records.size())throw std::out_of_range("Rank presentation requires actual peer record");
    const auto previous=human_frame_signed_word(state.records[identity].word(0x54));state.records[identity].word(0x54,rank);
    const bool demoted=human_frame_signed_word(rank)<previous;
    const bool local=identity==static_cast<std::uint32_t>(services.human_current_player_id());
    const auto phrase=services.human_damage_localize(local?(demoted?0x22c4u:0x22c5u):(demoted?0x22c6u:0x22c7u));
    const auto label=services.human_score_rank_label(state.records[identity]);
    const auto text=local?phrase+" "+label:state.records[identity].name()+" "+phrase+" "+label;
    if(text.size()>=512)throw std::length_error("Rank message exceeds native512-byte formatter");
    services.emit_human_rank_text(text,demoted?std::array<float,3>{{255,0,0}}:std::array<float,3>{{0,255,0}});
}
} //namespace hd2::game
#endif
