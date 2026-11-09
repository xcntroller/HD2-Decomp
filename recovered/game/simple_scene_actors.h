#ifndef HD2_GAME_SIMPLE_SCENE_ACTORS_H
#define HD2_GAME_SIMPLE_SCENE_ACTORS_H

#include "game/scene_actor.h"
#include "game/actor_initial_properties.h"
#include "game/sound_emission.h"

namespace hd2::game {

//Complete cold construction/frame/property paths for these bounded families.
//Fields explicitly left undefined by the original constructors are not exposed
//as usable gameplay state. No runtime update/render/interaction substitutes.
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
};

template<class Backend>
class ExplosivePlaceSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    explicit ExplosivePlaceSceneActor(Backend& backend) : SceneActor<Backend>(backend,22),
        properties(initial_explosive_place_properties()) {} //0060C230.
    bool initialize(Frame frame) override {return this->base_.initialize_frame(frame);} //0060C2F0.
    void load(SerializedBytes& bytes) override {properties.load(bytes);} //0060CA30.
    ExplosivePlaceActorProperties properties;
    std::array<std::uint32_t,24> fieldsd0{};
    bool flag90=false,flag170=false;
    std::uint32_t field94=4,fieldb0=0,fieldb4=0,fieldbc=0,fieldc0=0;
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
    std::vector<std::uint32_t> field94;
    std::uint32_t fielda4=0xffffffffu;
};

template<class Backend>
class PikeSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    explicit PikeSceneActor(Backend& backend) : SceneActor<Backend>(backend,28),
        properties(initial_pike_properties()) {} //005B8D00.
    bool initialize(Frame frame) override {return this->base_.initialize_frame(frame);} //005B8E00.
    void load(SerializedBytes& bytes) override {properties.load(bytes);} //005B9C80.
    PikeActorProperties properties;
    SoundEmission sound;
    std::array<std::uint32_t,24> fields1b0{};
    std::uint32_t field188=0,field1a0=0;
    bool flag130=false;
};

} //namespace hd2::game
#endif
