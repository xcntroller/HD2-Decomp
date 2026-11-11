#ifndef HD2_GAME_HUMAN_PRONE_POSE_H
#define HD2_GAME_HUMAN_PRONE_POSE_H

#include "game/human_joint_pose.h"
#include "game/human_attachment_blend.h"
#include "game/human_state_frame_timers.h"
#include <limits>
#include <exception>

namespace hd2::game {

//007A8520's six retained vector groups65FC..6610, contact bytes65F8,
//transition counters6614 and original100ms interpolation duration6618.
template<class Frames>class HumanPronePose {
public:
    using Frame=typename Frames::Frame;
    using Matrix=HumanMatrix;
    using Vector=HumanVector;
    HumanPronePose(){reset();}
    void reset(){
        for(unsigned group=2;group!=5;++group)for(auto& value:vectors_[group])value={{0,1,0,0}};
        vectors_[4][0][0]=std::numeric_limits<float>::quiet_NaN();
        counters_.fill(0);last_direction_={};
    }
    const auto& vectors()const{return vectors_;}
    const auto& contacts()const{return contacts_;}
    const auto& counters()const{return counters_;}
    template<class Human,class Control,class Movement>
    void present(Frames& frames,const HumanSkeleton<Frame>& skeleton,Frame model,Human& human,
            Control& control,Movement& movement,std::uint32_t elapsed,std::uint32_t seed){
        float minimum=1000000.f,maximum=-1000000.f;
        for(auto index:{21u,20u,0u,1u}){
            const auto world=frames.actor_world_matrix(model);
            const auto position=frames.actor_world_matrix(model);
            const auto bone=frames.actor_world_matrix(skeleton.bones()[index].frame);
            const auto x=(static_cast<long double>(bone[12])-position[12])*world[8];
            const auto y=(static_cast<long double>(bone[13])-position[13])*world[9];
            const auto z=(static_cast<long double>(bone[14])-position[14])*world[10];
            const float projection=static_cast<float>(index==21?(x+z)+y:(x+y)+z);
            if(projection>maximum)maximum=projection;
            else if(projection<minimum)minimum=projection;
        }
        if(static_cast<long double>(maximum)-minimum<1){vectors_[4][0][0]=std::numeric_limits<float>::quiet_NaN();return;}
        const auto spheres=control.physical().prone_pose_spheres();
        const bool periodic_normalize=(seed&3u)==0;
        bool transition=human_frame_signed_word(movement.field65f4)>0;
        const auto source=frames.actor_world_matrix(model);
        const Vector up{{source[4],source[5],source[6],source[7]}};
        if(!human_attachment_key_present(vectors_[4][0][0])){
            for(unsigned group=2;group!=5;++group){
                for(unsigned normal=0;normal!=3;++normal)vectors_[group][normal]=up;
                for(unsigned scalar=0;scalar!=3;++scalar)vectors_[group][3][scalar]=0;
            }
            counters_.fill(1);transition=true;
        }
        const auto direction=human.base().world_direction();
        const auto dot=(static_cast<long double>(direction[0])*last_direction_[0]+
            static_cast<long double>(direction[1])*last_direction_[1])+static_cast<long double>(direction[2])*last_direction_[2];
        const float angular_change=static_cast<float>(std::abs(1.L-dot));
        const auto current=frames.actor_world_matrix(model);
        Vector travel{};
        for(unsigned axis=0;axis!=3;++axis)travel[axis]=current[12+axis]-vectors_[4][4][axis];
        const auto distance=(static_cast<long double>(travel[0])*travel[0]+static_cast<long double>(travel[1])*travel[1])+
            static_cast<long double>(travel[2])*travel[2];
        if(transition||distance>9.999999974752427e-07f||angular_change>.0010000000474974513f){
            movement.field65f4-=elapsed;
            if(human_frame_signed_word(movement.field65f4)<0)movement.field65f4=0;
            for(unsigned index=0;index!=4;++index)vectors_[5][index]=spheres[index];
            if((seed&1u)||transition){
                observe_contacts(frames,skeleton,model,control,up,spheres,movement.field65f4!=0);
                observe_directions();
                observe_heights(frames,model,up,spheres);
            }
            interpolate(elapsed);
        }
        publish(frames,skeleton,model,human,control,up,spheres,periodic_normalize);
    }
private:
    static Vector difference(const Vector& first,const Vector& last){
        auto result=first;
        for(unsigned axis=0;axis!=3;++axis)result[axis]-=last[axis];
        return result;
    }
    static Vector normalized(Vector value,unsigned first=1,unsigned middle=2,unsigned last=0){
        const auto square=(static_cast<long double>(value[first])*value[first]+
            static_cast<long double>(value[middle])*value[middle])+static_cast<long double>(value[last])*value[last];
        normalize_human_ai_shot(value,square);
        return value;
    }
    static Vector perpendicular(Vector direction,const Vector& axis){
        const auto dot=(static_cast<long double>(direction[1])*axis[1]+
            static_cast<long double>(direction[2])*axis[2])+static_cast<long double>(direction[0])*axis[0];
        for(unsigned component=0;component!=3;++component){
            const auto projection=dot*axis[component];
            direction[component]=static_cast<float>(direction[component]-(component==2?projection:static_cast<float>(projection)));
        }
        return normalized(direction,1,0,2);
    }
    static void limit_up(Vector& value,float minimum,float tangent){
        if(!(value[1]<minimum))return;
        const auto length=std::sqrt(1.L-static_cast<long double>(value[1])*value[1]);
        if(length<=9.99999993922529e-09f){value[0]=tangent;value[2]=0;}
        else{
            const auto factor=static_cast<long double>(tangent)/length;
            value[0]=static_cast<float>(value[0]*factor);value[2]=static_cast<float>(value[2]*factor);
        }
        value[1]=minimum;
    }
    void retain_normal(unsigned index,const Vector& direction){
        const auto delta=difference(direction,vectors_[3][index]);
        const auto square=(static_cast<long double>(delta[1])*delta[1]+static_cast<long double>(delta[2])*delta[2])+
            static_cast<long double>(delta[0])*delta[0];
        if(!(square>9.999999747378752e-06f))return;
        vectors_[2][index]=vectors_[4][index];vectors_[3][index]=direction;counters_[index]=100;
    }
    template<class Control>void observe_contacts(Frames& frames,const HumanSkeleton<Frame>& skeleton,Frame model,
            Control& control,const Vector& up,const std::array<Vector,4>& spheres,bool transition){
        auto probes=vectors_[5];
        if(transition){
            const auto origin=frames.actor_world_matrix(model);
            const auto head=frames.actor_world_matrix(skeleton.bones()[4].frame);
            const float height=static_cast<float>(((static_cast<long double>(head[12])-origin[12])*up[0]+
                (static_cast<long double>(head[14])-origin[14])*up[2])+(static_cast<long double>(head[13])-origin[13])*up[1]);
            for(unsigned index=0;index!=4;++index){
                const auto world=frames.actor_world_matrix(model);
                const auto& point=vectors_[5][index];
                const auto projection=((static_cast<long double>(point[0])-world[12])*up[0]+
                    (static_cast<long double>(point[2])-world[14])*up[2])+(static_cast<long double>(point[1])-world[13])*up[1];
                if(!(projection<height))continue;
                for(unsigned axis=0;axis!=3;++axis)probes[index][axis]=static_cast<float>(
                    static_cast<long double>(point[axis])+static_cast<long double>(up[axis])*height);
            }
        }
        for(unsigned index=0;index!=4;++index){
            const float radius=static_cast<float>(static_cast<long double>(spheres[index][3])*.8999999761581421f);
            for(unsigned axis=0;axis!=3;++axis)vectors_[0][index][axis]=static_cast<float>(
                vectors_[5][index][axis]-static_cast<long double>(up[axis])*radius);
        }
        contacts_.fill(0);
        control.physical().exclude_query(false);
        try{
            Vector down=up;
            for(unsigned axis=0;axis!=3;++axis)down[axis]*=-10.f;
            for(unsigned index=0;index!=4;++index){
                const float radius=static_cast<float>(static_cast<long double>(spheres[index][3])*.8999999761581421f);
                const auto hit=control.prone_pose_sweep(probes[index],down,radius);
                if(!hit)continue;
                vectors_[0][index]=hit->position;vectors_[1][index]=hit->normal;
                for(unsigned axis=0;axis!=3;++axis){
                    const auto offset=static_cast<long double>(radius)*hit->normal[axis];
                    vectors_[0][index][axis]=static_cast<float>(vectors_[0][index][axis]-(axis?static_cast<float>(offset):offset));
                }
                const auto delta=difference(vectors_[0][index],vectors_[5][index]);
                const auto square=(static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[1])*delta[1])+
                    static_cast<long double>(delta[2])*delta[2];
                contacts_[index]=square>=.25f?2:1;
            }
        }catch(...){
            const auto failure=std::current_exception();
            try{control.physical().restore_query(false);}catch(...){frames.record_actor_cleanup_failure(std::current_exception());}
            std::rethrow_exception(failure);
        }
        control.physical().restore_query(false);
        for(unsigned index=0;index!=4;++index)if(contacts_[index]!=1){
            vectors_[1][index][0]=vectors_[1][index][2]=0;vectors_[1][index][1]=1;
        }
    }
    Vector contact_direction(unsigned first,unsigned last,float weight){
        auto normal=interpolate_human_directions(vectors_[1][first],vectors_[1][last],weight);
        normal=perpendicular(normal,normalized(difference(vectors_[0][first],vectors_[0][last])));
        return normal;
    }
    void observe_directions(){
        auto first=contact_direction(0,1,.25f);limit_up(first,.707099974155426f,.707099974155426f);retain_normal(0,first);
        auto last=contact_direction(2,3,.75f);limit_up(last,.707099974155426f,.707099974155426f);retain_normal(2,last);
        const auto middle=contact_direction(1,2,.5f);
        Vector first_axis{},last_axis{};
        for(unsigned axis=0;axis!=3;++axis){
            const auto first=static_cast<long double>(vectors_[0][0][axis])*.5f;
            const auto middle_first=static_cast<long double>(vectors_[0][2][axis])*.5f;
            first_axis[axis]=static_cast<float>(first-middle_first);
            const auto last=static_cast<long double>(vectors_[0][3][axis])*.5f;
            const auto middle_last=static_cast<long double>(vectors_[0][1][axis])*.5f;
            last_axis[axis]=static_cast<float>(last-middle_last);
        }
        first=perpendicular(middle,normalized(first_axis,0,1,2));
        last=perpendicular(middle,normalized(last_axis,0,1,2));
        auto normal=interpolate_human_directions(first,last,.5f);
        limit_up(normal,.707099974155426f,.707099974155426f);retain_normal(1,normal);
    }
    void observe_heights(Frames& frames,Frame model,const Vector& up,const std::array<Vector,4>& spheres){
        for(unsigned index=0;index!=4;++index)if(contacts_[index]!=1){
            const float radius=static_cast<float>(static_cast<long double>(spheres[index][3])*.8999999761581421f);
            for(unsigned axis=0;axis!=3;++axis)vectors_[0][index][axis]=static_cast<float>(
                vectors_[5][index][axis]-static_cast<long double>(up[axis])*radius);
        }
        for(unsigned section=0;section!=3;++section){
            const auto origin=frames.actor_world_matrix(model);
            Vector centre{};
            for(unsigned axis=0;axis!=3;++axis){
                const auto mean=(static_cast<long double>(vectors_[0][section][axis])+vectors_[0][section+1][axis])*.5f;
                centre[axis]=static_cast<float>(mean-origin[12+axis]);
            }
            const float height=static_cast<float>((static_cast<long double>(centre[0])*up[0]+
                static_cast<long double>(centre[2])*up[2])+static_cast<long double>(centre[1])*up[1]);
            if(!(std::abs(static_cast<long double>(height)-vectors_[3][3][section])>9.999999747378752e-05))continue;
            vectors_[2][3][section]=vectors_[4][3][section];vectors_[3][3][section]=height;counters_[section+3]=100;
        }
    }
    void interpolate(std::uint32_t elapsed){
        for(unsigned index=0;index!=6;++index){
            if(!counters_[index])continue;
            counters_[index]-=elapsed;
            if(human_frame_signed_word(counters_[index])<0)counters_[index]=0;
            const auto fraction=static_cast<long double>(human_frame_signed_word(counters_[index]))/100.f;
            if(index<3){
                const float influence=static_cast<float>(1.L-fraction);
                vectors_[4][index]=interpolate_human_directions(vectors_[2][index],vectors_[3][index],influence);
            }else{
                const auto scalar=index-3;
                const float latest=static_cast<float>((1.L-fraction)*vectors_[3][3][scalar]);
                vectors_[4][3][scalar]=static_cast<float>(fraction*vectors_[2][3][scalar]+latest);
            }
        }
    }
    template<class Human,class Control>void publish(Frames& frames,const HumanSkeleton<Frame>& skeleton,Frame model,
            Human& human,Control& control,const Vector& up,const std::array<Vector,4>& spheres,bool periodic_normalize){
        auto world=frames.actor_world_matrix(model);
        Matrix inverse=human_identity();
        if(const auto parent=frames.object_parent(model)){
            inverse=human_transpose(frames.actor_world_matrix(parent));inverse[3]=inverse[7]=inverse[11]=0;
        }
        auto normal=vectors_[4][1];limit_up(normal,.765999972820282f,.6427000164985657f);
        normal=normalized(normal,1,0,2);
        Vector forward{{world[8],world[9],world[10],world[11]}};
        const auto dot=(static_cast<long double>(normal[2])*forward[2]+static_cast<long double>(normal[1])*forward[1])+
            static_cast<long double>(normal[0])*forward[0];
        for(unsigned axis=0;axis!=3;++axis){
            const auto projection=dot*normal[axis];forward[axis]=static_cast<float>(forward[axis]-(axis?static_cast<float>(projection):projection));
        }
        forward=normalized(forward,2,1,0);
        world[0]=static_cast<float>(static_cast<long double>(normal[1])*forward[2]-static_cast<long double>(normal[2])*forward[1]);
        world[1]=static_cast<float>(static_cast<long double>(normal[2])*forward[0]-static_cast<long double>(normal[0])*forward[2]);
        world[2]=static_cast<float>(static_cast<long double>(normal[0])*forward[1]-static_cast<long double>(normal[1])*forward[0]);
        for(unsigned axis=0;axis!=4;++axis){world[4+axis]=normal[axis];world[8+axis]=forward[axis];}
        auto roll=human_matrix_roll(frames,world);
        constexpr float maximum_roll=.5235987901687622f;
        if(roll>maximum_roll){roll=maximum_roll;frames.human_set_direction_roll(world,forward,roll);}
        else if(roll< -maximum_roll){roll=-maximum_roll;frames.human_set_direction_roll(world,forward,roll);}
        last_direction_={{world[8],world[9],world[10],world[11]}};
        human.base().world_direction(last_direction_);control.physical().control_frame_words().set(0x250,roll);
        const auto current=frames.actor_world_matrix(model);
        vectors_[4][4]={{current[12],current[13],current[14],current[15]}};
        frames.human_publish_bone_rotation(model,frames.car_matrix_product(world,inverse),true);
        const auto& heights=vectors_[4][3];
        auto shift=(static_cast<long double>(heights[2])+heights[0]+heights[1])*.33000001311302185f;
        if(std::abs(static_cast<long double>(heights[1]))<std::abs(shift))shift=heights[1];
        Vector offset{};
        for(unsigned axis=0;axis!=3;++axis)offset[axis]=static_cast<float>(shift*vectors_[4][1][axis]);
        const auto anchor=skeleton.bones()[22].frame;
        const auto anchor_world=frames.actor_world_matrix(anchor);
        frames.object_position(anchor,{{anchor_world[12]+offset[0],anchor_world[13]+offset[1],anchor_world[14]+offset[2]}});
        const auto front=section_direction(normal,up,spheres,0);
        rotate_bone(frames,skeleton,13,normal,front,periodic_normalize);
        rotate_bone(frames,skeleton,8,front,vectors_[4][0],periodic_normalize);
        const auto back=section_direction(normal,up,spheres,2);
        const auto rotation=rotation_between(normal,back);
        publish_rotation(frames,skeleton,10,rotation,periodic_normalize);
        publish_rotation(frames,skeleton,11,rotation,periodic_normalize);
        const auto lower=rotation_between(back,vectors_[4][2]);
        publish_rotation(frames,skeleton,2,lower,periodic_normalize);
        publish_rotation(frames,skeleton,3,lower,periodic_normalize);
        frames.update_scene_object(model);
    }
    Vector section_direction(const Vector& normal,const Vector& up,const std::array<Vector,4>& spheres,unsigned section){
        auto desired=interpolate_human_directions(normal,vectors_[4][section],.75f);
        Vector axis{};
        for(unsigned component=0;component!=3;++component){
            const auto first=static_cast<long double>(spheres[section==0?0:3][component])*.5f;
            const auto middle=static_cast<long double>(spheres[section==0?2:1][component])*.5f;
            const auto selected=first+static_cast<long double>(up[component])*vectors_[4][3][section];
            const auto delta=selected-middle;
            const auto reference=static_cast<long double>(up[component])*vectors_[4][3][1];
            axis[component]=static_cast<float>(delta-reference);
        }
        return perpendicular(desired,normalized(axis,0,1,2));
    }
    static Matrix rotation_between(const Vector& first,const Vector& last){
        Matrix rotation{};human_align_vectors(first,last,rotation);rotation[15]=1;return rotation;
    }
    void rotate_bone(Frames& frames,const HumanSkeleton<Frame>& skeleton,unsigned index,const Vector& first,const Vector& last,bool normalize){
        publish_rotation(frames,skeleton,index,rotation_between(first,last),normalize);
    }
    void publish_rotation(Frames& frames,const HumanSkeleton<Frame>& skeleton,unsigned index,const Matrix& rotation,bool normalize){
        const auto& bone=skeleton.bones()[index];
        if(bone.parent<0)throw std::logic_error("Human prone rotation requires its actual skeleton parent");
        auto matrix=frames.rigid_matrix_product(frames.actor_world_matrix(bone.frame),rotation);
        auto inverse=human_transpose(frames.actor_world_matrix(skeleton.bones()[static_cast<unsigned>(bone.parent)].frame));
        inverse[3]=inverse[7]=inverse[11]=0;
        matrix=frames.car_matrix_product(matrix,inverse);
        if(normalize)orthonormalize_human_bone_matrix(matrix);
        frames.human_publish_bone_rotation(bone.frame,matrix,true);
    }
    alignas(16)std::array<std::array<Vector,6>,6> vectors_{};
    std::array<std::uint8_t,6> contacts_{};
    std::array<std::uint32_t,6> counters_{};
    Vector last_direction_{};
};

} //namespace hd2::game
#endif
