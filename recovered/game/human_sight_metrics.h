#ifndef HD2_GAME_HUMAN_SIGHT_METRICS_H
#define HD2_GAME_HUMAN_SIGHT_METRICS_H

#include "game/human_joint_math.h"
#include <cstdint>
#include <cstring>
#include <optional>

namespace hd2::game {
//These select the recovered B0 virtual34/38 bodies. Unwritten outputs from
//005830B0/C0 remain absent; a caller must not consume them as score100 or0.
enum class HumanSightMetricFamily {base,ai,player};
struct HumanSightMetric {std::optional<std::uint32_t> interval;std::optional<float> score;};

namespace human_sight_math {
inline float stored(long double value){return static_cast<float>(value);}
inline HumanVector normalize(HumanVector value){
    constexpr float epsilon=9.99999993922529e-09f;
    const auto extended=(static_cast<long double>(value[0])*value[0]+static_cast<long double>(value[2])*value[2])+static_cast<long double>(value[1])*value[1];
    const float square=stored(extended);
    if(std::abs(extended-1)>=epsilon){
        if(square>=epsilon){const auto inverse=1/std::sqrt(static_cast<long double>(square));for(unsigned n=0;n!=3;++n)value[n]=stored(value[n]*inverse);}
        else{const auto axis=value[0]!=0?0u:value[2]!=0?2u:1u;value[axis]=value[axis]<0?-1.f:1.f;}
    }
    return value;
}
inline long double distance(const HumanMatrix& first,const HumanMatrix& second){
    const auto x=static_cast<long double>(first[12])-second[12],y=static_cast<long double>(first[13])-second[13],z=static_cast<long double>(first[14])-second[14];
    const float square=stored((z*z+y*y)+x*x);std::int32_t bits;std::memcpy(&bits,&square,4);
    if(bits<0x322bcc77)return 0;
    return std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(square));
}
inline long double edge(float ax,float az,float bx,float bz,float px,float pz){
    return (static_cast<long double>(bx)-ax)*(static_cast<long double>(pz)-az)-(static_cast<long double>(px)-ax)*(static_cast<long double>(bz)-az);
}
inline bool triangle(const HumanMatrix& origin,HumanVector direction,float range,
                     long double upper_tangent,long double lower_tangent,const HumanMatrix& target,bool symmetric){
    direction[1]=0;direction=normalize(direction);
    const float side_x=direction[2],side_z=-direction[0]; //forward cross down.
    const float upper=stored(upper_tangent*range),lower=stored(lower_tangent*range);
    const float center_x=stored(static_cast<long double>(direction[0])*range+origin[12]);
    const float center_z=stored(static_cast<long double>(direction[2])*range+origin[14]);
    const float dx=stored(static_cast<long double>(side_x)*upper),dz=stored(static_cast<long double>(side_z)*upper);
    const float first_x=stored(static_cast<long double>(center_x)+dx),first_z=stored(static_cast<long double>(center_z)+dz);
    const float second_x=symmetric?stored(static_cast<long double>(center_x)-dx):stored(static_cast<long double>(center_x)+stored(static_cast<long double>(side_x)*lower));
    const float second_z=symmetric?stored(static_cast<long double>(center_z)-dz):stored(static_cast<long double>(center_z)+stored(static_cast<long double>(side_z)*lower));
    return 0<=edge(origin[12],origin[14],first_x,first_z,target[12],target[14])&&
           0<=edge(first_x,first_z,second_x,second_z,target[12],target[14])&&
           0<=edge(second_x,second_z,origin[12],origin[14],target[12],target[14]);
}
} //namespace human_sight_math

//005A59E0: TARGET range, including SAME roster, target B0 flags/B0 clock,
//inline98/DD0+DA4 and the actual optional754 material receiver.
template<class Human,class Queries>long double human_sight_range(Human& target,Queries& queries){
    using namespace human_sight_math;
    if(!target.loading_state)return -1;
    std::int32_t health;const auto bits=target.properties.stats_ec[0];std::memcpy(&health,&bits,4);
    if(health<1)return static_cast<long double>(queries.sight_range_factor(target))*50.f;
    const auto& words=target.loading_state->words();const auto flags=words.template get<std::uint32_t>(0x50);
    const auto clock=words.template get<std::int32_t>(0xb0);const auto threshold=queries.sight_controlled_target(target)?7000.f:8000.f;
    const float floor=clock<threshold?(flags&4u?450.f:flags&2u?380.f:300.f):0.f;
    float range=target.field160||(flags&4u)?100.f:flags&2u?70.f:50.f;
    if(target.properties.field19c==2&&range<30.f)range=30.f;
    if(flags&0xf8u){
        const float scale=flags&0x1000u?1.0499999523162842f:flags&0x8000u?1.2000000476837158f:flags&0x2000u?1.2999999523162842f:flags&0x4000u?1.600000023841858f:1.7999999523162842f;
        const float minimum=flags&0x1000u?2.f:flags&0x8000u?4.f:flags&0x2000u?5.f:flags&0x4000u?10.f:15.f;
        const auto adjusted=static_cast<long double>(range)*scale;range=stored(adjusted);if(adjusted<minimum)range=minimum;
    }
    if(range<floor)range=floor;
    range=stored(static_cast<long double>(queries.sight_range_factor(target))*range);
    if(flags&3u)if(auto factor=queries.sight_surface_factor(target)){
        auto value=static_cast<long double>(*factor);if(value!=1&&(flags&2u))value=(1-value)*.5f+value;
        return value*range;
    }
    return range;
}

//00542320 player and00570280 AI Human metric bodies. Native scalar words
//and real frames are read live; range and attachment virtuals remain required.
template<class Human,class Frames,class Queries>HumanSightMetric human_sight_human_metric(
        Human& observer,Human& target,Frames& frames,Queries& queries){
    using namespace human_sight_math;
    auto* state=observer.loading_state;if(!state)throw std::runtime_error("Sight metric requires actual Human B0");
    const auto family=state->sight_metric_family();if(family==HumanSightMetricFamily::base)return {};
    const auto& words=state->words();const auto flags=words.template get<std::uint32_t>(0x50);
    if(flags&(family==HumanSightMetricFamily::player?0x800u:0x40000800u))return {500,0.f};
    auto range=human_sight_range(target,queries);
    const auto kind=words.template get<std::uint32_t>(0x48);
    const bool ai=family==HumanSightMetricFamily::ai;
    float stored_range=stored(ai?range:range*1.399999976158142f);
    if(ai&&kind==3)stored_range=stored(static_cast<long double>(stored_range)*1.2999999523162842f);
    const auto view=queries.sight_view_distance();if(view<stored_range)stored_range=stored(view);
    if(ai&&kind!=3){std::int32_t limit;const auto bits=observer.properties.stats_ec[9];std::memcpy(&limit,&bits,4);if(limit<stored_range)stored_range=stored(limit);}
    if(!target.head_marker||!observer.head_marker)throw std::runtime_error("Sight metric requires actual target and observer head260 frames");
    const auto target_head=frames.actor_world_matrix(target.head_marker),head=frames.actor_world_matrix(observer.head_marker);
    if(!(distance(head,target_head)<=stored_range))return {99,0.f};
    HumanVector direction;
    if(ai&&observer.field160&&queries.sight_attachment_direction(observer,direction)){}
    else direction=normalize({{head[8],head[9],head[10],head[11]}});
    long double upper_angle=1.0471975803375244,lower_angle=-upper_angle; //QWORD00810D30, not its FLOAT low word2.
    if(ai){
        auto angle=std::atan2(static_cast<long double>(direction[0]),static_cast<long double>(direction[2]));
        const auto target_direction=words.template get<HumanVector>(0x10);
        auto desired=std::atan2(static_cast<long double>(target_direction[0]),static_cast<long double>(target_direction[2]));
        constexpr float turn=6.2831854820251465f,pi=3.1415927410125732f;
        if(angle<0)angle+=turn;else if(angle>turn)angle-=turn;
        if(desired<0)desired+=turn;else if(desired>turn)desired-=turn;
        auto difference=angle-desired;if(difference>pi)difference-=turn; //Native has no matching negative wrap.
        const auto half=static_cast<long double>(kind==3?60.f:50.f)*.01745329238474369f;
        if(difference>0){upper_angle=stored(half+difference);if(half+difference>1.399999976158142f)upper_angle=1.399999976158142f;lower_angle=-half;}
        else{upper_angle=stored(half);lower_angle=difference-half;if(lower_angle<-1.399999976158142f)lower_angle=-1.399999976158142f;}
    }
    const auto origin=frames.actor_world_matrix(observer.head_marker),model=frames.actor_world_matrix(target.base().frame());
    return {99,triangle(origin,direction,stored_range,std::tan(upper_angle),std::tan(lower_angle),model,!ai)?100.f:0.f};
}
//0056FC80 B0+34, reached through005A59A0. Player eligibility is tested by
//that wrapper first: it writes interval500 and leaves score UNWRITTEN.
template<class Human,class Actor,class Frames,class Queries,class DogQueries>HumanSightMetric human_sight_dog_metric(
        Human& observer,Actor target,Frames& frames,Queries& queries,DogQueries& dogs){
    using namespace human_sight_math;
    if(queries.human_is_player(observer))return {500,{}};
    auto* state=observer.loading_state;if(!state)throw std::runtime_error("Dog metric requires actual Human B0");
    if(state->sight_metric_family()!=HumanSightMetricFamily::ai)return {};
    const auto& words=state->words();if(words.template get<std::uint32_t>(0x50)&0x40000800u)return {500,0.f};
    if(words.template get<std::uint32_t>(0x48)!=3||observer.properties.field19c==2||observer.properties.field19c==3)return {500,0.f};
    float range=100;const auto view=queries.sight_view_distance();if(view<range)range=stored(view);
    const auto target_head=dogs.sight_dog_head_matrix(target);
    if(!observer.head_marker)throw std::runtime_error("Dog metric requires actual observer head260");
    const auto head=frames.actor_world_matrix(observer.head_marker);
    HumanVector delta{};for(unsigned n=0;n!=3;++n)delta[n]=stored(static_cast<long double>(head[12+n])-target_head[12+n]);
    if(static_cast<long double>(range)*range<(static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[1])*delta[1])+static_cast<long double>(delta[2])*delta[2])return {99,0.f};
    HumanVector direction;
    if(observer.field160&&queries.sight_attachment_direction(observer,direction)){}
    else direction=normalize({{head[8],head[9],head[10],head[11]}});
    const auto origin=frames.actor_world_matrix(observer.head_marker),model=frames.actor_world_matrix(target->base().frame());
    const auto tangent=std::tan(1.0471975803375244L);
    return {99,triangle(origin,direction,range,tangent,-tangent,model,true)?100.f:0.f};
}
} //namespace hd2::game
#endif
