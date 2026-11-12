#ifndef HD2_GAME_SESSION_FRAME_LOADING_H
#define HD2_GAME_SESSION_FRAME_LOADING_H
#include "game/session_activation.h"
#include "game/game_music_references.h"
#include "game/session_frame_input.h"
namespace hd2::game {
inline std::int32_t session_frame_signed(std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;}

//0069A500 offline branch: argument100 is unused, actual500/1000 counters.
template<class Services>void begin_loading_session_exit(SessionActivationState& state,Services& services){
    if(state.status==6)return;
    state.status=6;state.words.set(0x20,std::uint32_t{500});state.words.set(0x24,std::uint32_t{1000});
    if((services.mode_flags()&7u)!=7u)services.finish_nonoffline_session_exit();
}
template<class Services>bool finish_loading_objective_delay(SessionActivationState& state,Services& services){ //0069B230/0069B558.
    if(mission_frame_input_allowed(state.mission)&&!state.words.get<std::uint8_t>(0x6f))return false;
    state.words.set(0x88,std::uint32_t{0});
    if(!services.is_current_application_state())return true;
    if(services.application_saved_state_present()||state.words.get<std::uint8_t>(0x6f)){state.status=14;return true;}
    begin_loading_session_exit(state,services);
    if(services.current_player_available())services.objective_exit_camera();
    return true;
}

//0069A560: native initial500ms/fade2500 gates precede HUD or any input access.
//The positive containing body uses actual required UI/save/player consumers.
template<class Services>bool input_loading_session(SessionActivationState& state,Services& services,
        std::uint32_t* first,std::uint32_t* second){
    if(session_frame_signed(state.words.get<std::uint32_t>(0x20))>0)return true;
    if(state.words.get<std::uint32_t>(0x38))return true;
    return process_session_frame_input(state,services,first,second);
}

//0069AF10's offline7 fixed path. Called even during status5 warming. Original
//module+0C pass is ASCENDING, not the reverse finish pass. Shared transient
//records update006DF390(elapsed,false) is required, SAME process as finish.
template<class Services>void fixed_loading_session(SessionActivationState& state,Services& services,std::uint32_t elapsed){
    if(services.mode_flags()&2u){
        services.filter_hud_input(nullptr,nullptr);
        if(services.mode_flags()&2u)services.players_control_fixed(elapsed);
    }
    if((services.mode_flags()&7u)!=7u)throw std::runtime_error("Fixed session frame requires actual nonoffline match/peer consumers");
    if(!state.words.get<std::uint8_t>(0x64)){
        state.words.set(0x28,state.words.get<std::uint32_t>(0x28)+elapsed);
        if(!(services.mode_flags()&2u)){
            if(!state.words.get<std::uint32_t>(0x88)){
                state.words.set(0x30,state.words.get<std::uint32_t>(0x30)+elapsed);
                state.words.set(0x7c,state.words.get<std::uint32_t>(0x7c)+elapsed);
            }
            state.words.set(0x2c,state.words.get<std::uint32_t>(0x2c)+elapsed);
        }
        services.tick_authority_modules(elapsed);
        services.advance_shared_transient_records(elapsed);
        if(!(services.mode_flags()&2u)){
            services.advance_engine_scene(elapsed);services.advance_animated_models(elapsed);
        }
    }
}

//0069B230 ordinary offline7 coordinator. Services are actual registered module,
//HUD/Players/FPV/engine/audio/text/particle owners, not readiness placeholders.
//Music D0 is the SAME GameMusicReferences countdown, not a second wordsD0.
template<class Services>void update_loading_session(SessionActivationState& state,GameMusicReferences& music,Services& services,std::uint32_t elapsed){
    if(services.mode_flags()&2u)services.filter_hud_input(nullptr,nullptr);
    const auto subtract=[&](std::uint32_t word){const auto value=word-elapsed;return session_frame_signed(value)<0?0u:value;};
    if(session_frame_signed(state.words.get<std::uint32_t>(0x38))>0)state.words.set(0x38,subtract(state.words.get<std::uint32_t>(0x38)));
    if(music.countdown)music.countdown=subtract(music.countdown); //Native tests nonzero, not signed>0.
    if((services.mode_flags()&7u)!=7u)throw std::runtime_error("Session frame requires actual nonoffline clock/match/control consumers");
    if(state.words.get<std::uint32_t>(0x88)==1&&state.words.get<std::uint32_t>(0x8c)+4000u<state.words.get<std::uint32_t>(0x28))
        if(services.finish_offline_objective_delay())return; //Actual positive result/cutscene/controller receiver is required.
    services.recycle_collision_buckets(); //006EF330 even while game64 is set.
    if(!(services.mode_flags()&2u)){
        if(state.status==5)state.status=1;
        services.advance_authority_only_humans(elapsed);return;
    }
    if(services.is_current_application_state()){
        if(services.hud_enabled())services.update_loaded_hud(elapsed);
        else services.synchronize_selected_camera_direction();
    }
    if(!state.words.get<std::uint8_t>(0x64)&&(services.mode_flags()&2u)){
        if(!state.words.get<std::uint32_t>(0x88)){
            state.words.set(0x30,state.words.get<std::uint32_t>(0x30)+elapsed);
            state.words.set(0x7c,state.words.get<std::uint32_t>(0x7c)+elapsed);
        }
        state.words.set(0x2c,state.words.get<std::uint32_t>(0x2c)+elapsed);
        if(!(services.mode_flags()&1u))state.words.set(0x28,state.words.get<std::uint32_t>(0x28)+elapsed);
    }
    //Transition only when the counter was already zero at ENTRY to this gate;
    //a subtraction reaching zero keeps status5 until the following update.
    if(!state.words.get<std::uint32_t>(0x24)){
        if(!state.words.get<std::uint32_t>(0x20)){
            if(state.status==5){state.status=1;if(!services.full_viewport())services.show_loaded_hud(true);}
            else if(state.status==6)state.status=(services.mode_flags()&7u)==7u?(state.words.get<std::uint32_t>(0x1c)==2?10u:2u):10u;
        }else state.words.set(0x20,subtract(state.words.get<std::uint32_t>(0x20)));
    }else state.words.set(0x24,subtract(state.words.get<std::uint32_t>(0x24)));
    services.players_control_presentation(elapsed);
    if(!state.words.get<std::uint8_t>(0x64)){
        services.tick_presentation_modules(elapsed);
        services.advance_first_person(elapsed);
        services.advance_engine_scene(elapsed);
        services.advance_animated_models(elapsed); //REVERSE actual game48 references.
        services.advance_dubbing(elapsed);
        services.synchronize_dubbing_attachments();
        if(state.words.get<std::uint8_t>(0xb0)){state.status=12;state.words.set(0xb0,std::uint8_t{0});} //Native virtual24=0067EA10 actual status setter.
        services.advance_mission_text(elapsed);
        services.advance_car_particles(elapsed);
    }else{
        services.advance_camera(elapsed);
        services.advance_paused_mission_particles(elapsed);
        services.advance_mission_text(elapsed);
    }
}
} //namespace hd2::game
#endif
