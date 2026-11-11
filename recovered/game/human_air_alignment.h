#ifndef HD2_GAME_HUMAN_AIR_ALIGNMENT_H
#define HD2_GAME_HUMAN_AIR_ALIGNMENT_H

#include "game/human_ground_motion.h"
#include "game/human_animation_requests.h"

namespace hd2::game {

template<class Control,class Human,class Services,class Frames>
void align_human_air_motion(Control& control,Human& human,Services& services,Frames& frames,
        std::int32_t elapsed){ //0076DF20, terrain alignment through actual bones and collision queries.
    using namespace collision_motion_math;
    auto& physical=control.physical();auto& words=physical.control_frame_words();
    if(words.template get<std::int32_t>(0x9c0)==-10000||!words.template get<std::uint8_t>(0x9c8))return;
    const auto animation=words.template get<std::int32_t>(0x1d8);
    const bool lateral=animation==139||animation==129||animation==140||animation==130;
    const bool backward=words.template get<std::int32_t>(0xa10)==0x40;
    const auto original_direction=human.base().world_direction();
    const auto head=physical.bones().at(4);
    if(!head)throw std::runtime_error("Human air alignment requires its actual retained head bone");
    const auto head_matrix=frames.actor_world_matrix(head);
    const HumanVector head_position{{head_matrix[12],head_matrix[13],head_matrix[14],head_matrix[15]}};
    const auto previous_roll=words.template get<float>(0x250);
    control.movement().frame_words().set(0x64,std::int32_t{-10000});
    auto horizontal=original_direction;horizontal[1]=0;
    const auto horizontal_square=static_cast<long double>(horizontal[2])*horizontal[2]+static_cast<long double>(horizontal[0])*horizontal[0];
    if(std::abs(horizontal_square-1)>=1.e-8f){
        const float stored=static_cast<float>(horizontal_square);
        if(stored>=1.e-8f)horizontal=scale(horizontal,1.L/std::sqrt(static_cast<long double>(stored)));
        else if(horizontal[0]!=0)horizontal[0]=horizontal[0]<0?-1.f:1.f;
        else if(horizontal[2]!=0)horizontal[2]=horizontal[2]<0?-1.f:1.f;
    }
    auto forward=scale(horizontal,backward?-1.3f:1.3f);forward[1]=-1.8f;
    auto reverse=scale(horizontal,backward?.43478262424468994f:-.43478262424468994f);reverse[1]=-1.8f;
    auto half=scale(horizontal,backward?-.5f:.5f);half[1]=-1.3f;
    const auto original_position=human.base().world_position();
    auto floor_origin=original_position;floor_origin[1]+=.6f;
    auto midpoint=original_position;
    for(unsigned axis=0;axis!=3;++axis)
        midpoint[axis]=static_cast<float>(original_position[axis]+(static_cast<long double>(head_position[axis])+
            (axis==1?.6f:0.f)-original_position[axis])*.5f);
    using Hit=typename std::remove_reference_t<decltype(services.human_geometry_sweep())>::Hit;
    std::optional<Hit> floor_hit,pose_hit,middle_hit;
    {
        struct Restore {decltype(physical)& owner;~Restore(){owner.restore_query(false);}} restore{physical};
        physical.exclude_query(false);
        auto& sweep=services.human_geometry_sweep();
        floor_hit=sweep.sweep(floor_origin,{{0,-2.5f,0,0}},.15f,0xffffffffu,0x230000u);
        const auto origin=[&](const HumanVector& travel,float distance){
            auto direction=scale(travel,-1.L);
            normalize_human_ai_shot(direction,(static_cast<long double>(direction[1])*direction[1]+
                static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[0])*direction[0]);
            auto result=head_position;
            for(unsigned axis=0;axis!=3;++axis)result[axis]=static_cast<float>(head_position[axis]+static_cast<long double>(direction[axis])*distance);
            return result;
        };
        pose_hit=services.human_geometry_ray(origin(half,.3f),half,0xffffffffu,0x230000u);
        if(pose_hit)pose_hit->position[1]+=.3f;
        else{
            pose_hit=sweep.sweep(origin(forward,.3f),forward,.3f,0xffffffffu,0x230000u);
            if(!pose_hit)pose_hit=sweep.sweep(origin(forward,1.f),forward,.3f,0xffffffffu,0x230000u);
            if(!pose_hit)pose_hit=sweep.sweep(origin(reverse,.3f),reverse,.3f,0xffffffffu,0x230000u);
        }
        middle_hit=sweep.sweep(midpoint,{{0,-1.5f,0,0}},.3f,0xffffffffu,0x230000u);
    }
    if(!pose_hit)return;
    //Native later averages the floor normal even when its first sweep missed.
    //That path reads uninitialized stack data and has no recoverable normal.
    if(!floor_hit)throw std::runtime_error("Human air alignment has no initialized native floor normal");
    const bool settling=words.template get<std::int32_t>(0x9cc)!=-10000;
    const float pitch_limit=static_cast<float>(static_cast<long double>(elapsed)*(settling?.003f:.0008666666690260172f));
    const float vertical_limit=static_cast<float>(static_cast<long double>(elapsed)*(settling?.0015f:.001f));
    auto position=original_position;
    floor_hit->position[1]-=.1428571492433548f;
    pose_hit->position[1]-=.3f;
    if(middle_hit)middle_hit->position[1]-=.3f;
    if(std::abs(static_cast<long double>(position[1])-floor_hit->position[1])<=vertical_limit)position[1]=floor_hit->position[1];
    else position[1]=static_cast<float>(position[1]+(floor_hit->position[1]<=position[1]?-static_cast<long double>(vertical_limit):vertical_limit));
    if(middle_hit){
        const float difference=middle_hit->position[1]-pose_hit->position[1];
        const auto span=length(subtract(pose_hit->position,middle_hit->position));
        const auto distance=length(subtract(floor_hit->position,middle_hit->position));
        const auto minimum=(static_cast<long double>(distance)/span+1.L)*difference+pose_hit->position[1];
        if(position[1]<minimum&&position[1]<original_position[1])position[1]=floor_hit->position[1]=original_position[1];
    }
    auto direction=backward?subtract(position,pose_hit->position):subtract(pose_hit->position,position);
    const auto direction_square=(static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[1])*direction[1])+
        static_cast<long double>(direction[2])*direction[2];
    normalize_human_ai_shot(direction,direction_square);
    auto normal=pose_hit->normal;
    for(unsigned axis=0;axis!=3;++axis)normal[axis]=static_cast<float>((static_cast<long double>(normal[axis])+floor_hit->normal[axis])*.5f);
    const float surface_angle=static_cast<float>(human_motion_planar_angle(original_direction,normal));
    const float tilt=frames.car_vector_angle(normal,{{0,1,0,0}});
    const float yaw=static_cast<float>(-human_motion_planar_angle(original_direction,direction));
    frames.car_transform_in_place(direction,frames.human_y_rotation(yaw));
    const auto normalize=[&]{
        normalize_human_ai_shot(direction,(static_cast<long double>(direction[2])*direction[2]+static_cast<long double>(direction[0])*direction[0])+
            static_cast<long double>(direction[1])*direction[1]);
    };
    normalize();
    if(pitch_limit<std::abs(static_cast<long double>(original_direction[1])-direction[1])){
        direction[1]=original_direction[1]+(direction[1]<=original_direction[1]?-pitch_limit:pitch_limit);
        normalize();
    }
    auto count=words.template get<std::int32_t>(0xa18);
    const auto previous=words.template get<std::uint32_t>(0xa14);
    const bool upward=direction[1]>original_direction[1],downward=direction[1]<original_direction[1];
    if((upward&&previous==1)||(downward&&previous==0)){
        count=human_frame_signed_word(static_cast<std::uint32_t>(count)+1u);
        words.set(0xa18,count);
        if(static_cast<float>(count)>=6.f)words.set(0xa18,human_animation_time_integer(6.L+(upward?45.f:85.f)-elapsed/10));
    }else if(count>0)words.set(0xa18,count-1);
    if(upward||downward)words.set(0xa14,downward?1u:0u);
    if(static_cast<float>(words.template get<std::int32_t>(0xa18))<6.f&&animation!=128&&animation!=138){
        if(!lateral){
            const float signed_tilt=surface_angle<=0?-tilt:tilt;
            auto angle=std::abs(surface_angle);
            if(angle>1.5707963705062866f)angle=3.1415927410125732f-angle;
            auto roll=static_cast<float>(static_cast<long double>(angle)*signed_tilt*.6366197466850281f);
            const float limit=static_cast<float>(static_cast<long double>(elapsed)*.001f);
            if(limit<std::abs(static_cast<long double>(roll)-previous_roll))roll=previous_roll+(roll<=previous_roll?-limit:limit);
            words.set(0x250,roll);
        }
        direction[3]=original_direction[3];
        human.base().world_direction(direction);control.movement().frame_words().set(0x50,std::uint8_t{1});
    }
    if((control.mode_flags()&7u)==7u||control.human_is_player()||control.animation_authority_actor()||
            words.template get<std::int32_t>(0x1ac)!=1){
        human.base().world_position(position);control.movement().frame_words().set(0x51,std::uint8_t{1});
    }
    if(words.template get<std::int32_t>(0x1ac)!=1&&words.template get<std::int32_t>(0x9c0)<200&&
            words.template get<float>(0x9f8)==-10000.099609375f){
        const auto draw=frames.volume_random_unit(); //Actual engine RandFloat, separate from shared CRT draws.
        const auto bone=physical.bones().at(draw<=.6f?4u:draw<=.85f?5u:7u);
        if(!bone)throw std::runtime_error("Human air contact requires its actual selected retained bone");
        const auto matrix=frames.actor_world_matrix(bone);
        HumanVector origin{{matrix[12],matrix[13]+.1f,matrix[14],matrix[15]}};
        if(const auto hit=services.human_geometry_ray(origin,{{0,-.4f,0,0}},0xffffffffu,0x230000u)){
            words.set(0x9f0,hit->position);words.set(0xa00,hit->normal);
        }
    }
}

} //namespace hd2::game
#endif
