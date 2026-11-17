#ifndef HD2_GAME_DOG_CONTROLLER_MOVEMENT_H
#define HD2_GAME_DOG_CONTROLLER_MOVEMENT_H
#include "game/dog_controller_pursuit.h"
#include "game/dog_route_obstacles.h"
#include "game/human_loading_stats.h"
namespace hd2::game {
//005CB480: first argument is native-unused. Movement/script gates precede the
//position magnitude check, so005CBF00's all-zero request is a genuine no-op.
template<class State,class Queries>void request_dog_controller_position(State& state,const std::array<float,4>& target,Queries& q){
    if((state.flags20&0xcu)||(state.flags20&0x40000u))return;
    const auto square=(static_cast<long double>(target[0])*target[0]+static_cast<long double>(target[1])*target[1])+static_cast<long double>(target[2])*target[2];
    if(square<1.0000000168623835e-16f)return;
    request_dog_controller_direction(state.direction40,q.actor_position(),target);
}
//005CBF00. Shared route/pool and actual580/590/5A0 frame history. The source
//keeps the old planar distance across completion callbacks and consumes at most
//one point before recomputing completion. No nearest-node or segment clamp.
template<class State,class Queries>void advance_dog_controller_route(State& state,Queries& q){
    if(!(state.flags20&0xcu)||(state.flags20&0x10000u))return;
    float distance=0;
    if(!state.route78.empty()){
        if(!q.validate_route()){q.clear_route();return;}
        const auto model=q.model_position();const auto position=dog_route_point(state.route78.at(0)).position0.position();
        const float x=static_cast<float>(static_cast<long double>(position[0])-model[0]),z=static_cast<float>(static_cast<long double>(position[2])-model[2]);
        distance=static_cast<float>(dog_controller_length(static_cast<float>(static_cast<long double>(z)*z+static_cast<long double>(x)*x)));
        const auto flags=state.flags20;long double range=distance;
        if((flags&0x6000u)==0x4000u&&!(flags&0x20000000u))range*=.25f;
        if(range>2.f)range=2.f;
        const float threshold=static_cast<float>((flags&0x20000000u)?range*.10000000149011612f+.20000000298023224f:(range+.5f)*1.0471975803375244f);
        if(!(q.vector_angle({{x,0,z,0}},q.actor_direction())>=threshold))state.flags20=(state.flags20&0xdffffff7u)|4u;
        else state.flags20|=0x20000000u;
    }
    if(q.complete_route())return;
    const auto velocity=q.history_velocity();
    const auto velocity_square=(static_cast<long double>(velocity[0])*velocity[0]+static_cast<long double>(velocity[1])*velocity[1])+static_cast<long double>(velocity[2])*velocity[2];
    bool retire=false;
    if(!(velocity_square<1.0000000168623835e-16f)){
        if(distance<.30000001192092896f)retire=true;
        else if(distance<1.f){
            auto previous=q.history_position();const auto scale=static_cast<long double>(q.history_elapsed())*.0010000000474974513f;
            const float dx=static_cast<float>(scale*velocity[0]);const auto dy=scale*velocity[1],dz=scale*velocity[2];
            const std::array<float,3> movement{{dx,static_cast<float>(dy),static_cast<float>(dz)}};
            const float square=static_cast<float>((static_cast<long double>(dx)*dx+dy*dy)+dz*dz);
            const auto point=dog_route_point(state.route78.at(0)).position0.position();previous[1]=0;
            long double residual=-1.0000000272564224e+16f;
            if(!(square<9.99999993922529e-09f)){
                const std::array<float,3> difference{{static_cast<float>(static_cast<long double>(previous[0])-point[0]),0,static_cast<float>(static_cast<long double>(previous[2])-point[2])}};
                const float parameter=static_cast<float>(((static_cast<long double>(movement[0])*difference[0]+static_cast<long double>(movement[1])*difference[1])+static_cast<long double>(movement[2])*difference[2])/square);
                std::array<float,3> remaining{};
                for(unsigned axis=0;axis!=3;++axis){const float projection=static_cast<float>(static_cast<long double>(movement[axis])*parameter);remaining[axis]=static_cast<float>(static_cast<long double>(difference[axis])-projection);}
                residual=dog_controller_length(static_cast<float>((static_cast<long double>(remaining[0])*remaining[0]+static_cast<long double>(remaining[1])*remaining[1])+static_cast<long double>(remaining[2])*remaining[2]));
            }
            retire=residual<.30000001192092896f&&!(residual<0);
        }
    }
    if(retire){
        if(!state.route78.empty()){q.release(state.route78[0]);state.route78.erase(state.route78.begin());}
        if(q.complete_route())return;
    }
    if(!state.route78.empty()){
        auto* point=state.route78[0];const auto model=q.model_position();
        const auto position=dog_route_point(point).position0.position();
        request_dog_controller_direction(state.direction40,model,{{position[0],position[1],position[2],0}});
    }
    if((state.flags20&0x400000u)&&state.timer84==0){
        request_dog_controller_position(state,{{0,0,0,0}},q);
        state.timer84=dog_signed_word(human_loading_stat_word(q.random_unit()*2000.L+4000.f));
    }
}
//005CE190: authority policy order on the SAME selected controller, process
//scratch buffers and Body. Dead800 gates every side effect including timers.
template<class State,class Queries>void advance_dog_controller_decisions(State& state,std::uint32_t elapsed,Queries& q){
    if(state.flags20&0x800u)return;
    q.timers(elapsed);q.bind_zeroed_inputs();q.retire_heard();q.pursuit(elapsed);q.follow();q.route();
    q.publish_inputs();q.publish_actions();state.flags20&=~0x80000u;q.publish_direction();
}
//005CFA20: common decisions ALWAYS precede kind5 C4 handling. The timed
//voice still runs after a dead800 common-policy early return.
template<class State,class Queries>void advance_dog_selected_controller(State& state,std::uint32_t elapsed,Queries& q){
    q.decisions(elapsed);
    if(state.kind14!=5)return;
    if(!state.timerc4)throw std::runtime_error("Dog kind5 frame requires actual C4 timer producer");
    if(*state.timerc4<0)return;
    state.timerc4=dog_signed_word(static_cast<std::uint32_t>(*state.timerc4)-elapsed);
    if(*state.timerc4>0)return;
    state.timerc4=dog_signed_word(q.random_word())%25000+1000;q.timed_voice();
}
}
#endif
