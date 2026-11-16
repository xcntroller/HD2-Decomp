#ifndef HD2_GAME_PHOTO_PLACE_SCENE_ACTOR_H
#define HD2_GAME_PHOTO_PLACE_SCENE_ACTOR_H

#include "game/scene_actor.h"
#include "game/rigid_body.h"
#include "game/actor_material.h"
#include "frontend/cleanup.h"
#include <functional>
#include <utility>

namespace hd2::game {

//Type15, factory0060B640/constructor0060B6D0, vtable00813400. The actual
//Scene2 registry owns this actor; collision buckets and localization are borrowed.
template<class Backend>class PhotoPlaceSceneActor final:public SceneActor<Backend>{
public:
    using Frame=typename Backend::Frame;using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    PhotoPlaceSceneActor(Backend& backend,std::string initial_caption)
        :SceneActor<Backend>(backend,15){this->base_.name(std::move(initial_caption));}
    ~PhotoPlaceSceneActor()override{if(remove_collision_)remove_collision_();} //0060B7C0 before common lifetime.
    bool initialize(Frame frame)override{return this->base_.initialize_frame(frame);} //0060B7D0→00610200.
    void load(SerializedBytes& bytes)override{ //0060BC80: skip unknown chunks and short100 payloads.
        while(auto chunk=next_mission_chunk(bytes))if(chunk->id==100&&chunk->bytes.remaining()>=4)caption94=chunk->bytes.word();
    }
    template<class Activation,class Collisions>void activate(bool authority,Activation& activation,Collisions& collisions){ //0060B820.
        this->base_.activate_frame(authority,activation);
        if(flag90){if(!ready_)throw std::logic_error("Retire partial PhotoPlace activation before retry");return;}
        if(collision_owner_&&collision_owner_!=&collisions)throw std::logic_error("PhotoPlace collision owner changed");
        static_assert(noexcept(collisions.remove_for_cleanup(descriptor_)),"PhotoPlace collision cleanup must not throw");
        if(!collision_owner_){remove_collision_=[this,&collisions]()noexcept{collisions.remove_for_cleanup(descriptor_);};collision_owner_=&collisions;}
        photographed91=0;flag90=true; //Before caption/bounds/descriptor publication, as native.
        this->base_.name(activation.actor_localized(caption94));
        const auto frame=this->base_.frame();const auto bounds=this->backend_.physics_object_bounds(frame,false);
        if(!(bounds.minimum[0]<bounds.maximum[0]))throw std::runtime_error("PhotoPlace native collision centre is unwritten for invalid bounds");
        descriptor_=Descriptor{};descriptor_.flags=0x40;descriptor_.mask=0x800000;descriptor_.frame=frame;descriptor_.actor=this;
        descriptor_.minimum=bounds.minimum;descriptor_.maximum=bounds.maximum;
        const auto world=this->backend_.actor_world_matrix(frame);
        descriptor_.position={{world[12],world[13],world[14],world[15]}};descriptor_.radius=bounds.radius;
        descriptor_.matrix=this->backend_.actor_world_matrix_pointer(frame);
        inverse_=this->backend_.car_inverse_matrix(this->backend_.actor_world_matrix(frame));descriptor_.inverse=&inverse_;
        const auto scale=this->backend_.car_scale(frame);const auto yz=scale[1]>scale[2]?scale[1]:scale[2];
        descriptor_.radius=static_cast<float>(static_cast<long double>(scale[0]>yz?scale[0]:yz)*bounds.radius);
        descriptor_.position=this->backend_.car_transform(bounds.centre,this->backend_.actor_world_matrix(frame));
        collisions.insert(descriptor_);ready_=true;
    }
    template<class Activation,class Collisions>void deactivate(bool authority,Activation& activation,Collisions& collisions){ //0060BB50.
        flag90=false;ready_=false;frontend::CleanupErrors errors;
        errors.attempt([&]{collisions.remove(descriptor_);});
        errors.attempt([&]{this->base_.deactivate_frame(authority,activation);});errors.rethrow();
    }
    template<class Human>void complete(Human&){photographed91=1;} //0060C130 RET4; Human argument unused.
    void reset_completion(){photographed91=0;} //0060BE80.
    std::int32_t material(Frame picked){ //0060BE40, picked frame before actor-frame fallback.
        const auto material=picked?actor_material_id(this->backend_.actor_user_data(picked)):-1;if(material!=-1)return material;
        if(picked!=this->base_.frame()){
            const auto fallback=actor_material_id(this->backend_.actor_user_data(this->base_.frame()));if(fallback!=-1)return fallback;
        }
        return 11;
    }
    void read_snapshot(SerializedBytes& bytes){ //0060BF90:3000, one raw byte, extension/unknown chunks skipped.
        while(auto chunk=next_mission_chunk(bytes))if(chunk->id==3000&&chunk->bytes.remaining())photographed91=chunk->bytes.byte();
    }
    std::vector<unsigned char> write_snapshot()const{return {0xb8,0x0b,7,0,0,0,photographed91};} //0060BE90.
    const Descriptor& collision_descriptor()const{return descriptor_;}
    std::uint32_t caption94=0xffffffffu;std::uint8_t photographed91=0;bool flag90=false;
private:
    Descriptor descriptor_{};typename Backend::Matrix inverse_{};
    std::function<void()> remove_collision_;void* collision_owner_=nullptr;bool ready_=false;
};
} //namespace hd2::game
#endif
