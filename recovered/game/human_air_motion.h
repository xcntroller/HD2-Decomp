#ifndef HD2_GAME_HUMAN_AIR_MOTION_H
#define HD2_GAME_HUMAN_AIR_MOTION_H

#include "game/human_ground_motion.h"
#include "game/human_animation_requests.h"
#include "game/human_air_alignment.h"

namespace hd2::game {

template<class Control,class Services,class Frames>
bool query_human_air_edge(Control& control,Services& services,Frames& frames,HumanVector& output){ //00769E40.
    auto& physical=control.physical();
    const auto head=physical.bones().at(4),body=physical.bones().at(7);
    if(!head||!body)throw std::runtime_error("Human air edge query requires its actual retained bones");
    const auto head_matrix=frames.actor_world_matrix(head),body_matrix=frames.actor_world_matrix(body);
    HumanVector difference=output,near_point=output,middle=output,far_point=output;
    for(unsigned axis=0;axis!=3;++axis){
        difference[axis]=body_matrix[12+axis]-head_matrix[12+axis];
        const float front=difference[axis]*1.5f,back=difference[axis]*1.2f;
        far_point[axis]=front+body_matrix[12+axis];
        near_point[axis]=body_matrix[12+axis]-back;
        middle[axis]=body_matrix[12+axis];
        output[axis]=0;
    }
    near_point[1]+=.2f;middle[1]+=.2f;far_point[1]+=.2f;
    const auto hits=[&]{
        struct Restore {decltype(physical)& owner;~Restore(){owner.restore_query(false);}} restore{physical};
        physical.exclude_query(false);
        const HumanVector downward{{0,-1.85f,0,0}};
        return std::array{
            services.human_geometry_ray(near_point,downward,0xffffffffu,0x230000u),
            services.human_geometry_ray(middle,downward,0xffffffffu,0x230000u),
            services.human_geometry_ray(far_point,downward,0xffffffffu,0x230000u)};
    }();
    if(hits[1]||static_cast<bool>(hits[0])==static_cast<bool>(hits[2]))return false;
    const auto& normal=(hits[0]?hits[0]:hits[2])->normal;
    if((static_cast<long double>(normal[2])+normal[0])*0.f+normal[1]<=.7f)return false;
    for(unsigned axis=0;axis!=3;++axis)output[axis]=hits[0]?difference[axis]:-difference[axis];
    output[1]=0;
    normalize_human_ai_shot(output,static_cast<long double>(output[2])*output[2]+static_cast<long double>(output[0])*output[0]);
    for(unsigned axis=0;axis!=3;++axis)output[axis]*=.25f;
    return true;
}

template<class Control,class Look,class Human,class Services,class Frames,class Runtime,class Publish>
void advance_human_air_collision(Control& control,Look& look,Human& human,Services& services,Frames& frames,
        Runtime& runtime,Publish& publish,std::int32_t elapsed){ //0076A310, periodic100-ms air collision.
    using namespace collision_motion_math;
    auto& physical=control.physical();auto& words=physical.control_frame_words();
    auto& movement=control.movement().frame_words();auto& tracks=control.tracks();
    (void)frames.actor_world_matrix(human.base().frame());
    (void)frames.actor_world_matrix(human.base().frame()); //Native performs both cache reads before copying ActorBase position.
    const auto original=human.base().world_position();
    const auto offset=subtract(original,physical.posture_spheres()[0]);
    if(words.template get<std::int32_t>(0x9cc)==-10000)words.set(0x9cc,1400u);
    auto displacement=words.template get<HumanVector>(0x4d0);
    displacement[1]=static_cast<float>(displacement[1]-human_fall_displacement(elapsed,
        movement.template get<float>(0xf4),movement.template get<float>(0xf8)));
    movement.set(0x10,displacement);
    using Hit=typename std::remove_reference_t<decltype(services.human_geometry_sweep())>::Hit;
    CollisionMotion<Hit> request;
    const auto count=physical.primary_count();
    if(count<1||static_cast<std::size_t>(count)>request.centers.size())
        throw std::runtime_error("Human air collision requires its actual posture spheres");
    request.count=static_cast<std::size_t>(count);
    for(std::size_t index=0;index<request.count;++index){
        request.centers[index]=physical.posture_spheres().at(index);
        request.radii[index]=static_cast<float>(static_cast<long double>(request.centers[index][3])*.85);
    }
    request.displacement=displacement;request.excluded_layers=0xcc002u;request.excluded_flags=0x230000u;
    const auto hit=[&]{
        struct Restore {decltype(physical)& owner;~Restore(){owner.restore_query(false);}} restore{physical};
        physical.exclude_query(false);
        return advance_collision_motion(request,services,physical.frame_globals().motion_iterations);
    }();
    HumanVector normal{};std::optional<Hit> ground;
    bool steep=select_human_ground_contact(request,true,normal,ground);
    const bool meaningful=(static_cast<long double>(normal[0])*normal[0]+static_cast<long double>(normal[1])*normal[1])+
        static_cast<long double>(normal[2])*normal[2]>=1.e-16f;
    advance_human_walk_fall(control,look,human,services,hit,steep,0,elapsed!=0,true,runtime,publish); //Native passes elapsed0 to00767530.
    auto edge=tracks.words().template get<HumanVector>(0x2e0);
    if(query_human_air_edge(control,services,frames,edge)){
        steep=false;
        tracks.secondary_motion_impulse(edge,human_animation_time_integer(300.L/services.human_control_animation_scale()));
    }
    const bool local=(control.mode_flags()&7u)==7u||control.human_is_player()||control.animation_authority_actor();
    if(!hit){
        words.set(0x9cc,words.template get<std::uint32_t>(0x9cc)+static_cast<std::uint32_t>(elapsed));
        if(local){human.base().world_position(add(human.base().world_position(),movement.template get<HumanVector>(0x10)));movement.set(0x51,std::uint8_t{1});}
        return;
    }
    displacement=movement.template get<HumanVector>(0x10);
    const float distance=length(displacement);
    long double angle=0;
    if(request.contact_count){
        const auto& last=request.contacts[request.contact_count-1]->normal;
        angle=static_cast<long double>(frames.car_vector_angle(last,{{0,1,0,0}}))*.6366197466850281f;
    }
    if(!meaningful)angle=0;
    const float seconds=static_cast<float>(static_cast<long double>(elapsed)*.001f);
    if(!steep){
        words.set(0x9cc,human_animation_time_integer(static_cast<long double>(elapsed)*elapsed+words.template get<std::int32_t>(0x9cc)));
        movement.set(0xf4,static_cast<float>(movement.template get<float>(0xf4)-(1.L-angle)*9.81f*seconds));
    }else{
        const auto complement=1.L-angle;
        words.set(0x9cc,human_animation_time_integer(words.template get<std::int32_t>(0x9cc)-static_cast<long double>(elapsed/2)*complement));
        const auto speed=movement.template get<float>(0xf4)-complement*9.81f*seconds;
        movement.set(0xf4,speed<0?0.f:static_cast<float>(speed));
    }
    if(request.contact_count){
        const float rate=static_cast<float>(1000.L/static_cast<float>(elapsed));
        if(static_cast<long double>(distance)*rate>.4f){
            displacement=scale(displacement,1.L/distance);movement.set(0x10,displacement);
            const auto& last=request.contacts[request.contact_count-1]->normal;
            const auto reverse=scale(displacement,-1.L);
            const float impact_angle=frames.car_vector_angle(reverse,last);
            const float retained=std::min(1.f,static_cast<float>(static_cast<long double>(impact_angle)*.6366197466850281f));
            HumanVector axis{{
                static_cast<float>(static_cast<long double>(last[2])*displacement[1]-static_cast<long double>(displacement[2])*last[1]),
                static_cast<float>(static_cast<long double>(displacement[2])*last[0]-static_cast<long double>(displacement[0])*last[2]),
                static_cast<float>(static_cast<long double>(displacement[0])*last[1]-static_cast<long double>(last[0])*displacement[1]),0}};
            auto reflected=reverse;
            const auto axis_square=(static_cast<long double>(axis[1])*axis[1]+static_cast<long double>(axis[2])*axis[2])+static_cast<long double>(axis[0])*axis[0];
            if(axis_square>=1.e-16f){
                normalize_human_ai_shot(axis,axis_square);
                frames.car_transform_in_place(reflected,frames.human_axis_rotation(axis,impact_angle+impact_angle));
            }
            const float speed=static_cast<float>((1.L-retained)*distance*.5f);
            if(static_cast<long double>(rate)*speed>.15f){
                reflected=scale(reflected,speed);
                tracks.motion_deceleration(reflected,human_animation_time_integer(1000.L/services.human_control_animation_scale()));
            }
        }
    }
    (void)resolve_human_motion_contacts(control,look,human,services,request,false,runtime);
    if(local||words.template get<std::int32_t>(0x1ac)!=1){
        auto position=add(request.position,offset);position[3]=human.base().world_position()[3];
        human.base().world_position(position);movement.set(0x51,std::uint8_t{1});
    }
}

template<class Control,class Look,class Human,class Services,class Frames,class Runtime,class Publish>
void advance_human_air_motion(Control& control,Look& look,Human& human,Services& services,Frames& frames,
        Runtime& runtime,Publish& publish,std::uint32_t elapsed){ //00769920.
    auto& physical=control.physical();auto& words=physical.control_frame_words();
    auto& movement=control.movement().frame_words();
    (void)frames.actor_world_matrix(human.base().frame());
    const auto status=[&]{return words.template get<std::int32_t>(0x1ac);};
    const auto mark_shadow=[&]{if(const auto shadow=control.shadow().frame())frames.mark_human_air_shadow(shadow);};
    const auto deactivate_face=[&]{
        words.set(0x1ac,0u);words.set(0x18c,std::uint8_t{1});
        control.face().activate_air_pose(); //00781310, SAME face and shared morph-table owners.
    };
    if(status()<7){
        physical.refresh_shot_geometry();physical.publish_air_shots();
        words.set(0x9c0,0u);words.set(0x9b8,0u);
        physical.rebind_frame_posture(look.state().template get<std::uint32_t>(0x4c));
        mark_shadow();deactivate_face();return;
    }
    if(words.template get<std::uint32_t>(0x9d0)==0xc61c4000u){words.set(0x9d0,0.f);return;}
    if(words.template get<std::int32_t>(0x9cc)!=-10000){
        const auto remaining=words.template get<std::uint32_t>(0x9cc)-elapsed;
        words.set(0x9cc,remaining);
        if(human_frame_signed_word(remaining)<1){words.set(0x1ac,9u);words.set(0x18c,std::uint8_t{1});words.set(0x9cc,0u);}
    }
    const auto remaining_motion=words.template get<std::uint32_t>(0x9b8)-elapsed;
    words.set(0x9b8,human_frame_signed_word(remaining_motion)<0?0u:remaining_motion);
    const auto remaining_velocity=words.template get<std::uint32_t>(0x9bc)-elapsed;
    words.set(0x9bc,remaining_velocity);
    if(human_frame_signed_word(remaining_velocity)<0){
        if(status()==2||status()==9){
            auto output=movement.template get<HumanVector>(0xb0);
            control.tracks().motion_velocity(human.base().world_direction(),0,0,output);
            control.movement().control_frame_track_velocity(output);
        }
        words.set(0x9bc,0u);
    }
    if(status()!=3&&status()!=4){
        const auto remaining=words.template get<std::uint32_t>(0x9c0)-elapsed;
        words.set(0x9c0,remaining);
        if(human_frame_signed_word(remaining)<0){words.set(0x9c0,0u);deactivate_face();mark_shadow();}
        const auto secondary=words.template get<std::uint32_t>(0x9c4)-elapsed;
        words.set(0x9c4,human_frame_signed_word(secondary)<0?0u:secondary);
    }
    if(status()==8){
        auto direction=human.base().world_direction();
        const auto vertical=direction[1];
        if(vertical!=0){
            long double change=-static_cast<long double>(vertical);
            if(std::abs(change)>=static_cast<float>(.05000000074505806)){
                const float duration=static_cast<float>(human_frame_signed_word(elapsed));
                if(static_cast<long double>(duration)*.0008999999845400453f<std::abs(change))
                    change=(change<=0?-1.L:1.L)*duration*.0008999999845400453f;
                direction[1]=static_cast<float>(change+vertical);
            }else{direction[1]=0;words.set(0x98c,std::uint8_t{0});}
            normalize_human_ai_shot(direction,(static_cast<long double>(direction[1])*direction[1]+
                static_cast<long double>(direction[0])*direction[0])+static_cast<long double>(direction[2])*direction[2]);
            human.base().world_direction(direction);movement.set(0x50,std::uint8_t{1});
        }
        return;
    }
    const auto current=status();
    if(current==2||current==6||current==0||current==9||
            ((current==3||current==4)&&words.template get<std::int32_t>(0x9cc)!=-10000))
        align_human_air_motion(control,human,services,frames,human_frame_signed_word(elapsed)); //0076DF20, SAME terrain/pose owners.
    const auto accumulated=movement.template get<std::uint32_t>(0x9c)+elapsed;
    movement.set(0x9c,accumulated);
    if(human_frame_signed_word(accumulated)>99){
        movement.set(0x9c,accumulated-100u); //Native performs one periodic call per frame.
        if(status()==3||status()==6||status()==2)
            advance_human_air_collision(control,look,human,services,frames,runtime,publish,100);
    }
}

} //namespace hd2::game
#endif
