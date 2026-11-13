#ifndef HD2_GAME_CAR_BODY_INTEGRATION_H
#define HD2_GAME_CAR_BODY_INTEGRATION_H

#include "game/car_wheel_runtime.h"
#include "game/actor_activation_math.h"
#include "game/car_contact_callbacks.h"

namespace hd2::game {

//004EA780's pose cache, and the force/inertia/pose integration block inside
//0050DE60. Collision resolution must inspect/accept the returned candidate
//before publishing it to the actual model and shared collision descriptors.
//The containing solver still owns wheel forces, contact response, substeps and
//0050B180's finalization. Supplying this block alone cannot settle/activate Car.
template<class Frames>class CarBodyIntegration {
public:
    using Frame=typename Frames::Frame;using Vector=typename Frames::Vector;using Matrix=typename Frames::Matrix;
    struct Pose {Vector position,forward,up,right,centre;Matrix world,inverse,rotation,inverse_rotation;};
    struct Motion {Vector velocity{{0,0,0,0}},angular_velocity{{0,0,0,0}},direction{{0,0,0,0}};float travel=0;};
    struct Candidate {Vector position,forward,up,right;};
    CarBodyIntegration(Frames& frames,Frame model,const CarPhysicsScalars& coefficients)
        :frames_(frames),model_(model),coefficients_(coefficients){
        if(!model)throw std::runtime_error("Car body integration requires its actual model");
        refresh();
        initial_position_=pose_.position; //004EA780 core160..16C, retained independently of moving pose240.
    }
    //0050DE60 publishes the candidate BEFORE its optional BODYCOL pass. A
    //containing collision controller must perform the native rollback/retry;
    //this endpoint deliberately provides no collision acceptance decision.
    void publish(const Candidate& candidate){frames_.car_body_pose(model_,candidate.position,candidate.forward,candidate.up);}
    void refresh(){ //004EA780 and00511CEA use the SAME live model world matrix.
        const auto world=frames_.actor_world_matrix(model_);pose_.world=world;pose_.inverse=frames_.car_inverse_matrix(world);
        pose_.rotation=world;pose_.inverse_rotation=pose_.inverse;
        for(unsigned axis=0;axis!=3;++axis){pose_.rotation[12+axis]=0;pose_.inverse_rotation[12+axis]=0;}
        pose_.right={{world[0],world[1],world[2],world[3]}};
        pose_.up={{world[4],world[5],world[6],world[7]}};
        pose_.forward={{world[8],world[9],world[10],world[11]}};
        pose_.position={{world[12],world[13],world[14],world[15]}};
        pose_.centre=frames_.car_transform({{coefficients_.get(0x1e0),coefficients_.get(0x1e4),coefficients_.get(0x1e8),0}},world);
    }
    const Pose& pose()const{return pose_;}Pose& pose(){return pose_;}
    void reset_pose(){ //004E9DB0, native160 is republished on reset.
        const auto world=frames_.actor_world_matrix(model_);pose_.world=world;pose_.inverse=frames_.car_inverse_matrix(world);
        pose_.rotation=world;pose_.inverse_rotation=pose_.inverse;
        for(unsigned a=0;a!=3;++a){pose_.rotation[12+a]=0;pose_.inverse_rotation[12+a]=0;}
        pose_.right={{world[0],world[1],world[2],world[3]}};pose_.up={{world[4],world[5],world[6],world[7]}};
        pose_.forward={{world[8],world[9],world[10],world[11]}};pose_.position={{world[12],world[13],world[14],world[15]}};
        const auto current=frames_.actor_world_matrix(model_);initial_position_={{current[12],current[13],current[14],current[15]}};
        pose_.centre=frames_.car_transform({{coefficients_.get(0x1e0),coefficients_.get(0x1e4),coefficients_.get(0x1e8),0}},world);
    }
    const Motion& motion()const{return motion_;}Motion& motion(){return motion_;}
    const Vector& initial_position()const{return initial_position_;}
    CarContactState contact_state()const{ //Actual current producer for004F32A0 and the loading solver.
        return {pose_.centre,motion_.angular_velocity,motion_.velocity,
            {{coefficients_.get(0x220),coefficients_.get(0x224),coefficients_.get(0x228),0}},
            pose_.rotation,pose_.inverse_rotation,coefficients_.get(0x918)};
    }
    Candidate predict(float elapsed,const Vector& force,const Vector& torque){
        const auto scale=static_cast<long double>(elapsed)/coefficients_.get(0x918);
        const float stored_scale=static_cast<float>(scale);
        for(unsigned axis=0;axis!=3;++axis){
            auto net=static_cast<long double>(force[axis])+coefficients_.get(0x200+axis*4);
            if(axis==2)net=static_cast<float>(net);
            auto acceleration=(axis?static_cast<long double>(stored_scale):scale)*net;
            if(axis==0)acceleration=static_cast<float>(acceleration);
            motion_.velocity[axis]=static_cast<float>(acceleration+motion_.velocity[axis]);
        }
        motion_.direction=normalize(motion_.velocity);
        auto angular=torque;frames_.car_transform_in_place(angular,pose_.inverse_rotation);
        for(unsigned axis=0;axis!=3;++axis)angular[axis]*=coefficients_.get(0x220+axis*4);
        frames_.car_transform_in_place(angular,pose_.rotation);
        for(unsigned axis=0;axis!=3;++axis){
            auto change=static_cast<long double>(angular[axis])*elapsed;if(axis==2)change=static_cast<float>(change);
            motion_.angular_velocity[axis]=static_cast<float>(change+motion_.angular_velocity[axis]);
        }
        Vector displacement{};
        for(unsigned axis=0;axis!=3;++axis)displacement[axis]=elapsed*motion_.velocity[axis];
        motion_.travel+=length(displacement);
        return advance_pose(elapsed,{pose_.position,pose_.forward,pose_.up,pose_.right},pose_.inverse_rotation,pose_.centre);
    }
    //00511075..00511B26 retries from the saved pose with POST-contact motion.
    //No second acceleration or8E0 travel accumulation occurs on this path.
    Candidate advance_pose(float elapsed,const Candidate& base,Matrix inverse,const Vector& centre){
        Vector displacement{},angles{};
        for(unsigned axis=0;axis!=3;++axis){displacement[axis]=elapsed*motion_.velocity[axis];angles[axis]=elapsed*motion_.angular_velocity[axis];inverse[12+axis]=0;}
        frames_.car_transform_in_place(angles,inverse);
        auto rotation=frames_.car_axis_quaternion(base.right,angles[0]);
        rotation=car_quaternion_product(rotation,frames_.car_axis_quaternion(base.up,angles[1]));
        rotation=car_quaternion_product(rotation,frames_.car_axis_quaternion(base.forward,angles[2]));
        const auto matrix=frames_.car_quaternion_matrix(rotation);
        Candidate candidate=base;
        frames_.car_transform_in_place(candidate.forward,matrix);frames_.car_transform_in_place(candidate.up,matrix);
        auto before=identity(),after=identity();
        for(unsigned axis=0;axis!=3;++axis){before[12+axis]=-centre[axis];after[12+axis]=centre[axis];}
        const auto pivot=frames_.car_matrix_product(frames_.car_matrix_product(before,matrix),after);
        candidate.right=cross(candidate.up,candidate.forward);
        candidate.up=cross(candidate.forward,candidate.right);
        candidate.right=normalize(candidate.right);candidate.up=normalize(candidate.up);candidate.forward=normalize(candidate.forward);
        frames_.car_transform_in_place(candidate.position,pivot);
        for(unsigned axis=0;axis!=3;++axis)candidate.position[axis]+=displacement[axis];
        return candidate;
    }
private:
    static Matrix identity(){Matrix result{};result[0]=result[5]=result[10]=result[15]=1;return result;}
    static Vector normalize(Vector value){ //0051031A/00510C29 spill the square before comparing, unlike00610240.
        const float square=static_cast<float>((static_cast<long double>(value[0])*value[0]+static_cast<long double>(value[1])*value[1])+static_cast<long double>(value[2])*value[2]);
        constexpr float epsilon=9.99999993922529e-09f;
        if(!std::isfinite(square))throw std::runtime_error("Non-finite Car body direction");
        if(std::abs(static_cast<long double>(square)-1)>=epsilon){
            if(square>=epsilon){const auto scale=1/std::sqrt(static_cast<long double>(square));for(unsigned axis=0;axis!=3;++axis)value[axis]=static_cast<float>(value[axis]*scale);}
            else{const auto axis=value[0]!=0?0u:value[2]!=0?2u:1u;value[axis]=value[axis]<0?-1.f:1.f;}
        }
        return value;
    }
    static Vector cross(const Vector& first,const Vector& second){
        Vector result{};for(unsigned axis=0;axis!=3;++axis){const auto next=(axis+1)%3,last=(axis+2)%3;result[axis]=static_cast<float>(static_cast<long double>(first[next])*second[last]-static_cast<long double>(second[next])*first[last]);}return result;
    }
    static float length(const Vector& value){
        const float square=static_cast<float>((static_cast<long double>(value[1])*value[1]+static_cast<long double>(value[2])*value[2])+static_cast<long double>(value[0])*value[0]);
        std::int32_t bits;std::memcpy(&bits,&square,4);
        if(bits<0x322bcc77)return 0;
        if(std::abs(square-1.f)<9.99999993922529e-09f)return 1;
        return static_cast<float>(std::sqrt(static_cast<long double>(square)));
    }
    Frames& frames_;Frame model_;const CarPhysicsScalars& coefficients_;Pose pose_;Motion motion_;Vector initial_position_{};
};
} //namespace hd2::game
#endif
