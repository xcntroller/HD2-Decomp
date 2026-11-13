#ifndef HD2_GAME_HUMAN_JOINT_POSE_H
#define HD2_GAME_HUMAN_JOINT_POSE_H

#include "game/human_bone_direction.h"
#include "game/human_ai_shot_direction.h"

namespace hd2::game {

//Shared publication in007A4E80/007A50F0. The supplied world rotation is
//converted through the hand's actual engine parent, preserving its raw B0.
template<class Frames>
void publish_human_hand_rotation(Frames& frames,typename Frames::Frame hand,HumanMatrix world){
    world[12]=world[13]=world[14]=0;
    const auto parent=frames.object_parent(hand);
    if(!parent)throw std::logic_error("Human hand orientation requires its actual frame parent");
    auto inverse=human_transpose(frames.actor_world_matrix(parent));
    inverse[3]=inverse[7]=inverse[11]=0;
    world=frames.car_matrix_product(world,inverse);
    frames.human_publish_bone_rotation(hand,world);
}

//007A4E80 queries and publishes the left hand before querying the right source.
template<class Frames>
void pose_human_hand_frames(Frames& frames,const HumanSkeleton<typename Frames::Frame>& skeleton,
        typename Frames::Frame left,typename Frames::Frame right){
    if(left)publish_human_hand_rotation(frames,skeleton.bones()[21].frame,frames.actor_world_matrix(left));
    if(right)publish_human_hand_rotation(frames,skeleton.bones()[20].frame,frames.actor_world_matrix(right));
}

//007A5330 changes the joint's radial direction around its parent/first-child
//axis, retaining its original radius and the child's world orientation.
template<class Frames>
void pose_human_joint_radial(Frames& frames,const HumanSkeleton<typename Frames::Frame>& skeleton,
        typename Frames::Frame model,unsigned index,const HumanVector& preferred,float influence){
    const auto& bone=skeleton.bones().at(index);
    if(bone.parent<0||bone.children[0]<0)return;
    const auto parent=skeleton.bones()[static_cast<std::size_t>(bone.parent)].frame;
    const auto& child=skeleton.bones()[static_cast<std::size_t>(bone.children[0])];
    auto origin=frames.actor_world_matrix(parent);
    const auto end=frames.actor_world_matrix(child.frame);
    HumanVector axis{};
    for(unsigned component=0;component!=3;++component)axis[component]=end[12+component]-origin[12+component];
    const auto square=(static_cast<long double>(axis[2])*axis[2]+static_cast<long double>(axis[1])*axis[1])+
        static_cast<long double>(axis[0])*axis[0];
    normalize_human_ai_shot(axis,square);
    origin=frames.actor_world_matrix(parent);
    const auto selected=frames.actor_world_matrix(bone.frame);
    HumanVector current{};
    for(unsigned component=0;component!=3;++component)current[component]=selected[12+component]-origin[12+component];
    const auto project=[&](const HumanVector& value){
        const auto dot=(static_cast<long double>(value[2])*axis[2]+static_cast<long double>(value[1])*axis[1])+
            static_cast<long double>(value[0])*axis[0];
        HumanVector projection=value;
        for(unsigned component=0;component!=3;++component)projection[component]=static_cast<float>(dot*axis[component]);
        return projection;
    };
    const auto projection=project(current);
    HumanVector radial=current;
    for(unsigned component=0;component!=3;++component)radial[component]-=projection[component];
    const float radius_square=static_cast<float>((static_cast<long double>(radial[2])*radial[2]+
        static_cast<long double>(radial[1])*radial[1])+static_cast<long double>(radial[0])*radial[0]);
    const auto magnitude=[](float value)->std::optional<long double>{
        std::int32_t bits;std::memcpy(&bits,&value,4);
        if(bits<0x322bcc77)return std::nullopt;
        if(std::abs(static_cast<long double>(value)-1)<9.99999993922529e-09f)return 1.L;
        const auto length=std::sqrt(static_cast<long double>(value));
        if(length<9.99999993922529e-09f)return std::nullopt;
        return length;
    };
    const auto old_length=magnitude(radius_square);
    if(!old_length)return;
    const float radius=static_cast<float>(*old_length);
    const auto desired_projection=project(preferred);
    HumanVector desired=preferred;
    const auto complement=1.L-influence;
    for(unsigned component=0;component!=3;++component){
        const float perpendicular=preferred[component]-desired_projection[component];
        const float target=perpendicular*influence;
        const float previous=static_cast<float>(radial[component]*complement);
        desired[component]=target+previous;
    }
    const float desired_square=static_cast<float>((static_cast<long double>(desired[0])*desired[0]+
        static_cast<long double>(desired[1])*desired[1])+static_cast<long double>(desired[2])*desired[2]);
    const auto desired_length=magnitude(desired_square);
    if(!desired_length)return;
    const auto scale=static_cast<long double>(radius)/ *desired_length;
    for(unsigned component=0;component!=3;++component)desired[component]=static_cast<float>(desired[component]*scale);
    const auto saved=frames.actor_world_matrix(child.frame);
    auto direction=project(current);
    for(unsigned component=0;component!=3;++component)direction[component]+=desired[component];
    orient_human_bone(frames,skeleton,model,static_cast<unsigned>(bone.parent),direction,-1.f);
    const auto updated=frames.actor_world_matrix(bone.frame);
    for(unsigned component=0;component!=3;++component)direction[component]=saved[12+component]-updated[12+component];
    orient_human_bone(frames,skeleton,model,index,direction,-1.f);
    if(child.parent<0)throw std::logic_error("Human radial joint requires its child's retained skeleton parent");
    auto inverse=human_transpose(frames.actor_world_matrix(skeleton.bones()[static_cast<std::size_t>(child.parent)].frame));
    inverse[3]=inverse[7]=inverse[11]=0;
    frames.human_publish_bone_rotation(child.frame,frames.car_matrix_product(saved,inverse));
}

} //namespace hd2::game
#endif
