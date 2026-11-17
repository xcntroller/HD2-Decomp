#ifndef HD2_GAME_DOG_CONTROLLER_PURSUIT_H
#define HD2_GAME_DOG_CONTROLLER_PURSUIT_H
#include "game/dog_controller_targets.h"
#include "game/dog_controller_follow.h"
#include "game/dog_route_geometry.h"
namespace hd2::game {
//005CCCC0. The native force argument is unused. Distance28 is spilled to
//FLOAT before the Body movement-limit comparison. Unordered takes the far
//path, and an unordered already-tracked displacement keeps the existing route.
template<class State,class Queries>bool refresh_dog_target_route(State& state,Queries& q){
    const float distance=static_cast<float>(q.target_distance());
    if(!(q.movement_limit()>distance)||state.route78.size()>1){
        if(state.target88){
            if(!state.route78.empty()){
                const auto target=state.target88->base().world_position();
                if(!(dog_controller_length(dog_controller_square(state.target90.position(),target,true))>1.25f))return true;
            }
            if(!state.target88)throw std::runtime_error("Dog target route lost actual target88");
            state.target90.assign(state.target88->base().world_position());
            const auto target=state.target88->base().world_position();
            if(!q.request_route(q.actor_position(),target)){q.clear_target();q.clear_route();return false;}
            if(!q.validate_route()){q.clear_route();return false;}
        }
    }else{
        if(!state.route78.empty())q.clear_route();
        state.flags20|=0x10004u;
    }
    return true;
}
//005CCEC0. The initial range uses actual model X/Z, the displacement gate uses
//all three followA0 lanes, and failed route creation leaves A0 published.
template<class State,class Queries>bool refresh_dog_follow_route(State& state,Queries& q){
    const auto target=q.follow_position(),origin=q.model_position();
    const auto x=static_cast<long double>(target[0])-origin[0],z=static_cast<long double>(target[2])-origin[2];
    const auto distance=dog_controller_length(static_cast<float>(z*z+x*x));
    if(!(distance<3.5f)||state.route78.size()>1){
        if(state.follow18&&dog_controller_length(dog_controller_square(state.followa0.position(),target,true))>.5f){
            state.followa0.assign(target);
            if(!q.request_route(q.actor_position(),target)){q.clear_route();return false;}
        }
    }else{
        if(!state.route78.empty())q.clear_route();
        state.flags20|=0x10004u;
    }
    return true;
}
//005CD0A0. Signed rand remainders and Script callback mutations stay visible.
template<class State,class Queries>void retarget_dog_controller(State& state,Queries& q){
    state.timerb4=200-dog_signed_word(q.random_word())%400;q.select_target(true);
    if(state.target88){
        if(q.target_route()){state.flags20=(state.flags20&~0x2000u)|0x24000u;return;}
        if(state.timerb0<1){state.timerb0=dog_signed_word(q.random_word())%1800+100;state.flags20|=0x80000u;}
    }
}
//005CD1F0. Current Body364 and LAST queued metadata are distinct receivers.
//Prediction is native velocity1B0 * .1, with X prediction / X,Y deltas and Z
//velocity spilled to FLOAT, and the final Z delta retained in x87 precision.
template<class State,class Queries>void advance_dog_controller_pursuit(State& state,std::uint32_t elapsed,Queries& q){
    const bool evade=(state.flags20&0x200u)&&!((state.flags20&0x400u)&&state.damage_source8c==state.target88);
    if(state.timerb4<100000)state.timerb4=dog_signed_word(static_cast<std::uint32_t>(state.timerb4)+elapsed);
    if(!state.target88){
        if(state.kind14==4&&q.perception_nonempty()&&state.timerb4>1200)q.retarget();
        return;
    }
    const auto aim=[&]{
        if(!state.target88)throw std::runtime_error("Dog pursuit aim lost actual target88");
        const auto target=state.target88->base().world_position();request_dog_controller_direction(state.direction40,q.actor_position(),target);
    };
    if(!(state.flags20&0x20000u)){aim();return;}
    q.set_speed(1.f);
    if(!state.target88)throw std::runtime_error("Dog pursuit lost actual target after speed request");
    if(q.target_health()<1||q.target_attached()){q.clear_target();q.cancel_bite();return;}
    const auto velocity=q.target_velocity(),own=q.actor_position(),target=state.target88->base().world_position();
    const float z_shift=static_cast<float>(static_cast<long double>(velocity[2])*.10000000149011612f);
    const float predicted_x=static_cast<float>(static_cast<long double>(velocity[0])*.10000000149011612f+target[0]);
    const float x=static_cast<float>(static_cast<long double>(predicted_x)-own[0]);
    const float y=static_cast<float>((static_cast<long double>(velocity[1])*.10000000149011612f+target[1])-own[1]);
    const auto z=(static_cast<long double>(target[2])+z_shift)-own[2];
    const float distance=static_cast<float>(dog_controller_length(static_cast<float>((z*z+static_cast<long double>(y)*y)+static_cast<long double>(x)*x)));
    const auto threshold=evade?4.199999809265137f:q.current_category()==1?2.0999999046325684f:1.2999999523162842f;
    if(!(distance<threshold)){
        if(!(q.movement_limit()>distance)||state.route78.size()>1){q.select_target(false);state.flags20=(state.flags20&~0x10000u)|4u;return;}
        if(!state.route78.empty())q.clear_route();
        state.flags20|=0x10004u;aim();return;
    }
    if(q.current_category()!=2&&q.next_category()!=2){
        if(!state.route78.empty())q.clear_route();
        state.flags20|=0x10000u;
        if(!state.second64)throw std::runtime_error("Dog pursuit bite request requires actual second64 input buffer");
        (*state.second64)[6]=evade?0.f:1.f;
    }
    bool moving=!(distance<(evade?4.199999809265137f:1.399999976158142f));
    if(!moving){
        const auto fresh=q.target_velocity();
        const float square=static_cast<float>((static_cast<long double>(fresh[0])*fresh[0]+static_cast<long double>(fresh[1])*fresh[1])+static_cast<long double>(fresh[2])*fresh[2]);
        moving=!(dog_controller_length(square)<=9.99999993922529e-09f)&&!evade;
    }
    if(moving)state.flags20|=0x4004u;
    else{
        state.flags20&=~0xcu;
        if((state.flags20&0x200u)&&state.timerb0<1){state.timerb0=dog_signed_word(q.random_word())%1800+100;state.flags20|=0x80000u;}
    }
    aim();
}
//005CC420. A true result is the route-movement early return; successful target
//or follow replans can mutate flags, route and Script state before later reads.
template<class State,class Queries>bool complete_dog_controller_route(State& state,Queries& q){
    if(state.route78.empty()){
        q.clear_route();
        if(state.flags20&0x20000u){(void)q.target_route();return true;}
        if(state.flags20&0x10u)(void)q.follow_route();
        return true;
    }
    if((state.flags20&0x20000u)&&!q.target_route())return true;
    if(!(state.flags20&0x20000u)&&(state.flags20&0x10u)&&!q.follow_route())return true;
    const auto flags=state.flags20;
    if(!(flags&0x20000u)&&(flags&0x40000u)&&(flags&0xcu)&&state.route78.size()==1){
        const float range=(flags&0x6000u)==0x4000u?3.f:.800000011920929f;
        const auto first_model=q.model_position();
        auto* point=state.route78.at(0);if(!point)throw std::runtime_error("Dog route completion reaches native NULL point");
        const auto position=point->position0.position();
        const float x=static_cast<float>(static_cast<long double>(position[0])-first_model[0]),z=static_cast<float>(static_cast<long double>(position[2])-first_model[2]);
        const auto second_model=q.model_position();
        point=state.route78.at(0);if(!point)throw std::runtime_error("Dog route completion lost actual first point");
        const auto original=point->original10.position();
        const float ox=static_cast<float>(static_cast<long double>(original[0])-second_model[0]),oz=static_cast<float>(static_cast<long double>(original[2])-second_model[2]);
        if((state.flags20&0x200000u)&&state.kind14==5&&state.scriptc0){
            const auto radius=dog_route_required(point->radius28);
            if((static_cast<long double>(oz)*oz+static_cast<long double>(ox)*ox<=static_cast<long double>(radius)*radius)||
               (static_cast<long double>(z)*z+static_cast<long double>(x)*x<=static_cast<long double>(range)*range)){
                q.route_finished(state.scriptc0);state.flags20&=~0x200000u;
            }
        }
    }
    return false;
}
}
#endif
