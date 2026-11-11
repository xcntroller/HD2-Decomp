#ifndef HD2_GAME_HUMAN_RETAINED_CLIP_MOTION_H
#define HD2_GAME_HUMAN_RETAINED_CLIP_MOTION_H

#include "game/human_clip_pose.h"
#include "game/human_bone_direction.h"
#include "game/human_ai_shot_direction.h"
#include "game/human_animation_requests.h"
#include <exception>

namespace hd2::game {
namespace human_retained_clip {
using Vector=HumanVector;
inline Vector add(Vector a,const Vector& b){for(unsigned i=0;i!=3;++i)a[i]+=b[i];return a;}
inline Vector subtract(Vector a,const Vector& b){for(unsigned i=0;i!=3;++i)a[i]-=b[i];return a;}
inline Vector scale(Vector a,float amount){for(unsigned i=0;i!=3;++i)a[i]*=amount;return a;}
inline long double dot(const Vector& a,const Vector& b){
    return (static_cast<long double>(a[1])*b[1]+static_cast<long double>(a[2])*b[2])+static_cast<long double>(a[0])*b[0];
}
inline Vector normalize(Vector a){normalize_human_ai_shot(a,dot(a,a));return a;}
template<class Frames>Vector position(Frames& frames,typename Frames::Frame frame){
    const auto world=frames.actor_world_matrix(frame);return {{world[12],world[13],world[14],world[15]}};
}

//007B3C80 reason3. A successful transition returns immediately:007B0910
//replaces the SAME retained effect, invalidating the incoming reference.
template<class Frames,class Skeleton,class Effect,class Control,class Services>
bool check_seven_positions(Frames& frames,const Skeleton& skeleton,typename Frames::Frame model,
        Effect& effect,std::uint32_t elapsed,Control& control,Services& services){
    if(effect.contact_positions.size()!=7)throw std::logic_error("Human reaction clip requires its actual seven retained positions");
    const auto bone=[&](unsigned n){return position(frames,skeleton.bones()[n].frame);};
    const auto transition=[&](std::uint32_t flags,const Vector& delta,bool velocity){
        const float factor=static_cast<float>(1000.L/human_frame_signed_word(elapsed));
        control.begin_geometry_effect(flags,velocity?scale(delta,factor):Vector{},typename Frames::Frame{},4,nullptr);
    };
    const auto ground=[&](Vector probe){
        probe[1]=position(frames,model)[1]+.5f;
        return services.human_geometry_ray(probe,{{0,-1.5f,0,0}},0xcc002u,0x230000u,false).has_value();
    };
    auto& saved=effect.contact_positions;
    auto previous=add(scale(saved[0],.5f),scale(saved[1],.5f));
    const auto upper_first=scale(bone(6),.5f),upper_last=scale(bone(4),.5f);
    auto current=add(upper_first,upper_last);
    auto delta=subtract(current,previous);
    if(services.human_geometry_sweep().sweep(previous,delta,.2f,0xcc002u,0x230000u)){
        transition(1,delta,true);return true;
    }
    if(!ground(current)){transition(0x1000u,Vector{},false);return true;}
    previous=add(scale(saved[4],.2f),add(scale(saved[3],.3f),add(scale(saved[2],.3f),scale(saved[1],.2f))));
    const auto middle_first=scale(bone(9),.2f),middle_second=scale(bone(13),.3f),
        middle_third=scale(bone(8),.3f),middle_last=scale(bone(4),.2f);
    current=add(middle_first,add(middle_second,add(middle_third,middle_last)));
    delta=subtract(current,previous);
    if(services.human_geometry_sweep().sweep(previous,delta,.25f,0xcc002u,0x230000u)){
        transition(8,delta,true);return true;
    }
    if(!ground(current)){transition(0x1000u,Vector{},false);return true;}
    //The extra .2*root contribution is present in the original, even though
    //these coefficients total1.2. Do not replace it with an ordinary midpoint.
    previous=add(scale(saved[6],.5f),add(scale(saved[5],.5f),scale(saved[4],.2f)));
    const auto lower_first=scale(bone(3),.5f),lower_middle=scale(bone(2),.5f),lower_last=scale(bone(9),.2f);
    current=add(lower_first,add(lower_middle,lower_last));
    delta=subtract(current,previous);
    if(services.human_geometry_sweep().sweep(previous,delta,.2f,0xcc002u,0x230000u)){
        transition(8,delta,true);return true;
    }
    if(!ground(current)){transition(0x1000u,Vector{},false);return true;}
    constexpr std::array<unsigned,7> indices{{6,4,8,13,9,2,3}};
    for(unsigned n=0;n!=indices.size();++n)saved[n]=bone(indices[n]);
    return false;
}

//007B3C80's odd62B8 branch, through the actual World sweep/ray identities.
//Return the middle downward contact selected by the native expiry tail.
template<class Frames,class Skeleton,class Effect,class Human,class Control,class Services>
auto advance_three_positions(Frames& frames,const Skeleton& skeleton,typename Frames::Frame model,
        Effect& effect,std::uint32_t elapsed,void* attachment,Human& human,Control& control,Services& services){
    const auto bone=[&](unsigned n){return position(frames,skeleton.bones()[n].frame);};
    const float seconds=static_cast<float>(static_cast<long double>(human_frame_signed_word(elapsed))*.0010000000474974513f);
    effect.velocity[1]=static_cast<float>(effect.velocity[1]-static_cast<long double>(seconds)*45.f);
    const auto initial=frames.actor_world_matrix(model);
    const Vector up{{initial[4],initial[5],initial[6],initial[7]}};
    const auto offset=scale(up,.25f);
    const auto projected=[&](unsigned index){
        const auto origin=position(frames,model);
        Vector point;
        if(index<2)point=bone(index?22u:4u);
        else{const auto first=bone(3),last=bone(2);point=scale(add(first,last),.5f);}
        point=subtract(point,origin);
        const auto projection=dot(point,up);
        for(unsigned axis=0;axis!=3;++axis){
            auto amount=up[axis]*projection;if(axis)amount=static_cast<float>(amount);
            point[axis]=static_cast<float>(point[axis]-amount);
        }
        return add(offset,add(point,position(frames,model)));
    };
    std::array<Vector,3> points{{projected(0),projected(1),projected(2)}};
    const auto original_middle=points[1];
    if(!effect.contact_positions.empty()){
        if(effect.contact_positions.size()!=3)throw std::logic_error("Human reaction clip requires its actual three retained positions");
        for(unsigned n=0;n!=3;++n){
            const auto& previous=effect.contact_positions[n];
            const auto hit=services.human_geometry_sweep().sweep(previous,subtract(points[n],previous),.25f,0xcc102u,0x230000u);
            if(hit)points[n]=hit->position;
        }
    }
    const auto attached=[&](const auto& hit,bool dynamic_only){
        const auto flags=services.human_geometry_flags(hit);
        return attachment&&(flags&(dynamic_only?0x40u:0xc0u))&&services.human_ragdoll_contact_actor(hit)==attachment;
    };
    for(unsigned iteration=0;iteration!=2;++iteration){
        const auto displacement=scale(effect.velocity,seconds);
        std::array<Vector,3> normals{};float fraction=1;unsigned selected=0;
        for(unsigned n=0;n!=3;++n){
            const auto hit=services.human_geometry_sweep().sweep(points[n],displacement,.25f,0xcc102u,0x230000u);
            if(!hit)continue;
            normals[n]=hit->normal;
            if(attached(*hit,true))continue;
            const auto travel=subtract(hit->position,points[n]);
            const float projection=static_cast<float>(dot(travel,displacement));
            const float candidate=projection<=9.99999993922529e-09f?0.f:static_cast<float>(dot(travel,travel)/projection);
            if(candidate<fraction){fraction=candidate;selected=n;}
        }
        const auto travel=scale(displacement,fraction);
        for(auto& point:points)point=add(point,travel);
        if(1.L-fraction<9.99999993922529e-09f)break;
        const float projection=static_cast<float>(dot(effect.velocity,normals[selected]));
        effect.velocity=scale(subtract(effect.velocity,scale(normals[selected],projection)),.8999999761581421f);
        if(fraction<9.99999993922529e-09f||iteration==1)break;
    }
    if(!effect.contact_positions.empty())for(unsigned n=0;n!=3;++n)effect.contact_positions[n]=points[n];
    const auto shifted=subtract(add(points[1],position(frames,model)),original_middle);
    frames.car_suspension_position(model,shifted); //One direct model8; no Scene144 or model24 here.
    using Hit=decltype(services.human_geometry_sweep().sweep(points[0],offset,.25f,0xcc102u,0x230000u));
    std::array<Hit,3> contacts;
    std::array<Vector,3> ground{},normals{};
    const auto downward=scale(up,-10.f);
    for(unsigned n=0;n!=3;++n){
        contacts[n]=services.human_geometry_sweep().sweep(points[n],downward,.25f,0xcc102u,0x230000u);
        if(contacts[n]&&attached(*contacts[n],false))contacts[n].reset();
    }
    for(unsigned n=0;n!=3;++n){
        const auto origin=position(frames,model);
        if(!contacts[n]){
            const float projection=static_cast<float>(dot(subtract(origin,points[n]),up));
            ground[n]=add(scale(up,projection),points[n]);normals[n]={{0,1,0,0}};
        }else{
            normals[n]=contacts[n]->normal;ground[n]=subtract(contacts[n]->position,scale(normals[n],.25f));
            const float projection=static_cast<float>(dot(subtract(ground[n],origin),up));
            if(projection>.75f||projection<-.75f||normals[n][1]<.7070000171661377f){
                normals[n]={{0,1,0,0}};
                if(n==1)contacts[n].reset(); //Only the middle descriptor is nulled by native rejection.
            }
        }
    }
    auto desired=interpolate_human_directions(normals[0],normals[2],.5f);
    desired=interpolate_human_directions(desired,normals[1],.75f);
    const auto axis=normalize(subtract(ground[0],ground[2]));
    const float projection=static_cast<float>(dot(desired,axis));
    desired=normalize(subtract(desired,scale(axis,projection)));
    Vector relative=desired;
    for(unsigned row=0;row!=3;++row){
        const auto world=frames.actor_world_matrix(model);
        relative[row]=static_cast<float>((static_cast<long double>(desired[0])*world[row*4]+
            static_cast<long double>(desired[2])*world[row*4+2])+static_cast<long double>(desired[1])*world[row*4+1]);
    }
    if(relative[1]<.9847999811172485f){
        const float horizontal=static_cast<float>(std::sqrt(1.L-static_cast<long double>(relative[1])*relative[1]));
        if(horizontal<=9.99999993922529e-09f){relative[0]=.1736000031232834f;relative[2]=0;}
        else{const float factor=.1736000031232834f/horizontal;relative[0]*=factor;relative[2]*=factor;}
        relative[1]=.9847999811172485f;
        desired=normalize(frames.collision_sweep_rotate(relative,frames.actor_world_matrix(model)));
    }
    auto world=frames.actor_world_matrix(model),inverse=human_identity();
    if(const auto parent=frames.object_parent(model)){
        inverse=human_transpose(frames.actor_world_matrix(parent));inverse[3]=inverse[7]=inverse[11]=0;
    }
    Vector forward{{world[8],world[9],world[10],world[11]}};
    const float forward_projection=static_cast<float>(dot(desired,forward));
    forward=normalize(subtract(forward,scale(desired,forward_projection)));
    world[0]=static_cast<float>(static_cast<long double>(desired[1])*forward[2]-static_cast<long double>(desired[2])*forward[1]);
    world[1]=static_cast<float>(static_cast<long double>(desired[2])*forward[0]-static_cast<long double>(desired[0])*forward[2]);
    world[2]=static_cast<float>(static_cast<long double>(desired[0])*forward[1]-static_cast<long double>(desired[1])*forward[0]);
    for(unsigned n=0;n!=4;++n){world[4+n]=desired[n];world[8+n]=forward[n];}
    const auto roll=human_matrix_roll(frames,world);
    human.base().world_direction(forward);control.physical().control_frame_words().set(0x250,roll);
    frames.human_publish_bone_rotation(model,frames.car_matrix_product(world,inverse));
    return contacts[1];
}
} //namespace human_retained_clip

//007B3C80: borrowed clip/cursors and6840 positions belong to the containing
//geometry effect. No new animation resource, camera or collision graph is owned here.
template<class Frames,class Skeleton,class Effect,class Movement,class Human,class Control,class Services>
void present_human_retained_clip(Frames& frames,const Skeleton& skeleton,typename Frames::Frame model,
        Effect& effect,Movement& movement,std::uint32_t seed,std::uint32_t elapsed,void* attachment,
        bool& surface_emitted,Human& human,Control& control,Services& services){
    if(!effect.clip)return;
    effect.clip_time=static_cast<std::uint32_t>(human_animation_time_integer(
        static_cast<long double>(human_frame_signed_word(elapsed))*1.5f+human_frame_signed_word(effect.clip_time)));
    const auto time=human_frame_signed_word(effect.clip_time);
    auto sample=time;
    if(sample>frames.human_clip_duration(effect.clip))sample=frames.human_clip_duration(effect.clip);
    apply_human_ragdoll_clip_pose(frames,skeleton,effect.clip,effect.position_cursors,effect.rotation_cursors,sample,skeleton.root());
    frames.update_scene_object(model);
    auto& owner=services.human_inline(human);
    try{
        using namespace human_retained_clip;
        using Hit=decltype(services.human_geometry_sweep().sweep(Vector{},Vector{},.25f,0u,0u));
        Hit contact;
        if(effect.reason==3){
            owner.exclude_query(false);
            if(check_seven_positions(frames,skeleton,model,effect,elapsed,control,services))return;
            owner.restore_query(false);
        }else if(effect.reason!=2&&(seed&1u)){
            owner.exclude_query(false);
            contact=advance_three_positions(frames,skeleton,model,effect,elapsed,attachment,human,control,services);
        }
        const auto duration=frames.human_clip_duration(effect.clip);
        if(time>human_frame_signed_word(static_cast<std::uint32_t>(duration)+300u)&&
           (effect.reason==2||effect.reason==3||dot(effect.velocity,effect.velocity)<.0001||
            time>human_frame_signed_word(static_cast<std::uint32_t>(frames.human_clip_duration(effect.clip))+10000u))){
            movement.field6304&=~0x200u;
            control.refresh_ragdoll_geometry(true,true); //Actual00786E50, AFTER retiring6304.
            if(!surface_emitted&&contact&&!(services.human_geometry_flags(*contact)&0x40u)){
                const auto world=frames.actor_world_matrix(model);
                const auto point=position(frames,skeleton.bones()[22].frame);
                control.request_geometry_surface(point,Vector{{world[4],world[5],world[6],world[7]}});surface_emitted=true;
            }
        }
        if(control.human_is_player()&&services.human_geometry_camera_target()==control.geometry().camera_fpv()){
            const auto target=position(frames,skeleton.bones()[5].frame);
            const auto hit=services.human_geometry_sweep().sweep(effect.camera_position,subtract(target,effect.camera_position),.17000000178813934f,0xc8002u,0x230000u);
            effect.camera_position=hit?hit->position:position(frames,skeleton.bones()[5].frame);
            frames.car_suspension_position(control.geometry().camera_fpv(),effect.camera_position);
        }
        owner.restore_query(false); //Native common tail also clears exclusions when reason2/even62B8 skipped exclusion.
    }catch(...){
        const auto primary=std::current_exception();
        try{owner.restore_query(false);}catch(...){frames.record_actor_cleanup_failure(std::current_exception());}
        std::rethrow_exception(primary);
    }
}
} //namespace hd2::game
#endif
