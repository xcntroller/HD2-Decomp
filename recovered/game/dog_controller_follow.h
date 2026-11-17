#ifndef HD2_GAME_DOG_CONTROLLER_FOLLOW_H
#define HD2_GAME_DOG_CONTROLLER_FOLLOW_H
#include "game/dog_controller_vector.h"
#include "game/human_ai_shot_direction.h"
#include "game/dog_damage.h"
namespace hd2::game {
//005CC5F0 writes current direction40 XYZ and an unproduced scratch W. Keep
//that lane's provenance separate from the produced planar direction.
inline void request_dog_controller_direction(DogControllerVector& direction,const HumanVector& from,const HumanVector& to){
    HumanVector value{{static_cast<float>(static_cast<long double>(to[0])-from[0]),0,
        static_cast<float>(static_cast<long double>(to[2])-from[2]),0}};
    const float square=static_cast<float>((static_cast<long double>(value[0])*value[0]+static_cast<long double>(value[1])*value[1])+static_cast<long double>(value[2])*value[2]);
    normalize_human_ai_shot(value,square);
    direction.xyz=std::array<float,3>{{value[0],value[1],value[2]}};direction.w.reset();
}
//005CD890's entire follow frame. Queries borrow current SAME Actor10/follow18
//on each call; clear_route may run a Script callback that changes the state.
template<class State,class Queries>void advance_dog_controller_follow(State& state,Queries& queries){
    if((state.flags20&0x20000u)||!state.follow18)return;
    const auto aim=[&](const HumanVector& from,const HumanVector& to){request_dog_controller_direction(state.direction40,from,to);};
    if(!(state.flags20&0x10u)){const auto destination=queries.follow_position();aim(queries.actor_position(),destination);return;}
    const auto destination=queries.follow_position();const auto origin=queries.actor_position();
    const float x=static_cast<float>(static_cast<long double>(destination[0])-origin[0]);
    const auto extended_z=static_cast<long double>(destination[2])-origin[2];const float z=static_cast<float>(extended_z);
    //Native FST leaves extended Z on stack before multiplying by its FLOAT copy.
    const float square=static_cast<float>(extended_z*z+static_cast<long double>(x)*x);
    std::int32_t bits;std::memcpy(&bits,&square,4);
    const float distance=bits<0x322bcc77?0.f:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.f:static_cast<float>(std::sqrt(static_cast<long double>(square)));
    const auto reciprocal=1.L/distance;
    state.flags20&=0xffff9fffu;const auto flags=state.flags20;
    const auto direction=queries.actor_direction();
    const float dot=static_cast<float>((static_cast<long double>(direction[2])*(static_cast<long double>(z)*reciprocal)+static_cast<long double>(direction[1])*(reciprocal*0.f))+(static_cast<long double>(x)*reciprocal)*direction[0]);
    float spacing=.20000000298023224f;
    if(dot<=0.f)spacing=1.2000000476837158f;
    else if(dot<=.8500000238418579f)spacing=static_cast<float>((1.0-static_cast<long double>(dot)*1.17)+.20000000298023224);
    float speed=1;
    if(static_cast<long double>(spacing)+5.f<distance)state.flags20=flags|0x4000u;
    else{
        state.flags20=flags|0x2000u;
        if(static_cast<long double>(spacing)+1.f<distance)speed=1.399999976158142f;
        else if(static_cast<long double>(spacing)+.4000000059604645f<distance)speed=1.2000000476837158f;
        else if(static_cast<long double>(spacing)+.05000000074505806f<distance)speed=1.100000023841858f;
    }
    queries.set_speed(speed);
    if(!(distance<3.5f)||state.route78.size()>1){state.flags20=(state.flags20&0xfffeffffu)|4u;return;} //Native unordered also takes far branch.
    if(!state.route78.empty())queries.clear_route();
    const auto forward=[&]{const auto facing=queries.follow_direction(),position=queries.follow_actor_position();auto target=position;
        for(unsigned n=0;n!=3;++n)target[n]=static_cast<float>(static_cast<long double>(facing[n])+position[n]);
        aim(position,target);
    };
    if(!(state.flags20&0xcu)){
        if(static_cast<long double>(spacing)+.20000000298023224f<distance){state.flags20|=4u;aim(queries.actor_position(),destination);return;}
        forward();return;
    }
    if(distance<spacing){state.flags20&=0xfffffff3u;forward();return;}
    state.flags20|=4u;aim(queries.actor_position(),destination);
}
}
#endif
