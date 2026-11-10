#ifndef HD2_GAME_PHYSICS_OBJECT_H
#define HD2_GAME_PHYSICS_OBJECT_H

#include "game/physics_object_parameters.h"
#include "game/physics_object_contacts.h"
#include "game/actor_frame_data.h"
#include "game/rigid_body.h"
#include "game/effects_frame_fades.h"
#include "game/physics_object_motion.h"
#include "game/physics_object_pivot.h"
#include "game/physics_object_tipping.h"
#include "frontend/cleanup.h"
#include <optional>

namespace hd2::game {

//0040B5E0/0040B7E0 common owner and00407390/004079F0 type5,
//0040F6A0 type1,0040E810 type2,0040DAA0 type3 and0040BD70 type4 loading.
//Retain the real frame, SAME userdata store and dynamic
//collision grid. Runtime queue pointers refer to THIS object, never an Item.
//Detached movement/contact/break producers must be supplied before using them.
template<class Frames,class Collisions,class RuntimeActors>
class PhysicsObject {
public:
    using Frame=typename Frames::Frame;
    using Matrix=typename Frames::Matrix;
    using Vector=typename Frames::Vector;
    using Parameters=PhysicsObjectParameters<Frame>;
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    using FrameOwner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    PhysicsObject(Frames& frames,Collisions& collisions,RuntimeActors& actors,const Parameters& parameters,PhysicsObjectHandle& handle,
                  EffectsFrameFades<Frames>* fades=nullptr)
        : frames_(frames),collisions_(collisions),actors_(actors),parameters_(parameters),handle_(handle),fades_(fades) {
        if(!parameters.frame)throw std::logic_error("Physics object requires a real frame");
        if(parameters.type!=0&&parameters.type!=1&&parameters.type!=2&&parameters.type!=3&&parameters.type!=4&&parameters.type!=5)
            throw std::runtime_error("Required physics object constructor is unavailable: "+std::to_string(parameters.type));
        if(parameters.type==4&&!fades_)throw std::runtime_error("Physics type4 requires the actual shared GEffects frame-fade owner");
        frames_.physics_link_root(parameters.frame);frames_.update_scene_object(parameters.frame);
        frame_=frames_.retain_actor_frame(parameters.frame);
        const auto initial=frames_.actor_world_matrix(frame_.get());
        motion_.position={{initial[12],initial[13],initial[14],initial[15]}};
        motion_.rotation=frames_.actor_world_rotation(frame_.get());
    }
    ~PhysicsObject() {close();}
    PhysicsObject(const PhysicsObject&)=delete;
    PhysicsObject& operator=(const PhysicsObject&)=delete;
    void initialize() { //Separate call allows rollback through the complete native owner.
        if(initialized_)throw std::logic_error("Physics object initialized twice");
        initialized_=true;
        if(parameters_.type==0)return; //0040E320 disables common collision registration.
        if(parameters_.type==4) { //0040BD70 has no collision descriptor registration.
            ground_stop_.enabled=frames_.physics_inverse_world_below_half(frame_.get());ground_stop_.cached=false;
            fades_->fade_out(frame_.get(),5000,false);return;
        }
        const auto bounds=frames_.physics_object_bounds(frame_.get(),parameters_.type==5);
        if((parameters_.type==1||parameters_.type==3)&&!(bounds.minimum[0]<bounds.maximum[0]))
            throw std::runtime_error("Physics aggregate bounds leave native transformed centre undefined");
        world_=frames_.actor_world_matrix(frame_.get());inverse_=frames_.car_inverse_matrix(world_);
        descriptor_.frame=frame_.get();descriptor_.minimum=bounds.minimum;descriptor_.maximum=bounds.maximum;
        descriptor_.matrix=&world_;descriptor_.inverse=&inverse_;
        if(parameters_.type==1||parameters_.type==2) {
            descriptor_.flags=0x42;descriptor_.mask=0;
            radius170_=parameters_.radius<0?bounds.radius:parameters_.radius;
            descriptor_.radius=radius170_;descriptor_.position=motion_.position;
            if(parameters_.type==1) {
                const auto scale=frames_.car_scale(frame_.get());
                const auto yz=scale[1]>scale[2]?scale[1]:scale[2];const auto largest=scale[0]>yz?scale[0]:yz;
                descriptor_.radius=static_cast<float>(static_cast<long double>(largest)*bounds.radius); //Native overrides descriptor radius, not170.
                descriptor_.position=frames_.car_transform(bounds.centre,world_);
            }
        } else if(parameters_.type==3) {
            descriptor_.flags=0x40;descriptor_.mask=2;descriptor_.radius=bounds.radius;
            descriptor_.position=frames_.car_transform(bounds.centre,world_);
        } else {
            descriptor_.flags=0x40|(parameters_.sight_pass?0x100000u:0u);
            descriptor_.mask=0x200000;descriptor_.radius=bounds.radius;
            descriptor_.position=frames_.car_transform(bounds.centre,world_);
            const auto high=frames_.car_transform(bounds.maximum,world_);
            const auto low=frames_.car_transform(bounds.minimum,world_);
            span180_={{static_cast<float>(static_cast<long double>(high[0])-low[0]),0,
                       static_cast<float>(static_cast<long double>(high[2])-low[2]),0}};
            centre170_[0]=static_cast<float>((static_cast<long double>(high[0])+low[0])*.5f);
            centre170_[1]=static_cast<float>((static_cast<long double>(low[1])+low[1])*.5f);
            const auto zsum=static_cast<float>(static_cast<long double>(high[2])+low[2]);
            centre170_[2]=static_cast<float>(static_cast<long double>(zsum)*.5f);
            endpoint190_=frames_.car_transform(high,inverse_);
            auto projected=low;projected[1]=high[1];endpoint1a0_=frames_.car_transform(projected,inverse_);
            tipping_.saved=frames_.actor_rotation(frame_.get());rotation1f0_=frames_.actor_rotation(frame_.get());rotation_ready_=true;
            pickup_=parameters_.pickup;
            flag161_=true;
            if(parameters_.pickup) {
                descriptor_.flags|=0x100000;
                actors_.queue_static_item(frame_.get(),this,descriptor_);queued_=true;
            }
        }
        collisions_.insert(descriptor_);collisions_.move(descriptor_);
        if(parameters_.type==3)initialize_principal_axes(); //0040DAA0 writes endpoints AFTER registration.
    }
    void publish() { //0040FB00 after construction and list/handle publication.
        auto& data=frames_.actor_frame_data(frame_.get());
        if(data.words[4]!=ActorFrameData::empty_text_token())throw std::runtime_error("Physics frame already has an object handle");
        data.words[4]=reinterpret_cast<std::uintptr_t>(&handle_);published_=true;
    }
    void close() noexcept {
        if(closed_)return;
        closed_=true;
        if(queued_) {actors_.remove_static_item(this);queued_=false;}
        frontend::CleanupErrors errors;
        if(frame_&&rotation_ready_) {
            errors.attempt([&]{frames_.object_rotation(frame_.get(),rotation1f0_);});
            errors.attempt([&]{frames_.update_scene_object(frame_.get());});
        }
        collisions_.remove_for_cleanup(descriptor_);
        errors.attempt([&]{(void)feedback(3);}); //0040B816 ECX3, SAME handle, before EC retraction/frame release.
        if(frame_) {
            errors.attempt([&]{unpublish_handle();});
        }
        handle_.index=-1;frame_.reset();
        try {errors.rethrow();}catch(...) {frames_.record_actor_cleanup_failure(std::current_exception());}
    }
    void unpublish_handle(bool reclaim_empty=true){
        if(!frame_)return;
        if(published_)if(const auto* data=frames_.find_actor_frame_data(frame_.get())) {
            if(data->words[4]==reinterpret_cast<std::uintptr_t>(&handle_)) {
                frames_.actor_frame_data(frame_.get()).words[4]=ActorFrameData::empty_text_token();
            }
        }
        published_=false;
        if(reclaim_empty)frames_.remove_unused_actor_frame_data(frame_.get()); //After event3; native release keeps the EC block through that callback.
    }
    Frame frame() const {return frame_.get();}
    Descriptor& descriptor() {return descriptor_;}
    const Parameters& parameters() const {return parameters_;}
    PhysicsObjectHandle& handle(){return handle_;}
    PhysicsObjectMotion& motion(){return motion_;}
    void install_feedback(){feedback_installed_=true;} //0040FB00 installs08/C only after complete derived construction.
    std::optional<std::int32_t> feedback(unsigned event){
        if(feedback_installed_&&parameters_.feedback)return parameters_.feedback(event,handle_);
        return std::nullopt; //Actual absent14C branch, not a replacement callback.
    }
    bool delete_when_stopped()const{return flag158_;}
    void impulse(const Vector& force,const Vector& point,float scale){
        if(parameters_.type==5)return; //004094D0 is genuine RET12, absent frozen/no credit. Manager still emits event2.
        struct Binding {
            PhysicsObject& object;Matrix inverse{};
            bool physics_frame_exists()const{return bool(object.frame_);}
            void physics_begin_inverse(){inverse=object.frames_.car_inverse_matrix(object.frames_.actor_world_matrix(object.frame_.get()));}
            Vector physics_inverse_point(const Vector& point){return object.frames_.car_transform(point,inverse);}
            Vector physics_world_position(){const auto world=object.frames_.actor_world_matrix(object.frame_.get());return {{world[12],world[13],world[14],world[15]}};}
        } binding{*this,{}};
        impulse_physics_object(motion_,force,point,scale,parameters_.type==2,binding);
    }
    template<class Simulation>bool advance(float seconds,Simulation& simulation){
        struct Binding {
            PhysicsObject& object;Simulation& simulation;
            bool physics_frame_exists()const{return bool(object.frame_);}
            Vector physics_world_position(){const auto world=object.frames_.actor_world_matrix(object.frame_.get());return {{world[12],world[13],world[14],world[15]}};}
            auto physics_rotate_quaternion(const Vector& rotation,const Vector& angles){return object.frames_.physics_rotate_quaternion(rotation,angles);}
            auto physics_point_query(const Vector& start,const Vector& delta){return simulation.physics_point_query(start,delta);}
            auto physics_sphere_query(const Vector& start,const Vector& delta,float radius){return simulation.physics_sphere_query(start,delta,radius);}
            void physics_collision_feedback(){(void)object.feedback(1);} //0040B5BB ECX1/EDX150/stack154.
            Vector physics_axis_world(unsigned n){return object.frames_.car_transform(object.principal_axes_.at(n),object.frames_.actor_world_matrix(object.frame_.get()));}
            Vector physics_frame_rotation(){return object.frames_.actor_rotation(object.frame_.get());}
            void physics_publish_position(const Vector& position){object.frames_.physics_publish_position(object.frame_.get(),position);}
            void physics_publish_rotation(const Vector& rotation){object.frames_.object_rotation(object.frame_.get(),rotation);object.frames_.update_scene_object(object.frame_.get());}
            void physics_restore_pose(const Vector& position,const Vector& rotation){object.frames_.physics_restore_pose(object.frame_.get(),position,rotation);}
            void physics_publish_axis_rotation(const Vector& rotation,const Vector& axis,float angle){object.frames_.physics_publish_axis_rotation(object.frame_.get(),rotation,axis,angle);}
            Vector physics_rotate_offset(const Vector& offset,const Vector& axis,float angle){return object.frames_.physics_rotate_offset(offset,axis,angle);}
            auto physics_pivot_query(const Vector& start,const Vector& delta){return simulation.physics_pivot_query(start,delta);}
            Vector physics_tipping_endpoint(unsigned n){return object.frames_.car_transform(n?object.endpoint1a0_:object.endpoint190_,object.frames_.actor_world_matrix(object.frame_.get()));}
            auto physics_tipping_query(const Vector& start,const Vector& delta){return simulation.physics_tipping_query(start,delta);}
            Vector physics_slerp(const Vector& first,const Vector& last,float factor,bool shortest){return object.frames_.physics_slerp(first,last,factor,shortest);}
            void physics_exclude_self(){simulation.physics_exclude(object.descriptor_);}
            void physics_reset_exclusions(){simulation.physics_reset_exclusions();}
            void physics_publish_extended_position(const Vector& position){
                object.frames_.physics_publish_position(object.frame_.get(),position);
                object.world_=object.frames_.actor_world_matrix(object.frame_.get());
                object.inverse_=object.frames_.car_inverse_matrix(object.world_);
                object.descriptor_.position=object.frames_.physics_local_translation(object.frame_.get());
                object.collisions_.move(object.descriptor_);
            }
        } binding{*this,simulation};
        if(parameters_.type==5)return advance_tipping_physics_object(motion_,tipping_,seconds,binding);
        if(parameters_.type==3)return advance_aggregate_physics_object(motion_,pivot_,seconds,binding);
        if(parameters_.type==1)return advance_extended_ray_physics_object(motion_,seconds,radius170_,parameters_.flag15,binding);
        if(parameters_.type==4)return advance_fading_physics_object(motion_,ground_stop_,flag158_,seconds,parameters_.flag15,binding);
        return advance_basic_physics_object(motion_,seconds,parameters_.type==2,radius170_,parameters_.flag15,binding);
    }
    void synchronize_pose(){ //0040AB80, ALWAYS called after the virtual04 result, including false.
        if(!frame_)return;
        frames_.physics_publish_pose(frame_.get(),motion_.position,motion_.rotation);
        if(parameters_.type==0||parameters_.type==4||!collision_enabled_||flag161_)return;
        world_=frames_.actor_world_matrix(frame_.get());inverse_=frames_.car_inverse_matrix(world_);
        descriptor_.position=frames_.car_transform(frames_.physics_local_sphere_centre(frame_.get()),world_);
        collisions_.move(descriptor_);
    }
    bool contact(unsigned kind,const Vector& velocity,const Vector&) {
        if(parameters_.type==2)return true; //0040EB00, genuine AL1/RET12.
        if(parameters_.type!=5)return false; //0040B790, genuine AL0/RET12, types0/1/3/4.
        if(tipping_.active&&kind!=3)return false;
        if(kind==3){
            if(!pickup_)return true;
            pickup_=false; //Native204 clears BEFORE required pickup notification.
            throw std::runtime_error("Physics type5 contact requires actual00407BD0 pickup notification");
        }
        if(tipping_.progress!=0)return false;
        contact1b0_=velocity;
        if(!frame_)return false;
        if(!parameters_.free_axis){
            const auto matrix=frames_.actor_world_matrix(frame_.get());span180_={{matrix[0],matrix[1],matrix[2],matrix[3]}};
            if(static_cast<long double>(velocity[2])*span180_[0]-static_cast<long double>(span180_[2])*velocity[0]>0)
                physics_object_pivot_math::damp(span180_,-1.f);
        }else span180_=physics_object_pivot_math::cross({{0,-1,0,0}},velocity);
        physics_object_motion_math::normalize(span180_);
        tipping_.saved=frames_.actor_rotation(frame_.get());tipping_.progress=0;tipping_.active=true;tipping_.amplitude=1;
        if(kind==0){
            if(!parameters_.tree){flag161_=true;tipping_.angle=.0172787606716156f;tipping_.oscillating=true;tipping_.speed=12;tipping_.damping=.6000000238418579f;tipping_.phase=1;}
            else tipping_.active=false; //202 remains exactly as it was, including unwritten.
        }else{
            tipping_.oscillating=false;flag161_=false;tipping_.angle=parameters_.angle;
            if(parameters_.angle>1.2566370964050293f&&collision_enabled_){collision_enabled_=false;collisions_.remove(descriptor_);} //Native160 changes BEFORE removal.
            tipping_.speed=1;tipping_.damping=.8999999761581421f;
            const auto parent=parameters_.tree?frames_.actor_parent(frame_.get()):Frame{};
            if(parent&&frames_.frame_kind(parent)==9)throw std::runtime_error("Physics tree tipping requires actual00408780 model replacement/proximity/cache receivers");
        }
        tipping_.positive=tipping_.negative=tipping_.saved;
        tipping_.positive=frames_.physics_rotate_about_axis(*tipping_.positive,span180_,tipping_.angle);
        tipping_.negative=frames_.physics_rotate_about_axis(*tipping_.negative,span180_,-tipping_.angle);
        return true;
    }
    const Vector& centre170() const {return centre170_;}
    const Vector& span180() const {return span180_;}
    const auto& principal_axes()const{return principal_axes_;} //Type3 native170/180/190/1A0, XYZ only.
    bool inverse_flag170()const{return ground_stop_.enabled;} //Type4 native inverse-matrix+2C comparison.
private:
    void initialize_principal_axes() {
        const auto& low=descriptor_.minimum;const auto& high=descriptor_.maximum;Vector centre{};
        std::array<long double,3> extent{};
        for(unsigned axis=0;axis!=3;++axis) {
            const auto span=std::abs(static_cast<long double>(high[axis])-low[axis]);
            extent[axis]=axis==1?span:static_cast<long double>(static_cast<float>(span)); //Native Y remains in x87 for comparisons.
            centre[axis]=static_cast<float>((static_cast<long double>(low[axis])+high[axis])*.5f);
        }
        unsigned first,second;
        if(extent[0]>=extent[1]&&extent[0]>=extent[2]){first=0;second=extent[1]<extent[2]?2:1;}
        else if(extent[1]>=extent[0]&&extent[1]>=extent[2]){first=1;second=extent[0]<extent[2]?2:0;}
        else {first=2;second=extent[1]<extent[0]?0:1;} //Native Z-first secondary tie prefers Y.
        for(unsigned pair=0;pair!=2;++pair) {
            const auto axis=pair?second:first;auto& minimum=principal_axes_[pair*2];auto& maximum=principal_axes_[pair*2+1];
            maximum=minimum=centre;maximum[axis]=high[axis];minimum[axis]=low[axis];
        }
    }
    Frames& frames_;Collisions& collisions_;RuntimeActors& actors_;Parameters parameters_;PhysicsObjectHandle& handle_;
    EffectsFrameFades<Frames>* fades_;
    FrameOwner frame_{nullptr,nullptr};Descriptor descriptor_{};Matrix world_{},inverse_{};
    PhysicsObjectMotion motion_;
    PhysicsObjectGroundStop ground_stop_;
    PhysicsObjectPivot pivot_;
    PhysicsObjectTipping tipping_;
    Vector centre170_{},span180_{},endpoint190_{},endpoint1a0_{};
    Vector contact1b0_{};std::array<float,4> rotation1f0_{};
    std::array<Vector,4> principal_axes_{};
    float radius170_=0;
    bool initialized_=false,published_=false,queued_=false,closed_=false,feedback_installed_=false,flag158_=false,flag161_=false,rotation_ready_=false,collision_enabled_=true,pickup_=false;
};

} //namespace hd2::game
#endif
