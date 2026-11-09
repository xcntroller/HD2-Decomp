#ifndef HD2_GAME_STATIC_WEAPON_ACTOR_H
#define HD2_GAME_STATIC_WEAPON_ACTOR_H

#include "game/scene_actor.h"
#include "game/static_weapon_loading_lifetime.h"
#include <array>
#include <vector>

namespace hd2::game {

//004C19D0/004C1AA0 native cold owner, at StaticWeapon+160 (not+150).
//No shooting, ammunition or targeting receivers are supplied by these values.
struct StaticWeaponControl {
    std::array<std::uint32_t,24> fields60{};
    std::vector<std::uint32_t> field10c;
    std::uint32_t fieldc8=0xffffffffu,fieldcc=0,fieldd0=0,field120=0,field124=0,
        field13c=0,field140=0,field144=0,field148=0,field158=0,field16c=0,field170=0;
    float field128=-1,field12c=-1,field130=-1,field134=-1;
    std::uint32_t fieldd8=0,field160=0x3ec90fdb,field164=0x3ec90fdb;
    bool flagd4=false,flagdc=false,flag104=false,flag168=false;
};

template<class Backend>
class StaticWeaponSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    explicit StaticWeaponSceneActor(Backend& backend) : SceneActor<Backend>(backend,21) {} //00602EC0.
    bool initialize(Frame frame) override {return this->base_.initialize_frame(frame);} //00603040.
    void load(SerializedBytes&) override {} //00604CE0 genuinely returns1 without reading.
    std::array<std::uint32_t,24> fieldsf0{};
    StaticWeaponControl control;
    std::vector<std::array<std::uint32_t,6>> field2e4;
    std::uint32_t field2f0=3;
    bool flag150=false,flag158=false,flag159=false;
    std::unique_ptr<StaticWeaponLoadingLifetime> loading;
};

} //namespace hd2::game
#endif
