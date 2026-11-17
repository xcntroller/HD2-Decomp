#ifndef HD2_GAME_HUMAN_ACTIVE_AI_GRENADE_H
#define HD2_GAME_HUMAN_ACTIVE_AI_GRENADE_H
#include "game/rigid_body_motion_math.h"
#include "game/human_joint_math.h"
namespace hd2::game {
namespace human_grenade_reaction {
inline HumanVector normalize(HumanVector value,unsigned order){
    const auto x=static_cast<long double>(value[0])*value[0],y=static_cast<long double>(value[1])*value[1],z=static_cast<long double>(value[2])*value[2];
    const auto extended=order==0?x+z:order==1?(x+z)+y:(x+y)+z;
    const float square=static_cast<float>(extended);
    if(std::abs(extended-1)>=9.99999993922529e-9f){
        if(square>=9.99999993922529e-9f)value=rigid_motion::scale(value,1.L/std::sqrt(static_cast<long double>(square)));
        else if(value[0]!=0)value[0]=value[0]<0?-1.f:1.f;
        else if(value[2]!=0)value[2]=value[2]<0?-1.f:1.f;
        else value[1]=value[1]<0?-1.f:1.f;
    }
    return value;
}
inline long double square_xyz(const HumanVector& value){return (static_cast<long double>(value[0])*value[0]+static_cast<long double>(value[1])*value[1])+static_cast<long double>(value[2])*value[2];}
//0041CFD0 denominator stays extended for comparison, FLOAT for division.
inline long double line_distance(const HumanVector& point,const HumanVector& origin,const HumanVector& direction,bool spill){
    const auto extended=square_xyz(direction);const float denominator=static_cast<float>(extended);
    if(extended<9.99999993922529e-9f)return -1.0000000272564224e16f;
    const auto offset=rigid_motion::subtract(point,origin);
    const float parameter=static_cast<float>(rigid_motion::dot(direction,offset)/denominator);
    const auto residual=rigid_motion::subtract(offset,rigid_motion::scale(direction,parameter));
    const float square=static_cast<float>(square_xyz(residual));std::int32_t bits;std::memcpy(&bits,&square,4);
    const auto distance=bits<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-9f?1.L:std::sqrt(static_cast<long double>(square));
    return spill?static_cast<long double>(static_cast<float>(distance)):distance;
}
}
//00530FF0, actual Active AI kind3 grenade notice. No projectile or item ownership
//changes. Query publication precedes pending18C and the optional formation loan.
template<class State,class Human,class World>
void react_human_active_ai_grenade(State& state,const HumanVector& position,const HumanVector& incoming,Human* source,World& world){
    auto* human=state.human();if(human==source)return;
    auto& query=state.resources().query();
    if((query.state.template get<std::uint32_t>(0x50)&0x10000000u)&&!state.state().template get<std::uint8_t>(0x18c))return;
    auto matrix=world.human_command_frames().actor_world_matrix(human->base().frame());
    HumanVector current{{matrix[12],0,matrix[14],matrix[15]}},origin=position,direction=incoming;
    origin[1]=0;direction[1]=0;direction=human_grenade_reaction::normalize(direction,0);
    auto offset=rigid_motion::subtract(current,origin);offset[1]=0;
    auto unit=human_grenade_reaction::normalize(offset,0);
    const auto dx=static_cast<long double>(origin[0])-current[0],dz=static_cast<long double>(origin[2])-current[2];
    if(!(dx*dx+dz*dz<=900.f))return;
    if(!((static_cast<long double>(unit[2])*direction[2]+static_cast<long double>(unit[0])*direction[0])+static_cast<long double>(unit[1])*direction[1]>=0))return;
    const float distance=static_cast<float>(human_grenade_reaction::line_distance(current,origin,direction,true));
    const auto length=rigid_motion::length(offset);
    const HumanVector side{{static_cast<float>(static_cast<long double>(direction[1])*0-direction[2]),static_cast<float>(static_cast<long double>(direction[2])*0-static_cast<long double>(direction[0])*0),static_cast<float>(static_cast<long double>(direction[0])-static_cast<long double>(direction[1])*0),0}};
    const auto forward=human_grenade_reaction::line_distance(current,origin,side,false);
    if(!(distance<20.f&&static_cast<long double>(distance)+distance<=length&&forward>=5.f))return;
    auto axis=human_grenade_reaction::normalize(rigid_motion::cross(direction,unit),1);
    auto escape=human_grenade_reaction::normalize(rigid_motion::cross(axis,direction),2);
    matrix=world.human_command_frames().actor_world_matrix(human->base().frame());
    const HumanVector actual{{matrix[12],matrix[13],matrix[14],matrix[15]}};
    auto destination=rigid_motion::add(actual,rigid_motion::scale(escape,10.L-distance));
    world.human_grenade_navigation_probe(destination); //0052D720(destination,0,NULL): nearest5 BEFORE false gate.
    state.resources().query().request_position(destination);
    state.state().set(0x18c,std::uint8_t{1});
    world.human_grenade_formation_position(*human,destination);
}
}
#endif
