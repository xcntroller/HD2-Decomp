#ifndef HD2_GAME_DETACHED_ITEM_ACTOR_H
#define HD2_GAME_DETACHED_ITEM_ACTOR_H
#include "game/scene_actor.h"
#include "game/rigid_body.h"
#include "game/car_contact_callbacks.h"

namespace hd2::game {
//005FDA20/005FDAB0, runtime type12/00812E18. Body90 registers in the SAME
//rigid-body list; it retires before common actor/frame ownership. This is not
//module2 Physics type4 and carries no inventory entry/catalogue substitute.
template<class Frames,class Runtime,class Collisions>class DetachedItemActor final : public SceneActor<Frames> {
public:
    using Frame=typename Frames::Frame;using Body=RigidBody<Frames,Collisions>;
    DetachedItemActor(Frames& frames,Runtime& runtime,Collisions& collisions,typename Body::Registry& bodies)
        :SceneActor<Frames>(frames,12),runtime_(runtime),body_(frames,collisions,bodies){}
    bool initialize(Frame frame)override{return this->base_.initialize_frame(frame);} //005FDBA0→00610200.
    void load(SerializedBytes&)override{} //005FDB00, genuine AL0/RET4, ignored by registry.
    void activate(bool authority){this->base_.activate_frame(authority,runtime_);fading_=false;} //005FDBB0.
    void deactivate(bool authority){body_.close();this->base_.deactivate_frame(authority,runtime_);} //005FDBD0.
    void launch(const CarDetachedWheelRequest& request){ //005FDC00, RET4; does NOT write695.
        publish_lifetime();body_.bind(this->base_.frame());
        if(request.field24)throw std::runtime_error("Detached wheel callback requires its actual typed producer");
        typename Body::WheelParameters parameters{request.velocity,request.field10,request.field14,request.mass,
            request.field1c,request.field20,{},request.field28!=0};
        body_.configure_wheel(parameters);body_.reset();body_.actor(this);
    }
    void launch(const CarDetachedPartRequest& request,bool retire){ //005FDC50, RET8.
        publish_lifetime();retire_=retire;body_.bind(this->base_.frame());
        if(request.field38)throw std::runtime_error("Detached part callback requires its actual typed producer");
        typename Body::Parameters parameters{request.velocity,request.mass,request.coefficient,{},nullptr,request.field40!=0,request.field41!=0};
        body_.configure(parameters);body_.reset();body_.actor(this);
    }
    void update(bool authority,std::uint32_t elapsed){ //005FDD00 presentation is genuine RET4.
        if(!authority)return;
        runtime_.update_detached_body(body_,elapsed); //005FDD10 calls SAME body24 BEFORE even lifetime reads.
        if(!lifetime_)throw std::runtime_error("Detached Item690 lifetime has no native producer");
        if(*lifetime_<0)return;
        const auto bits=static_cast<std::uint32_t>(*lifetime_)-elapsed;std::int32_t remaining;std::memcpy(&remaining,&bits,4);lifetime_=remaining;
        if(remaining<=0){
            if(required_retire()){runtime_.restore_detached_camera_amount(this->base_.frame());runtime_.remove(this);return;}
        }else if(remaining<=500&&!required_fading()&&required_retire()){
            fading_=true;runtime_.fade_detached(this->base_.frame(),remaining);
        }
    }
    Body& body(){return body_;}const auto& lifetime()const{return lifetime_;}
    const auto& retire_on_expiry()const{return retire_;}
private:
    bool required_retire()const{if(!retire_)throw std::runtime_error("Detached wheel695 expiry byte has no native producer");return *retire_;}
    bool required_fading()const{if(!fading_)throw std::logic_error("Detached Item694 requires actual activation");return *fading_;}
    void publish_lifetime(){
        const auto value=static_cast<long double>(this->backend_.volume_random_unit())*10000.f+30000.f;
        if(!std::isfinite(value)||value>2147483647.L||value< -2147483648.L)throw std::runtime_error("Detached lifetime exceeds native signed range");
        lifetime_=static_cast<std::int32_t>(value); //ENGINE RandFloat, native FTOL truncation.
    }
    Runtime& runtime_;Body body_;
    std::optional<std::int32_t> lifetime_;std::optional<bool> fading_,retire_; //Ctor genuinely leaves690/694/695 unwritten.
};
}
#endif
