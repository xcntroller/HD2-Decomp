#ifndef HD2_GAME_HUMAN_WATER_MOTION_H
#define HD2_GAME_HUMAN_WATER_MOTION_H

#include "game/human_ground_motion.h"

namespace hd2::game {

template<class Frames>HumanVector rotate_human_pitch(Frames& frames,const HumanVector& direction,float angle){ //007884F0.
    auto matrix=human_identity();
    const auto sine=std::sin(static_cast<long double>(angle));
    const auto cosine=std::cos(static_cast<long double>(angle));
    matrix[5]=matrix[10]=static_cast<float>(cosine);
    matrix[9]=static_cast<float>(sine);
    matrix[6]=static_cast<float>(-sine);
    auto result=frames.collision_sweep_rotate(direction,matrix);
    result[3]=direction[3]; //Native returned-vector W is unwritten.
    return result;
}

template<class Control,class Human,class Frames>
void advance_human_water_direction(Control& control,Human& human,Frames& frames,HumanVector desired,std::int32_t elapsed){ //007694F0.
    const auto original=human.base().world_direction();
    auto square=[](const HumanVector& v){return (static_cast<long double>(v[0])*v[0]+static_cast<long double>(v[2])*v[2])+static_cast<long double>(v[1])*v[1];};
    if(square(desired)<1.e-16f){desired[0]=original[0];desired[2]=original[2];}
    normalize_human_ai_shot(desired,square(desired));
    const float current_angle=frames.car_vector_angle(original,{{0,1,0,0}});
    const float maximum=static_cast<float>(static_cast<long double>(elapsed)*.0015384615398943424f);
    const auto difference=static_cast<long double>(current_angle)-frames.car_vector_angle(desired,{{0,1,0,0}});
    float change=static_cast<float>(difference);
    const auto magnitude=std::abs(difference);
    if(magnitude<=.0001)return;
    if(maximum<magnitude)change=change<=0?-maximum:maximum;
    const float yaw=static_cast<float>(-human_motion_planar_angle({{0,0,1,0}},original));
    auto direction=original;
    frames.car_transform_in_place(direction,frames.human_y_rotation(yaw));
    direction=rotate_human_pitch(frames,direction,change);
    frames.car_transform_in_place(direction,frames.human_y_rotation(-yaw));
    normalize_human_ai_shot(direction,square(direction));
    direction[3]=original[3];
    if((control.mode_flags()&7u)==7u||control.human_is_player()||control.animation_authority_actor()||
            control.physical().control_frame_words().template get<std::int32_t>(0x1ac)!=1){
        human.base().world_direction(direction);
        control.movement().frame_words().set(0x50,std::uint8_t{1});
    }
}

template<class Control,class Look,class Human,class Services,class Publish>
void finish_human_water_motion(Control& control,Look& look,Human& human,Services& services,Publish& publish,bool equipment){ //0076AE70.
    look.control_movement_mode(4,false,publish);
    look.force_frame_commands(true);
    try{
        if(look.state().template get<std::uint32_t>(0x4c)<=2)look.frame_movement_animation(190,publish);
    }catch(...){look.force_frame_commands(false);throw;}
    look.force_frame_commands(false);
    control.physical().control_frame_words().set(0x764,std::int32_t{-1});
    auto& movement=control.movement().frame_words();
    auto amount=.5f;
    const auto depth=movement.template get<float>(0x10c);
    if(depth!=-10000.099609375f&&depth<amount)amount=depth;
    auto impulse=control.tracks().words().template get<HumanVector>(0x280);
    impulse[0]=impulse[2]=0;impulse[1]=-amount;
    control.tracks().motion_impulse(impulse,500);
    movement.set(0x110,std::int32_t{-300});
    if(equipment)services.human_presentation_equipment(human).set_hidden(false);
    control.movement().fade_motion_audio();
}

template<class Control,class Look,class Human,class Services,class Frames,class Runtime,class Publish>
void advance_human_water_motion(Control& control,Look& look,Human& human,Services& services,Frames& frames,
        Runtime& runtime,Publish& publish,std::int32_t elapsed){ //00768E50.
    using namespace collision_motion_math;
    auto& physical=control.physical();auto& words=physical.control_frame_words();
    auto& movement=control.movement().frame_words();
    const float seconds=static_cast<float>(static_cast<long double>(elapsed)*.001f);
    const float milliseconds=static_cast<float>(elapsed);
    const auto roll=words.template get<float>(0x250);
    if(roll!=0){
        auto change=-static_cast<long double>(roll);
        if(std::abs(change)>=.05000000074505806){
            if(static_cast<long double>(milliseconds)*.001f<std::abs(change))
                change=(change<=0?-1.L:1.L)*milliseconds*.001f;
            words.set(0x250,static_cast<float>(change+roll));
        }else words.set(0x250,0.f);
    }
    movement.set(0x98,std::uint8_t{0});
    look.mark_frame_feedback(0x420u);
    decrement_positive_human_state_timer(movement,0x110,static_cast<std::uint32_t>(elapsed));
    if(!query_human_ground_water(control,look,human,services,runtime)&&!movement.template get<std::uint32_t>(0x110)){
        finish_human_water_motion(control,look,human,services,publish,true);
        return;
    }
    physical.rebind_frame_posture(2);
    if(!physical.primary_count())return;
    const auto original=human.base().world_position();
    const auto offset=subtract(original,physical.posture_spheres()[0]);
    auto displacement=words.template get<HumanVector>(0x4d0);
    movement.set(0x10,displacement);
    const auto delay=movement.template get<std::int32_t>(0xd0);
    if(delay!=-10000){
        const auto remaining=human_frame_signed_word(static_cast<std::uint32_t>(delay)-static_cast<std::uint32_t>(elapsed));
        movement.set(0xd0,remaining<1?-10000:remaining);
    }
    auto direction=words.template get<HumanVector>(0x940);
    const auto depth=movement.template get<float>(0x10c);
    bool allow_pitch=false;
    if(static_cast<long double>(depth)-displacement[1]>=.12f){
        allow_pitch=(static_cast<long double>(depth)+.4f)-displacement[1]<=movement.template get<float>(0x108);
        if(!allow_pitch&&displacement[1]<0)displacement[1]=0;
    }else if(displacement[1]>0)displacement[1]=0;
    if(!allow_pitch)direction[1]=0;
    movement.set(0x10,displacement);
    const auto length=(static_cast<long double>(displacement[0])*displacement[0]+static_cast<long double>(displacement[1])*displacement[1])+static_cast<long double>(displacement[2])*displacement[2];
    const bool stationary=length<1.e-16f;
    if(stationary){
        direction[1]=0;
        normalize_human_ai_shot(direction,static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[2])*direction[2]);
    }
    advance_human_water_direction(control,human,frames,direction,elapsed);
    const auto phase=words.template get<float>(0x234);
    if(stationary&&phase==0)return;
    if(phase!=0&&depth>.16f){
        const auto amount=depth<.26f?1./28.:.125;
        displacement[1]=static_cast<float>(std::sin(static_cast<long double>(phase))*seconds*amount+displacement[1]);
        movement.set(0x14,displacement[1]);
    }
    if((control.mode_flags()&7u)!=7u&&!control.human_is_player()&&!control.animation_authority_actor()&&
            words.template get<std::int32_t>(0x1ac)==1)return;
    using Hit=typename std::remove_reference_t<decltype(services.human_geometry_sweep())>::Hit;
    CollisionMotion<Hit> request;
    const auto count=physical.primary_count();
    if(count<1||static_cast<std::size_t>(count)>request.centers.size())
        throw std::runtime_error("Human water motion requires actual posture spheres");
    request.count=static_cast<std::size_t>(count);
    for(std::size_t index=0;index<request.count;++index){
        request.centers[index]=physical.posture_spheres().at(index);
        request.radii[index]=request.centers[index][3];
    }
    request.displacement=displacement;request.excluded_layers=0xc8002u;request.excluded_flags=0x230000u;
    const auto hit=[&]{
        struct Restore {decltype(physical)& owner;~Restore(){owner.restore_query(false);}} restore{physical};
        physical.exclude_query(false);
        return advance_collision_motion(request,services,physical.frame_globals().motion_iterations);
    }();
    request.position=add(request.position,offset);
    if(!hit){
        auto position=add(human.base().world_position(),displacement);
        position[3]=human.base().world_position()[3];
        human.base().world_position(position);movement.set(0x51,std::uint8_t{1});
    }else{
        const auto position=human.base().world_position();
        if(request.position[0]!=position[0]||request.position[1]!=position[1]||request.position[2]!=position[2]){
            (void)resolve_human_motion_contacts(control,look,human,services,request,true,runtime);
            request.position[3]=position[3];
            human.base().world_position(request.position);movement.set(0x51,std::uint8_t{1});
        }
    }
}

} //namespace hd2::game
#endif
