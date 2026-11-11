#ifndef HD2_GAME_HUMAN_STATE_ACTION_COMPLETION_H
#define HD2_GAME_HUMAN_STATE_ACTION_COMPLETION_H

#include <cstdint>
#include <stdexcept>

namespace hd2::game {

//00584E30, shared B0 virtual0C. Complete against the existing Human, E0,
//runtime actor and inventory owners. Required positive effects run before the
//action slots are cleared. The stored timer is the caller's responsibility.
template<bool may_succeed=true,class State,class World,class Runtime>void complete_human_state_action(State& state,bool success,
        World& world,Runtime& runtime){
    auto* human=state.human();
    if(!human)throw std::logic_error("Human action completion requires its bound B0");
    state.refresh_item();
    auto& fields=human->activation;
    fields.field17c=fields.field180=0;
    fields.field184=0xffffffffu;
    auto& words=state.words();
    auto* target=state.action_target();
    const bool authority_success=success&&(world.mode_flags()&1u);
    (void)authority_success;(void)runtime; //Death cancellation instantiates only the native failure path.
    if constexpr(!may_succeed)if(success)throw std::logic_error("Human cancellation cannot complete a successful action");
    const auto command=[&](std::uint32_t subtype,typename State::ActionTarget* actor){
        world.human_action_completion_command(*human,subtype,actor);
    };
    switch(words.template get<std::uint32_t>(0x60)){
    case 2:
        command(0x15,target);
        if constexpr(may_succeed)if(authority_success&&target)runtime.finish_human_runtime_pickup(*target);
        break;
    case 3:
        if constexpr(may_succeed)if(success&&target&&target->base().type()==15)runtime.finish_human_timed_type15(*target,*human);
        break;
    case 4:
        if constexpr(may_succeed)if(authority_success){
            if(!target)throw std::logic_error("Human explosive placement requires the actual target");
            runtime.finish_human_explosive_placement(*target,*human);
        }
        command(0x17,target);
        break;
    case 0x0e:
        command(0x13,target);
        if(authority_success&&target)world.finish_human_lockpick(*target,*human);
        break;
    case 0x0f:
        command(0x0f,target);command(0x11,human);
        if constexpr(may_succeed)if(authority_success&&target)runtime.finish_human_treatment(state,target->base().id10(),-1);
        break;
    case 0x10:
        command(0x1d,target);
        if constexpr(may_succeed)if(authority_success&&target)runtime.finish_human_actor_motion(*target,state);
        break;
    case 0x11:
        if constexpr(may_succeed)if(authority_success&&state.item())runtime.finish_human_action_drop(state,*human);
        command(0x17,target);state.refresh_item();
        break;
    case 0x13:
        command(0x19,target);
        if(target)command(0x1b,human);
        if(success)world.finish_human_action_unavailability(state,*human);
        break;
    case 0x23:
        command(0x25,human);
        if constexpr(may_succeed)if(authority_success&&target)runtime.finish_human_treatment(state,human->base().id10(),
            words.template get<std::int32_t>(0x6c));
        break;
    case 0x24:
        if(success&&target){command(0x1f,target);command(0x21,human);}
        break;
    case 0x26:
        command(0x15,target);
        if(authority_success&&target)world.finish_human_explosive_disarm(*target);
        break;
    }
    state.action_target(nullptr);
    for(auto offset:{0x60u,0x64u,0x6cu})words.set(offset,std::int32_t{-1});
}

} //namespace hd2::game
#endif
