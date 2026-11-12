#ifndef HD2_GAME_SESSION_TRANSMIT_TARGETS_H
#define HD2_GAME_SESSION_TRANSMIT_TARGETS_H
#include "frontend/control_startup.h"
#include <array>
namespace hd2::game {
//00724570. The external interface belongs to actual process wrapper0C. Side
//queries may mutate records; never keep a record/vector pointer over a callback.
template<class Runtime>void update_session_transmit_targets(frontend::ControlStartupState& state,Runtime& runtime){
    if(!state.server4e00.flag88&&!(runtime.mode_flags()&1u))return;
    const auto local=static_cast<std::uint32_t>(runtime.human_current_player_id());
    if(local>=state.records.size())throw std::logic_error("Transmit targets require the actual local peer record");
    auto* human=state.records[local].human_actor();
    if(!human||!state.wrapper0c)return;
    const auto side=runtime.human_transmit_side(human);
    //0069BFE0 is a genuine RET. Its debug strings do not copy/dereference names.
    std::array<std::uint32_t,16> targets{};std::size_t count=0;
    {
        struct Lock{Runtime& r;explicit Lock(Runtime& value):r(value){r.enter_control_lock(0);}~Lock(){r.leave_control_lock(0);}} lock(runtime);
        for(std::size_t n=0;n<state.records.size();++n){
            if(!state.records[n].word(0))continue;
            human=state.records[n].human_actor();if(!human)continue;
            if(runtime.human_transmit_side(human)==side){
                //Native local44 has sixteen target slots; reject its overflow at
                //the reached append, preserving unlock and withholding submission.
                if(count==targets.size())throw std::length_error("Transmit targets exceed native sixteen-slot storage");
                targets[count++]=state.records.at(n).word(0);
            }
        }
    }
    if(!state.wrapper0c||!state.wrapper0c->object)throw std::logic_error("Transmit targets lost the actual wrapper interface");
    runtime.submit_human_transmit_targets(state.wrapper0c->object,count?targets.data():nullptr,static_cast<std::uint32_t>(count));
}
} //namespace hd2::game
#endif
