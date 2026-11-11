#ifndef HD2_GAME_HUMAN_PACKET_RANK_H
#define HD2_GAME_HUMAN_PACKET_RANK_H

#include "game/human_packet_notifications.h"
#include "game/human_loading_stats.h"
#include "frontend/control_startup.h"

namespace hd2::game {
inline std::int32_t human_packet_rank_signed(std::uint32_t word){std::int32_t value;std::memcpy(&value,&word,4);return value;}

//006A1CB0: a missing profile preserves the caller defaults. Rank scales the
//six skill fields using exact FLOAT0080FD38; strength/health are unscaled.
template<class Store>std::array<std::uint32_t,9> human_packet_rank_stats(Store& profiles,std::int32_t profile,std::int32_t rank){
    std::array<std::uint32_t,9> result{{800,80,80,80,80,80,50,30,20}};
    const auto* record=profiles.record(profile);if(!record)return result;
    const auto real=[&](unsigned index){float value;std::memcpy(&value,&record->stats[index],4);return value;};
    const auto factor=static_cast<long double>(rank)*0.029999999329447746f+1.f;
    result[0]=human_loading_stat_word(real(1));result[1]=human_loading_stat_word(real(2));
    for(const auto pair:std::array<std::pair<unsigned,unsigned>,6>{{{2,3},{4,5},{5,6},{6,7},{7,8},{8,9}}}){
        auto value=human_loading_stat_word(static_cast<long double>(real(pair.second))*factor);
        if(human_packet_rank_signed(value)>100)value=100;
        result[pair.first]=value;
    }
    return result;
}

template<class Human>void apply_human_packet_rank_stats(Human& human,const std::array<std::uint32_t,9>& stats){ //005A96A0.
    for(unsigned index:{2u,1u,4u,6u,7u,8u})human.properties.stats_ec[index]=stats[index];
    for(unsigned index:{2u,1u,4u,6u,7u,8u})human.properties.stats_11c[index]=stats[index];
}

//0072FAC0 uses process lock418 (source lock0) THROUGH HUD publication. Copy
//all display values before invoking HUD: its callbacks can replace actor/records.
template<class Startup,class Hud>void publish_human_packet_rank(Startup& startup,Hud& hud,std::int32_t identity,std::uint32_t rank){
    struct Lock {Startup& startup;explicit Lock(Startup& s):startup(s){startup.enter_control_lock(0);}~Lock(){startup.leave_control_lock(0);}} lock(startup);
    auto& record=startup.game_control_records().records.at(static_cast<std::size_t>(identity));
    const auto previous=record.word(0x54);record.word(0x54,rank);
    const bool lower=human_packet_rank_signed(rank)<human_packet_rank_signed(previous),local=identity==startup.local_player_identity();
    const auto caption=frontend::lookup_localized_text(startup.text(),local?(lower?0x22c4:0x22c5):(lower?0x22c6:0x22c7));
    auto rank_family=record.word(0x64);
    if(startup.game_configuration().hud_style224()==0&&record.word(0x3c))
        throw std::runtime_error("Rank display00725B00 requires actual process record3C Human/clothing owner binding");
    const auto caption_id=rank+(rank_family==1?0x2792u:0x279cu);
    const auto name=frontend::lookup_localized_text(startup.text(),human_packet_rank_signed(caption_id));
    const auto message=local?format_human_packet_message("%s %s",{caption,name},512):
        format_human_packet_message("%s %s %s",{record.name(),caption,name},512);
    hud.append_message(message.c_str(),lower?std::array<float,3>{{255,0,0}}:std::array<float,3>{{0,255,0}});
}

//005406C0 and00592FE0 command16 differ: Player writes record54 BEFORE stats
//and final publication, Remote uses the OLD rank for stats. Do not unify gates.
template<class Human,class Store,class Startup,class Hud>void receive_human_packet_rank(Human& human,Store& profiles,Startup& startup,Hud& hud,
        std::uint32_t rank,bool player){
    if((startup.application_flags()&7u)!=7u)throw std::runtime_error("Rank packet requires actual nonoffline publication");
    const auto identity=human.base().id10();
    auto& record=startup.game_control_records().records.at(static_cast<std::size_t>(identity));
    if(player)record.word(0x54,rank);
    const auto stats=human_packet_rank_stats(profiles,human_packet_rank_signed(human.properties.field170),human_packet_rank_signed(record.word(0x54)));
    apply_human_packet_rank_stats(human,stats);
    publish_human_packet_rank(startup,hud,identity,rank);
}
} //namespace hd2::game
#endif
