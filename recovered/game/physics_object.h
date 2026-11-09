#ifndef HD2_GAME_PHYSICS_OBJECT_H
#define HD2_GAME_PHYSICS_OBJECT_H

#include "game/physics_object_parameters.h"
#include "game/actor_frame_data.h"
#include "game/rigid_body.h"
#include "frontend/cleanup.h"
#include <optional>

namespace hd2::game {

struct PhysicsObjectHandle {std::int32_t index=-1;bool first_list=false;};

//0040B5E0/0040B7E0 common owner and00407390/004079F0 type5,
//0040E810 type2 loading. Retain the real frame, SAME userdata store and dynamic
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
    PhysicsObject(Frames& frames,Collisions& collisions,RuntimeActors& actors,const Parameters& parameters,PhysicsObjectHandle& handle)
        : frames_(frames),collisions_(collisions),actors_(actors),parameters_(parameters),handle_(handle) {
        if(!parameters.frame)throw std::logic_error("Physics object requires a real frame");
        if(parameters.type!=0&&parameters.type!=2&&parameters.type!=5)
            throw std::runtime_error("Required physics object constructor is unavailable: "+std::to_string(parameters.type));
        frames_.physics_link_root(parameters.frame);frames_.update_scene_object(parameters.frame);
        frame_=frames_.retain_actor_frame(parameters.frame);
        const auto initial=frames_.actor_world_matrix(frame_.get());
        position30_={{initial[12],initial[13],initial[14],initial[15]}};
        rotation40_=frames_.actor_world_rotation(frame_.get());
    }
    ~PhysicsObject() {close();}
    PhysicsObject(const PhysicsObject&)=delete;
    PhysicsObject& operator=(const PhysicsObject&)=delete;
    void initialize() { //Separate call allows rollback through the complete native owner.
        if(initialized_)throw std::logic_error("Physics object initialized twice");
        initialized_=true;
        if(parameters_.type==0)return; //0040E320 disables common collision registration.
        const auto bounds=frames_.physics_object_bounds(frame_.get(),parameters_.type==5);
        world_=frames_.actor_world_matrix(frame_.get());inverse_=frames_.car_inverse_matrix(world_);
        descriptor_.frame=frame_.get();descriptor_.minimum=bounds.minimum;descriptor_.maximum=bounds.maximum;
        descriptor_.matrix=&world_;descriptor_.inverse=&inverse_;
        if(parameters_.type==2) {
            descriptor_.flags=0x42;descriptor_.mask=0;
            radius170_=parameters_.radius<0?bounds.radius:parameters_.radius;
            descriptor_.radius=radius170_;descriptor_.position=position30_;
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
            rotation1e0_=frames_.actor_rotation(frame_.get());rotation1f0_=frames_.actor_rotation(frame_.get());rotation_ready_=true;
            flag161_=true;
            if(parameters_.pickup) {
                descriptor_.flags|=0x100000;
                actors_.queue_static_item(frame_.get(),this,descriptor_);queued_=true;
            }
        }
        collisions_.insert(descriptor_);collisions_.move(descriptor_);
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
        collisions_.remove_for_cleanup(descriptor_);
        if(frame_) {
            frontend::CleanupErrors errors;
            if(rotation_ready_) {
                errors.attempt([&]{frames_.object_rotation(frame_.get(),rotation1f0_);});
                errors.attempt([&]{frames_.update_scene_object(frame_.get());});
            }
            errors.attempt([&]{if(published_) {
                //No allocation during cleanup; this is the SAME store's block.
                if(const auto* data=frames_.find_actor_frame_data(frame_.get())) {
                    if(data->words[4]==reinterpret_cast<std::uintptr_t>(&handle_)) {
                        auto& writable=frames_.actor_frame_data(frame_.get());writable.words[4]=ActorFrameData::empty_text_token();
                        frames_.remove_unused_actor_frame_data(frame_.get());
                    }
                }
                published_=false;
            }});
            try {errors.rethrow();}catch(...) {frames_.record_actor_cleanup_failure(std::current_exception());}
        }
        handle_.index=-1;frame_.reset();
    }
    Frame frame() const {return frame_.get();}
    Descriptor& descriptor() {return descriptor_;}
    const Parameters& parameters() const {return parameters_;}
    const Vector& centre170() const {return centre170_;}
    const Vector& span180() const {return span180_;}
private:
    Frames& frames_;Collisions& collisions_;RuntimeActors& actors_;Parameters parameters_;PhysicsObjectHandle& handle_;
    FrameOwner frame_{nullptr,nullptr};Descriptor descriptor_{};Matrix world_{},inverse_{};
    Vector position30_{},centre170_{},span180_{},endpoint190_{},endpoint1a0_{};
    std::array<float,4> rotation40_{},rotation1e0_{},rotation1f0_{};
    float radius170_=0;
    bool initialized_=false,published_=false,queued_=false,closed_=false,flag161_=false,rotation_ready_=false;
};

} //namespace hd2::game
#endif
