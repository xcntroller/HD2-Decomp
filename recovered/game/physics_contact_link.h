#ifndef HD2_GAME_PHYSICS_CONTACT_LINK_H
#define HD2_GAME_PHYSICS_CONTACT_LINK_H

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
        receiver_=std::move(next);physics_=&physics;
    }
    template<class Physics>void unbind(Physics& physics)noexcept{if(physics_==&physics){receiver_={};physics_=nullptr;}}
    void contact(std::uintptr_t handle,unsigned kind,const Vector& velocity,const Vector& point){
        if(!physics_)throw std::logic_error("Physics contact has no actual containing module");
        receiver_(handle,kind,velocity,point);
    }
    bool bound()const noexcept{return physics_!=nullptr;}
private:
    Frames& frames_;const void* physics_=nullptr;std::function<void(std::uintptr_t,unsigned,const Vector&,const Vector&)> receiver_;
};

} //namespace hd2::game
#endif
