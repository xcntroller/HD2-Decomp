#ifndef HD2_GAME_HUMAN_ACTIVE_AI_SPECIAL_TARGET_H
#define HD2_GAME_HUMAN_ACTIVE_AI_SPECIAL_TARGET_H
#include "game/human_active_ai_commands.h"
#include "game/human_state_frame_timers.h"
#include <optional>
#include <cmath>
namespace hd2::game {
//0052B580, vector aim for Car/StaticWeapon/Animation/Dog. Native temporary W
//is unwritten; preserve the typed target lane without claiming its native value.
template<class AI,class Actor,class Frames,class World>void aim_active_special_target(AI& ai,Actor& actor,Frames& frames,World& world){
    const auto kind=actor.base().type();if(kind!=4&&kind!=21&&kind!=25&&kind!=30)return;
    auto* human=ai.human();if(!human)throw std::logic_error("Special target aim lost Human44");
    const auto validate=world.human_common_receiver_guard(*human);
    auto model=actor.base().frame();if(kind==4)model=frames.actor_find(model,"body",0xffffu);
    else if(kind==30)model=frames.actor_find(model,"c",0xffffu);
    if(!human->head_marker)throw std::runtime_error("Special target aim requires actual Human260 head");
    const auto head=frames.actor_world_matrix(human->head_marker);validate();
    if(!model)throw std::runtime_error("Special target aim reached native missing target frame");
    const auto target=frames.actor_world_matrix(model);validate();auto vector=ai.resources().target().state.template get<HumanVector>(0x10);
    for(unsigned n=0;n!=3;++n)vector[n]=static_cast<float>(static_cast<long double>(target[12+n])-head[12+n]);
    if(world.active_hand_item_kind(*human)==std::optional<std::uint32_t>{9}){
        const auto own=human->base().world_position(),position=ai.state().template get<HumanVector>(0x1e0);
        const auto x=static_cast<long double>(position[0])-own[0],y=static_cast<long double>(position[1])-own[1],z=static_cast<long double>(position[2])-own[2];
        if(36.f<(z*z+y*y)+x*x){ai.resources().target().bind_position(position);ai.state().set(0x214,static_cast<std::uint32_t>(ai.pending_ranged_animation_duration()));}
        else{ai.reject_pending_task();validate();ai.state().set(0x206,std::uint8_t{1});}
        return;
    }
    ai.resources().direction().set_target(vector);ai.resources().target().bind_vector(vector);
}
inline bool active_task22_in_range(std::uint32_t command,long double square){ //0052B180; strict ordered comparisons from native TEST AH,5/JP.
    if(command==3&&square<2500.f)return true;
    const float limit=command==1||command==4?400.f:command==2?100.f:command==3||command==6?1.440000057220459f:900.f;
    return square<limit;
}
//0052B180 task22 runs visibility BEFORE its query-state/range progression.
//Its returned actor is the freshly read task224 after a possible initializer.
template<class AI,class Frames,class World>auto active_task22_nearby_target(AI& ai,Frames& frames,World& world)->decltype(ai.task_target){
    ai.require_live_task_target();auto* target=ai.task_target;auto* human=ai.human();
    if(!target||!human||!ai.item())throw std::logic_error("Task22 nearby query requires its actual selected prefix");
    const auto own=human->base().world_position(),other=target->base().world_position();
    const auto x=static_cast<long double>(own[0])-other[0],y=static_cast<long double>(own[1])-other[1],z=static_cast<long double>(own[2])-other[2];
    const float square=static_cast<float>((z*z+y*y)+x*x);
    const bool within=active_task22_in_range(world.active_follow_item_command(ai.item()),square);
    const auto visible=world.active_mounted_task_visible(*target,*human);
    auto& query=ai.resources().query().state;
    if(!(query.template get<std::uint32_t>(0x50)&0x10000000u)){
        if(!within&&!world.active_task_actor_unavailable(*target)&&!ai.state().template get<std::uint32_t>(0x210)){
            const auto m=frames.actor_world_matrix(target->base().frame());const auto destination=query.template get<HumanVector>(0x10);
            const auto dx=static_cast<long double>(m[12])-destination[0],dy=static_cast<long double>(m[13])-destination[1],dz=static_cast<long double>(m[14])-destination[2];
            if((dz*dz+dy*dy)+dx*dx<.04000000283122063f){
                ai.state().set(0x206,std::uint8_t{1});const auto action=ai.state().template get<std::uint32_t>(0x220);const auto radius=ai.state().template get<float>(0x21c);
                start_active_target_task(ai,*target,action,false,radius,false,frames,world);ai.state().set(0x204,std::uint8_t{1});
            }
        }
    }else if(within){if(!visible)return nullptr;query.set(0x50,query.template get<std::uint32_t>(0x50)|0x2000u);ai.state().set(0x200,std::uint8_t{1});}
    return visible?ai.task_target:nullptr;
}
//004DDEE0. This classification call is observable even at the task22 caller
//which discards its return. Unknown roles continue the reverse seat scan.
template<class Car,class World>std::uint32_t classify_active_car_occupants(Car& car,World& world){
    for(auto n=car.occupants.size();n;){auto* occupant=car.occupants[--n];if(!occupant)continue;auto& human=world.human_ai_target_human(*occupant);
        if(human_frame_signed_word(human.properties.stats_ec[0])<1||!world.human_speech_player_controlled(human)||world.human_selected_suppressed(human))continue;
        const auto role=human.properties.field19c;if(role==3)continue;if(role==2||role==5)return 0;if(role==4)return 1;if(role==1)return world.human_specialization(human,false);
    }
    return 2;
}
//00535250, original vehicle/Animation attack producer. Weapon selection and
//transfer precede task publication; rejection marks completion before speech.
template<class AI,class Actor,class Frames,class World>void produce_active_vehicle_attack(AI& ai,Actor* target,Frames& frames,World& world){
    auto* human=ai.human();if(!human)throw std::logic_error("Vehicle attack producer lost Human44");
    const auto validate=world.human_common_receiver_guard(*human);bool allowed=false;
    if(target!=human->field160){
        if(!human->field160){std::int32_t slot=7;const auto item=world.active_vehicle_attack_weapon(ai,target,slot);validate();
            if(item){world.active_select_vehicle_attack_weapon(*human,item,slot);validate();allowed=true;}
        }else if(world.human_pose_actor_has_control(human->field160,human->field164)){
            allowed=!world.active_vehicle_attack_armoured(target)||world.active_has_heavy_weapon(*human);
        }
    }
    if(!allowed){ai.state().set(0x202,std::uint8_t{1});ai.state().set(0x206,std::uint8_t{1});ai.reject_pending_task();return;}
    if(!target)throw std::runtime_error("Vehicle attack task dereferences native NULL target");
    start_active_target_task(ai,*target,22,false,5.f,true,frames,world);validate();ai.state().set(0x204,std::uint8_t{1});
    ai.state().set(0x174,ai.state().template get<std::uint32_t>(0x174)&~4u);delay_active_focused_task(ai,world);
}
}
#endif
