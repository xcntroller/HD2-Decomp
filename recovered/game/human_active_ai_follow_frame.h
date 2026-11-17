#ifndef HD2_GAME_HUMAN_ACTIVE_AI_FOLLOW_FRAME_H
#define HD2_GAME_HUMAN_ACTIVE_AI_FOLLOW_FRAME_H
#include "game/human_state_frame_timers.h"
#include "game/human_joint_owners.h"
#include "game/human_active_ai_entry.h"
#include <cmath>
namespace hd2::game {
//005324D0. FIRST queued F8 job, not the current104 job.6 maps to job5.
template<class AI,class World>bool active_follow_needs_standing(AI& ai,World& world){
    if(!(ai.resources().query().state.template get<std::uint32_t>(0x50)&0x10000000u))return false;
    const auto& jobs=ai.resources().navigation_jobs();if(jobs.empty())return false;
    if(world.active_follow_floor(*ai.human()))return false;
    const auto* point=ai.point();if(!point)return false;
    const auto mode=point->byte16;if(mode!=1&&mode!=2&&mode!=3&&mode!=4&&mode!=6)return false;
    if(!jobs.front())throw std::logic_error("Active follow posture lost first queued F8 job");
    return jobs.front()->words.template get<std::uint8_t>(0x26)==(mode==6?5:mode);
}
//00536120 on foot. Its caller ignores EAX; pointer/quotient returns from the
//native void-like worker are not reconstructed as usable return values.
template<class AI,class Frames,class World>void advance_human_active_follow(AI& ai,Frames& frames,World& world){
    auto& state=ai.state();
    if(!state.template get<std::uint8_t>(0x189)){
        if(state.template get<std::uint32_t>(0x198)||state.template get<std::uint32_t>(0x19c))return;
        const auto& query=ai.resources().query().state;
        if(query.template get<std::uint32_t>(0x50)&0x10000000u)return;
        auto* human=ai.human();if(!human)throw std::logic_error("Active idle direction lost Human44");
        if(!human->perception.lists()[0].empty())return;
        const auto desired=state.template get<HumanVector>(0x160),current=world.active_follow_current_direction(*human);
        const auto dot=(static_cast<long double>(desired[2])*current[2]+static_cast<long double>(desired[1])*current[1])+static_cast<long double>(desired[0])*current[0];
        if(!(dot<.995f))return; //Native unordered also returns before rand.
        if(world.human_frame_random_word()%20u==0){ai.set_target_direction(desired);ai.set_current_direction(desired);}
        return;
    }
    ai.refresh_item(); //00584830 BEFORE leader/null gate.
    auto* human=ai.human();if(!human)throw std::logic_error("Active follow lost Human44");
    auto* leader=human->formation_leader;if(!leader)return;
    auto* formation=leader->formation.get();
    if(!formation||!formation->leader()||!formation->has_slots()||!formation->contains(human))
        throw std::runtime_error("Active follow requires actual leader2C8/member position; native output is otherwise unwritten");
    const auto validate=world.active_follow_guard(*human,*leader,formation);
    HumanVector position;std::uint16_t point=0xffff;
    (void)formation->consume_member_position(human,position,&point); //Outputs even when changed88 is false.
    const auto model=frames.actor_world_matrix(human->base().frame());validate();
    const auto x=static_cast<long double>(model[12])-position[0];auto y=static_cast<long double>(model[13])-position[1];
    const float z=static_cast<float>(static_cast<long double>(model[14])-position[2]); //Only Z spills before square.
    if(std::abs(y)<5.)y=0;
    const float distance=static_cast<float>((x*x+static_cast<long double>(z)*z)+y*y);
    if(advance_active_follow_attachment(ai,*formation,*leader,distance,frames,world,validate))return;
    auto& query=ai.resources().query().state;
    bool reroute=false;
    if(!(query.template get<std::uint32_t>(0x50)&0x10000000u)){
        if(distance<.25f){
            auto direction=state.template get<HumanVector>(0x160);
            if(formation->member_direction(human,direction))state.set(0x160,direction); //Actual slot40, not50.
        }
    }else{
        const auto destination=query.template get<HumanVector>(0x10);
        const auto dx=static_cast<long double>(destination[0])-position[0],dy=static_cast<long double>(destination[1])-position[1],dz=static_cast<long double>(destination[2])-position[2];
        const auto square=(dx*dx+dz*dz)+dy*dy; //Extended through both comparisons.
        if(leader->field160)reroute=square>36.f;
        else{
            const auto velocity=leader->activation.vector1b0;
            const auto speed=(static_cast<long double>(velocity[0])*velocity[0]+static_cast<long double>(velocity[1])*velocity[1])+static_cast<long double>(velocity[2])*velocity[2];
            reroute=(!(speed<1.0000000168623835e-16f)||!(square>.25f))?square>36.f:true;
        }
    }
    if(distance>.25f&&(!state.template get<std::uint32_t>(0x1a4)||reroute)){
        if(world.active_follow_floor(*human))ai.resources().query().request_position(position);
        else{
            const auto root=world.active_follow_root_sector();
            if(human_frame_signed_word(state.template get<std::uint32_t>(0x1a8))>=0){
                if(world.active_follow_sector(*leader)!=root)ai.resources().query().request_point_position(point,position);
                else{
                    auto raised=position;raised[1]=static_cast<float>(static_cast<long double>(raised[1])+1.f);
                    const auto sector=world.active_follow_probe_sector(raised);validate();
                    if(sector==root||world.active_follow_sector(*human)!=root)ai.resources().query().request_point_position(point,position);
                }
            }
        }
        state.set(0x1a4,2200u);world.active_follow_query_changed();validate(); //Players virtual20=00459490, NOT00456F00.
    }
    if(active_follow_needs_standing(ai,world)){ai.pending_posture(4);validate();}
    else{
        const auto own=ai.words().template get<std::uint32_t>(0x50)&7u;
        const auto other=world.active_follow_state(*leader).words().template get<std::uint32_t>(0x50)&7u;
        if((!state.template get<std::uint32_t>(0x1a8)||(query.template get<std::uint32_t>(0x50)&0x10000000u))&&
                !state.template get<std::uint8_t>(0x18b)&&!ai.has_pending_orders_loading()&&own!=other){
            const auto draw=world.human_frame_random_word(); //Before1A8 and174 tests, even if1A8 is nonzero.
            if((draw%10u==0||state.template get<std::uint32_t>(0x1a8))&&(!(state.template get<std::uint32_t>(0x174)&2u)||other<own)){
                const auto a=leader->base().world_position(),b=human->base().world_position();
                const auto dx=static_cast<long double>(a[0])-b[0],dy=static_cast<long double>(a[1])-b[1],dz=static_cast<long double>(a[2])-b[2];
                const auto square=(dz*dz+dy*dy)+dx*dx;
                if(!query.template get<std::uint8_t>(0x58)&&square<1225.f){
                    const bool nearby=square<225.f;
                    if(nearby||world.active_follow_force_sight(*human,*leader)){validate();ai.pending_posture(other==1?1u:other==2?2u:4u);validate();}
                    else validate();
                }
            }
        }
    }
    if(!state.template get<std::uint32_t>(0x19c))return;
    auto& other=world.active_follow_state(*leader);
    if(!other.item()||ai.words().template get<std::uint8_t>(0xad)||!other.words().template get<std::uint8_t>(0xad))return;
    if(world.human_frame_random_word()%6u!=0||!ai.item())return;
    const auto command=world.active_follow_item_command(ai.item());
    if(command==3){
        const auto movement=query.template get<std::uint32_t>(0x54)&0x1f000u;
        if((query.template get<std::uint32_t>(0x50)&0x10000000u)&&movement>0x1000u&&movement!=0x8000u){
            ai.resources().target().bind_actor(nullptr);
            if(ai.words().template get<std::uint8_t>(0xac))world.clear_script_formation_item_ready(ai,world.human_inline(*human));
            return;
        }
    }else if(world.active_follow_item_kind(ai.item())==5)return;
    if(human->properties.field19c!=5)world.active_follow_ready_item(ai);
}
} //namespace hd2::game
#endif
