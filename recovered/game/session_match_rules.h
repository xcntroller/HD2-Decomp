#ifndef HD2_GAME_SESSION_MATCH_RULES_H
#define HD2_GAME_SESSION_MATCH_RULES_H
#include "game/session_match_frame.h"
#include "game/session_frame_input.h"
#include <array>
#include <cmath>
#include <cstdio>
namespace hd2::game {
inline std::int32_t frame_match_score(float score){
    const auto wide=static_cast<long double>(score);
    if(!(wide>=-0x1p63L&&wide<0x1p63L))return 0; //Native __ftol invalid low DWORD.
    return human_frame_signed_word(static_cast<std::uint32_t>(static_cast<std::int64_t>(wide)));
}
inline std::string frame_match_result_text(const std::string& left,std::uint32_t first,const std::string& right,std::uint32_t second){ //00699360→0069BF70, ONE formatting pass over actual string arguments.
    std::array<char,512> bytes{};
    const auto count=std::snprintf(bytes.data(),bytes.size(),"%s%d %s%d",left.c_str(),human_frame_signed_word(first),right.c_str(),human_frame_signed_word(second));
    if(count<0||static_cast<std::size_t>(count)>=bytes.size())throw std::length_error("Match result exceeds native game formatter buffer");
    return std::string(bytes.data(),static_cast<std::size_t>(count));
}
//00699B50, all four rule styles. The process counters and team wins remain
//on their original process owners; callbacks never receive a copied World.
template<class State,class Services>void evaluate_session_match_rules(State& state,SessionMatchFrameProcess& process,Services& q,std::uint32_t elapsed){
    process.rule_008ae5d4+=elapsed;
    if(human_frame_signed_word(process.rule_008ae5d4)<1000)return;
    auto& w=state.words;const auto phase=w.template get<std::uint32_t>(0x88);
    if(phase==1||phase==2||phase==3)return;
    process.rule_008ae5d4=0;
    const auto timed_out=[&]{const auto minutes=q.frame_rule(0x230);return minutes&&w.template get<std::int32_t>(0x30)>=human_frame_signed_word(minutes*60000u);};
    const auto publish=[&](std::uint32_t result_phase,std::uint32_t winner,bool objectives,bool win){
        w.set(0x88,result_phase);w.set(0x8c,w.template get<std::uint32_t>(0x28));
        if(win)q.frame_increment_team_win(winner);
        if(objectives)q.frame_finish_objectives(winner);
        q.frame_update_all_ranks();q.frame_publish_match_result(winner);
    };
    switch(q.frame_rule(0x224)){
    case 0:{
        const auto maximum=q.frame_maximum_peer_score();
        const auto limit=q.frame_rule(0x22c);
        if((human_frame_signed_word(limit)<=maximum&&q.frame_rule(0x22c))||timed_out()){
            w.set(0x88,2u);w.set(0x8c,w.template get<std::uint32_t>(0x28));
            q.frame_update_all_ranks();q.frame_queue_command(8,0,0,0);q.frame_queue_command(13,0,0,0);
        }
        break;
    }
    case 1:{
        const auto flags=q.frame_flag_side_counts(); //Count total and actual Actor37 side98, not active players.
        const auto limit=human_frame_signed_word(q.frame_rule(0x22c));
        const bool score=(frame_match_score(q.frame_team_score(0))>=limit||frame_match_score(q.frame_team_score(1))>=limit)&&limit!=0;
        if(!score&&!timed_out()&&(q.frame_peer_team_alive(0)||flags[1])&&(q.frame_peer_team_alive(1)||flags[2])&&flags[0]!=flags[1]&&flags[0]!=flags[2])break;
        const auto winner=flags[0]==flags[2]||(frame_match_score(q.frame_team_score(0))<frame_match_score(q.frame_team_score(1))&&flags[2])?1u:0u;
        publish(1,winner,false,true);break;
    }
    case 2:{
        const bool first=q.frame_peer_team_alive(0),second=q.frame_peer_team_alive(1);
        const auto count=q.frame_live_team_counts();
        if(!timed_out()&&mission_frame_input_allowed(state.mission)&&(first||count[0]==0)&&(second||count[1]==0))break;
        const auto side=q.frame_first_objective_side();auto winner=q.frame_opposite_side(side);
        if(!first)winner=1;
        if(!second)winner=0;
        if(!mission_frame_input_allowed(state.mission)){
            std::size_t complete=0;const auto& list=q.frame_objectives();
            for(std::size_t n=0;n<list.size();++n){if(!list[n])throw std::logic_error("Match lost an actual objective descriptor");if(list[n]->word10==1u)++complete;}
            winner=complete==q.frame_objectives().size()?side:q.frame_opposite_side(side);
        }
        if(winner==side){if(!first||!second)q.frame_award_objective_points(2,winner);}
        else for(std::size_t n=0;n<q.frame_objectives().size();++n){
            if(q.frame_objectives().at(n)->word10==0u)q.frame_award_objective_points(q.frame_objective_score(n),winner);
        }
        publish(1,winner,true,true);break;
    }
    case 3:{
        const bool first=q.frame_peer_team_alive(0),second=q.frame_peer_team_alive(1);
        const auto limit=q.frame_rule(0x270);const bool below_limit=q.frame_any_peer_below_life_limit(limit);
        const auto count=q.frame_live_team_counts();
        if(mission_frame_input_allowed(state.mission)){
            if(first||second)break;
            const auto shared=q.frame_rule(0x27c);
            const bool global=shared&&limit!=0xffffffffu&&w.template get<std::int32_t>(0x84)>=human_frame_signed_word(limit);
            const bool individual=!shared&&limit!=0xffffffffu&&!below_limit&&(count[0]||count[1]);
            if(!global&&!individual)break;
        }
        const auto winner=mission_frame_input_allowed(state.mission)?0u:1u;
        publish(2,winner,true,false);break;
    }
    default:break;
    }
    ++process.sequence_008ae5d0;
    if(human_frame_signed_word(process.rule_008ae5d4)>299&&w.template get<std::uint32_t>(0x88)==0){
        process.rule_008ae5d4=0; //00731C20 is a genuine RET, even on reentrant counter change.
    }
}
//0069EED0. Hazard payload production belongs to the actual Human receiver;
//the two native unwritten vectors must not be invented by this coordinator.
template<class State,class Services>void advance_session_match_hazards(State& state,SessionMatchFrameProcess& process,Services& q,std::uint32_t elapsed){
    process.hazard_008ae588+=elapsed;
    if(human_frame_signed_word(process.hazard_008ae588)<500)return;
    const auto phase=state.words.template get<std::uint32_t>(0x88);if(phase==1||phase==2||phase==3)return;
    process.hazard_008ae588=0;
    if(q.mode_flags()&1u)q.frame_authority_hazards();
    if(q.mode_flags()&2u)if(auto* human=q.frame_local_human()){
        if(q.player_control_surface_flags(*human)&0x4000u){state.words.set(0x80,std::uint8_t{1});q.frame_set_hint(0x1cf1);}
        //Original calls005A9720 TWICE; the notification can mutate its source.
        if(!(q.player_control_surface_flags(*human)&0x4000u)&&state.words.template get<std::uint8_t>(0x80)){
            state.words.set(0x80,std::uint8_t{0});q.frame_set_hint(-1);
        }
    }
}
}
#endif
