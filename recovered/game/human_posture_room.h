#ifndef HD2_GAME_HUMAN_POSTURE_ROOM_H
#define HD2_GAME_HUMAN_POSTURE_ROOM_H

#include "game/human_motion_velocity.h"
#include <utility>

namespace hd2::game {

//00749D60 uses the actual CRT-initialized vectors009831B0/C0/D0. These
//asymmetric vectors must retain their original components through the actual
//00788490 engine rotation. The selector is restricted to the two real callers.
template<class Frames>HumanVector human_posture_side_direction(Frames& frames,const HumanVector& forward,unsigned kind){
    constexpr std::array<std::array<std::uint32_t,3>,2> vectors{{
        {{0xbb8a47edu,0xbd64c943u,0x3f171de7u}},{{0x3d3d9a95u,0xbed09e56u,0xbf06a162u}}
    }};
    HumanVector value{},reference{};
    const auto& selected=vectors.at(kind);
    for(unsigned axis=0;axis!=3;++axis)std::memcpy(&value[axis],&selected[axis],4);
    const std::uint32_t x=0x3ca3d450u,z=0xbf7ff2e5u;
    std::memcpy(&reference[0],&x,4);std::memcpy(&reference[2],&z,4);
    const float angle=static_cast<float>(human_motion_planar_angle(reference,forward));
    return frames.car_transform(value,frames.human_axis_rotation({{0,1,0,0}},angle));
}

//0074EF60, actual six posture-room queries and displacement/queued retry
//producers. Every query uses the SAME physical descriptors and process sweep.
template<class Control,class Look,class Human,class Services,class Frames,class Sweep>
bool human_posture_room_available(Control& control,Look& look,Human& human,
        Services& services,Frames& frames,Sweep& sweep,unsigned kind){
    if((control.mode_flags()&7u)!=7u&&!control.human_is_player()&&!control.animation_authority_actor())return true;
    auto& physical=control.physical();auto& state=physical.control_frame_words();
    auto& movement=control.movement().frame_words();
    const auto word=[&](unsigned offset){return state.template get<std::int32_t>(offset);};
    const auto scale=[](HumanVector value,float factor){
        for(unsigned axis=0;axis!=3;++axis)value[axis]=static_cast<float>(static_cast<long double>(value[axis])*factor);
        return value;
    };
    const auto query=[&](const HumanVector& origin,const HumanVector& displacement,float radius,std::uint32_t layers){
        physical.exclude_query(false);
        using Physical=std::remove_reference_t<decltype(physical)>;
        struct Restore {Physical& physical;~Restore(){physical.restore_query(false);}} restore{physical};
        return sweep.sweep(origin,displacement,radius,layers,0x230000u);
    };
    const auto pair=[&](const HumanVector& first,const HumanVector& second,const HumanVector& first_direction,
            const HumanVector& second_direction,float first_radius,float second_radius,std::uint32_t layers){
        physical.exclude_query(false);
        using Physical=std::remove_reference_t<decltype(physical)>;
        struct Restore {Physical& physical;~Restore(){physical.restore_query(false);}} restore{physical};
        auto a=sweep.sweep(first,first_direction,first_radius,layers,0x230000u);
        auto b=sweep.sweep(second,second_direction,second_radius,layers,0x230000u);
        return std::make_pair(std::move(a),std::move(b));
    };
    const auto duration=[&](float value){
        const auto result=static_cast<long double>(value)/services.human_control_animation_scale();
        if(!std::isfinite(result)||result< -0x1p63L||result>=0x1p63L)return std::int32_t{0};
        return human_frame_signed_word(static_cast<std::uint32_t>(static_cast<std::int64_t>(result)));
    };
    const auto offset=[&](HumanVector origin,const HumanVector& direction,int step){
        origin[0]=static_cast<float>(static_cast<long double>(origin[0])+static_cast<long double>(direction[0])*step);
        for(unsigned axis=1;axis!=3;++axis){
            const float delta=static_cast<float>(static_cast<long double>(direction[axis])*step);
            origin[axis]=static_cast<float>(static_cast<long double>(origin[axis])+delta);
        }
        return origin;
    };
    const auto last_sphere=[&]()->const HumanVector&{
        const auto count=physical.primary_count();
        if(count<1)throw std::runtime_error("Human posture room query requires actual primary spheres");
        return physical.posture_spheres().at(static_cast<std::size_t>(count-1));
    };
    const auto normal_y=[](const auto& hit){return static_cast<float>((static_cast<long double>(hit.normal[2])+hit.normal[0])*0.f+hit.normal[1]);};
    const auto queue_posture=[&](std::uint32_t posture){physical.control_action_queue().push_back({{250u,posture,0u}});};
    const auto primary=word(0x1d8),secondary=word(0x1e8);
    const bool reloading=secondary>=80&&secondary<=82;
    if(kind==2){
        physical.rebind_frame_posture(look.state().template get<std::uint32_t>(0x4c));
        const auto sphere=last_sphere();const auto position=human.base().world_position();
        HumanVector displacement{{0,static_cast<float>(static_cast<long double>(sphere[3])*.050000011920928955f+
            (static_cast<long double>(physical.offset740()[1])-(static_cast<long double>(sphere[1])-position[1]))),0,0}};
        return !query(sphere,displacement,static_cast<float>(static_cast<long double>(sphere[3])*.95f),0xcc002u);
    }
    if(kind==3){
        physical.rebind_frame_posture(0);
        const auto sphere=physical.posture_spheres().at(0);
        const auto displacement=scale(human.base().world_direction(),static_cast<float>(static_cast<long double>(sphere[3])+.1f));
        const auto hit=query(sphere,displacement,static_cast<float>(static_cast<long double>(last_sphere()[3])*.8f),0xcc002u);
        if(hit&&std::abs(static_cast<long double>(normal_y(*hit)))<static_cast<double>(.8f))
            control.tracks().secondary_motion_impulse(scale(human.base().world_direction(),-.15f),300);
        return true;
    }
    if(kind>5)return true;
    if(kind<2&&reloading){queue_posture(2);return false;}
    if(kind>=4&&(reloading||secondary==35||secondary==36)){queue_posture(kind==5?1u:0u);return false;}
    physical.rebind_frame_posture(kind<2?static_cast<std::uint32_t>(kind!=0):2u);
    const auto forward=human.base().world_direction();
    HumanVector step_direction{{forward[0],0,forward[2],forward[3]}};
    normalize_human_ai_shot(step_direction,static_cast<long double>(forward[2])*forward[2]+static_cast<long double>(forward[0])*forward[0]);
    if(kind<2){
        const bool moving=primary>=190;
        unsigned attempts=4;float spacing=.3333333432674407958984375f;
        if(moving){
            const auto& velocity=human.activation.vector1b0;
            const float speed=static_cast<float>((static_cast<long double>(velocity[0])*velocity[0]+
                static_cast<long double>(velocity[1])*velocity[1])+static_cast<long double>(velocity[2])*velocity[2]);
            if(speed<1){attempts=2;spacing=.25f;}
            else if(speed<2)attempts=2;
            else if(speed<4)attempts=3;
            else if(speed>=9)spacing=.454545438289642333984375f;
        }
        step_direction=scale(step_direction,spacing);
        const auto first_direction=scale(human_posture_side_direction(frames,human.base().world_direction(),1),2);
        const auto second_direction=scale(human_posture_side_direction(frames,human.base().world_direction(),0),2);
        unsigned obstruction=0;
        for(int step=0;;++step){
            if(step>0&&((!moving&&!obstruction)||static_cast<unsigned>(step)>=attempts)){
                if(obstruction)return false;
                if(step>1){
                    const auto delay=duration(kind==0?500.f:400.f);movement.set(0xd0,delay);
                    control.tracks().secondary_motion_impulse(scale(step_direction,static_cast<float>(1-step)),delay);
                    movement.set(0xd4,2u);
                }else movement.set(0xd0,std::int32_t{-10000});
                return true;
            }
            const auto& spheres=physical.posture_spheres();
            const auto first=offset(spheres.at(1),step_direction,moving?step:-step);
            const auto second=offset(spheres.at(0),step_direction,moving?step:-step);
            const auto hits=pair(first,second,first_direction,second_direction,
                static_cast<float>(static_cast<long double>(spheres[1][3])*.95f),
                static_cast<float>(static_cast<long double>(spheres[0][3])*.95f),0xc8002u);
            if(hits.first&&!(normal_y(*hits.first)>.707f)){
                if(!obstruction)obstruction=1;
                else if(obstruction==2)return false;
                if(moving)return false;
            }else if(obstruction==1)obstruction=0;
            if(hits.second&&!(normal_y(*hits.second)>.707f)){
                if(!step&&!obstruction){step_direction=scale(step_direction,-1);obstruction=2;}
                else if(obstruction==1)return false;
                if(moving)return false;
            }else if(obstruction==2)obstruction=0;
            if(!hits.first&&!hits.second)obstruction=0;
        }
    }
    step_direction=scale(step_direction,.3333333432674407958984375f);
    const auto first_direction=scale(human_posture_side_direction(frames,human.base().world_direction(),1),-1.4f);
    const auto second_direction=scale(human_posture_side_direction(frames,human.base().world_direction(),0),-1.4f);
    unsigned obstruction=0;
    for(int step=0;;++step){
        if(step>0){
            if(!obstruction){
                auto position=human.base().world_position();position[1]=static_cast<float>(static_cast<long double>(position[1])+.05f);
                if(step>1)position=offset(position,step_direction,step-1);
                if(query(position,{{0,kind==4?1.5f:1.f,0,0}},.25f,0xc9002u))return false;
                if(step>1){
                    const auto delay=duration(100.f);movement.set(0xd0,delay);
                    control.tracks().secondary_motion_impulse(scale(step_direction,static_cast<float>(step-1)),delay);
                    movement.set(0xd4,2u);
                }else movement.set(0xd0,std::int32_t{-10000});
                return true;
            }
            if(step>=3)return false;
        }
        const auto& spheres=physical.posture_spheres();
        const auto hits=pair(offset(spheres.at(3),step_direction,-step),offset(spheres.at(0),step_direction,-step),
            first_direction,second_direction,static_cast<float>(static_cast<long double>(spheres[3][3])*1.1f),
            static_cast<float>(static_cast<long double>(spheres[0][3])*1.1f),0xc9002u);
        unsigned pending=0;
        const auto wall=[&](const auto& hit){return hit&&!(frames.car_vector_angle(hit->normal,{{0,1,0,0}})<1.0471975803375244140625f);};
        if(wall(hits.first)){if(!step)obstruction=1;else pending=1;}
        else if(obstruction==1)obstruction=0;
        if(wall(hits.second)){
            if(!obstruction&&!step){step_direction=scale(step_direction,-1);obstruction=2;}
            else {pending=2;if(!obstruction)obstruction=pending;}
        }else if(obstruction==2||!obstruction)obstruction=pending;
        if(!hits.first&&!hits.second)obstruction=0;
    }
}

} //namespace hd2::game
#endif
