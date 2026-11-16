#ifndef HD2_GAME_HUMAN_ACTIVE_AI_FRAME_POLICY_H
#define HD2_GAME_HUMAN_ACTIVE_AI_FRAME_POLICY_H
#include "game/human_active_ai_follow_frame.h"
#include "game/human_loading_stats.h"

namespace hd2::game {
//00537690/005376C0. Nonzero, including negative, timers subtract with DWORD
//wrap; equality expires. The mode word stays unchanged until expiry.
template<class Words>void expire_active_mode(Words& state,unsigned timer,unsigned mode,std::uint32_t elapsed){
    const auto old=state.template get<std::uint32_t>(timer);if(!old)return;
    const auto next=old-elapsed;state.set(timer,next);
    if(human_frame_signed_word(next)<=0){state.set(timer,0u);state.set(mode,0xffffffffu);}
}
//0052D460. The query movement mask is selected here, not in the path owner.
template<class AI,class World>void select_active_navigation_gait(AI& ai,bool following,World& world){
    auto& state=ai.state();auto* human=ai.human();if(!human)throw std::logic_error("Active gait lost Human44");
    const auto flags=state.template get<std::uint32_t>(0x174);
    auto slow=flags&2u?0x1000u:0x2000u,fast=flags&2u?0x1000u:0x4000u;
    if(state.template get<std::uint8_t>(0x188))slow=fast=0x1000u;
    if(following&&!(flags&1u))if(auto* leader=human->formation_leader){
        auto movement=0x1000u;const auto v=leader->activation.vector1b0;
        const auto square=(static_cast<long double>(v[0])*v[0]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[2])*v[2];
        if(square!=0)movement=world.active_follow_state(*leader).words().template get<std::uint32_t>(0x50)&0x1f000u;
        fast=movement&0x10000u?0x10000u:flags&2u?movement:0x4000u;
    }
    auto& query=ai.resources().query().state;const bool active=(query.template get<std::uint32_t>(0x50)&0x10000000u)!=0;
    auto movement=state.template get<std::uint32_t>(0x170);
    if(active){
        const auto a=human->base().world_position(),b=query.template get<HumanVector>(0x10);
        const auto x=static_cast<long double>(a[0])-b[0],y=static_cast<long double>(a[1])-b[1],z=static_cast<long double>(a[2])-b[2];
        const auto square=(z*z+y*y)+x*x; //No FLOAT spill before thresholds.
        const bool short_timer=state.template get<std::uint32_t>(0x1b0)!=0;
        if(square>(short_timer?2.25f:36.f))movement=fast;
        else if(square<(short_timer?.25f:9.f))movement=slow; //Middle/equality/unordered retains170.
    }
    if(state.template get<std::uint8_t>(0x202)){
        if(active&&!state.template get<std::uint8_t>(0x18c)){
            if((flags&1u)||!state.template get<std::uint8_t>(0x189))query.set(0x50,query.template get<std::uint32_t>(0x50)|0x2000u);
        }
        if(following&&!state.template get<std::uint8_t>(0x18c))if(auto* leader=human->formation_leader){
            //Native reads1C0 and computes a discarded stack value. It does not
            //publish leader speed or overwrite the selected ESI movement mask.
            const auto v=leader->activation.vector1c0;
            const auto z=static_cast<long double>(v[2])*v[2],x=static_cast<long double>(v[0])*v[0],y=static_cast<long double>(v[1])*v[1];
            if((y+z)+x!=0){const volatile float discarded=static_cast<float>(std::abs((z+x)-1.f));(void)discarded;}
        }
    }
    if(const auto override=state.template get<std::uint32_t>(0x180)){state.set(0x1b0,0u);movement=override;}
    if(human->activation.field14c==0&&movement==0x10000u)movement=0x4000u;
    if(human->field174&&(movement==0x10000u||movement==0x4000u))movement=0x2000u;
    if(state.template get<std::uint8_t>(0x244))movement=0x1000u;
    state.set(0x170,movement);query.set(0x54,(query.template get<std::uint32_t>(0x54)&0xfffe0fffu)|movement);
}
//005325C0. First order0C is sampled BEFORE refresh_item. Direction preset
//publication follows actual name lookup/debug calls, even though debug is RET.
template<class AI,class Frames,class World>void select_active_direction_preset(AI& ai,Frames& frames,World& world){
    auto& state=ai.state();auto& orders=ai.pending_orders();
    const auto order_kind=orders.empty()?0u:ai.first_pending_order().state.template get<std::uint32_t>(0xc);
    ai.refresh_item();auto* human=ai.human();if(!human)throw std::logic_error("Active preset lost Human44");
    float distance=0;
    if(state.template get<std::uint8_t>(0x189)&&state.template get<std::uint8_t>(0x202))if(auto* leader=human->formation_leader)if(auto* formation=leader->formation.get()){
        if(!formation->leader()||!formation->has_slots()||!formation->contains(human))
            throw std::runtime_error("Active preset requires actual formation output; native member position is otherwise unwritten");
        const auto validate=world.active_follow_guard(*human,*leader,formation);
        HumanVector position;(void)formation->consume_member_position(human,position,nullptr);
        const auto model=frames.actor_world_matrix(human->base().frame());validate();
        const auto x=static_cast<long double>(model[12])-position[0];auto y=static_cast<long double>(model[13])-position[1];
        const float z=static_cast<float>(static_cast<long double>(model[14])-position[2]);
        if(std::abs(y)<5.)y=0;
        distance=static_cast<float>((static_cast<long double>(z)*z+x*x)+y*y);
    }
    const auto choose=[&]{
        if(!ai.item()||world.active_follow_floor(*human))return 1u;
        if(!(ai.resources().query().state.template get<std::uint32_t>(0x50)&0x10000000u))return 4u;
        if(active_follow_needs_standing(ai,world))return 1u;
        const auto task=state.template get<std::uint32_t>(0x220);
        if(!state.template get<std::uint8_t>(0x202)&&task!=40&&task!=42&&task!=37&&task!=32)return 1u;
        if(state.template get<std::uint8_t>(0x189)&&(state.template get<std::uint32_t>(0x1b0)||distance>49.f))return 1u;
        if(order_kind==1){
            if(ai.words().template get<std::uint32_t>(0x50)&0x400u)return 1u;
            if(orders.empty())throw std::logic_error("Active preset lost first actual order after refresh");
            auto& flags=ai.first_pending_order().state;auto movement=flags.template get<std::uint32_t>(0x44);
            if(!(movement&0x1f000u)){movement|=0x4000u;flags.set(0x44,movement);}
            if(movement&0x10000u)return 1u;
        }
        if(state.template get<std::uint32_t>(0x174)&0x239u)return 4u;
        const auto& target=ai.resources().target();const auto v=target.state.template get<HumanVector>(0x10);
        const auto square=(static_cast<long double>(v[0])*v[0]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[2])*v[2];
        return !target.actor&&square<1.0000000168623835e-16f&&!state.template get<std::uint32_t>(0x198)?1u:4u;
    };
    const auto preset=choose();auto& flag=ai.resources().flag();
    if(flag.state.template get<std::uint32_t>(0)==preset)return;
    world.active_task_completion_debug_name(*human);flag.request(preset);
}
//00526400. ENGINE RandFloat is separate from CRT rand, and runs only AFTER
//the immediate common command returns. Native return values are ignored.
template<class AI,class World>void advance_active_self_treatment(AI& ai,World& world){
    auto& state=ai.state();if(state.template get<std::uint32_t>(0x234))return;
    if((ai.words().template get<std::uint32_t>(0x50)&1u)||human_frame_signed_word(ai.words().template get<std::uint32_t>(0xb0))<5000||
            (state.template get<std::uint32_t>(0x220)!=52&&!state.template get<std::uint8_t>(0x206))){state.set(0x234,2000u);return;}
    auto* human=ai.human();if(human&&human->field174)return;
    if(!human)throw std::runtime_error("Active self-treatment requires Human44 for actual stat7 query");
    const auto skill=human_frame_signed_word(human->properties.stats_ec[7]);
    if(skill<10){state.set(0x234,60000u);return;}
    if(world.active_self_treatment_health(*human)>=95){state.set(0x234,5000u);return;}
    const auto draw=world.human_frame_random_word();
    const auto inverse=human_frame_signed_word(100u-static_cast<std::uint32_t>(skill));
    const auto divisor=inverse/2+10;if(!divisor)throw std::runtime_error("Active self-treatment reached native zero IDIV divisor");
    if(static_cast<std::int32_t>(draw)%divisor)return;
    const auto identity=world.active_self_treatment_item(*human);if(!identity)return;
    const auto validate=world.human_common_receiver_guard(*human);
    world.execute_active_task_command(ai,35u,*identity,0u);validate();
    const auto random=world.active_self_treatment_random();validate();
    const auto base=human_frame_signed_word(static_cast<std::uint32_t>(inverse)*200u);
    state.set(0x234,human_loading_stat_word(static_cast<long double>(random)*4000.f+base));
}
} //namespace hd2::game
#endif
