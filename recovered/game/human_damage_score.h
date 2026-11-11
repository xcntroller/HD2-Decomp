#ifndef HD2_GAME_HUMAN_DAMAGE_SCORE_H
#define HD2_GAME_HUMAN_DAMAGE_SCORE_H
#include "frontend/control_startup.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {
//0072E7E0 resets player words under418, then the three team records UNLOCKED.
template<class Locks>void reset_human_match_scores(frontend::ControlStartupState& state,Locks& locks){
    locks.enter_control_lock(0);
    try{for(auto& record:state.records){for(unsigned at=0x40;at!=0x64;at+=4)record.word(at,0);record.word(0x7c,0);}}
    catch(...){locks.leave_control_lock(0);throw;}
    locks.leave_control_lock(0);
    for(unsigned n=0;n!=3;++n){state.team_statistics464[n].fill(0);state.team_scores44c[n]=0;}
}
//0072FD60. Native00844F94 is10,20,...80; the first threshold ABOVE score
//selects rank0..7. Actual B0 command allocation precedes record54 publication.
template<class Services>void update_human_score_rank(frontend::ControlStartupState& state,std::uint32_t identity,Services& services){
    if(!(services.mode_flags()&1u))return;
    if(identity>=state.records.size())throw std::out_of_range("Rank update requires actual peer record");
    auto& record=state.records[identity];const auto named=record.byte(8);
    if(!named)throw std::logic_error("Rank record name has no original producer");
    if(!*named||!record.human_actor())return;
    const auto score=human_frame_signed_word(record.word(0x4c));std::uint32_t rank=0;
    while(rank<7&&static_cast<std::int32_t>((rank+1)*10)<=score)++rank;
    if(rank!=record.word(0x54)&&services.queue_human_score_rank(record.human_actor(),rank))record.word(0x54,rank);
}
//0072EE60, not a network-packet leaf. Mutates the process records in native
//order. The friendly-kill removal occurs AFTER releasing418. Source bounds
//reject the native remote-AI source==count off-by-one instead of indexing OOB.
template<class Services>void score_human_death(frontend::ControlStartupState& state,std::uint32_t victim,std::uint32_t source,Services& services){
    services.enter_control_lock(0);
    struct Unlock{Services& services;bool held=true;void release(){if(held){held=false;services.leave_control_lock(0);}}~Unlock(){release();}}lock{services};
    const auto add=[](auto& record,unsigned offset,std::uint32_t amount){record.word(offset,record.word(offset)+amount);};
    const auto team=[&](std::uint32_t identity)->auto&{
        if(identity>=state.team_statistics464.size())throw std::out_of_range("Scoring requires actual team0..2");
        return state.team_statistics464[identity];
    };
    if(victim==0xffffffffu||victim>=state.records.size()){
        if(services.human_remote_ai_enabled()&&source!=0xffffffffu&&source<=state.records.size()){
            if(source==state.records.size())throw std::out_of_range("Original remote-AI scoring indexed beyond peer records");
            auto& record=state.records[source];const auto kind=services.human_score_victim_kind(victim);
            const bool friendly=kind==3||kind==4||kind==5;
            if(!friendly){add(record,0x44,1);add(record,0x58,1);}
            add(record,0x4c,friendly?0xfffffff6u:1u);add(record,0x60,friendly?0xfffffff6u:1u);
            if(services.mode_flags()&1u)update_human_score_rank(state,source,services);
        }
        return;
    }
    add(state.records[victim],0x48,1);add(state.records[victim],0x5c,1);
    auto& victim_team=team(state.records[victim].word(0x64));++victim_team[2];++victim_team[7];
    if(source==victim){
        add(state.records[victim],0x4c,0xffffffffu);add(state.records[victim],0x60,0xffffffffu);
        update_human_score_rank(state,victim,services);return;
    }
    if(source==0xffffffffu||source>=state.records.size())return;
    auto& record=state.records[source];auto* human=record.human_actor();
    if(!human||services.human_score_state_kind(human)!=1)return;
    if(state.records[victim].word(0x64)!=record.word(0x64)||services.human_score_style()==0){
        for(auto at:{0x44u,0x58u,0x4cu,0x60u})add(record,at,1);
        auto& a=team(record.word(0x64));++a[1];++a[6];
        auto& b=team(record.word(0x64));++b[3];++b[8];
        if(services.human_score_style()==2){const auto side=record.word(0x64);(void)team(side);state.team_scores44c[side]+=1.f;}
        if(services.mode_flags()&1u){update_human_score_rank(state,source,services);update_human_score_rank(state,victim,services);}
        return;
    }
    add(record,0x4c,0xfffffff6u);add(record,0x60,0xfffffff6u);add(record,0x7c,1);
    auto& stats=team(record.word(0x64));stats[3]-=10;stats[8]-=10;
    lock.release();
    if((services.mode_flags()&1u)&&human_frame_signed_word(state.records[source].word(0x7c))>=3&&source!=static_cast<std::uint32_t>(services.human_current_player_id()))
        services.remove_human_score_peer(source,1);
}
} //namespace hd2::game
#endif
