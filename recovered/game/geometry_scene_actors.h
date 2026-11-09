#ifndef HD2_GAME_GEOMETRY_SCENE_ACTORS_H
#define HD2_GAME_GEOMETRY_SCENE_ACTORS_H

#include "game/scene_actor.h"
#include "game/actor_initial_properties.h"
#include "game/ladder_geometry.h"
#include "game/door_portal_binding.h"
#include "game/sound_emission.h"
#include "game/switch_actor_state.h"
#include "game/rigid_body.h"
#include "frontend/cleanup.h"
#include <functional>
#include <optional>

namespace hd2::game {

//Concrete loading and bounded activation owners. Interaction/simulation and
//the remaining Door activation receivers are separate required consumers.
template<class Backend>
class LadderSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    LadderSceneActor(Backend& backend,std::string localized_name) : SceneActor<Backend>(backend,8),
        properties(initial_ladder_properties()) {this->base_.name(std::move(localized_name));} //005EA3D0.
    bool initialize(Frame frame) override { //005EA5D0, geometry precedes common retain.
        if(!frame)return false;
        if(geometry)throw std::logic_error("Ladder frame initialization repeated");
        geometry.emplace(this->backend_,frame);
        return this->base_.initialize_frame(frame);
    }
    void load(SerializedBytes& bytes) override {properties.load(bytes);} //005EBBD0.
    ~LadderSceneActor() override {if(remove_collisions_)remove_collisions_();}
    template<class Activation,class Collisions>void activate(bool authority,Activation& activation,Collisions& collisions) { //005EAD20.
        const auto frame=this->base_.frame();
        if(!frame)throw std::logic_error("Ladder activation requires its initialized frame");
        const auto parent=this->backend_.actor_parent(frame); //Snapshot BEFORE common auto-parenting.
        this->base_.activate_frame(authority,activation);
        this->backend_.actor_link_preserving_world(frame,parent);
        if(flag90)return;
        if(!geometry)throw std::logic_error("Ladder activation requires its actual marker geometry");
        if(collision_owner_&&collision_owner_!=&collisions)throw std::logic_error("Ladder collision owner changed");
        static_assert(noexcept(collisions.remove_for_cleanup(std::declval<Descriptor&>())),"Ladder collision teardown must not throw");
        if(!collision_owner_) {
            remove_collisions_=[this,&collisions]() noexcept {
                for(auto& descriptor:descriptors_)if(descriptor)collisions.remove_for_cleanup(*descriptor);
            };
            collision_owner_=&collisions;
        }
        flag90=true;
        auto bounds=this->backend_.actor_bounds(frame);
        const auto scale=this->backend_.actor_local_scale(frame);
        for(unsigned axis=0;axis!=3;++axis) {
            if(!std::isfinite(scale[axis])||scale[axis]==0)throw std::runtime_error("Ladder collision requires a finite nonzero local scale");
            bounds.minimum[axis]=static_cast<float>(static_cast<long double>(bounds.minimum[axis])*scale[axis]);
            bounds.maximum[axis]=static_cast<float>(static_cast<long double>(bounds.maximum[axis])*scale[axis]);
        }
        world_=this->backend_.actor_world_matrix(frame);
        for(unsigned axis=0;axis!=3;++axis) {
            const auto reciprocal=1.L/scale[axis];
            for(unsigned component=0;component!=3;++component)
                world_[axis*4+component]=static_cast<float>(reciprocal*world_[axis*4+component]);
        }
        inverse_=this->backend_.car_inverse_matrix(world_);
        typename Backend::Vector upper{{0,bounds.maximum[1],0,0}};
        upper=this->backend_.car_transform(upper,world_);
        bounds.maximum[1]=static_cast<float>(((static_cast<long double>(geometry->up_position()[1])-upper[1])+bounds.maximum[1])+.300000011920928955078125f);
        descriptors_[0]=std::make_unique<Descriptor>();
        auto& primary=*descriptors_[0];
        prepare(primary,bounds,frame);
        primary.radius=static_cast<float>((static_cast<long double>(bounds.maximum[1])-bounds.minimum[1])+.300000011920928955078125f);
        collisions.insert(primary); //Original inserts both, without006EF7F0.
        const auto upper_local=this->backend_.car_transform(geometry->up_position(),inverse_);
        bounds.minimum[1]=static_cast<float>(static_cast<long double>(upper_local[1])-.0074999998323619365692138671875f);
        bounds.minimum[2]=static_cast<float>(static_cast<long double>(bounds.minimum[2])-.100000001490116119384765625f);
        bounds.maximum[2]=static_cast<float>(static_cast<long double>(bounds.minimum[2])+.25f);
        descriptors_[1]=std::make_unique<Descriptor>();prepare(*descriptors_[1],bounds,frame);
        descriptors_[1]->radius=primary.radius;collisions.insert(*descriptors_[1]);
    }
    template<class Activation,class Collisions>void deactivate(bool authority,Activation& activation,Collisions& collisions) { //005EB120.
        frontend::CleanupErrors errors;
        errors.attempt([&]{this->base_.deactivate_frame(authority,activation);});
        if(flag90) {
            flag90=false;
            for(auto& descriptor:descriptors_)if(descriptor) {
                errors.attempt([&]{collisions.remove(*descriptor);});
                //Even a failed deferred-bucket allocation must not leave its
                //descriptor published after matching native destruction.
                if(descriptor->bucket)remove_collisions_();
                descriptor.reset();
            }
            field164.clear();
        }
        errors.rethrow();
    }
    const Descriptor* collision_descriptor(unsigned index) const {return descriptors_.at(index).get();}
    LadderActorProperties properties;
    std::optional<LadderGeometry<Backend>> geometry;
    std::vector<std::uint32_t> field164;
    bool flag90=false;
private:
    template<class Bounds>void prepare(Descriptor& descriptor,const Bounds& bounds,Frame frame) {
        descriptor.flags=0x40;descriptor.mask=0x4000;descriptor.actor=this;descriptor.frame=frame;
        const auto world=this->backend_.actor_world_matrix(frame);
        descriptor.position={{world[12],world[13],world[14],world[15]}};
        descriptor.matrix=&world_;descriptor.inverse=&inverse_;
        descriptor.minimum=bounds.minimum;descriptor.maximum=bounds.maximum;
    }
    typename Backend::Matrix world_{},inverse_{};
    std::array<std::unique_ptr<Descriptor>,2> descriptors_;
    std::function<void()> remove_collisions_;
    void* collision_owner_=nullptr;
};

template<class Backend>
class DoorSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    explicit DoorSceneActor(Backend& backend) : SceneActor<Backend>(backend,6),
        properties(initial_door_properties()) {} //005FF210.
    bool initialize(Frame frame) override {return this->base_.initialize_frame(frame);} //005FF3D0.
    void load(SerializedBytes& bytes) override { //00601AC0, including real00600A50.
        properties.load(bytes,[&] {portal.refresh(this->backend_,this->base_.frame(),properties.flaga0!=0);});
    }
    DoorActorProperties properties;
    DoorPortalBinding<Backend> portal;
    SoundEmission sound;
    std::array<std::uint32_t,24> fields130{};
    std::string field180;
    bool flag90=false;
    std::uint32_t field94=0,field9c=0,field11c=0,field18c=0,field1a0=0,field264=0,field268=0;
};

template<class Backend,class Runtime>
class SwitchSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    SwitchSceneActor(Backend& backend,Runtime& runtime,std::string localized_name)
        : SceneActor<Backend>(backend,13),state(backend),runtime_(runtime) {
        this->base_.name(std::move(localized_name)); //00613450, name1BF3.
    }
    bool initialize(Frame frame) override { //00613550.
        return this->base_.initialize_frame(frame)&&state.initialize(frame,runtime_);
    }
    void load(SerializedBytes& bytes) override {state.load(bytes,runtime_);} //00613E10.
    ~SwitchSceneActor() override {if(remove_collision_)remove_collision_();}
    template<class Activation,class Collisions>void activate(bool authority,Activation& activation,Collisions& collisions) { //00613920.
        this->base_.activate_frame(authority,activation);
        if(!authority&&(activation.mode_flags()&1))return;
        const auto frame=this->base_.frame();if(!frame)return;
        if(collision_owner_&&collision_owner_!=&collisions)throw std::logic_error("Switch collision owner changed");
        static_assert(noexcept(collisions.remove_for_cleanup(descriptor_)),"Switch collision teardown must not throw");
        if(!collision_owner_) {
            remove_collision_=[this,&collisions]() noexcept {collisions.remove_for_cleanup(descriptor_);};
            collision_owner_=&collisions;
        }
        //Original overwritesF0; retract a repeated/partially registered native
        //descriptor first so an exception never strands its borrowed address.
        if(descriptor_.bucket)collisions.remove(descriptor_);
        const auto bounds=this->backend_.physics_object_bounds(frame,false);
        if(!(bounds.minimum[0]<bounds.maximum[0]))throw std::runtime_error("Switch collision centre is unwritten for invalid bounds");
        descriptor_=Descriptor{};descriptor_.flags=0x40;descriptor_.mask=1;descriptor_.frame=frame;
        descriptor_.minimum=bounds.minimum;descriptor_.maximum=bounds.maximum;
        descriptor_.matrix=this->backend_.actor_world_matrix_pointer(frame);
        inverse_=this->backend_.car_inverse_matrix(this->backend_.actor_world_matrix(frame));descriptor_.inverse=&inverse_;
        const auto scale=this->backend_.car_scale(frame);
        const auto yz=scale[1]>scale[2]?scale[1]:scale[2];
        descriptor_.radius=static_cast<float>(static_cast<long double>(scale[0]>yz?scale[0]:yz)*bounds.radius);
        descriptor_.position=this->backend_.car_transform(bounds.centre,this->backend_.actor_world_matrix(frame));
        descriptor_.actor=this;
        collisions.insert(descriptor_);collisions.move(descriptor_);state.begin_activation();
    }
    template<class Activation,class Collisions>void deactivate(bool authority,Activation& activation,Collisions& collisions) { //00613BD0.
        frontend::CleanupErrors errors;
        errors.attempt([&]{this->base_.deactivate_frame(authority,activation);});
        errors.attempt([&]{collisions.remove(descriptor_);});
        if(authority)errors.attempt([&]{state.restore_activation(runtime_);});
        errors.rethrow();
    }
    const Descriptor& collision_descriptor() const {return descriptor_;}
    SwitchActorState<Backend> state;
private:
    Runtime& runtime_;
    typename Backend::Matrix inverse_{};
    Descriptor descriptor_{};
    std::function<void()> remove_collision_;
    void* collision_owner_=nullptr;
};

} //namespace hd2::game
#endif
