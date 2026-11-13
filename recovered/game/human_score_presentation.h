#ifndef HD2_GAME_HUMAN_SCORE_PRESENTATION_H
#define HD2_GAME_HUMAN_SCORE_PRESENTATION_H
#include "game/human_loading_stats.h"
#include "game/human_packet_notifications.h"
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
//00725B00: typed process3C, actual Human holder4 and item definition44.
//Resolvers borrow the SAME Scene2/Items owners; no record-ID/Players fallback.
template<class Record,class Resolve,class Alignment,class Localize>
std::string human_rank_record_label(Record& record,std::uint32_t style,Resolve&& resolve,Alignment&& alignment,Localize&& localize){
    auto side=record.word(0x64);
    if(style==0)if(auto* actor=record.human_actor()){
        auto& human=resolve(actor);auto* storage=human.base().storage();auto* holder=storage?storage->holder(4):nullptr;
        if(!holder)throw std::logic_error("Rank label requires actual Human holder4");
        const auto item=holder->entry();side=item&&alignment(item)==0?1u:0u;
    }
    return localize((side==1?0x2792u:0x279cu)+record.word(0x54));
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
    const auto text=local?format_human_packet_message("%s %s",{phrase,label},512):
        format_human_packet_message("%s %s %s",{state.records[identity].name(),phrase,label},512);
    services.emit_human_rank_text(text,demoted?std::array<float,3>{{255,0,0}}:std::array<float,3>{{0,255,0}});
}
} //namespace hd2::game
#endif
