#ifndef HD2_GAME_SIMPLE_SCENE_ACTORS_H
#define HD2_GAME_SIMPLE_SCENE_ACTORS_H

#include "game/scene_actor.h"
#include "game/actor_initial_properties.h"
#include "game/sound_emission.h"
#include "game/rigid_body.h"
#include "game/fowl_loading_lifetime.h"
#include "game/prepared_spawn_packets.h"
#include "frontend/cleanup.h"
#include <functional>
#include <cmath>
#include <cstring>

namespace hd2::game {

//Complete cold construction/frame/property paths for these bounded families.
//Fields explicitly left undefined by the original constructors are not exposed
//as usable gameplay state. Bounded loading/first-update paths reject missing
//movement or detonation receivers before claiming those operations succeeded.
template<class Backend>
class NullSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    explicit NullSceneActor(Backend& backend) : SceneActor<Backend>(backend,33) {} //005E2B10.
    bool initialize(Frame frame) override {return this->base_.initialize_frame(frame);} //005E2B60.
    void load(SerializedBytes&) override {} //005E2BC0 is a genuine RET1.
    template<class Activation>void activate(bool authority,Activation& activation) {this->base_.activate_frame(authority,activation);} //005E2B90.
    template<class Activation>void deactivate(bool authority,Activation& activation) {this->base_.deactivate_frame(authority,activation);} //005E2BA0.
};

template<class Backend>
class SpawnPointSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    explicit SpawnPointSceneActor(Backend& backend) : SceneActor<Backend>(backend,10) {} //006118D0.
    bool initialize(Frame frame) override {return this->base_.initialize_frame(frame);} //00611950.
    void load(SerializedBytes& bytes) override {properties.load(bytes);} //006119B0.
    template<class Activation>void activate(bool authority,Activation& activation) { //00611970.
        this->base_.activate_frame(authority,activation);flag9c=true;
    }
    template<class Activation>void deactivate(bool authority,Activation& activation) { //00611990.
        this->base_.deactivate_frame(authority,activation);flag9c=true;
    }
    SpawnPointActorProperties properties;
    bool flag9c=true;
    std::uint32_t fielda8=0;
    SceneActor<Backend>* spawned_human=nullptr; //Actual nativeA8 borrower, never an actor pointer in fielda8 DWORD.
};

template<class Backend>
class ExplosivePlaceSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    explicit ExplosivePlaceSceneActor(Backend& backend) : SceneActor<Backend>(backend,22),
        properties(initial_explosive_place_properties()) {} //0060C230.
    bool initialize(Frame frame) override {return this->base_.initialize_frame(frame);} //0060C2F0.
    void load(SerializedBytes& bytes) override {properties.load(bytes);} //0060CA30.
    ~ExplosivePlaceSceneActor()override{if(remove_collision_)remove_collision_();}
    template<class Activation,class Collisions>void activate(bool authority,Activation& activation,Collisions& collisions) { //0060C3B0.
        this->base_.activate_frame(authority,activation);
        if(flag90){if(!ready_)throw std::logic_error("Retire partial ExplosivePlace activation before retry");return;}
        if(descriptor_.bucket)throw std::logic_error("Retire the actual ExplosivePlace descriptor before retry");
        bind_collisions(collisions);flag90=true;fieldc4_=0xffffffffu;timer_c8_=0;
        const auto frame=this->base_.frame();const auto bounds=this->backend_.physics_object_bounds(frame,false);
        if(!(bounds.minimum[0]<bounds.maximum[0]))throw std::runtime_error("ExplosivePlace collision centre is unwritten for invalid bounds");
        descriptor_=Descriptor{};descriptor_.flags=0x40;descriptor_.mask=2;descriptor_.frame=frame;descriptor_.actor=this;
        descriptor_.minimum=bounds.minimum;descriptor_.maximum=bounds.maximum;
        descriptor_.matrix=this->backend_.actor_world_matrix_pointer(frame);
        inverse_=this->backend_.car_inverse_matrix(this->backend_.actor_world_matrix(frame));descriptor_.inverse=&inverse_;
        const auto scale=this->backend_.car_scale(frame);const auto yz=scale[1]>scale[2]?scale[1]:scale[2];
        descriptor_.radius=static_cast<float>(static_cast<long double>(scale[0]>yz?scale[0]:yz)*bounds.radius);
        descriptor_.position=this->backend_.car_transform(bounds.centre,this->backend_.actor_world_matrix(frame));
        on_=this->backend_.actor_find(frame,"explosive_on",0xffff);off_=this->backend_.actor_find(frame,"explosive_off",0xffff);
        set_state(properties.flagb8?1u:0u,activation,collisions);ready_=true;
    }
    template<class Activation,class Collisions>void deactivate(bool authority,Activation& activation,Collisions& collisions) { //0060C6B0.
        frontend::CleanupErrors errors;
        if(flag90){flag90=false;ready_=false;errors.attempt([&]{set_state(4,activation,collisions);});
            if(descriptor_.bucket&&remove_collision_){remove_collision_();flag170=false;}}
        errors.attempt([&]{this->base_.deactivate_frame(authority,activation);});errors.rethrow();
    }
    template<class Activation,class Collisions>void set_state(std::uint32_t state,Activation& activation,Collisions& collisions) { //0060D380.
        if(state==field94)return;
        field94=state;if(!on_||!off_)return; //Actual missing authored-pair branch has no registration or visibility calls.
        bool insert=false,remove=false;const auto frame=this->base_.frame();
        switch(state) {
        case 0:
            this->backend_.object_visible(on_,false);timer_c8_=0;this->backend_.object_visible(off_,!properties.flagb9);
            if(!properties.flagb9){this->base_.name(activation.actor_localized(properties.field9c));insert=true;}
            else {this->base_.name("Hidden, you should not see this text!!");remove=true;}
            break;
        case 1:
            this->backend_.object_visible(on_,true);this->backend_.object_visible(off_,false);timer_c8_=properties.fielda4;
            this->base_.name(activation.actor_item_caption(properties.fielda0));insert=true;break;
        case 2:
            this->backend_.object_visible(on_,false);this->backend_.object_visible(off_,false);timer_c8_=0;
            this->base_.name("Exploded, you should not see this text!!");remove=true;break;
        case 3:
            this->backend_.object_visible(on_,false);this->backend_.object_visible(off_,false);timer_c8_=0;
            this->base_.name("Hidden, you should not see this text!!");remove=true;break;
        case 4:
            timer_c8_=0;this->backend_.object_visible(on_,true);this->backend_.object_visible(off_,true);remove=true;break;
        default:break;
        }
        if(insert){if(!flag170)collisions.insert(descriptor_);flag170=true;}
        if(remove){if(flag170)collisions.remove(descriptor_);flag170=false;}
        this->backend_.update_scene_object(frame);
    }
    template<class Activation,class Collisions>void disarm(Activation& activation,Collisions& collisions){ //0060D9F0.
        set_state(0,activation,collisions);
        auto& packet=activation.session_broadcast_packets().begin(13,0);
        packet.write(static_cast<std::uint32_t>(this->base_.id10()),10);
        packet.write(1,2);
        packet.finish_broadcast();
    }
    template<class Activation,class Collisions>void receive_loading_packet(SessionPacketStream& packet,
            Activation& activation,Collisions& collisions){ //0060CDA0, presentation side.
        switch(packet.read(2)){
        case 1:
            if(!(activation.mode_flags()&1u))set_state(0,activation,collisions); //0060DA50.
            return;
        case 3:set_state(packet.read(2),activation,collisions);return;
        default:throw std::runtime_error("ExplosivePlace packet requires its actual detonation or placement receiver");
        }
    }
    void update_authority(std::uint32_t elapsed) { //0060C330, before actual0060D870 detonation.
        if(!properties.flag98||signed_word(timer_c8_)<1)return;
        timer_c8_-=elapsed;
        if(signed_word(timer_c8_)<=0)throw std::runtime_error("ExplosivePlace expiry requires its actual detonation/effect receiver");
    }
    void update_presentation(std::uint32_t elapsed,std::uint32_t mode) { //0060C370.
        if((mode&1u)||!properties.flag98||signed_word(timer_c8_)<1)return;
        timer_c8_-=elapsed;if(signed_word(timer_c8_)<=0)timer_c8_=0;
    }
    const Descriptor& collision_descriptor()const{return descriptor_;}Frame on_frame()const{return on_;}Frame off_frame()const{return off_;}
    std::uint32_t countdown()const{return timer_c8_;}
    ExplosivePlaceActorProperties properties;
    bool flag90=false,flag170=false;
    std::uint32_t field94=4,fieldb0=0,fieldb4=0;
private:
    static std::int32_t signed_word(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
    template<class Collisions>void bind_collisions(Collisions& collisions) {
        if(collision_owner_&&collision_owner_!=&collisions)throw std::logic_error("ExplosivePlace collision owner changed");
        static_assert(noexcept(collisions.remove_for_cleanup(descriptor_)),"ExplosivePlace descriptor cleanup must not throw");
        if(!collision_owner_){remove_collision_=[this,&collisions]()noexcept{collisions.remove_for_cleanup(descriptor_);};collision_owner_=&collisions;}
    }
    Frame on_{},off_{};Descriptor descriptor_{};typename Backend::Matrix inverse_{};
    std::function<void()> remove_collision_;void* collision_owner_=nullptr;std::uint32_t fieldc4_=0xffffffffu,timer_c8_=0;bool ready_=false;
};

template<class Backend>
class FowlGeneratorSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    explicit FowlGeneratorSceneActor(Backend& backend) : SceneActor<Backend>(backend,29),
        properties(initial_fowl_generator_properties()) {} //005BA4D0.
    bool initialize(Frame frame) override {return this->base_.initialize_frame(frame);} //005BA4B0.
    void load(SerializedBytes& bytes) override {properties.load(bytes);} //005BB7E0.
    FowlGeneratorActorProperties properties;
    std::unique_ptr<FowlLoadingLifetime> loading; //Actual generated-bird/listener lifetime, before common frame retirement.
    std::uint32_t fielda4=0xffffffffu;
};

template<class Backend>
class PikeSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    explicit PikeSceneActor(Backend& backend) : SceneActor<Backend>(backend,28),
        properties(initial_pike_properties()) {} //005B8D00.
    bool initialize(Frame frame) override {return this->base_.initialize_frame(frame);} //005B8E00.
    void load(SerializedBytes& bytes) override {properties.load(bytes);} //005B9C80.
    ~PikeSceneActor()override{if(remove_collision_)remove_collision_();}
    template<class Activation,class Collisions>void activate(bool authority,Activation& activation,Collisions& collisions) { //005B9670.
        this->base_.activate_frame(authority,activation);
        if(flag130){if(!ready_)throw std::logic_error("Retire partial Pike activation before retry");return;}
        if(descriptor_.bucket)throw std::logic_error("Retire the actual Pike descriptor before retry");
        if(((activation.mode_flags()&1u)&&properties.field184==1)||properties.field184==2||properties.field184==3)
            throw std::runtime_error("Pike initial movement requires its actual packet/rotation receiver");
        if(collision_owner_&&collision_owner_!=&collisions)throw std::logic_error("Pike collision owner changed");
        static_assert(noexcept(collisions.remove_for_cleanup(descriptor_)),"Pike collision teardown must not throw");
        if(!collision_owner_){remove_collision_=[this,&collisions]()noexcept{collisions.remove_for_cleanup(descriptor_);};collision_owner_=&collisions;}
        flag130=true;field1a0=0;saved_rotation_=this->backend_.actor_rotation(this->base_.frame());
        descriptor_=Descriptor{};descriptor_.flags=0x40;descriptor_.mask=0x800;descriptor_.frame=this->base_.frame();descriptor_.actor=this;
        refresh_collision(collisions,true);
        const auto world=this->backend_.actor_world_matrix(this->base_.frame());sound.position({{world[12],world[13],world[14],world[15]}});
        sound.words[7]=1;sound.words[4]=sound.words[5]=sound.words[6]=0;voice180_=-1;ready_=true;
    }
    template<class Activation,class Collisions>void deactivate(bool authority,Activation& activation,Collisions& collisions) { //005B9BC0.
        frontend::CleanupErrors errors;errors.attempt([&]{this->base_.deactivate_frame(authority,activation);});
        if(flag130) {
            errors.attempt([&]{this->backend_.object_rotation(this->base_.frame(),saved_rotation_);this->backend_.update_scene_object(this->base_.frame());});
            errors.attempt([&]{collisions.remove(descriptor_);});
            if(descriptor_.bucket&&remove_collision_)remove_collision_();
            flag130=false;ready_=false;
            if(voice180_!=-1)errors.attempt([&]{activation.stop_actor_sound_voice(voice180_);voice180_=-1;});
        }
        errors.rethrow();
    }
    void update_authority(std::uint32_t,std::uint32_t mode)const { //005B9340 cold/nonmoving/actual mode2 skip.
        if(properties.field184!=0&&properties.field184!=1&&!(mode&2u))throw std::runtime_error("Pike authority movement requires its actual rotation receiver");
    }
    void update_presentation(std::uint32_t elapsed) { //005B9470 cold stationary branch.
        if(field188){field188-=elapsed;if(signed_word(field188)>0)return;field188=0;}
        if(properties.field184==2||properties.field184==3)throw std::runtime_error("Pike presentation movement requires its actual rotation/sound receiver");
    }
    template<class Collisions>void refresh_collision(Collisions& collisions,bool insert) { //005B97B0.
        const auto frame=descriptor_.frame;if(!frame)return;
        typename Backend::Vector centre{};typename Backend::Bounds bounds;float radius;
        if(this->backend_.frame_kind(frame)==6) {
            bounds=this->backend_.car_dummy_bounds(frame);
            centre[0]=static_cast<float>((static_cast<long double>(bounds.minimum[0])+bounds.maximum[0])*.5f);
            for(unsigned a=1;a!=3;++a){const float sum=static_cast<float>(static_cast<long double>(bounds.minimum[a])+bounds.maximum[a]);centre[a]=static_cast<float>(static_cast<long double>(sum)*.5f);}
            radius=static_cast<float>((static_cast<long double>(bounds.maximum[1])-bounds.minimum[1])*.5f);
        }else {
            const auto aggregate=this->backend_.physics_object_bounds(frame,false);
            bounds.minimum=aggregate.minimum;bounds.maximum=aggregate.maximum;centre=aggregate.centre;radius=aggregate.radius;
            if(!(bounds.minimum[0]<bounds.maximum[0]))throw std::runtime_error("Pike aggregate centre is unwritten for invalid bounds");
        }
        const auto scale=this->backend_.actor_local_scale(frame);
        for(unsigned a=0;a!=3;++a) {
            if(!std::isfinite(scale[a])||scale[a]==0)throw std::runtime_error("Pike collision requires finite nonzero local scale");
            bounds.minimum[a]=static_cast<float>(static_cast<long double>(bounds.minimum[a])*scale[a]);bounds.maximum[a]=static_cast<float>(static_cast<long double>(bounds.maximum[a])*scale[a]);
        }
        const auto matrix=this->backend_.actor_world_matrix(frame);world_=matrix;
        for(unsigned a=0;a!=3;++a){const auto reciprocal=1.L/scale[a];for(unsigned c=0;c!=3;++c)world_[a*4+c]=static_cast<float>(static_cast<long double>(world_[a*4+c])*reciprocal);}
        inverse_=this->backend_.car_inverse_matrix(world_);descriptor_.matrix=&world_;descriptor_.inverse=&inverse_;
        descriptor_.minimum=bounds.minimum;descriptor_.maximum=bounds.maximum;
        const auto world_scale=this->backend_.car_scale(frame);const auto yz=world_scale[1]>world_scale[2]?world_scale[1]:world_scale[2];
        descriptor_.radius=static_cast<float>(static_cast<long double>(world_scale[0]>yz?world_scale[0]:yz)*radius);descriptor_.position=this->backend_.car_transform(centre,matrix);
        if(insert)collisions.insert(descriptor_);
        collisions.move(descriptor_);
    }
    const Descriptor& collision_descriptor()const{return descriptor_;}
    PikeActorProperties properties;
    SoundEmission sound;
    std::uint32_t field188=0,field1a0=0;
    bool flag130=false;
private:
    static std::int32_t signed_word(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
    Descriptor descriptor_{};typename Backend::Matrix world_{},inverse_{};std::array<float,4> saved_rotation_{};
    std::function<void()> remove_collision_;void* collision_owner_=nullptr;std::int32_t voice180_=-1;bool ready_=false;
};

} //namespace hd2::game
#endif
