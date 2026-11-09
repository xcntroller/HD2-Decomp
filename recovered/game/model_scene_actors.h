#ifndef HD2_GAME_MODEL_SCENE_ACTORS_H
#define HD2_GAME_MODEL_SCENE_ACTORS_H

#include "game/scene_actor.h"
#include "game/actor_initial_properties.h"
#include "game/supply_box_geometry.h"
#include "game/animation_actor_collisions.h"
#include <exception>
#include <optional>

namespace hd2::game {

//Cold actor owners and frame/property paths. Animation contexts, item/effect
//production and simulation remain separate required loading/activation work.
template<class Backend>
class DogSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    DogSceneActor(Backend& backend,std::string localized_name) : SceneActor<Backend>(backend,30),
        properties(initial_dog_properties()) { //005D4B00/005D4C70.
        this->base_.name(std::move(localized_name));
        backend.reset_dog_shared_state(); //008AC4B0, all0x45 DWORDs on every construction.
    }
    bool initialize(Frame frame) override { //005D4D70, retain precedes model-kind check.
        if(!this->base_.initialize_frame(frame)||this->backend_.frame_kind(frame)!=9)return false;
        field534=0;return true;
    }
    void load(SerializedBytes& bytes) override {properties.load(bytes);} //005D6630.
    //Only1c/20 are initialized by005D4C70; native padding/unknown words are
    //defined zero for safety and earn no original behavior credit.
    struct Record {std::array<std::uint32_t,7> unknown_words{};
        std::uint32_t field1c=0xffffffffu,field20=0xffffffffu;};
    DogActorProperties properties;
    std::uint32_t field330=0xffffffffu,field334=0xffffffffu,field534=0;
    std::array<Record,5> records;
    std::array<std::vector<std::uint32_t>,3> lists; //005D4CF0 frees these before00610170.
};

template<class Backend>
class AnimationSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    explicit AnimationSceneActor(Backend& backend) : SceneActor<Backend>(backend,25),
        properties(initial_animation_properties()) {} //005B3740, cold005B4790.
    ~AnimationSceneActor() override { //005B3930/005B4790, before common frame release.
        collisions.reset(); //Actual E0 record destructors remove dynamic descriptors before model retirement.
        const auto frame=this->base_.frame();
        if(frame) {
            cleanup([&] {this->backend_.object_visible(frame,true);});
            cleanup([&] {this->backend_.update_scene_object(frame);});
        }
        fieldd4=0;
    }
    bool initialize(Frame frame) override { //005B3710.
        return this->base_.initialize_frame(frame)&&this->backend_.frame_kind(frame)==9;
    }
    void load(SerializedBytes& bytes) override {properties.load(bytes);} //005B5AF0.
    void script_visibility(bool enabled) { //005B54C0, actual E0 owner.
        if(!collisions)throw std::runtime_error("Animation visibility requires its actual BODYCOLB collision owner");
        if(enabled)collisions->rebuild();else collisions->clear();
    }
    AnimationActorProperties properties;
    std::vector<std::uint32_t> fieldc4;
    std::unique_ptr<AnimationActorCollisionLifetime> collisions;
    std::uint32_t field98=0,fieldd4=0,fieldd8=3;
    bool flaga0=false,flaga1=false;
private:
    template<class Work> void cleanup(Work&& work) noexcept {
        try {work();} catch(...) {this->backend_.record_actor_cleanup_failure(std::current_exception());}
    }
};

template<class Frames,class Collisions>class AnimationActorVisibility {
public:
    using Actor=AnimationSceneActor<Frames>;
    AnimationActorVisibility(Frames& frames,Collisions& collisions):frames_(frames),collisions_(collisions){}
    void set_animation_actor_visibility(SceneActor<Frames>& base,bool enabled) {
        auto* actor=dynamic_cast<Actor*>(&base);
        if(!actor||actor->base().type()!=25)throw std::runtime_error("Animation visibility requires the actual animation actor");
        if(!actor->collisions)actor->collisions=std::make_unique<AnimationActorCollisions<Frames,Collisions,Actor>>(frames_,collisions_,*actor);
        actor->script_visibility(enabled);
    }
private:
    Frames& frames_;Collisions& collisions_;
};

template<class Backend>
class SupplyBoxSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    explicit SupplyBoxSceneActor(Backend& backend) : SceneActor<Backend>(backend,9) {} //005F97D0.
    bool initialize(Frame frame) override { //005FA6A0, including005F98E0.
        if(!frame)return false;
        if(geometry)throw std::logic_error("SupplyBox frame initialization repeated");
        geometry.emplace(this->backend_,frame,typename SupplyBoxGeometry<Backend>::SnapshotOnly{});
        for(auto& items:properties.items)items.clear();
        properties.field11c=0;properties.field128=5;properties.field12c=20;
        properties.field134=0xffffffffu;flag90=false;field1c0=0;
        const auto initialized=this->base_.initialize_frame(frame);
        field138=6000;field130=properties.field12c;
        field244=0;field248=0xffffffffu;field24c=5;field250=field254=0;
        flag224=false;properties.field210=0x1bfa;
        geometry->complete(this->backend_,frame);
        return initialized;
    }
    void load(SerializedBytes& bytes) override {properties.load(bytes);} //005FC200.
    SupplyBoxActorProperties properties;
    std::optional<SupplyBoxGeometry<Backend>> geometry;
    std::array<std::uint32_t,6> fields22c{};
    std::uint32_t field130=0,field138=0,field1c0=0,field214=0,
        field244=0,field248=0,field24c=0,field250=0,field254=0;
    bool flag90=false,flag224=false;
};

} //namespace hd2::game
#endif
