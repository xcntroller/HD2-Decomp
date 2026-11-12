#ifndef HD2_GAME_EXPLOSION_CONTACT_LINK_H
#define HD2_GAME_EXPLOSION_CONTACT_LINK_H
#include <cstdint>
#include <functional>
#include <stdexcept>
namespace hd2::game {
struct CarDetachedPartRequest;struct CarDetachedWheelRequest;
//Checked borrowers across the actual World/runtime/Car/blast construction cycle.
//No actor, damage state, process wave or registry is owned by these links.
template<class Frames,class Actor>class RuntimeActorContactLink {
public:
    explicit RuntimeActorContactLink(Frames& frames):frames_(frames){}
    template<class Runtime>void bind(Runtime& runtime){
        if(owner_||&runtime.frames()!=&frames_)throw std::logic_error("Runtime contact requires SAME actual frame owner");
        std::function<void(Actor*,Actor*,std::uint32_t)> receiver=[&runtime](auto* actor,auto* touching,auto slot){runtime.contact_mine(actor,touching,slot);};
        std::function<Actor*(typename Frames::Frame,const typename Frames::Vector&,const typename Frames::Vector&)> clone=[&runtime](auto frame,const auto& position,const auto& direction){return runtime.clone_car_detached(frame,position,direction);};
        std::function<void(Actor*,const CarDetachedPartRequest&,unsigned)> part=[&runtime](auto* actor,const auto& request,unsigned mode){runtime.launch_car_detached(actor,request,mode);};
        std::function<void(Actor*,const CarDetachedWheelRequest&,unsigned)> wheel=[&runtime](auto* actor,const auto& request,unsigned mode){runtime.launch_car_detached(actor,request,mode);};
        receiver_=std::move(receiver);clone_=std::move(clone);part_=std::move(part);wheel_=std::move(wheel);owner_=&runtime;
    }
    template<class Runtime>void unbind(Runtime& runtime)noexcept{if(owner_==&runtime){receiver_={};clone_={};part_={};wheel_={};owner_=nullptr;}}
    void contact(Actor* actor,Actor* touching,std::uint32_t slot){if(!owner_)throw std::logic_error("Runtime Mine contact has no actual owner");receiver_(actor,touching,slot);}
    Actor* clone(typename Frames::Frame frame,const typename Frames::Vector& position,const typename Frames::Vector& direction){
        if(!owner_)throw std::logic_error("Car clone requires its actual runtime owner");
        return clone_(frame,position,direction);
    }
    void launch(Actor* actor,const CarDetachedPartRequest& request,unsigned mode){if(!owner_)throw std::logic_error("Car part launch requires actual runtime owner");part_(actor,request,mode);}
    void launch(Actor* actor,const CarDetachedWheelRequest& request,unsigned mode){if(!owner_)throw std::logic_error("Car wheel launch requires actual runtime owner");wheel_(actor,request,mode);}
private:
    Frames& frames_;const void* owner_=nullptr;std::function<void(Actor*,Actor*,std::uint32_t)> receiver_;
    std::function<Actor*(typename Frames::Frame,const typename Frames::Vector&,const typename Frames::Vector&)> clone_;
    std::function<void(Actor*,const CarDetachedPartRequest&,unsigned)> part_;
    std::function<void(Actor*,const CarDetachedWheelRequest&,unsigned)> wheel_;
};
template<class Frames,class Actor>class ExplosionContactLink {
public:
    using Vector=typename Frames::Vector;
    explicit ExplosionContactLink(Frames& frames):frames_(frames){}
    template<class Blast>void bind(Blast& blast){
        if(owner_||&blast.frames()!=&frames_)throw std::logic_error("Explosion requires SAME actual World/frame owner");
        std::function<void(Actor*,const Vector&,float,float,std::int32_t)> receiver=[&blast](auto* source,const auto& position,auto radius,auto power,auto sound){blast.explode(source,position,radius,power,sound);};
        receiver_=std::move(receiver);owner_=&blast;
    }
    template<class Blast>void unbind(Blast& blast)noexcept{if(owner_==&blast){receiver_={};owner_=nullptr;}}
    void explode(Actor* source,const Vector& position,float radius,float power,std::int32_t sound){
        if(!owner_)throw std::logic_error("Explosion has no actual containing blast owner");
        receiver_(source,position,radius,power,sound);
    }
private:
    Frames& frames_;const void* owner_=nullptr;std::function<void(Actor*,const Vector&,float,float,std::int32_t)> receiver_;
};
}
#endif
