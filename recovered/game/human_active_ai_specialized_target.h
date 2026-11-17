#ifndef HD2_GAME_HUMAN_ACTIVE_AI_SPECIALIZED_TARGET_H
#define HD2_GAME_HUMAN_ACTIVE_AI_SPECIALIZED_TARGET_H
#include "game/human_active_ai_commands.h"
#include "game/mission_navigation.h"
#include <string>
namespace hd2::game {
inline long double active_specialized_square(const HumanVector& a,const HumanVector& b){
    const auto x=static_cast<long double>(a[0])-b[0],y=static_cast<long double>(a[1])-b[1],z=static_cast<long double>(a[2])-b[2];
    return (z*z+y*y)+x*x;
}
inline float active_specialized_range(std::uint32_t kind){ //0052D0D0 native kind table.
    switch(kind){
    case 0:case 1:return 1.440000057220459f;
    case 2:return 225.f;case 3:return 1600.f;case 4:case 12:return 22500.f;
    case 5:return 4900.f;case 6:return 2500.f;case 9:return 400.f;
    default:return 625.f;
    }
}
//0052D0D0 AFTER actual005710B0 selection. This may return a generic task
//actor, so retaining Human* as its return type loses the native target family.
template<class AI,class Actor,class Frames,class World>Actor* refine_active_task_target(AI& ai,Actor* selected,Frames& frames,World& world){
    auto& state=ai.state();const auto action=state.template get<std::uint32_t>(0x220);
    if(state.template get<std::uint8_t>(0x202)||(action!=25&&action!=26))return selected;
    ai.require_live_task_target();auto* target=ai.task_target;if(!target)throw std::runtime_error("Active specialized selection dereferences native NULL224");
    const auto validate=world.human_common_receiver_guard(*ai.human());
    const bool hostile=world.active_event_hostile(*ai.human(),target);
    const float task_square=static_cast<float>(active_specialized_square(ai.human()->base().world_position(),target->base().world_position()));
    const bool in_range=task_square<active_specialized_range(world.active_follow_item_kind(ai.item()));
    auto& query=ai.resources().query().state;
    if(query.template get<std::uint32_t>(0x50)&0x10000000u){
        if(in_range){
            if(!hostile)return selected;
            query.set(0x50,query.template get<std::uint32_t>(0x50)|0x2000u);state.set(0x200,std::uint8_t{1});
        }
    }else if(!in_range&&!world.active_task_actor_unavailable(*target)&&!state.template get<std::uint32_t>(0x210)){
        const auto model=frames.actor_world_matrix(target->base().frame());validate();const auto position=query.template get<HumanVector>(0x10);
        if(active_specialized_square({{model[12],model[13],model[14],model[15]}},position)<.04000000283122063f){
            state.set(0x206,std::uint8_t{1});start_active_target_task(ai,*target,action,false,1.f,false,frames,world);validate();
            state.set(0x204,std::uint8_t{1});
        }
    }
    if(!hostile)return selected;
    if(!selected)return target;
    const auto own=frames.actor_world_matrix(ai.human()->base().frame());validate();
    const auto other=frames.actor_world_matrix(selected->base().frame());validate();
    const float square=static_cast<float>(active_specialized_square({{other[12],other[13],other[14],other[15]}},{{own[12],own[13],own[14],own[15]}}));
    std::int32_t bits;std::memcpy(&bits,&square,4);
    const auto length=bits<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1.f)<9.99999993922529e-9f?1.L:std::sqrt(static_cast<long double>(square));
    return std::sqrt(static_cast<long double>(task_square))*.4000000059604645f<length?target:selected;
}
//00532190 uses actual game54 navigation, NOT a copied/private point set.
//Matching labels append in reverse; routes scan that result in reverse again.
template<class AI,class Actor,class Frames,class World>bool replan_active_target_position(AI& ai,HumanVector& output,float& radius,Actor& target,Frames& frames,World& world){
    const auto name=frames.frame_name(target.base().frame());
    auto& navigation=world.active_task_navigation();std::vector<std::uint16_t> references;
    navigation.append_named_references(std::string("#")+name,references);if(references.empty())return false;
    const auto own=frames.actor_world_matrix(ai.human()->base().frame());
    const auto start=navigation.nearest_volume({{own[12],own[13],own[14]}},5.f,-1.f);
    std::uint16_t selected=0xffff;
    if(start==0xffff)selected=references[world.human_frame_random_word()%references.size()];
    else{
        float best=std::numeric_limits<float>::max();
        for(auto n=references.size();n;){const auto candidate=references[--n];std::vector<std::uint16_t> route;
            if(navigation.find_route(start,candidate,0u,route)){
                const auto length=navigation.route_length(route);if(length<best){best=static_cast<float>(length);selected=candidate;}
            }
        }
    }
    if(selected==0xffff)return false;
    const auto* point=navigation.record(selected);if(!point)throw std::runtime_error("Active replan selected a missing native navigation record");
    const auto position=navigation.position(selected);for(unsigned n=0;n!=3;++n)output[n]=position[n];
    //Native successful route scratch capacity was zeroed at3C; in the random
    //fallback branch this W is unwritten. Retain typed W there without a claim.
    if(start!=0xffff)output[3]=0;
    radius=static_cast<unsigned char>(point->prefix[0x13])*.0625f;return true;
}
//00526550 result-10.206 precedes task initialization (which clears it);
//204 and210 publish only AFTER successful initialization. Speech precedes cancel.
template<class AI,class Frames,class World>void replan_active_query_failure(AI& ai,Frames& frames,World& world){
    auto& state=ai.state();const auto task=state.template get<std::uint32_t>(0x220);
    if(state.template get<std::uint8_t>(0x202)||(task!=25&&task!=26))return;
    ai.require_live_task_target();auto* target=ai.task_target;if(!target)throw std::runtime_error("Active replan dereferences native NULL224");
    const auto validate=world.human_common_receiver_guard(*ai.human());HumanVector position=state.template get<HumanVector>(0x1e0);float radius=0;
    const auto found=replan_active_target_position(ai,position,radius,*target,frames,world);validate();
    if(!found){ai.reject_pending_task();validate();ai.cancel_task();validate();return;}
    state.set(0x206,std::uint8_t{1});
    ai.initialize_task(target,task,position,ai.human()->base().world_direction(),false,radius<=.5f?.5f:radius);validate();
    state.set(0x204,std::uint8_t{1});state.set(0x210,1u);
}
} //namespace hd2::game
#endif
