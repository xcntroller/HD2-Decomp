#ifndef HD2_GAME_HUMAN_MODEL_GEOMETRY_H
#define HD2_GAME_HUMAN_MODEL_GEOMETRY_H

#include "game/human_loading_joints.h"
#include "game/human_look_state.h"
#include "game/human_weapon_binding.h"
#include "game/human_face_model.h"
#include "game/human_state_frame_timers.h"
#include "game/human_clip_pose.h"
#include "game/human_bone_direction.h"
#include "game/human_ai_shot_direction.h"
#include "game/human_two_bone_ik.h"
#include <limits>
#include <exception>
#include <optional>

namespace hd2::game {

//The6950 owner0079B910/0079BAC0 and ordinary loading path through0079BD30,
//0079C320,007B85C0 and007C0D90. This does not implement model replacement
//with preserve-pose or any simulation producer. Offline62B0 requires the
//actual shared face resources and current profile-quality/mission settings.
//Unknown native scalar/padding bytes are not represented as recovered fields.
template<class Frames,class Random>
class HumanModelGeometry {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    using Matrix=HumanMatrix;
    using Vector=HumanVector;
    using OfflineFaces=HumanFaceResources<Frames>;
    struct OfflineBinding {bool player;std::optional<float> clothing;};
    HumanModelGeometry(Frames& frames,Random& random)
        :frames_(frames),random_(random),joints_(frames,skeleton_) {reset_loading();}
    ~HumanModelGeometry(){clear();}
    HumanModelGeometry(const HumanModelGeometry&)=delete;
    HumanModelGeometry& operator=(const HumanModelGeometry&)=delete;
    void configure_offline_faces(OfflineFaces& shared,bool quality_enabled,std::int32_t camouflage) {
        if(model_&&offline_faces_!=&shared)throw std::logic_error("Cannot replace offline face dependencies on bound geometry");
        offline_faces_=&shared;offline_quality_=quality_enabled;offline_camouflage_=camouflage;
    }
    void bind_loading(Frame model,const void* human,const void* control,std::uint32_t mode,
            std::optional<OfflineBinding> offline=std::nullopt) {
        if((mode&7)==7&&(!offline_faces_||(offline_quality_&&!offline)))
            throw std::runtime_error("Offline Human geometry requires actual shared face/profile/Human bindings");
        clear();model_=model;human_=human;control_=human?control:nullptr;reset_loading();
        if(!model)return;
        try {
            skeleton_.bind_graph(frames_,model);
            bind_frames();
            for(const auto& bone:skeleton_.bones()) {
                if(!bone.frame)throw std::runtime_error("Required Human skeleton marker is absent");
                frames_.human_unit_pose(bone.frame);
            }
            frames_.human_unit_pose(model);
            if((mode&7)==7&&offline_quality_) {
                face_model_=std::make_unique<HumanFaceModel<Frames>>(frames_,*offline_faces_);
                face_model_->bind(model,offline->player,offline_camouflage_,offline->clothing,0);
            }
            reset_movement();
            if(human)joints_.build();
        }catch(...) {clear();throw;}
    }
    void clear() noexcept {
        joints_.clear(); //0079BAC0 retires body then point trees before camera references.
        skeleton_.clear();
        equipped_weapon_.clear();
        face_model_.reset(); //0079BAC0 retires62B0 before retained camera frames62F8/FC.
        camera3rd_.reset();camera_fpv_.reset();model_=Frame{};human_=control_=nullptr;
        weapon1_=weapon2_=marker_z_=Frame{};feet_={};
    }
    const auto& skeleton() const {return skeleton_;}
    const auto& joints() const {return joints_;}
    Frame model() const {return model_;}
    Frame camera3rd() const {return camera3rd_?camera3rd_->get():Frame{};}
    Frame camera_fpv() const {return camera_fpv_?camera_fpv_->get():Frame{};}
    Frame weapon1() const {return weapon1_;}
    Frame weapon2() const {return weapon2_;}
    const auto& feet() const {return feet_;}
    std::uint32_t random_seed() const {return seed_;}
    std::uint32_t frame_elapsed()const{return frame_elapsed_;}
    bool geometry_detail_enabled()const{return geometry_detail_;}
    struct GeometryEffect {
        std::uint16_t reason;
        const void* source;
        Frame clip{};
        Vector root_position{},camera_position{};
        void* support_actor=nullptr; //Native6414, borrowed from actual frame EC binding.
    };
    const auto& geometry_effect()const{return geometry_effect_;}
    void select_secondary_pose(Frame animation,std::uint32_t mode){ //007AE320, borrowed6630 and actual23-bone scratch.
        if(!(mode&2u))return;
        secondary_pose_=animation;
        if(!animation){movement_.field6304&=~0x80u;return;}
        secondary_pose_current_.fill(0);secondary_pose_previous_.fill(0);secondary_pose_time_=0;
        movement_.field6304|=0x80u;
    }
    void select_equipment_overlay(std::uint32_t mode){ //007AE390, native stack clip argument is unused.
        if(!(mode&2u))return;
        equipment_overlay_time_=0;movement_.field6304|=0x100u;
    }
    void present_secondary_clip(){ //007AFC20, actual6630/6634 and per-bone6638/6694.
        if(secondary_pose_){
            const auto time=human_frame_signed_word(secondary_pose_time_);
            auto influence=static_cast<long double>(time)*.0033333334140479565f;
            float weight=static_cast<float>(influence);
            if(influence>1){
                const auto remaining=human_frame_signed_word(static_cast<std::uint32_t>(frames_.human_clip_duration(secondary_pose_))-secondary_pose_time_);
                influence=static_cast<long double>(remaining)*.0033333334140479565f;
                weight=influence>1?1.f:static_cast<float>(influence);
            }
            auto bone_weight=[](int bone){return static_cast<long double>(human_secondary_clip_bone_weight(bone));};
            apply_human_clip_pose(frames_,skeleton_,secondary_pose_,secondary_pose_current_,secondary_pose_previous_,
                time,22,weight,bone_weight);
            frames_.update_scene_object(model_);
            const auto next=static_cast<long double>(human_frame_signed_word(frame_elapsed_))*1.399999976158142f+time;
            secondary_pose_time_=static_cast<std::uint32_t>(human_animation_time_integer(next));
            if(human_frame_signed_word(secondary_pose_time_)<=frames_.human_clip_duration(secondary_pose_))return;
        }
        movement_.field6304&=~0x80u;
    }
    void present_equipment_overlay(){ //007AFD70.
        if(equipped_weapon_.model()){
            equipment_overlay_time_+=frame_elapsed_;
            const float influence=static_cast<float>(1.L-static_cast<long double>(human_frame_signed_word(equipment_overlay_time_))*.0033333334140479565f);
            if(!std::signbit(influence)){
                const float factor=static_cast<float>(static_cast<long double>(influence)*.5f);
                const Vector weights{{factor,factor,factor,0}};
                if(movement_.field62cc!=4){
                    present_weapon_overlay(weights);
                    return;
                }
                auto desired=movement_.field6350;
                desired[1]-=.1736000031232834f;
                const auto square=(static_cast<long double>(desired[2])*desired[2]+
                    static_cast<long double>(desired[1])*desired[1])+static_cast<long double>(desired[0])*desired[0];
                normalize_human_ai_shot(desired,square);
                for(unsigned bone:{18u,16u,8u})
                    correct_human_bone_direction(frames_,skeleton_,model_,bone,desired,movement_.field6350,weights);
                return;
            }
        }
        movement_.field6304&=~0x100u;
    }
    void pose_equipment_weapon(const Vector& direction,bool align_hands,bool align_weapon,float influence){ //007C02F0.
        auto orientation=frames_.human_direction_matrix(direction,Vector{{0,1,0,0}});
        const auto left=equipped_weapon_.left(),right=equipped_weapon_.right();
        if(align_hands&&left&&right){
            const auto align=[&](Frame marker,unsigned index){
                auto target=frames_.rigid_matrix_product(frames_.car_local_matrix(marker),orientation);
                target[12]=target[13]=target[14]=0;
                const auto hand=skeleton_.bones()[index].frame;
                const auto parent=frames_.object_parent(hand);
                if(!parent)throw std::logic_error("Human hand pose requires its actual frame parent");
                auto inverse=human_transpose(frames_.actor_world_matrix(parent));
                inverse[3]=inverse[7]=inverse[11]=0;
                if(influence<1){
                    const auto desired=frames_.human_matrix_rotation(target);
                    const auto current=frames_.human_matrix_rotation(frames_.actor_world_matrix(hand));
                    frames_.human_set_matrix_rotation(target,frames_.physics_slerp(current,desired,influence,false));
                }
                target=frames_.car_matrix_product(target,inverse);
                frames_.human_publish_bone_rotation(hand,target);
            };
            if(movement_.field62d8&0x80000000u)align(left,21);
            align(right,20);
        }
        if(!align_weapon)return;
        auto marker=right,hand=weapon1_;
        if((movement_.field62d8&0xfu)!=1){marker=left?left:right;hand=weapon2_;}
        if(!marker||!hand)throw std::logic_error("Human weapon pose requires its actual selected hand and grip");
        const auto hand_world=frames_.actor_world_matrix(hand);
        const auto grip=frames_.collision_sweep_rotate(frames_.car_position(marker),orientation);
        Vector position{{hand_world[12]-grip[0],hand_world[13]-grip[1],hand_world[14]-grip[2],hand_world[15]}};
        if(equipped_weapon_.butt()&&(movement_.field6304&1u)){
            const Vector offset=movement_.field62dc==3&&movement_.field62d0==0?
                Vector{{.03f,-.051f,.11f,0}}:Vector{{.033f,.036f,.08f,0}};
            const auto butt=frames_.collision_sweep_rotate(frames_.car_position(equipped_weapon_.butt()),orientation);
            const auto shoulder=frames_.car_transform(offset,frames_.actor_world_matrix(skeleton_.bones()[14].frame));
            const auto amount=human_animation_word_float(movement_.field6318);
            const auto complement=1.L-amount;
            for(unsigned axis=0;axis!=3;++axis){
                const auto difference=static_cast<long double>(shoulder[axis])-butt[axis];
                const float target=static_cast<float>((axis==2?static_cast<float>(difference):difference)*amount);
                const auto previous=static_cast<long double>(position[axis])*complement;
                position[axis]=static_cast<float>(target+(axis?static_cast<float>(previous):previous));
            }
        }
        const auto weapon=equipped_weapon_.model();
        if(!weapon)throw std::logic_error("Human weapon pose requires its actual retained weapon model");
        for(unsigned axis=0;axis!=4;++axis)orientation[12+axis]=position[axis];
        frames_.human_equipment_local_matrix(weapon,orientation);
        frames_.update_scene_object(weapon);
    }
    void pose_hand_chain(unsigned first,unsigned second,const Vector& desired,const Vector& offset,
            Frame grip,float influence,const Vector* preferred){ //007C09C0, same skeleton lengths and actual hand frame.
        auto grip_rotation=frames_.actor_world_matrix(grip);
        grip_rotation[12]=grip_rotation[13]=grip_rotation[14]=0;
        const auto& root_bone=skeleton_.bones().at(first);
        const auto& end_bone=skeleton_.bones().at(second);
        const auto root_world=frames_.actor_world_matrix(root_bone.frame);
        const Vector root{{root_world[12],root_world[13],root_world[14],root_world[15]}};
        const auto grip_world=frames_.actor_world_matrix(grip);
        const auto complement=1.L-influence;
        Vector target=offset;
        for(unsigned axis=0;axis!=3;++axis){
            const auto first_product=static_cast<long double>(desired[axis])*influence;
            const auto second_product=static_cast<long double>(offset[axis])*complement;
            auto mixed=axis?static_cast<float>(first_product):first_product;
            mixed+=axis?static_cast<float>(second_product):second_product;
            if(axis==2)mixed=static_cast<float>(mixed);
            const auto world_position=mixed+grip_world[12+axis];
            target[axis]=static_cast<float>((axis?world_position:static_cast<float>(world_position))-offset[axis]);
        }
        const auto middle_world=frames_.actor_world_matrix(end_bone.frame);
        Vector middle{{middle_world[12],middle_world[13],middle_world[14],middle_world[15]}};
        //0079BDC5/0079BDCF initialize these private weights. No other direct
        //producer occurs in the original image's normal constructed lifetime.
        const bool solved=solve_human_two_bone_positions(root,root_bone.lengths[0],target,end_bone.lengths[0],middle,preferred,.5f,.9f);
        Vector direction=middle;
        for(unsigned axis=0;axis!=3;++axis)direction[axis]=(solved?middle[axis]:target[axis])-root[axis];
        orient_human_bone(frames_,skeleton_,model_,first,direction,-1.f);
        if(solved)for(unsigned axis=0;axis!=3;++axis)direction[axis]=target[axis]-middle[axis];
        orient_human_bone(frames_,skeleton_,model_,second,direction,-1.f);
        auto inverse=human_transpose(frames_.actor_world_matrix(end_bone.frame));
        inverse[3]=inverse[7]=inverse[11]=0;
        grip_rotation=frames_.car_matrix_product(grip_rotation,inverse);
        frames_.human_publish_bone_rotation(grip,grip_rotation);
    }
    void present_stowed_weapon(std::uint32_t mode){ //007B9750.
        if(!(mode&2u)||movement_.field62cc==4||!(movement_.field62d8&0x80000000u))return;
        const auto weapon=equipped_weapon_.model();
        if(!weapon)throw std::logic_error("Human stowed pose requires its actual retained weapon model");
        const auto parent=frames_.object_parent(weapon);
        frames_.human_geometry_weapon_link(weapon,Frame{});
        try{
            const auto hand=frames_.actor_world_matrix(weapon1_);
            const Vector reference{{hand[0],hand[1],hand[2],hand[3]}};
            const auto blend=human_animation_word_float(movement_.field631c);
            const auto prepare_direction=[&]{
                refresh_weapon_hand_offsets();
                auto direction=movement_.left_hand_offset6330;
                for(unsigned axis=0;axis!=3;++axis)direction[axis]-=movement_.right_hand_offset6340[axis];
                const auto square=(static_cast<long double>(direction[2])*direction[2]+
                    static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[0])*direction[0];
                normalize_human_ai_shot(direction,square);
                movement_.field6350=interpolate_human_directions(reference,direction,blend);
            };
            prepare_direction();
            pose_equipment_weapon(movement_.field6350,true,false,blend);
            prepare_direction();
            pose_equipment_weapon(movement_.field6350,false,true,1.f);
            if(geometry_detail_){
                const auto left_hand=skeleton_.bones()[21].frame;
                movement_.left_hand_offset6330=frames_.car_transform(movement_.offset63a0,frames_.actor_world_matrix(left_hand));
                const auto point=frames_.actor_world_matrix(equipped_weapon_.left());
                const Vector target{{point[12],point[13],point[14],point[15]}};
                pose_hand_chain(17,19,target,movement_.left_hand_offset6330,left_hand,blend,nullptr);
            }
        }catch(...){
            const auto failure=std::current_exception();
            try{frames_.human_geometry_weapon_link(weapon,parent);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
            std::rethrow_exception(failure);
        }
        frames_.human_geometry_weapon_link(weapon,parent);
    }
    void present_standing_weapon(const Vector* velocity){ //007BCFC0.
        limit_aim_vertical();
        limit_aim_forward(.34f,.9404f);
        if(movement_.field62cc==4){
            present_kind_four_aim(velocity&&aim_motion_square(*velocity)>9.99999993922529e-09f);
            return;
        }
        const auto first=frames_.actor_world_matrix(skeleton_.bones()[9].frame);
        const auto second=frames_.actor_world_matrix(skeleton_.bones()[8].frame);
        Vector up{{second[12]-first[12],second[13]-first[13],second[14]-first[14],second[15]}};
        normalize_human_ai_shot(up,(static_cast<long double>(up[0])*up[0]+
            static_cast<long double>(up[1])*up[1])+static_cast<long double>(up[2])*up[2]);
        auto basis=frames_.actor_world_matrix(model_);
        align_human_matrix_up(frames_,basis,up);
        float motion=velocity?static_cast<float>(aim_motion_square(*velocity)*.0625f):0.f;
        if(motion>1)motion=1;
        const auto complement=1.L-motion;
        const float first_x=static_cast<float>(static_cast<long double>(motion)*.15f+.35f*complement);
        const float weight_y=static_cast<float>(static_cast<long double>(motion)*.7f+.95f*complement);
        const float second_x=static_cast<float>(static_cast<long double>(motion)*.25f+complement*.45f);
        auto direction=aim_hand_direction();
        const auto saved_right=movement_.right_hand_offset6340;
        const auto blend=human_animation_word_float(movement_.field6318);
        Vector weights{{first_x*blend,weight_y*blend,blend*0.f,0}};
        correct_human_bone_reference(frames_,skeleton_,model_,12,direction,movement_.direction6360,basis,weights);
        direction=aim_hand_direction();
        weights={{second_x*blend,weight_y*blend,blend*0.f,0}};
        correct_human_bone_reference(frames_,skeleton_,model_,7,direction,movement_.direction6360,basis,weights);
        if(motion<9.99999993922529e-09f&&!equipped_weapon_.butt()&&geometry_detail_)
            restore_aim_right_hand(saved_right);
        if(movement_.field62d8&0x80000000u)direction=aim_hand_direction();
        finish_aimed_weapon(direction);
        present_aimed_arm_chains();
    }
    void present_crouched_weapon(const Vector* velocity){ //007BB540.
        limit_aim_vertical();
        limit_aim_forward(.34f,.9404f);
        const bool moving=velocity&&aim_motion_square(*velocity)>9.99999993922529e-09f;
        if(movement_.field62cc==4){present_kind_four_aim(moving);return;}
        const auto first=frames_.actor_world_matrix(skeleton_.bones()[9].frame);
        const auto second=frames_.actor_world_matrix(skeleton_.bones()[8].frame);
        Vector up{{second[12]-first[12],second[13]-first[13],second[14]-first[14],second[15]}};
        normalize_human_ai_shot(up,(static_cast<long double>(up[0])*up[0]+
            static_cast<long double>(up[1])*up[1])+static_cast<long double>(up[2])*up[2]);
        auto basis=frames_.actor_world_matrix(model_);
        align_human_matrix_up(frames_,basis,up);
        auto direction=aim_hand_direction<true>();
        const auto saved_right=movement_.right_hand_offset6340;
        const auto blend=human_animation_word_float(movement_.field6318);
        Vector weights{{(moving?.15f:.25f)*blend,(moving?.7f:.75f)*blend,blend*0.f,0}};
        correct_human_bone_reference(frames_,skeleton_,model_,12,direction,movement_.direction6360,basis,weights);
        direction=aim_hand_direction<true>();
        weights[0]=.25f*blend;
        correct_human_bone_reference(frames_,skeleton_,model_,7,direction,movement_.direction6360,basis,weights);
        if(!moving&&!equipped_weapon_.butt()&&geometry_detail_)restore_aim_right_hand(saved_right);
        if(movement_.field62d8&0x80000000u)direction=aim_hand_direction<true>();
        finish_aimed_weapon(direction);
        present_aimed_arm_chains();
    }
    void present_special_weapon(){ //007BE6F0.
        limit_aim_vertical();
        limit_special_aim_forward();
        if(movement_.field62cc==4){present_kind_four_aim(false);return;}
        auto direction=aim_hand_direction<true>();
        const auto saved_right=movement_.right_hand_offset6340;
        const auto blend=human_animation_word_float(movement_.field6318);
        const Vector first_weights{{.35f*blend,.95f*blend,blend*0.f,0}};
        correct_human_bone_direction(frames_,skeleton_,model_,12,direction,movement_.direction6360,first_weights);
        direction=aim_hand_direction<true>();
        const Vector second_weights{{.45f*blend,.95f*blend,blend*0.f,0}};
        correct_human_bone_direction(frames_,skeleton_,model_,7,direction,movement_.direction6360,second_weights);
        if(!equipped_weapon_.butt()&&geometry_detail_)restore_aim_right_hand(saved_right);
        if(movement_.field62d8&0x80000000u)direction=aim_hand_direction<true>();
        finish_aimed_weapon(direction);
        present_aimed_arm_chains();
    }
    template<class Queries>void present_prone_weapon(Queries& queries){ //007BA830.
        //660C borrows a retained direction-work allocation, separate from the
        //body/point trees at6848/6874. Its second vector is initialized by0079BD30.
        const auto up=directions_[4][1];
        const auto right=frames_.actor_world_matrix(skeleton_.bones()[20].frame);
        const auto left=frames_.actor_world_matrix(skeleton_.bones()[21].frame);
        Vector start=up,displacement=up;
        for(unsigned axis=0;axis!=3;++axis){
            const float first=static_cast<float>(static_cast<long double>(right[12+axis])*.25f);
            const float second=static_cast<float>(static_cast<long double>(left[12+axis])*.75f);
            const float hands=first+second;
            const float offset=static_cast<float>(static_cast<long double>(up[axis])*.5f);
            start[axis]=offset+hands;
            displacement[axis]=static_cast<float>(static_cast<long double>(up[axis])*-2.f);
        }
        auto normal=up;
        if(const auto hit=queries.human_geometry_ray(start,displacement,0xc8102,0x230000,true))normal=hit->normal;
        auto& direction=movement_.direction6360;
        const auto dot=(static_cast<long double>(normal[2])*direction[2]+
            static_cast<long double>(normal[1])*direction[1])+static_cast<long double>(normal[0])*direction[0];
        const float stored=static_cast<float>(dot);
        if(dot<-.5f||stored>.3226f){
            const bool below=dot<-.5f;
            Vector planar=direction;
            for(unsigned axis=0;axis!=3;++axis){
                const float projection=static_cast<float>(static_cast<long double>(normal[axis])*stored);
                planar[axis]=direction[axis]-projection;
            }
            const auto length=std::sqrt(1.L-static_cast<long double>(stored)*stored);
            if(!(length>9.99999993922529e-09f)){
                direction=normal;
                if(below)for(unsigned axis=0;axis!=3;++axis)direction[axis]=-normal[axis];
            }else{
                const auto scale=(below?.866f:.9465f)/length;
                for(unsigned axis=0;axis!=3;++axis)planar[axis]=static_cast<float>(planar[axis]*scale);
                for(unsigned axis=0;axis!=3;++axis){
                    const float projection=static_cast<float>(static_cast<long double>(normal[axis])*(below?-.5f:.3226f));
                    direction[axis]=planar[axis]+projection;
                }
            }
        }
        limit_aim_forward(.866f,.5f);
        if(movement_.field62cc==4){
            movement_.field6350=direction;
            correct_aimed_bone(14,.25f,.5f);
            pose_equipment_weapon(movement_.field6350,true,true,1.f);
            if(movement_.field62d8&0x80000000u){
                movement_.left_hand_offset6330=frames_.car_transform(movement_.offset63a0,frames_.actor_world_matrix(skeleton_.bones()[21].frame));
                const auto grip=frames_.actor_world_matrix(equipped_weapon_.left());
                pose_hand_chain(17,19,Vector{{grip[12],grip[13],grip[14],grip[15]}},movement_.left_hand_offset6330,
                    skeleton_.bones()[21].frame,human_animation_word_float(movement_.field631c),nullptr);
            }
            return;
        }
        Vector current=direction;
        if(movement_.field62d8&0x80000000u)current=aim_hand_direction();
        finish_aimed_weapon(current);
        if(!geometry_detail_)return;
        refresh_weapon_hand_offsets();
        const Vector down{{0,-1,0,0}};
        if(movement_.field62d8&0x80000000u){
            //The original reads both RW and LW here, even when BUTT is absent.
            const auto right_grip=frames_.actor_world_matrix(equipped_weapon_.right());
            const auto left_grip=frames_.actor_world_matrix(equipped_weapon_.left());
            Vector target=movement_.left_hand_offset6330;
            for(unsigned axis=0;axis!=3;++axis){
                const float first=static_cast<float>(static_cast<long double>(right_grip[12+axis])*.25f);
                const auto second=static_cast<long double>(left_grip[12+axis])*.75f;
                target[axis]=static_cast<float>((axis==2?static_cast<float>(second):second)+first);
            }
            pose_hand_chain(17,19,target,movement_.left_hand_offset6330,skeleton_.bones()[21].frame,
                human_animation_word_float(movement_.field631c),equipped_weapon_.butt()?&down:nullptr);
        }
        if(equipped_weapon_.butt()){
            const auto grip=frames_.actor_world_matrix(equipped_weapon_.right());
            pose_hand_chain(16,18,Vector{{grip[12],grip[13],grip[14],grip[15]}},movement_.right_hand_offset6340,
                skeleton_.bones()[20].frame,1.f,nullptr);
        }
    }
    template<class Human,class Control>void present_aimed_weapon(Human& human,Control& control){ //007B9CB0.
        advance_aim_direction();
        if(!(control.mode_flags()&2u))return;
        const auto weapon=equipped_weapon_.model();
        if(!weapon)throw std::logic_error("Human aimed pose requires its actual retained weapon model");
        const auto parent=frames_.object_parent(weapon);
        frames_.human_geometry_weapon_link(weapon,Frame{});
        try{
            switch(movement_.field62d0){
            case 0:present_standing_weapon(&human.activation.vector1b0);break;
            case 1:present_crouched_weapon(&human.activation.vector1b0);break;
            case 2:present_prone_weapon(control);break;
            case 3:present_special_weapon();break;
            }
            if(control.human_is_player())adjust_aimed_camera();
        }catch(...){
            const auto failure=std::current_exception();
            try{frames_.human_geometry_weapon_link(weapon,parent);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
            std::rethrow_exception(failure);
        }
        frames_.human_geometry_weapon_link(weapon,parent);
    }
    void present_weapon_mount(std::uint32_t mode){ //007BA0C0.
        movement_.time62c0-=frame_elapsed_; //Always subtracts, including an initially zero timer.
        const auto remaining=human_frame_signed_word(movement_.time62c0);
        if(remaining>=0)interpolate_aim_direction(remaining);
        if(!(mode&2u)||movement_.field62d0==2)return;
        auto& direction=movement_.direction6360;
        auto world=frames_.actor_world_matrix(model_);
        const auto vertical=(static_cast<long double>(world[6])*direction[2]+
            static_cast<long double>(world[5])*direction[1])+static_cast<long double>(world[4])*direction[0];
        if(std::abs(vertical)>.699999988079071){ //Original DOUBLE00810898.
            const float stored=static_cast<float>(vertical);
            world=frames_.actor_world_matrix(model_);
            const auto excess=static_cast<long double>(stored)-(std::signbit(stored)?-1.L:1.L)*.7f;
            for(unsigned axis=0;axis!=3;++axis){
                const float projection=static_cast<float>(excess*world[4+axis]);
                direction[axis]-=projection;
            }
            //This body stores the square before the unit-length comparison.
            normalize_human_ai_shot(direction,static_cast<float>(aim_motion_square(direction)));
        }
        world=frames_.actor_world_matrix(model_);
        const auto forward=(static_cast<long double>(direction[2])*world[10]+
            static_cast<long double>(direction[1])*world[9])+static_cast<long double>(direction[0])*world[8];
        if(forward<.34f){
            const float stored=static_cast<float>(forward);
            world=frames_.actor_world_matrix(model_);
            const auto excess=static_cast<long double>(stored)-.34f;
            direction[0]=static_cast<float>(direction[0]-excess*world[8]);
            const float y=static_cast<float>(excess*world[9]),z=static_cast<float>(excess*world[10]);
            direction[1]-=y;direction[2]-=z;
            normalize_human_ai_shot(direction,static_cast<float>(aim_motion_square(direction)));
        }
        const Vector weights{{.85f,.85f,0,0}};
        for(unsigned bone:{12u,7u}){
            const auto body=frames_.actor_world_matrix(skeleton_.bones()[7].frame);
            correct_human_bone_direction(frames_,skeleton_,model_,bone,
                Vector{{body[8],body[9],body[10],body[11]}},direction,weights);
        }
    }
    void pose_head_direction(const Vector& desired,float influence){ //007ADA10.
        const auto& head=skeleton_.bones()[6];
        const auto first=frames_.actor_world_matrix(head.frame);
        Vector forward{{first[8],first[9],first[10],first[11]}};
        normalize_human_ai_shot(forward,(static_cast<long double>(forward[1])*forward[1]+
            static_cast<long double>(forward[2])*forward[2])+static_cast<long double>(forward[0])*forward[0]);
        auto alignment=human_identity();
        human_align_vectors(forward,desired,alignment);
        alignment[15]=1;
        const auto current=frames_.human_matrix_rotation(frames_.actor_world_matrix(head.frame));
        const auto target=frames_.human_matrix_rotation(frames_.rigid_matrix_product(frames_.actor_world_matrix(head.frame),alignment));
        frames_.human_set_matrix_rotation(alignment,frames_.physics_slerp(current,target,influence,false));
        if(head.parent<0)throw std::logic_error("Human head direction requires its actual skeleton parent");
        auto inverse=human_transpose(frames_.actor_world_matrix(skeleton_.bones()[static_cast<std::size_t>(head.parent)].frame));
        inverse[3]=inverse[7]=inverse[11]=0;
        alignment=frames_.car_matrix_product(alignment,inverse);
        frames_.human_publish_bone_rotation(head.frame,alignment);
    }
    template<class Human,class Control,class Services>
    void begin_mounted_effect(Human& human,Control& control,Human& source,Services& services){
        //007B0910(flags8,zero,null,reason4,source). Other reasons and the
        //source-null impulse/clip branches remain separate required receivers.
        if(!(control.mode_flags()&2u)){joints_.field68d0=0;return;}
        if(human_!=&human||control_!=&control||!model_)
            throw std::logic_error("Human geometry effect requires its SAME bound DD0 and model");
        geometry_effect_.emplace(GeometryEffect{4,&source});
        if((control.mode_flags()&7u)==7u)(void)services.human_geometry_effect_quality();
        if(services.human_unavailable(human)){
            joints_.build(false);
            for(unsigned index=0;index!=saved_body_angles_.size();++index)
                if(auto* body=joints_.bodies()[index])body->set(0x180,saved_body_angles_[index]);
        }
        joints_.gravity[0]=0;joints_.gravity[1]=-9;joints_.gravity[2]=0;
        joints_.field68d0=1;
        for(auto* point:joints_.points())if(point)point->set(0x24,1.f);
        joints_.field6900=6;joints_.field68e8=0;joints_.field68ec=1;joints_.field68f0=4;
        if(joints_.field690c)joints_.retire_contact_frames();
        joints_.flag6912=false;
        constexpr std::array<unsigned,9> bones{{9,12,13,7,15,14,4,5,6}};
        constexpr std::array<unsigned,9> saved{{0,1,2,3,5,6,7,8,9}};
        for(unsigned index=0;index!=bones.size();++index){
            if(index==4)(void)services.human_unavailable(human); //Native repeated query before restoring both arms.
            const auto frame=skeleton_.bones()[bones[index]].frame;
            if(!frame)throw std::runtime_error("Human geometry effect lost its actual saved-pose bone");
            frames_.human_equipment_local_matrix(frame,joints_.saved_local()[saved[index]]);
        }
        const auto root=skeleton_.bones()[9].frame;
        if(!root)throw std::runtime_error("Human geometry effect requires its actual pose root");
        frames_.update_scene_object(root);
        joints_.rebind_effect_bodies([&]{return services.human_unavailable(human);});
        //The original calculates a local velocity without external calls, then
        //overwrites all XYZ with zero for this nonnull-source invocation.
        joints_.gravity[0]=0;joints_.gravity[1]=-9.81f;joints_.gravity[2]=0;
        joints_.initialize_mounted_effect_points();
        (void)services.human_unavailable(human); //Source is nonnull, so native arm-removal condition is false.
        joints_.capture_body_pose();joints_.snapshot_body_endpoints();
        auto* point=joints_.points()[0];
        if(!point)throw std::runtime_error("Human geometry effect requires its actual point root");
        joints_.reset_point(point,point->template get<Vector>(0x30),false);
        frames_.detach_human_effect_model(model_);
        geometry_effect_->root_position=point->template get<Vector>(0x30);
        frames_.register_human_effect_model(model_,marker_z_,skeleton_.bones()[1].frame);
        frames_.refresh_dynamic_human_model(model_);
        movement_.field6304|=0x200u;
        const auto camera=camera_fpv();
        if(!camera)throw std::runtime_error("Human geometry effect requires its actual FPV camera");
        const auto camera_matrix=frames_.actor_world_matrix(camera);
        geometry_effect_->camera_position={{camera_matrix[12],camera_matrix[13],camera_matrix[14],camera_matrix[15]}};
        auto& inline_owner=services.human_inline(human);
        inline_owner.exclude_query(false);
        try{
            for(auto* joint:joints_.points())if(joint){
                auto* actor=services.human_geometry_containing_actor(joint->template get<Vector>(0x30),0xd8102u);
                if(actor){geometry_effect_->support_actor=actor;movement_.field6304|=0x10u;break;}
            }
        }catch(...){inline_owner.restore_query(false);throw;}
        inline_owner.restore_query(false);
    }
    void command_look(std::uint32_t flags,const Vector* target) {
        look_.command(frames_,random_,model_,skeleton_.bones()[6].frame,flags,target);
    }
    const HumanLookState& look() const {return look_;}
    void equipment_face_flag(bool enabled) const { //007B85A0 dispatches only when62B0 exists.
        if(!model_)throw std::logic_error("Equipment face flag requires bound Human geometry");
        if(face_model_)face_model_->set_shadow(enabled);
    }
    const auto* offline_face_model() const{return face_model_.get();}
    void set_equipment_weapon(Frame model,std::int32_t kind){equipped_weapon_.select(model,kind);}
    void equipment_weapon_pose(unsigned command){equipped_weapon_.pose(command);}
    void stow_equipment_weapon(bool enabled){equipped_weapon_.stow(enabled);}
    const auto& equipped_weapon() const{return equipped_weapon_;}
    struct Movement {
        bool flag62c8=false,flag65f0=false,flag6624=false;
        std::uint32_t field62cc=0,field62d0=0,field65f4=0,field62d8=1,
            field6304=0,field6308=0,field630c=300,field6310=0,field6314=0,field6318=0,field631c=0,field6410=100;
        std::int32_t field62dc=0;
        std::uint32_t time62c0=0,time62c4=0;
        float angle63c0=0,sine63c4=0;
        Vector direction6360{},direction6370{},direction6380{},offset6390{},
            position6400{},field6350{},field63d0{},field63e0{},field63f0{},left_hand_offset6330{},right_hand_offset6340{},
            offset63a0{{-.075f,-.041f,-.003f,0}},offset63b0{{.075f,-.041f,-.003f,0}};
    };
    const Movement& movement() const {return movement_;}
    void clear_ground_geometry_flag(){movement_.flag65f0=false;} //007637D0, native65F0.
    void set_posture_transition(std::uint32_t posture){ //0076AF70, SAME geometry62D0/65F0/65F4.
        movement_.field62d0=posture;
        if(posture==2){movement_.flag65f0=true;movement_.field65f4=500;}
    }
    void capture_action_overlay(Frame clip){ //007B8C10, six retained key pairs for each original bone.
        if(!clip){movement_.flag6624=false;return;}
        std::uint32_t time=0; //Native shares this ignored key-time output between both engine calls.
        for(unsigned bone=0;bone!=human_bone_names.size();++bone){
            const auto index=frames_.human_clip_bone_index(clip,human_bone_names[bone]);
            for(std::int32_t key=0;key!=6;++key){
                auto& position=fields6620_[static_cast<unsigned>(key)][bone];
                if(key<frames_.human_clip_position_count(clip,index))
                    frames_.human_clip_position_key(clip,index,key,time,position);
                else position[0]=std::numeric_limits<float>::quiet_NaN();
                auto& rotation=fields661c_[static_cast<unsigned>(key)][bone];
                if(key<frames_.human_clip_rotation_count(clip,index))
                    frames_.human_clip_rotation_key(clip,index,key,time,rotation);
                else rotation[3]=std::numeric_limits<float>::quiet_NaN();
            }
        }
        movement_.flag6624=true;
    }
    void apply_hit_impulse(std::uint32_t mask,const Vector& value,std::int32_t duration){joints_.distribute_impulse(random_,mask,value,duration);} //007B3270, SAME process CRT and joint queues.
    template<class Services>void command_hit(std::uint32_t mask,const Vector& position,const Vector& direction,Services& services){ //007B8450.
        if(face_model_){
            auto clock=[&]{return services.game_frame_serial();};
            face_model_->apply_hit(position,direction,mask,random_,clock);
        }
        if(!(movement_.field6304&0x200u)||(geometry_effect_&&geometry_effect_->source))return;
        const float square=static_cast<float>((static_cast<long double>(direction[0])*direction[0]+
            static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[2])*direction[2]);
        if(!mask||!(square>9.99999993922529e-09f))return;
        const auto length=std::sqrt(static_cast<long double>(square)),inverse=1.L/length;
        auto impulse=direction; //Native temporary W is unwritten. Retain the known input padding.
        for(unsigned axis=0;axis!=3;++axis)impulse[axis]=static_cast<float>(inverse*direction[axis]);
        const auto scaled=length*.1111111119389534f;
        const float amount=scaled>150.f?150.f:static_cast<float>(scaled);
        for(unsigned axis=0;axis!=3;++axis)impulse[axis]=static_cast<float>(static_cast<long double>(impulse[axis])*amount);
        impulse[1]=static_cast<float>(static_cast<long double>(amount)*.3333333432674408f+impulse[1]);
        if(joints_.field68d0<9.99999993922529e-09f){
            auto* root=joints_.points()[0];
            if(!root)throw std::logic_error("Human hit impulse requires its actual initialized root point");
            joints_.reset_point(root,root->template get<Vector>(0x30),true);
        }
        apply_hit_impulse(mask,impulse,200);
    }
    void command_target_direction(const Vector& target,bool player) { //007B8E80, no normalization of its input.
        if(!(movement_.field6304&1u))return;
        auto snap=[&]{movement_.direction6360=movement_.direction6380=movement_.direction6370=target;movement_.time62c0=0;};
        if(human_&&player){snap();return;} //Native leaves62C4 and angle/sine untouched.
        const auto& current=movement_.direction6360;
        const auto dot=(static_cast<long double>(current[2])*target[2]+static_cast<long double>(current[1])*target[1])+static_cast<long double>(current[0])*target[0];
        movement_.angle63c0=static_cast<float>(dot);
        if(dot>1)movement_.angle63c0=1;else if(dot< -1)movement_.angle63c0=-1;
        if(1.L-std::abs(static_cast<long double>(movement_.angle63c0))<9.99999993922529e-09){snap();return;}
        const auto angle=std::acos(static_cast<long double>(movement_.angle63c0));
        movement_.angle63c0=static_cast<float>(angle);movement_.sine63c4=static_cast<float>(std::sin(angle));
        const auto duration=static_cast<long double>(movement_.angle63c0)*95.49296569824219f; //00816E78.
        movement_.time62c0=movement_.time62c4=std::isfinite(duration)?static_cast<std::uint32_t>(duration):0; //__ftol QWORD indefinite has LOW DWORD0.
        movement_.direction6370=movement_.direction6360;movement_.direction6380=target;
    }
    void blend_equipment_weapon(std::int32_t duration){equipped_weapon_.blend(duration);} //007B8D00.
    void unblend_equipment_weapon(std::int32_t duration){equipped_weapon_.unblend(duration);} //007B8D60.
    void suspend_equipment_weapon(std::int32_t duration,bool detach){equipped_weapon_.suspend(duration,detach);} //007B8E10.
    void suspend_equipment_for_item_action(std::int32_t duration){equipped_weapon_.suspend_for_item_action(duration);} //007B8DC0.
    void set_weapon_blend_duration(std::int32_t duration){ //00778B20 writes630C before6308, retaining actual FLOAT6318.
        movement_.field630c=static_cast<std::uint32_t>(duration);
        movement_.field6308=static_cast<std::uint32_t>(human_animation_time_integer(static_cast<long double>(duration)*human_animation_word_float(movement_.field6318)));
    }
    template<class Human,class Control,class Runtime>void present_frame(Human& human,Control& control,std::uint32_t elapsed,Runtime& runtime){ //007A7470.
        if(human_!=&human||control_!=&control||!model_)throw std::logic_error("Human geometry frame lost its actual owners");
        frame_elapsed_=elapsed;++seed_; //Native62B4/62B8.
        auto countdown=movement_.field6410-elapsed;
        movement_.field6410=countdown;
        if(human_frame_signed_word(countdown)<1){
            movement_.field6410=countdown+100u;
            movement_.field63e0=movement_.field63d0;
            const auto world=frames_.actor_world_matrix(model_);
            const float scale=static_cast<float>(1000.L/human_frame_signed_word(elapsed)); //0080F708.
            for(unsigned axis=0;axis!=2;++axis)
                movement_.field63f0[axis]=static_cast<float>((static_cast<long double>(world[12+axis])-movement_.position6400[axis])*scale);
            const float z=world[14]-movement_.position6400[2];
            movement_.field63f0[2]=z*scale;
        }
        const auto world=frames_.actor_world_matrix(model_);
        movement_.position6400={{world[12],world[13],world[14],world[15]}};
        const float fraction=static_cast<float>(static_cast<long double>(human_frame_signed_word(movement_.field6410))*.01f);
        const float complement=1.f-fraction;
        for(unsigned axis=0;axis!=3;++axis){
            const float latest=complement*movement_.field63f0[axis];
            movement_.field63d0[axis]=static_cast<float>(static_cast<long double>(fraction)*movement_.field63e0[axis]+latest);
        }
        geometry_detail_=(control.animation_state_word(0x22c)&0x380u)==0; //Native62C9.
        if(movement_.field6304&0x200u){
            runtime.present_human_ragdoll_geometry(*this,control,human,elapsed);
            return;
        }
        if(human_frame_signed_word(human.properties.stats_ec[0])<1)return;
        if(movement_.flag65f0&&control.animation_look_word(0xb8)!=5&&
           ((control.mode_flags()&7u)==7u||control.human_is_player()))
            runtime.present_human_foot_geometry(*this,control,human); //007A8520.
        const auto flags=movement_.field6304;
        if((flags&1u)||(!(flags&0x3du)&&(flags&2u))){
            const auto weapon=equipped_weapon_.model();
            if(!weapon||!frames_.object_parent(weapon)||(!equipped_weapon_.left()&&!equipped_weapon_.right())||
               ((movement_.field62d8&0x80000000u)&&!equipped_weapon_.left())){
                movement_.field6304=0;movement_.flag62c8=false;
                return;
            }
            if(flags&1u)present_aimed_weapon(human,control); //007B9CB0.
            else if((control.mode_flags()&2u)&&movement_.field62cc!=4&&(movement_.field62d8&0x80000000u))
                present_stowed_weapon(control.mode_flags()); //007B9750.
        }else if(flags&4u)present_weapon_mount(control.mode_flags()); //007BA0C0.
        else if(flags&8u){
            runtime.present_human_attached_weapon_geometry(*this,control,human); //007AE3C0.
            const auto world=frames_.actor_world_matrix(model_);
            pose_head_direction(Vector{{world[8],world[9],world[10],world[11]}},1.f); //007ADA10.
        }
        else if(flags&0x10u)runtime.present_human_vehicle_weapon_geometry(*this,control,human); //007AF320.
        else if(flags&0x20u)runtime.present_human_target_weapon_geometry(*this,control,human); //007AEBE0.
        equipped_weapon_.advance_frame(elapsed);
        if(movement_.field6304&0x40u)runtime.present_human_target_weapon_geometry(*this,control,human);
        if(movement_.field6304&0x100u)present_equipment_overlay(); //007AFD70.
        if(movement_.field6304&0x80u)present_secondary_clip(); //007AFC20.
        if(control.mode_flags()&2u){
            if(look_.flags||(movement_.field6304&0x35u))runtime.present_human_look_geometry(*this,control,human,elapsed); //007ACCC0.
            control.resolve_model_geometry_camera();
        }
        if(runtime.human_model_debug_enabled())runtime.present_human_model_debug(*this);
    }
private:
    void advance_aim_direction(){ //007B9CB0, actual62C0 countdown and retained angle/sine.
        if(!movement_.time62c0)return;
        movement_.time62c0-=frame_elapsed_;
        const auto remaining=human_frame_signed_word(movement_.time62c0);
        if(remaining<0){movement_.time62c0=0;movement_.direction6360=movement_.direction6380;return;}
        interpolate_aim_direction(remaining);
    }
    void interpolate_aim_direction(std::int32_t remaining){ //Shared arithmetic007B9CB0/007BA0C0, distinct timer policies.
        const float fraction=static_cast<float>(static_cast<long double>(remaining)/human_frame_signed_word(movement_.time62c4));
        const float inverse_sine=static_cast<float>(1.L/movement_.sine63c4);
        const auto finish=std::sin((1.L-fraction)*movement_.angle63c0)*inverse_sine;
        Vector target=movement_.direction6360;
        for(unsigned axis=0;axis!=3;++axis)target[axis]=static_cast<float>(finish*movement_.direction6380[axis]);
        const auto start=std::sin(static_cast<long double>(fraction)*movement_.angle63c0)*inverse_sine;
        for(unsigned axis=0;axis!=3;++axis){
            const float previous=static_cast<float>(start*movement_.direction6370[axis]);
            movement_.direction6360[axis]=target[axis]+previous;
        }
    }
    void adjust_aimed_camera(){ //007B9CB0, real retained62F8 and head bone6.
        const auto camera=camera3rd();
        if(!camera)throw std::logic_error("Human aimed camera requires its actual retained camera3RD frame");
        const auto origin=frames_.actor_world_matrix(model_);
        auto delta=frames_.car_transform(movement_.offset6390,frames_.actor_world_matrix(skeleton_.bones()[6].frame));
        for(unsigned axis=0;axis!=3;++axis)delta[axis]-=origin[12+axis];
        const auto basis=frames_.actor_world_matrix(model_);
        const auto position=frames_.actor_world_matrix(model_);
        const auto previous=frames_.actor_world_matrix(camera);
        const float old_projection=static_cast<float>(((static_cast<long double>(previous[14])-position[14])*basis[2]+
            (static_cast<long double>(previous[13])-position[13])*basis[1])+
            (static_cast<long double>(previous[12])-position[12])*basis[0]);
        const auto axis=frames_.actor_world_matrix(model_);
        const auto projection_basis=frames_.actor_world_matrix(model_);
        const auto difference=((static_cast<long double>(delta[1])*projection_basis[1]+
            static_cast<long double>(delta[2])*projection_basis[2])+static_cast<long double>(delta[0])*projection_basis[0])-old_projection;
        delta[0]=static_cast<float>(delta[0]-difference*axis[0]);
        const float y=static_cast<float>(difference*axis[1]),z=static_cast<float>(difference*axis[2]);
        delta[1]-=y;delta[2]-=z;
        const auto world=frames_.actor_world_matrix(model_);
        for(unsigned component=0;component!=3;++component)delta[component]+=world[12+component];
        const float vertical=std::abs(movement_.direction6360[1]);
        const auto current=frames_.actor_world_matrix(camera);
        const auto amount=static_cast<long double>(vertical)*human_animation_word_float(movement_.field6318);
        const auto complement=1.L-amount;
        const float old_x=static_cast<float>(complement*current[12]),old_y=static_cast<float>(complement*current[13]);
        const float new_y=static_cast<float>(delta[1]*amount),new_z=static_cast<float>(delta[2]*amount);
        frames_.object_position(camera,{{static_cast<float>(delta[0]*amount+old_x),new_y+old_y,
            static_cast<float>(new_z+complement*current[14])}});
    }
    static long double aim_motion_square(const Vector& value){
        return (static_cast<long double>(value[0])*value[0]+static_cast<long double>(value[1])*value[1])+
            static_cast<long double>(value[2])*value[2];
    }
    void limit_aim_vertical(){ //Shared prefix007BCFC0/007BB540/007BE6F0.
        auto world=frames_.actor_world_matrix(model_);
        auto& direction=movement_.direction6360;
        const auto dot=(static_cast<long double>(direction[2])*world[6]+
            static_cast<long double>(direction[1])*world[5])+static_cast<long double>(direction[0])*world[4];
        const float stored=static_cast<float>(dot);
        if(!(dot>.7f||stored<-.7f))return;
        const float sign=std::signbit(stored)?-1.f:1.f;
        world=frames_.actor_world_matrix(model_);
        Vector planar=direction;
        for(unsigned axis=0;axis!=3;++axis){
            const float projection=static_cast<float>(static_cast<long double>(stored)*world[4+axis]);
            planar[axis]=direction[axis]-projection;
        }
        const auto length=std::sqrt(1.L-static_cast<long double>(stored)*stored);
        if(!(length>9.99999993922529e-09f)){
            world=frames_.actor_world_matrix(model_);
            for(unsigned axis=0;axis!=3;++axis)direction[axis]=sign*world[4+axis];
            return;
        }
        const auto scale=.7141f/length;
        for(unsigned axis=0;axis!=3;++axis)planar[axis]=static_cast<float>(planar[axis]*scale);
        world=frames_.actor_world_matrix(model_);
        for(unsigned axis=0;axis!=3;++axis){
            const float projection=static_cast<float>(static_cast<long double>(sign)*.7f*world[4+axis]);
            direction[axis]=planar[axis]+projection;
        }
    }
    void limit_aim_forward(float minimum,float tangent){ //007BCFC0/007BB540 and007BA830.
        auto world=frames_.actor_world_matrix(model_);
        auto& direction=movement_.direction6360;
        const auto dot=(static_cast<long double>(direction[2])*world[10]+
            static_cast<long double>(direction[1])*world[9])+static_cast<long double>(direction[0])*world[8];
        if(!(dot<minimum))return;
        const float stored=static_cast<float>(dot);
        world=frames_.actor_world_matrix(model_);
        Vector planar=direction;
        for(unsigned axis=0;axis!=3;++axis){
            const float projection=static_cast<float>(static_cast<long double>(stored)*world[8+axis]);
            planar[axis]=direction[axis]-projection;
        }
        const auto length=std::sqrt(1.L-static_cast<long double>(stored)*stored);
        if(!(length>9.99999993922529e-09f)){
            world=frames_.actor_world_matrix(model_);
            direction={{world[8],world[9],world[10],world[11]}};
            return;
        }
        const auto scale=static_cast<long double>(tangent)/length;
        for(unsigned axis=0;axis!=3;++axis)planar[axis]=static_cast<float>(planar[axis]*scale);
        world=frames_.actor_world_matrix(model_);
        for(unsigned axis=0;axis!=3;++axis){
            const float projection=static_cast<float>(static_cast<long double>(minimum)*world[8+axis]);
            direction[axis]=planar[axis]+projection;
        }
    }
    void limit_special_aim_forward(){ //007BE6F0 has distinct projection/addition spills.
        auto world=frames_.actor_world_matrix(model_);
        auto& direction=movement_.direction6360;
        const auto dot=(static_cast<long double>(direction[2])*world[10]+
            static_cast<long double>(direction[1])*world[9])+static_cast<long double>(direction[0])*world[8];
        if(!(dot<.707f))return;
        const float stored=static_cast<float>(dot);
        world=frames_.actor_world_matrix(model_);
        const float z=static_cast<float>(static_cast<long double>(stored)*world[10]);
        const float x=static_cast<float>(direction[0]-static_cast<long double>(stored)*world[8]);
        const auto y=direction[1]-static_cast<long double>(stored)*world[9];
        const auto length=std::sqrt(1.L-static_cast<long double>(stored)*stored);
        if(!(length>9.99999993922529e-09f)){
            world=frames_.actor_world_matrix(model_);
            direction={{world[8],world[9],world[10],world[11]}};
            return;
        }
        const auto scale=.707f/length;
        const Vector planar{{static_cast<float>(x*scale),static_cast<float>(y*scale),
            static_cast<float>((static_cast<long double>(direction[2])-z)*scale),direction[3]}};
        world=frames_.actor_world_matrix(model_);
        for(unsigned axis=0;axis!=3;++axis)
            direction[axis]=static_cast<float>(static_cast<long double>(world[8+axis])*.707f+planar[axis]);
    }
    template<bool XFirst=false>Vector aim_hand_direction(){
        refresh_weapon_hand_offsets();
        auto direction=movement_.left_hand_offset6330;
        for(unsigned axis=0;axis!=3;++axis)direction[axis]-=movement_.right_hand_offset6340[axis];
        const auto x=static_cast<long double>(direction[0])*direction[0];
        const auto y=static_cast<long double>(direction[1])*direction[1];
        const auto z=static_cast<long double>(direction[2])*direction[2];
        normalize_human_ai_shot(direction,XFirst?(x+z)+y:(z+y)+x);
        return direction;
    }
    void correct_aimed_bone(unsigned bone,float x,float y){
        const auto hand=frames_.actor_world_matrix(skeleton_.bones()[20].frame);
        const Vector direction{{hand[0],hand[1],hand[2],hand[3]}};
        const auto blend=human_animation_word_float(movement_.field6318);
        const Vector weights{{x*blend,y*blend,blend*0.f,0}};
        correct_human_bone_direction(frames_,skeleton_,model_,bone,direction,movement_.direction6360,weights);
    }
    void present_kind_four_aim(bool moving){ //Identical kind4 tail007BCFC0/007BB540, also007BE6F0's stationary case.
        if(moving){
            const auto first=frames_.actor_world_matrix(skeleton_.bones()[7].frame);
            const auto second=frames_.actor_world_matrix(skeleton_.bones()[4].frame);
            Vector up{{second[12]-first[12],second[13]-first[13],second[14]-first[14],second[15]}};
            normalize_human_ai_shot(up,(static_cast<long double>(up[2])*up[2]+
                static_cast<long double>(up[1])*up[1])+static_cast<long double>(up[0])*up[0]);
            const auto blend=human_animation_word_float(movement_.field6318);
            const Vector weights{{blend*0.f,blend*0.f,blend*.5f,0}};
            correct_human_bone_direction(frames_,skeleton_,model_,7,up,Vector{{0,1,0,0}},weights);
            correct_aimed_bone(12,.15f,.35f);
            correct_aimed_bone(7,.25f,.35f);
            correct_aimed_bone(14,.25f,.5f);
        }else{
            correct_aimed_bone(12,.35f,.75f);
            correct_aimed_bone(7,.35f,.75f);
            correct_aimed_bone(14,.75f,.75f);
        }
        refresh_weapon_hand_offsets();
        const auto hand=frames_.actor_world_matrix(skeleton_.bones()[20].frame);
        movement_.field6350=interpolate_human_directions(Vector{{hand[0],hand[1],hand[2],hand[3]}},
            movement_.direction6360,human_animation_word_float(movement_.field6318));
        pose_equipment_weapon(movement_.field6350,true,true,1.f);
        if(movement_.field62d8&0x80000000u){
            movement_.left_hand_offset6330=frames_.car_transform(movement_.offset63a0,frames_.actor_world_matrix(skeleton_.bones()[21].frame));
            const auto grip=frames_.actor_world_matrix(equipped_weapon_.left());
            pose_hand_chain(17,19,Vector{{grip[12],grip[13],grip[14],grip[15]}},movement_.left_hand_offset6330,
                skeleton_.bones()[21].frame,human_animation_word_float(movement_.field631c),nullptr);
        }
    }
    void restore_aim_right_hand(const Vector& saved){
        refresh_weapon_hand_offsets();
        const auto& current=movement_.right_hand_offset6340;
        Vector target=saved;
        target[0]=static_cast<float>((static_cast<long double>(current[0])-saved[0])*.8f+saved[0]);
        const float y=static_cast<float>((static_cast<long double>(current[1])-saved[1])*.8f);
        target[1]=y+saved[1];
        const float z=current[2]-saved[2];
        target[2]=static_cast<float>(static_cast<long double>(z)*.8f+saved[2]);
        pose_hand_chain(16,18,target,current,skeleton_.bones()[20].frame,1.f,nullptr);
    }
    void finish_aimed_weapon(Vector direction){
        const auto selected=movement_.field62d8&0xffu;
        if(selected==0||selected==1){
            const auto hand=frames_.actor_world_matrix(selected==1?weapon1_:weapon2_);
            Vector reference{{hand[0],hand[1],hand[2],hand[3]}};
            if(!selected)for(unsigned axis=0;axis!=3;++axis)reference[axis]=-reference[axis];
            direction=(movement_.field62d8&0x80000000u)?
                interpolate_human_directions(reference,direction,human_animation_word_float(movement_.field631c)):reference;
        }
        movement_.field6350=interpolate_human_directions(direction,movement_.direction6360,human_animation_word_float(movement_.field6318));
        pose_equipment_weapon(movement_.field6350,true,true,human_animation_word_float(movement_.field631c));
    }
    void present_aimed_arm_chains(){
        if(!geometry_detail_)return;
        refresh_weapon_hand_offsets();
        const Vector down{{0,-1,0,0}};
        if(movement_.field62d8&0x80000000u){
            const auto grip=frames_.actor_world_matrix(equipped_weapon_.left());
            pose_hand_chain(17,19,Vector{{grip[12],grip[13],grip[14],grip[15]}},movement_.left_hand_offset6330,
                skeleton_.bones()[21].frame,human_animation_word_float(movement_.field631c),equipped_weapon_.butt()?&down:nullptr);
        }
        if(equipped_weapon_.butt()){
            const auto grip=frames_.actor_world_matrix(equipped_weapon_.right());
            pose_hand_chain(16,18,Vector{{grip[12],grip[13],grip[14],grip[15]}},movement_.right_hand_offset6340,
                skeleton_.bones()[20].frame,1.f,nullptr);
        }
    }
    void refresh_weapon_hand_offsets(){ //007AFD70/007B9750, actual right-before-left engine queries.
        movement_.right_hand_offset6340=frames_.car_transform(movement_.offset63b0,frames_.actor_world_matrix(skeleton_.bones()[20].frame));
        movement_.left_hand_offset6330=frames_.car_transform(movement_.offset63a0,frames_.actor_world_matrix(skeleton_.bones()[21].frame));
    }
    void present_weapon_overlay(const Vector& weights){ //007AFD70, non-kind4 tail.
        const auto weapon=equipped_weapon_.model();
        const auto parent=frames_.object_parent(weapon);
        frames_.human_geometry_weapon_link(weapon,Frame{});
        try{
            auto desired=movement_.field6350;
            desired[0]=static_cast<float>(static_cast<long double>(movement_.field6350[0])*.9396926164627075f-
                static_cast<long double>(movement_.field6350[2])*.3420201539993286f);
            desired[2]=static_cast<float>(static_cast<long double>(movement_.field6350[0])*.3420201539993286f+
                static_cast<long double>(movement_.field6350[2])*.9396926164627075f);
            const auto square=(static_cast<long double>(desired[2])*desired[2]+
                static_cast<long double>(desired[1])*desired[1])+static_cast<long double>(desired[0])*desired[0];
            normalize_human_ai_shot(desired,square);
            correct_human_bone_direction(frames_,skeleton_,model_,9,desired,movement_.field6350,weights);
            correct_human_bone_direction(frames_,skeleton_,model_,8,desired,movement_.field6350,weights);
            const auto blend=human_animation_word_float(movement_.field631c);
            pose_equipment_weapon(movement_.field6350,true,true,blend);
            const auto right_hand=skeleton_.bones()[20].frame,left_hand=skeleton_.bones()[21].frame;
            refresh_weapon_hand_offsets();
            if(movement_.field62d8&0x80000000u){
                const auto point=frames_.actor_world_matrix(equipped_weapon_.left());
                const Vector target{{point[12],point[13],point[14],point[15]}},preferred{{0,-1,0,0}};
                pose_hand_chain(17,19,target,movement_.left_hand_offset6330,left_hand,blend,equipped_weapon_.butt()?&preferred:nullptr);
            }
            if(equipped_weapon_.butt()){
                const auto point=frames_.actor_world_matrix(equipped_weapon_.right());
                const Vector target{{point[12],point[13],point[14],point[15]}};
                pose_hand_chain(16,18,target,movement_.right_hand_offset6340,right_hand,1.f,nullptr);
            }
        }catch(...){
            const auto failure=std::current_exception();
            try{frames_.human_geometry_weapon_link(weapon,parent);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
            std::rethrow_exception(failure);
        }
        frames_.human_geometry_weapon_link(weapon,parent);
    }
    void reset_loading() { //0079BD30(false), state written before any model-dependent work.
        const auto random=random_();if(random>0x7fff)throw std::runtime_error("Human geometry requires shared CRT random");
        seed_=random+1;
        look_={};
        for(unsigned group=2;group!=5;++group)for(auto& value:directions_[group])value=Vector{{0,1,0,0}};
        directions_[4][0][0]=std::numeric_limits<float>::quiet_NaN(); //Native660C first word only.
        fields6614_={};
        saved_body_angles_.fill(0); //0079C18B..0079C1A1 clears eleven words at691C.
        geometry_effect_.reset();
        secondary_pose_=Frame{}; //0079BD30 clears6630, scratch6638/6694 is reset by007AE320.
    }
    void bind_frames() {
        auto camera=frames_.actor_find(model_,"camera3RD",0xffff);
        if(camera)camera3rd_.emplace(frames_.retain_actor_frame(camera));
        camera=frames_.actor_find(model_,"cameraFPV",0xffff);
        if(camera)camera_fpv_.emplace(frames_.retain_actor_frame(camera));
        weapon1_=frames_.actor_find(model_,"hw1",0xffff);weapon2_=frames_.actor_find(model_,"hw2",0xffff);
        marker_z_=frames_.actor_find(model_,"z",0xffff);
        feet_={{skeleton_.bones()[21].frame,skeleton_.bones()[20].frame}};
    }
    void reset_movement() { //007B85C0(false).
        equipped_weapon_.clear();
        movement_={};
        for(auto* direction:{&movement_.direction6370,&movement_.direction6380,&movement_.direction6360}) {
            const auto world=frames_.actor_world_matrix(model_);
            *direction=actor_activation_direction(Vector{{world[8],world[9],world[10],world[11]}});
        }
        const auto world=frames_.actor_world_matrix(model_);
        movement_.position6400={{world[12],world[13],world[14],world[15]}};
        //007B85C0 repeats these two lookups after releasing its motion resource.
        weapon1_=frames_.actor_find(model_,"hw1",0xffff);weapon2_=frames_.actor_find(model_,"hw2",0xffff);
        equipped_weapon_.bind_hands(weapon1_,weapon2_);
        if(camera3rd_)movement_.offset6390={{0,.25f,0,0}};
    }
    Frames& frames_;Random& random_;
    HumanSkeleton<Frame> skeleton_;
    HumanLoadingJoints<Frames> joints_;
    Frame model_{};const void* human_=nullptr;const void* control_=nullptr;
    std::optional<Owner> camera3rd_,camera_fpv_;
    Frame weapon1_{},weapon2_{},marker_z_{};
    std::array<Frame,2> feet_{};
    std::uint32_t seed_=0,frame_elapsed_=0;
    bool geometry_detail_=true;
    Movement movement_;
    HumanWeaponBinding<Frames,Movement> equipped_weapon_{frames_,movement_};
    OfflineFaces* offline_faces_=nullptr;bool offline_quality_=false;std::int32_t offline_camouflage_=0;
    std::unique_ptr<HumanFaceModel<Frames>> face_model_;
    HumanLookState look_;
    std::array<std::uint32_t,11> saved_body_angles_{};
    std::optional<GeometryEffect> geometry_effect_;
    Frame secondary_pose_{};
    std::uint32_t secondary_pose_time_=0;
    std::uint32_t equipment_overlay_time_=0;
    std::array<std::uint32_t,23> secondary_pose_current_{},secondary_pose_previous_{};
    std::array<std::uint8_t,6> fields65f8_{};
    alignas(16)std::array<std::array<Vector,6>,6> directions_{};
    std::array<std::uint32_t,6> fields6614_{};
    alignas(16)std::array<std::array<Vector,23>,6> fields661c_{},fields6620_{};
};
} //namespace hd2::game
#endif
