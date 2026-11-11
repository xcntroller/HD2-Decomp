#ifndef HD2_GAME_HUMAN_ATTACHMENT_POSE_H
#define HD2_GAME_HUMAN_ATTACHMENT_POSE_H

#include "game/human_pose_chains.h"
#include "game/human_joint_pose.h"
#include "game/human_attachment_blend.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {

//007ADD30's borrowed6414..6428 and retained6430..64F0/66F4..6730 state.
//The pose graph borrows the containing geometry's original23-bone skeleton.
template<class Frames,class Movement>class HumanAttachmentPose {
public:
    using Frame=typename Frames::Frame;
    using Matrix=HumanMatrix;
    using Vector=HumanVector;
    using Poses=HumanPoseChains<Frames>;
    using Chain=typename Poses::Chain;
    HumanAttachmentPose(Frames& frames,const HumanSkeleton<Frame>& skeleton,Movement& movement)
        :frames_(frames),skeleton_(skeleton),movement_(movement),poses_(frames,skeleton){}
    void clear(){actor_=nullptr;left_=right_=Frame{};chain_={};poses_.clear();}
    void* actor()const{return actor_;}
    void actor(void* value){actor_=value;} //007B0910's reason4 writes the SAME6414.
    const auto& chain()const{return chain_;}
    const auto& poses()const{return poses_;}
    template<class Services,class Keys>void present_vehicle(std::uint32_t mode,bool detail,Frame human_model,
            const Keys& rotations,const Keys& positions,Services& services){ //007AF320.
        if(!(mode&2u)||!detail)return;
        if(movement_.flag6624){
            movement_.direction6360=services.human_pose_actor_direction(actor_,seat_);
            Vector local=movement_.direction6360;
            for(unsigned row=0;row!=3;++row){
                const auto world=frames_.actor_world_matrix(model(services));
                const auto& direction=movement_.direction6360;
                local[row]=static_cast<float>((static_cast<long double>(direction[2])*world[row*4+2]+
                    static_cast<long double>(direction[1])*world[row*4+1])+static_cast<long double>(direction[0])*world[row*4]);
            }
            const auto blend=human_attachment_blend(local,services.human_pose_actor_limits(actor_,seat_));
            const auto key=blend.pair*2;
            for(unsigned bone=0;bone!=23;++bone){
                const auto frame=skeleton_.bones()[bone].frame;
                if(human_attachment_key_present(rotations[key][bone][3])&&human_attachment_key_present(rotations[key+1][bone][3])&&
                   human_attachment_key_present(rotations[key+2][bone][3])&&human_attachment_key_present(rotations[key+3][bone][3])){
                    const auto first=frames_.physics_slerp(rotations[key][bone],rotations[key+2][bone],blend.horizontal,false);
                    const auto second=frames_.physics_slerp(rotations[key+1][bone],rotations[key+3][bone],blend.horizontal,false);
                    frames_.car_steering_rotation(frame,frames_.physics_slerp(first,second,blend.vertical,false));
                    frames_.update_scene_object(frame);
                }
                if(human_attachment_key_present(positions[key][bone][0])&&human_attachment_key_present(positions[key+1][bone][0])&&
                   human_attachment_key_present(positions[key+2][bone][0])&&human_attachment_key_present(positions[key+3][bone][0])){
                    const auto position=blend_human_attachment_position(positions[key][bone],positions[key+1][bone],
                        positions[key+2][bone],positions[key+3][bone],blend.horizontal,blend.vertical);
                    frames_.car_suspension_position(frame,position);frames_.update_scene_object(frame);
                }
            }
        }
        if(!left_||!right_)return;
        movement_.direction6360=services.human_pose_actor_direction(actor_,seat_);
        target_hands();poses_.refresh(chain_);chain_.count=2;
        poses_.solve(chain_,16,true);poses_.publish(chain_);
        pose_human_hand_frames(frames_,skeleton_,left_,right_);
        const auto guide=frames_.collision_sweep_rotate(Vector{{0,-1,0,0}},frames_.actor_world_matrix(model(services)));
        pose_human_joint_radial(frames_,skeleton_,human_model,19,guide,1.f);
        pose_human_joint_radial(frames_,skeleton_,human_model,18,guide,1.f);
    }
    template<class Services>void select(std::uint32_t flags,void* actor,std::uint32_t seat,Services& services){ //007ADD30.
        if(movement_.field6304&8u)services.human_pose_clear_car_steering(actor_);
        movement_.field6304&=0xffffff87u;
        if(!actor){
            actor_=nullptr;movement_.flag6624=false;movement_.field62d0=movement_.field62d4;
            if(movement_.field62d4==2){movement_.flag65f0=true;movement_.field65f4=500;}
            return;
        }
        if(movement_.field62d0!=3)movement_.field62d4=movement_.field62d0;
        const auto type=services.human_pose_actor_type(actor);
        const auto source=services.human_pose_actor_model(actor);
        if(type==4){
            if(!(flags&0x10u)){
                if(flags&1u){
                    left_=frames_.actor_find(source,"LWC",0xffff);right_=frames_.actor_find(source,"RWC",0xffff);
                    if(!left_||!right_){movement_.field6304=0;return;}
                    movement_.field6304=8;
                    const auto parent=frames_.object_parent(left_);
                    if(!parent){movement_.field6304=0;return;}
                    saved_parent_=human_transpose(frames_.car_local_matrix(parent));
                    saved_parent_[3]=saved_parent_[7]=saved_parent_[11]=0;
                    auto inverse=human_transpose(frames_.actor_world_matrix(source));
                    inverse[3]=inverse[7]=inverse[11]=0;
                    saved_left_=frames_.actor_world_matrix(left_);saved_right_=frames_.actor_world_matrix(right_);
                    retain_source_pose(source,inverse);
                    reset_motion(services.human_pose_car_velocity(actor));
                    configure_chain(3);
                }else if(flags&2u){
                    movement_.field62d0=3;
                    reset_motion(services.human_pose_car_velocity(actor));
                    left_=right_=Frame{};
                    if((flags&0x100u)&&services.human_pose_actor_has_control(actor,seat)){
                        left_=frames_.actor_find(source,"LWS",0xffff);right_=frames_.actor_find(source,"RWS",0xffff);
                    }
                    if(left_&&right_){
                        configure_chain(3);movement_.field6318=0x3f800000u;movement_.field6304=0x20;
                    }else{configure_chain(1);movement_.field6304|=0x40u;}
                }
            }
            flags_=flags;seat_=seat;source_state_=services.human_pose_car_state(actor);actor_=actor;
            return;
        }
        if(type==21){
            left_=frames_.actor_find(source,"LWS",0xffff);right_=frames_.actor_find(source,"RWS",0xffff);
            if(!left_||!right_){movement_.field6304=0;return;}
            seat_=seat;movement_.field6318=0x3f800000u;movement_.field6304=0x10;
            configure_chain(2);actor_=actor;
        }
    }
    template<class Services>void present_target(std::uint32_t mode,bool detail,std::uint32_t elapsed,Frame human_model,Services& services){ //007AEBE0.
        if(!(mode&2u)||!detail)return;
        poses_.refresh(chain_);
        advance_motion(elapsed,services);
        if(left_&&right_){
            target_hands();chain_.count=2;poses_.solve(chain_,16,false);
            target_body(false);
            chain_.count=3;poses_.solve(chain_,16,false);poses_.publish(chain_);
            pose_human_hand_frames(frames_,skeleton_,left_,right_);
            const auto left=frames_.collision_sweep_rotate(Vector{{-.5f,-.866f,0,0}},frames_.actor_world_matrix(model(services)));
            const auto right=frames_.collision_sweep_rotate(Vector{{.5f,-.866f,0,0}},frames_.actor_world_matrix(model(services)));
            pose_human_joint_radial(frames_,skeleton_,human_model,19,left,1.f);
            pose_human_joint_radial(frames_,skeleton_,human_model,18,right,1.f);
            movement_.direction6360=services.human_pose_actor_direction(actor_,seat_);
            return;
        }
        const auto body=frames_.actor_world_matrix(skeleton_.bones()[4].frame);
        auto motion=limited_motion(.008999999612569809f,.44999998807907104f);
        auto& target=chain_.objectives[0];target.weight=1;
        for(unsigned axis=0;axis!=3;++axis)target.target[axis]=static_cast<float>(body[12+axis]-motion[axis]);
        target.target[3]=body[15];chain_.count=1;
        poses_.solve(chain_,8,false);poses_.publish(chain_);
    }
    template<class Services>void present_attached(std::uint32_t mode,bool detail,std::uint32_t elapsed,Frame human_model,Services& services){ //007AE3C0.
        if(!(mode&2u)||!detail||!left_||!right_)return;
        const auto parent=frames_.object_parent(left_);
        if(!parent)throw std::logic_error("Attached Human pose requires its actual left source parent");
        const auto relative=frames_.rigid_matrix_product(frames_.car_local_matrix(parent),saved_parent_);
        const auto angle=std::atan2(static_cast<long double>(relative[1]),static_cast<long double>(relative[0]));
        float stored_angle=static_cast<float>(angle);
        Matrix left,right;
        if(angle>1.0471975803375244f){
            left=frames_.rigid_matrix_product(saved_left_,frames_.actor_world_matrix(model(services)));
            right=frames_.actor_world_matrix(right_);
        }else if(stored_angle< -1.0471975803375244f){
            right=frames_.rigid_matrix_product(saved_right_,frames_.actor_world_matrix(model(services)));
            left=frames_.actor_world_matrix(left_);
        }else{left=frames_.actor_world_matrix(left_);right=frames_.actor_world_matrix(right_);}
        target_hands(left,right);
        poses_.refresh(chain_);chain_.count=2;poses_.solve(chain_,16,false);
        advance_motion(elapsed,services);
        target_body(true);
        chain_.count=3;poses_.solve(chain_,16,false);poses_.publish(chain_);
        publish_human_hand_rotation(frames_,skeleton_.bones()[21].frame,left); //007A50F0.
        publish_human_hand_rotation(frames_,skeleton_.bones()[20].frame,right);
        stored_angle=static_cast<float>(static_cast<long double>(stored_angle)*.47746482491493225f+.5f);
        if(std::signbit(stored_angle))stored_angle=0;
        else if(stored_angle>1)stored_angle=1;
        const auto left_angle=(1.L-stored_angle)*.7853981852531433f;
        const auto right_angle=static_cast<long double>(stored_angle)*.7853981852531433f;
        const auto left_guide=frames_.collision_sweep_rotate(Vector{{static_cast<float>(-std::sin(left_angle)),static_cast<float>(-std::cos(left_angle)),0,0}},frames_.actor_world_matrix(model(services)));
        const auto right_guide=frames_.collision_sweep_rotate(Vector{{static_cast<float>(std::sin(right_angle)),static_cast<float>(-std::cos(right_angle)),0,0}},frames_.actor_world_matrix(model(services)));
        pose_human_joint_radial(frames_,skeleton_,human_model,19,left_guide,1.f);
        pose_human_joint_radial(frames_,skeleton_,human_model,18,right_guide,1.f);
        auto inverse=human_transpose(frames_.actor_world_matrix(model(services)));
        inverse[3]=inverse[7]=inverse[11]=0;
        saved_left_=left;saved_right_=right;
        retain_source_pose(model(services),inverse);
    }
private:
    template<class Services>Frame model(Services& services){
        if(!actor_)throw std::logic_error("Human attachment pose requires its borrowed source actor");
        return services.human_pose_actor_model(actor_);
    }
    void configure_chain(std::int32_t count){
        chain_.root=9;chain_.count=count;
        chain_.objectives[0].bone=count==1?4:21;chain_.objectives[0].weight=1;
        if(count>1){chain_.objectives[1].bone=20;chain_.objectives[1].weight=1;}
        if(count>2){chain_.objectives[2].bone=4;chain_.objectives[2].weight=1;}
        poses_.reset_graph();poses_.build(chain_);
    }
    void retain_source_pose(Frame source,const Matrix& inverse){
        auto world=frames_.actor_world_matrix(source);
        for(unsigned axis=0;axis!=3;++axis)saved_left_[12+axis]-=world[12+axis];
        world=frames_.actor_world_matrix(source);
        for(unsigned axis=0;axis!=3;++axis)saved_right_[12+axis]-=world[12+axis];
        saved_left_=frames_.car_matrix_product(saved_left_,inverse);
        saved_right_=frames_.car_matrix_product(saved_right_,inverse);
    }
    void reset_motion(const Vector& velocity){
        velocity_=velocity;
        for(auto* value:{&motion_,&previous_motion_,&next_motion_})for(unsigned axis=0;axis!=3;++axis)(*value)[axis]=0;
        motion_time_=0;
    }
    template<class Services>void advance_motion(std::uint32_t elapsed,Services& services){
        motion_time_-=elapsed;
        if(human_frame_signed_word(motion_time_)<0){
            motion_time_=200;previous_motion_=next_motion_;
            const auto velocity=services.human_pose_car_velocity(actor_);
            for(unsigned axis=0;axis!=3;++axis){
                const auto difference=static_cast<long double>(velocity[axis])-velocity_[axis];
                next_motion_[axis]=static_cast<float>((axis==2?static_cast<float>(difference):difference)*5.f);
            }
            velocity_=velocity;
        }
        const auto fraction=static_cast<long double>(human_frame_signed_word(motion_time_))*.004999999888241291f;
        const auto complement=1.L-fraction;
        for(unsigned axis=0;axis!=3;++axis){
            const auto previous=static_cast<long double>(previous_motion_[axis])*fraction;
            const auto latest=static_cast<long double>(next_motion_[axis])*complement;
            motion_[axis]=static_cast<float>((axis==2?static_cast<float>(previous):previous)+
                (axis==2?latest:static_cast<float>(latest)));
        }
    }
    void target_hands(){
        movement_.right_hand_offset6340=frames_.car_transform(movement_.offset63b0,frames_.actor_world_matrix(right_));
        movement_.left_hand_offset6330=frames_.car_transform(movement_.offset63a0,frames_.actor_world_matrix(left_));
        mirror_hand(0,frames_.actor_world_matrix(left_),movement_.left_hand_offset6330);
        mirror_hand(1,frames_.actor_world_matrix(right_),movement_.right_hand_offset6340);
    }
    void target_hands(const Matrix& left,const Matrix& right){
        movement_.right_hand_offset6340=frames_.car_transform(movement_.offset63b0,right);
        movement_.left_hand_offset6330=frames_.car_transform(movement_.offset63a0,left);
        mirror_hands(left,right);
    }
    void mirror_hands(const Matrix& left,const Matrix& right){
        mirror_hand(0,left,movement_.left_hand_offset6330);
        mirror_hand(1,right,movement_.right_hand_offset6340);
    }
    void mirror_hand(unsigned hand,const Matrix& matrix,const Vector& offset){
        auto& objective=chain_.objectives[hand];objective.weight=1;
        for(unsigned axis=0;axis!=3;++axis){
            const auto doubled=2.L*matrix[12+axis];
            objective.target[axis]=static_cast<float>((axis==2?static_cast<float>(doubled):doubled)-offset[axis]);
        }
    }
    std::array<long double,3> limited_motion(float amount,float maximum,bool attached=false)const{
        auto value=motion_;
        for(unsigned axis=0;axis!=3;++axis)value[axis]*=amount;
        std::array<long double,3> result{{value[0],value[1],value[2]}};
        const auto first=attached?0u:1u,last=attached?1u:0u;
        const float square=static_cast<float>((static_cast<long double>(value[first])*value[first]+
            static_cast<long double>(value[2])*value[2])+static_cast<long double>(value[last])*value[last]);
        std::int32_t bits;std::memcpy(&bits,&square,4);
        if(bits<0x322bcc77)return result;
        const auto length=std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(square));
        if(length<=maximum)return result;
        const auto scale=static_cast<long double>(maximum)/length;
        for(unsigned axis=0;axis!=3;++axis){
            const auto product=value[axis]*scale;result[axis]=attached&&axis==2?product:static_cast<float>(product);
        }
        return result;
    }
    void target_body(bool attached){
        const auto& body=poses_.nodes().at(static_cast<std::size_t>(chain_.objectives[2].bone));
        const auto motion=limited_motion(.007499999832361937f,.4000000059604645f,attached);
        auto& objective=chain_.objectives[2];objective.weight=.75f;
        for(unsigned axis=0;axis!=3;++axis){
            const auto offset=static_cast<long double>(body.length284)*body.world180[8+axis];
            const float end=static_cast<float>((axis?static_cast<float>(offset):offset)+body.world180[12+axis]);
            objective.target[axis]=static_cast<float>(end-motion[axis]);
        }
    }
    Frames& frames_;
    const HumanSkeleton<Frame>& skeleton_;
    Movement& movement_;
    Poses poses_;
    Chain chain_;
    void* actor_=nullptr;
    std::uint32_t flags_=0,seat_=0,source_state_=0,motion_time_=0;
    Frame left_{},right_{};
    Matrix saved_parent_{},saved_left_{},saved_right_{};
    Vector velocity_{},motion_{},previous_motion_{},next_motion_{};
};

} //namespace hd2::game
#endif
