#ifndef HD2_GAME_SESSION_FRAME_INPUT_H
#define HD2_GAME_SESSION_FRAME_INPUT_H

#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace hd2::game {
//Same missionA8.related/native34 descriptors used by0069A560,0045EF00 and
//0069B230. All4 and an empty vector admit; otherwise an actual0 is required.
template<class Mission>bool mission_frame_input_allowed(const Mission* mission){
    if(!mission)return true;
    bool all_four=true;
    for(const auto& objective:mission->related){
        if(!objective)throw std::logic_error("Session frame lost an actual objective descriptor");
        if(objective->word10!=4){all_four=false;break;}
    }
    if(all_four)return true;
    for(const auto& objective:mission->related){
        if(!objective)throw std::logic_error("Session frame lost an actual objective descriptor");
        if(!objective->word10)return true;
    }
    return false;
}
inline bool session_input_active(const std::uint32_t* input,unsigned index){
    float value;std::memcpy(&value,input+index,4);
    return value!=0.f; //Native TEST AH,44 parity: unordered is active too.
}

//0069A560 positive body after20/38 gates. Services must supply actual UI,
//controller, player input and process-neutral arrays. No copied/polled inputs,
//replacement camera, successful missing UI or save/activation operation.
template<class State,class Services>bool process_session_frame_input(State& state,Services& services,
        std::uint32_t* first,std::uint32_t* second){
    if(services.mode_flags()&2u)services.filter_hud_input(first,second); //0049FC10.
    if(second&&services.is_current_application_state()&&(services.mode_flags()&7u)==7u&&
            state.words.template get<std::uint8_t>(0x6e)){
        if(!session_input_active(second,0x2f)||(state.mission&&services.mission_save_blocked())){
            if(session_input_active(second,0x30)&&state.words.template get<std::uint8_t>(0xc))
                services.request_application_state(3);
        }else{
            if(services.session_difficulty()==4)services.show_session_warning(0x19c7,false);
            else if(services.session_difficulty()==3&&state.words.template get<std::int32_t>(0x10)>=3)
                services.show_session_warning(0x2536,true); //00405100(format,0).
            else{
                const auto root=services.application_root();
                const auto ordinal=services.selected_mission_ordinal(root); //Even when root!=0.
                if(root||ordinal)services.request_application_state(2);
            }
        }
    }
    if((services.mode_flags()&7u)!=7u&&!services.hud_nonoffline_input_blocked())
        services.process_nonoffline_session_input(first,second); //0069AAB0, required non7 consumer.
    if(!state.words.template get<std::uint8_t>(0x6e)||!first){
        if(second&&session_input_active(second,0x2e)&&services.full_viewport()){
            services.end_session_cutscene();return true; //00423EB0 BEFORE neutral substitution.
        }
        first=services.neutral_first_input();second=services.neutral_second_input();
    }else if(!second){first=services.neutral_first_input();second=services.neutral_second_input();}
    if((services.mode_flags()&7u)==7u){
        if(state.status==1&&mission_frame_input_allowed(state.mission)){
            if(first&&second)services.process_hud_controls(first,second); //0049C3C0.
            if(!services.selected_player_available()||state.words.template get<std::uint8_t>(0x64)){
                if(!second)return true;
                if(session_input_active(second,0x1a))services.toggle_session_first_person();
            }else{
                const bool blocked=services.hud_player_input_blocked()||state.words.template get<std::uint32_t>(0x88)!=0;
                //Resolve selected player AGAIN in the consumer, after UI calls.
                services.process_selected_player_input(blocked?nullptr:first,blocked?nullptr:second);
            }
        }
    }else if((services.mode_flags()&2u)&&state.status==1){
        if(first&&second)services.process_hud_controls(first,second);
        if(auto* player=services.network_input_player()){
            const bool blocked=services.hud_player_input_blocked()||state.words.template get<std::uint32_t>(0x88)!=0;
            services.process_network_player_input(*player,blocked?nullptr:first,blocked?nullptr:second);
        }
        if(!second)return true;
        if(session_input_active(second,0x3e)){state.status=17;return true;}
    }
    if(second&&session_input_active(second,0x2e))state.status=14;
    return true;
}
} //namespace hd2::game
#endif
