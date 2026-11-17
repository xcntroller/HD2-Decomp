#ifndef HD2_GAME_HUMAN_ACTIVE_AI_AUTHORITY_PREFIX_H
#define HD2_GAME_HUMAN_ACTIVE_AI_AUTHORITY_PREFIX_H
#include "game/human_state_frame_timers.h"
#include "game/human_joint_owners.h"
#include <optional>
#include <utility>
namespace hd2::game {
//00478AE0. Actual registry+10 owner sequence, not active+20. Radius is a
//wrapped signed integer square, converted to FLOAT before extended comparison.
template<class Registry,class Frames>bool active_nearby_runtime_actor(Registry& registry,Frames& frames,const HumanVector& position,std::uint32_t radius){
    const float limit=static_cast<float>(human_frame_signed_word(radius*radius));
    return registry.any_owned_reverse([&](auto& actor){
        if(actor.base().type()!=2)return false;
        const auto model=frames.actor_world_matrix(actor.base().frame());
        const auto x=static_cast<long double>(position[0])-model[12],y=static_cast<long double>(position[1])-model[13],z=static_cast<long double>(position[2])-model[14];
        return ((z*z+x*x)+y*y)<limit; //Equality/unordered fail; no FLOAT distance spill.
    });
}
//00526550 through005376C0. nullopt is the native bit800 return; the bool is
//the original stack follow flag for the still-required containing body.
template<class AI,class Queries,class World>std::optional<bool> begin_active_authority_frame(AI& ai,std::uint32_t elapsed,Queries& queries,World& world){
    if(ai.words().template get<std::uint32_t>(0x50)&0x800u)return std::nullopt;
    auto* human=ai.human();if(!human)throw std::logic_error("Active authority lost actual Human44");
    const auto validate=world.human_common_receiver_guard(*human);
    ai.advance_frame_timers(elapsed,queries);validate();
    auto& state=ai.state();const auto nearby=state.template get<std::uint32_t>(0x248);
    if(nearby){
        const auto next=nearby-elapsed;state.set(0x248,next);
        if(human_frame_signed_word(next)<=0){
            state.set(0x248,250u);const auto position=human->base().world_position();
            const auto found=world.active_authority_nearby(position);validate();state.set(0x244,std::uint8_t(found));
        }
    }
    const auto decrement=[&](unsigned offset,bool equality){
        const auto old=state.template get<std::uint32_t>(offset);if(!old)return;
        const auto next=old-elapsed;state.set(offset,next);
        if(equality?human_frame_signed_word(next)<=0:human_frame_signed_word(next)<0)state.set(offset,0u);
    };
    decrement(0x234,false);
    const auto old=state.template get<float>(0x1c4);
    if(old!=0){
        const auto next=static_cast<long double>(old)-static_cast<long double>(elapsed)*.1f;
        state.set(0x1c4,static_cast<float>(next));if(next<0)state.set(0x1c4,0.f);
    }
    if(state.template get<std::uint32_t>(0x1a4)&&!(ai.resources().query().state.template get<std::uint32_t>(0x50)&0x10000000u))decrement(0x1a4,false);
    decrement(0x1b0,true);
    auto& inputs=queries.human_ai_frame_inputs();inputs.clear();ai.bind_frame_inputs(inputs.first,inputs.second);
    bool following=false;
    if(state.template get<std::uint32_t>(0x220)==32&&!state.template get<std::uint8_t>(0x206)){
        (void)ai.advance_pending_task(elapsed);validate();ai.advance_follow_frame();validate();following=true;
    }else if(!ai.process_pending_orders(elapsed)){
        validate();if(!ai.advance_pending_task(elapsed)){validate();ai.advance_follow_frame();validate();following=true;}else validate();
    }else validate();
    ai.drain_frame_events();validate();ai.expire_frame_modes(elapsed);return following;
}
} //namespace hd2::game
#endif
