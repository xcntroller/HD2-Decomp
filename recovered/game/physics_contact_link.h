#ifndef HD2_GAME_PHYSICS_CONTACT_LINK_H
#define HD2_GAME_PHYSICS_CONTACT_LINK_H
#include "game/physics_object_parameters.h"

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <utility>

namespace hd2::game {

//Checked source borrower for the Car/physics construction cycle. The scene's
//Car owner must exist before Scene2 parsing, while module2 needs the loaded
//collision grid. Bind once module2 exists and unbind before it retires. This
//owns no handles/objects/grid and supplies no successful unbound operation.
template<class Frames>class PhysicsContactLink {
public:
    using Vector=typename Frames::Vector;
    explicit PhysicsContactLink(Frames& frames):frames_(frames){}
    PhysicsContactLink(const PhysicsContactLink&)=delete;PhysicsContactLink& operator=(const PhysicsContactLink&)=delete;
    template<class Physics>void bind(Physics& physics){
        if(physics_||&physics.frames()!=&frames_)throw std::logic_error("Physics contact requires its SAME live frame/module owner");
        std::function<void(std::uintptr_t,unsigned,const Vector&,const Vector&)> next=[&physics](auto handle,auto kind,const auto& velocity,const auto& point){(void)physics.contact(handle,kind,velocity,point);};
        std::function<PhysicsObjectHandle*(const PhysicsObjectParameters<typename Frames::Frame>&)> create=[&physics](const auto& parameters){return physics.create(parameters);};
        std::function<void(PhysicsObjectHandle*,const Vector&,const Vector&,float)> impulse=[&physics](auto* handle,const auto& force,const auto& point,float scale){physics.impulse(handle,force,point,scale);};
        receiver_=std::move(next);create_=std::move(create);impulse_=std::move(impulse);physics_=&physics;
    }
    template<class Physics>void unbind(Physics& physics)noexcept{if(physics_==&physics){receiver_={};create_={};impulse_={};physics_=nullptr;}}
    void contact(std::uintptr_t handle,unsigned kind,const Vector& velocity,const Vector& point){
        if(!physics_)throw std::logic_error("Physics contact has no actual containing module");
        receiver_(handle,kind,velocity,point);
    }
    bool bound()const noexcept{return physics_!=nullptr;}
    PhysicsObjectHandle* create(const PhysicsObjectParameters<typename Frames::Frame>& parameters){
        if(!physics_)throw std::logic_error("Fragment construction requires actual containing Physics module");
        return create_(parameters);
    }
    void impulse(PhysicsObjectHandle* handle,const Vector& force,const Vector& point,float scale){
        if(!physics_)throw std::logic_error("Fragment impulse requires actual containing Physics module");
        impulse_(handle,force,point,scale);
    }
private:
    Frames& frames_;const void* physics_=nullptr;std::function<void(std::uintptr_t,unsigned,const Vector&,const Vector&)> receiver_;
    std::function<PhysicsObjectHandle*(const PhysicsObjectParameters<typename Frames::Frame>&)> create_;
    std::function<void(PhysicsObjectHandle*,const Vector&,const Vector&,float)> impulse_;
};

} //namespace hd2::game
#endif
