#ifndef HD2_GAME_HUMAN_ACTIVE_AI_TASK_EXECUTION_H
#define HD2_GAME_HUMAN_ACTIVE_AI_TASK_EXECUTION_H
#include "game/human_ai_shot_direction.h"
#include <array>
namespace hd2::game {
template<class Properties>std::uint32_t active_task_door_phase(const Properties& properties){ //006029F0.
    return properties.flag234?4u:properties.fields220[1];
}
//005374A0. Raw switch tables5375F8/53765C recover arguments omitted by C.
template<class AI>void publish_active_task_second_mode(AI& ai,std::uint32_t action){
    constexpr std::array<int,39> modes{{15,16,17,18,19,20,21,-1,22,23,24,25,26,27,28,29,30,-1,32,33,-1,34,35,36,37,-1,-1,38,39,-1,-1,-1,-1,21,21,-1,-1,-1,17}};
    if(action<modes.size()&&modes[action]>=0)ai.pending_second_mode(static_cast<unsigned>(modes[action]));
}
//00535E70, selected Human/Door/generic target families. Engine inverse,
//transform and bounds calls remain actual frame endpoints, not synthetic AABBs.
template<class AI,class Actor,class Frames,class World>bool active_task_target_arrived(AI& ai,Actor* target,Frames& frames,World& world){
    if(!target)return false;
    const auto kind=target->base().type();
    if(kind==6){
        const auto* point=ai.point();if(!point)return false;
        auto* navigation=ai.navigation();if(!navigation)throw std::logic_error("Active Door arrival lost actual A0 navigation");
        const auto* first=navigation->record(0);if(!first)return false;
        const auto address=reinterpret_cast<std::uintptr_t>(point),begin=reinterpret_cast<std::uintptr_t>(first);
        if(address<begin)return false;
        const auto offset=address-begin;if(offset%sizeof(*first))return false;
        const auto index=offset/sizeof(*first);
        if(index>=0xffffu||navigation->record(static_cast<std::uint16_t>(index))!=point)return false;
        return navigation->object_for_reference(static_cast<std::uint16_t>(index))==target;
    }
    auto* human=ai.human();if(!human)throw std::logic_error("Active target arrival lost Human44");
    if(kind==3){
        const auto& own=frames.active_task_borrow_world_matrix(human->base().frame());
        const auto& other=frames.active_task_borrow_world_matrix(target->base().frame());
        const auto x=static_cast<long double>(other[12])-own[12],y=static_cast<long double>(other[13])-own[13],z=static_cast<long double>(other[14])-own[14];
        return ((z*z+y*y)+x*x)<2.25f; //No FLOAT spill; equality/unordered fail.
    }
    const auto model=world.active_task_arrival_frame(*target);if(!model)return false;
    const auto own=frames.actor_world_matrix(human->base().frame());
    const auto matrix=frames.actor_world_matrix(model);
    const auto inverse=frames.car_inverse_matrix(matrix);
    const auto local=frames.car_transform(HumanVector{{own[12],own[13],own[14],own[15]}},inverse);
    const auto bounds=frames.blast_frame_bounds(model); //Native driver+A8; dummy6 uses120.
    long double square=0;
    for(unsigned axis=0;axis!=3;++axis){
        const auto value=static_cast<long double>(local[axis]);long double delta=0;
        if(value<bounds.minimum[axis])delta=value-bounds.minimum[axis];
        else if(value>bounds.maximum[axis])delta=value-bounds.maximum[axis];
        square+=delta*delta; //Native ignores an unordered axis, rather than propagating it.
    }
    return square<2.25f;
}
//00528950,201-clear phase. The desired direction spills XYZ; the model's
//SECOND horizontal normalization retains X/Z products in x87 until the dot.
template<class AI>bool turn_and_start_active_task(AI& ai){
    auto& state=ai.state();HumanVector desired=state.template get<HumanVector>(0x160);
    if(state.template get<std::uint8_t>(0x209)){
        desired=state.template get<HumanVector>(0x1f0);state.set(0x160,desired);
    }else{
        ai.require_live_task_target();
        auto* human=ai.human();if(!human)throw std::logic_error("Active task turn lost Human44");
        const auto own=human->base().world_position();
        const auto destination=ai.task_target?ai.task_target->base().world_position():state.template get<HumanVector>(0x1e0);
        desired[0]=static_cast<float>(static_cast<long double>(destination[0])-own[0]);
        desired[2]=static_cast<float>(static_cast<long double>(destination[2])-own[2]);
        if(ai.task_target)desired[3]=own[3]; //Native528E9F copies Human cached W.
        //With NULL224 native temporary W is unwritten. Retain that typed lane
        //without a native-value claim; raw160 changes only in209's direct copy.
    }
    desired[1]=0;
    normalize_human_ai_shot(desired,static_cast<long double>(desired[0])*desired[0]+static_cast<long double>(desired[2])*desired[2]);
    ai.set_target_direction(desired);ai.set_current_direction(desired);
    auto forward=ai.pending_task_forward();
    normalize_human_ai_shot(forward,(static_cast<long double>(forward[0])*forward[0]+static_cast<long double>(forward[1])*forward[1])+static_cast<long double>(forward[2])*forward[2]);
    long double x=forward[0],z=forward[2];float y=0;
    constexpr float epsilon=9.99999993922529e-09f;
    const auto square=x*x+z*z;const float stored=static_cast<float>(square);
    if(!(std::abs(square-1)<epsilon)){
        if(!(stored<epsilon)){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));x*=inverse;z*=inverse;y=static_cast<float>(0.L*inverse);}
        else if(x!=0)x=x<0?-1:1;
        else if(z!=0)z=z<0?-1:1;
        else y=1;
    }
    const float dot=static_cast<float>((x*desired[0]+z*desired[2])+static_cast<long double>(y)*desired[1]);
    bool gate=ai.pending_task_turn_gate()!=0;
    if(gate&&(ai.resources().query().state.template get<std::uint32_t>(0x50)&0x10000000u))gate=false;
    if((!(dot>.95f)||gate)&&!state.template get<std::uint8_t>(0x207))return true;
    state.set(0x201,std::uint8_t{1});state.set(0x214,1500u);
    const auto flags=state.template get<std::uint32_t>(0x174);
    if(flags&0x200u)return true;
    if(flags&8u){
        auto* human=ai.human();if(!human)throw std::logic_error("Active ranged task lost Human44");
        const auto own=human->base().world_position(),destination=state.template get<HumanVector>(0x1e0);
        const auto dx=static_cast<long double>(destination[0])-own[0],dy=static_cast<long double>(destination[1])-own[1],dz=static_cast<long double>(destination[2])-own[2];
        if(!(((dz*dz+dy*dy)+dx*dx)>36.f)){state.set(0x206,std::uint8_t{1});ai.reject_pending_task();return true;}
        throw std::runtime_error("Active task turn requires actual00560750/005AC200 ranged command receiver");
    }
    ai.start_pending_task_effect();return true;
}
//00528950 after turn publication. Vehicle seat/interaction cases are kept at
//their reached H8.4 boundary; specialized task effects remain H8.2 work.
template<class AI>void start_active_task_effect(AI& ai){
    auto& state=ai.state();const auto action=state.template get<std::uint32_t>(0x220);
    if(action==3){
        ai.require_live_task_target();
        if(ai.task_target){const auto kind=ai.task_target->base().type();
            if(kind==4||kind==20||kind==21)throw std::runtime_error("Active task requires H8.4 attachment seat selection and approach");
        }
    }
    if(state.template get<std::uint8_t>(0x204)){
        if(action==32)return;
        state.set(0x214,0u);
        if(state.template get<std::uint8_t>(0x208))throw std::runtime_error("Active task requires actual Players00456F00 notification");
        return;
    }
    publish_active_task_second_mode(ai,action);
    if(!ai.pending_task_effect_ready()){ai.reject_pending_task();return;}
    if(action==6||action==23||action==24||action==33)
        throw std::runtime_error("Active task requires its specialized00537300/00537350 or item/handoff effect receiver");
    state.set(0x214,1500u);
    ai.execute_pending_task_command(action,state.template get<std::int32_t>(0x20c),0u);
    if(ai.words().template get<std::int32_t>(0x60)!=-1)state.set(0x214,ai.words().template get<std::uint32_t>(0x64));
}
} //namespace hd2::game
#endif
