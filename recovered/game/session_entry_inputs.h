#ifndef HD2_GAME_SESSION_ENTRY_INPUTS_H
#define HD2_GAME_SESSION_ENTRY_INPUTS_H
namespace hd2::game {
//00724110, received command0 prefix. This is destructive manager REPLACEMENT,
//before old-world retirement, unlike status9's0072E890 record clear.
template<class State,class Backend>void begin_received_session_inputs(State& state,Backend& backend){
    state.word5044=1;
    if(state.server4e00.flag88&&backend.session_voice_enabled())backend.create_session_voice(false);
    backend.replace_game_human_inputs();
}
}
#endif
