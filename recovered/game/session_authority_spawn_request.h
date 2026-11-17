#ifndef HD2_GAME_SESSION_AUTHORITY_SPAWN_REQUEST_H
#define HD2_GAME_SESSION_AUTHORITY_SPAWN_REQUEST_H
#include "game/prepared_spawn_packets.h"
#include "game/human_score_presentation.h"
namespace hd2::game {
inline std::array<std::uint32_t,12> application_spawn_stat_defaults(){return {{800,80,80,80,80,80,50,30,20,50,0,80}};}
template<class Runtime>void initialize_application_spawn_stats(std::array<std::uint32_t,12>& stats,
        const frontend::PreMissionSoldier& soldier,Runtime& r){ //00727800 cases0/1, BEFORE rank update.
    if(const auto* profile=r.application_spawn_profile(soldier.human)){
        for(unsigned n=0;n!=9;++n){float value;std::memcpy(&value,&profile->stats[n+1],4);stats[n]=human_loading_stat_word(value);
            if(n==1&&soldier.flag09)stats[1]=human_loading_stat_word(static_cast<long double>(human_frame_signed_word(stats[1]))+
                r.application_spawn_clothing_weight(soldier.equipment[1].item)+1.f); //Native54 is clothing, +1 at0080F524.
        }
    }
}
template<class Packet,class Runtime>void receive_authority_spawn_request(std::uint32_t kind,Packet& packet,Runtime& r){ //00727800 cases0/1 ONLY, distinct presentation grammar.
    if(kind!=0&&kind!=1)throw std::runtime_error("Required authority application command is not reconstructed");
    const auto peer=packet.read(32);
    if(kind==0){
        auto stats=application_spawn_stat_defaults();frontend::PreMissionSoldier soldier;
        read_prepared_soldier(packet,soldier,false);soldier.split_spawn_stacks();initialize_application_spawn_stats(stats,soldier,r);
        r.admit_application_spawn(peer,soldier,(r.application_mode_flags()&7u)==7u?nullptr:&stats);
    }else{
        auto soldier=std::make_unique<frontend::PreMissionSoldier>();
        read_prepared_soldier(packet,*soldier,false);soldier->split_spawn_stacks();
        auto stats=std::make_unique<std::array<std::uint32_t,12>>(application_spawn_stat_defaults());
        initialize_application_spawn_stats(*stats,*soldier,r);
        const auto rank=r.application_spawn_peer_rank(peer); //0072F4C0 is an unlocked40..60 view; read actual54.
        apply_human_rank_statistics(*stats,r.application_spawn_profile(soldier->human),rank);
        r.receive_application_pending_spawn(peer,soldier,stats); //Only successful9C/A0 publication transfers ownership.
    }
}
}
#endif
