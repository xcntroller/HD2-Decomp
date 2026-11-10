#ifndef HD2_GAME_ACTOR_BASE_H
#define HD2_GAME_ACTOR_BASE_H

#include "game/actor_storage_owner.h"
#include "game/actor_activation_math.h"
#include <optional>
#include <stdexcept>
#include <string>

namespace hd2::game {

//Native common actor state00610070/00610200/00610170. Derived owners still
//perform their real geometry initialization, properties and callbacks. This
//class neither registers an actor nor implies that actor loading has succeeded.
//Backend must retain/release actual frames, release registered entries, and
//remove the actor from every live script context (004CFF70) without throwing.
template<class Backend>
class ActorBase {
public:
    using Frame=typename Backend::Frame;
    using FrameOwner=decltype(std::declval<Backend&>().retain_actor_frame(std::declval<Frame>()));
    using Storage=ActorStorageOwner<Backend>;
    ActorBase(Backend& backend,std::uint32_t type) : backend_(backend),type_(type) {
        static_assert(noexcept(std::declval<Backend&>().clear_actor_references(std::declval<ActorBase*>())),
                      "Actor reference cleanup must not throw");
    }
    ~ActorBase() {
        storage_.reset();field7c_=0;parent_.reset();frame_.reset(); //00610170, after derived cleanup.
        backend_.clear_actor_references(this);
    }
    ActorBase(const ActorBase&)=delete;
    ActorBase& operator=(const ActorBase&)=delete;
    bool initialize_frame(Frame value,HumanStorageScalars scalars={}) { //00610200/+08.
        if(!value)return false;
        if(frame_)throw std::logic_error("Actor frame initialization repeated");
        frame_.emplace(backend_.retain_actor_frame(value));
        storage_=std::make_unique<Storage>(backend_,type_,scalars);
        return true;
    }
    Frame frame() const {return frame_?frame_->get():Frame{};}
    void release_runtime_frame() {
        if(!registered_)throw std::logic_error("Only a registered runtime actor may release its frame before deactivation");
        frame_.reset(); //005B6920 releases20 before the registered00610510 branch.
    }
    Storage* storage() {return storage_.get();}
    const Storage* storage() const {return storage_.get();}
    //Human0059E3E0 invokes its005A2A40 before freeing the second control owner.
    void clear_storage() noexcept {storage_.reset();}
    //00610240 /00610510. Services are the actual Scene2/runtime registries
    //and current mode owner; neither is replaced with a default successful call.
    template<class Services>void activate_frame(bool authority,Services& services) {
        if(!frame())throw std::logic_error("Actor activation requires its initialized frame");
        auto matrix=backend_.actor_world_matrix(frame());
        direction_=actor_activation_direction({matrix[8],matrix[9],matrix[10],matrix[11]});
        matrix=backend_.actor_world_matrix(frame());
        position_={matrix[12],matrix[13],matrix[14],matrix[15]};
        if(!registered_) {
            if(!field70()) {
                if(auto parent=backend_.actor_parent(frame()))parent_.emplace(backend_.retain_actor_frame(parent));
                local_position_=backend_.actor_position(frame());
                local_rotation_=backend_.actor_rotation(frame());
            }
            if(authority||!(services.mode_flags()&1))services.set_scene_actor_active(*this,true);
        } else if(authority||!(services.mode_flags()&1))services.set_runtime_actor_active(*this,true);
        backend_.auto_parent_object(frame());
    }
    template<class Services>void deactivate_frame(bool authority,Services& services) {
        field7c_=0; //00611080(2), like00611080(0), is a genuine RET4.
        if(registered_) {
            if(authority||!(services.mode_flags()&1))services.set_runtime_actor_active(*this,false);
            return;
        }
        if(!frame())throw std::logic_error("Scene actor deactivation requires its initialized frame");
        if(authority||!(services.mode_flags()&1))services.set_scene_actor_active(*this,false);
        backend_.object_visible(frame(),true);
        if(!field70())return;
        const auto parent_name=backend_.frame_name(field70());
        backend_.link_scene_object(frame(),field70());backend_.update_scene_object(frame());
        parent_.reset();
        if(!backend_.actor_parent(frame())) {
            if(auto replacement=backend_.find_scene_frame(parent_name,0xffff)) {
                backend_.link_scene_object(frame(),replacement);backend_.update_scene_object(frame());
            }
        }
        backend_.object_position(frame(),{local_position_[0],local_position_[1],local_position_[2]});
        backend_.object_rotation(frame(),local_rotation_);backend_.update_scene_object(frame());
    }
    const std::array<float,4>& world_position() const {return position_;}
    void world_position(const std::array<float,4>& value){position_=value;} //Human0075C4F0 writes its actual base30 slot.
    const std::array<float,4>& world_direction() const {return direction_;}
    void world_direction(const std::array<float,4>& value){direction_=value;} //Car004D38F0 copies its native core240 into base40.
    std::uint32_t type() const {return type_;}
    std::int32_t id10() const {return id10_;}
    std::int32_t id14() const {return id14_;}
    void id10(std::int32_t value) {id10_=value;}
    void id14(std::int32_t value) {id14_=value;}
    bool registered() const {return registered_;} //Original byte1C: runtime actor, versus ordinary Scene2 actor.
    void registered(bool value) {registered_=value;}
    const std::string& name() const {return name_;}
    void name(std::string value) {name_=std::move(value);}
    Frame field70() const {return parent_?parent_->get():Frame{};}
    std::uint32_t field7c() const {return field7c_;}
    void field7c(std::uint32_t value) {field7c_=value;}
private:
    Backend& backend_;
    std::uint32_t type_;
    std::int32_t id10_=-1,id14_=-1;
    bool registered_=false;
    std::optional<FrameOwner> frame_;
    std::optional<FrameOwner> parent_;
    std::unique_ptr<Storage> storage_;
    std::array<float,4> position_{},direction_{},local_position_{},local_rotation_{};
    std::uint32_t field7c_=0;
    std::string name_="Missing actor name!!";
};

} //namespace hd2::game
#endif
