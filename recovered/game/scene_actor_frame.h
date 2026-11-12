#ifndef HD2_GAME_SCENE_ACTOR_FRAME_H
#define HD2_GAME_SCENE_ACTOR_FRAME_H

#include "game/car_scene_actor.h"
#include "game/geometry_scene_actors.h"
#include "game/model_scene_actors.h"
#include "game/simple_scene_actors.h"
#include "game/static_weapon_actor.h"

namespace hd2::game {

struct SceneHumanFrameSummary {bool eligible=false,suspended=false,selected=false;};

//005A5D30 tests only AL, not the incidental upper bytes of the state pointer.
//The suspension byte belongs to the SAME Script AI state as suspend_script().
template<class Human>bool human_script_suspended(const Human& human) {
    const auto* state=human.loading_state;
    return state&&state->words().template get<std::uint32_t>(0x48)==5&&state->derived_loading_word(0x1d2)!=0;
}
template<class Human,class Services>SceneHumanFrameSummary summarize_frame_human(const Human& human,Services& services) {
    std::int32_t health;const auto bits=human.properties.stats_ec[0];std::memcpy(&health,&bits,4);
    if(human.properties.field19c!=2||health<=0||!human.loading_state||services.human_unavailable(human))return {};
    return {true,human_script_suspended(human),human.properties.flags1a4[0]!=0};
}

//00476080/00476160. Authority movement005647C0 receives the ACTIVE20 list,
//not the identity10 array. Proximity006E0760 subtracts exactly100 per call.
//Services must bind real movement, retirement and derived actor receivers.
template<class Registry,class Services>void tick_scene_actor_registry(Registry& registry,bool authority,
        std::uint32_t elapsed,Services& services) {
    if(authority) {
        if(!(services.mode_flags()&2u))services.flush_retired_scene_actors(elapsed);
        services.advance_actor_proximity100();
        services.advance_human_movement(elapsed,registry.active_actors());
    }else if((services.mode_flags()&7u)!=7u)services.flush_retired_scene_actors(elapsed);
    registry.visit_active([&](auto& actor){services.update_scene_actor(actor,authority,elapsed);});
    if(!authority)registry.summarize_humans(services);
}

//Actual family vtable+18/+14 dispatch. Human and Dog simulation are mandatory
//derived receivers; this policy provides no successful generic actor update.
//Common Spawn/Switch/Null and Ladder methods are verified native RET4 bodies.
template<class Frames,class World,class Derived>void update_loading_scene_actor(SceneActor<Frames>& actor,
        bool authority,std::uint32_t elapsed,World& world,Derived& derived) {
    switch(actor.base().type()) {
    case 3:derived.update_human(actor,authority,elapsed);return;
    case 4:world.update_car(static_cast<CarSceneActor<Frames>&>(actor),authority,elapsed);return;
    case 6:if(!authority||!(world.mode_flags()&2u))world.update_door(static_cast<DoorSceneActor<Frames>&>(actor),elapsed);return;
    case 8:case 10:case 13:case 33:return;
    case 9:world.update_supply(static_cast<SupplyBoxSceneActor<Frames>&>(actor),authority,elapsed);return;
    case 21:world.update_static_weapon(static_cast<StaticWeaponSceneActor<Frames>&>(actor),authority,elapsed);return;
    case 22:{auto& value=static_cast<ExplosivePlaceSceneActor<Frames>&>(actor);
        if(authority)value.update_authority(elapsed);else value.update_presentation(elapsed,world.mode_flags());return;}
    case 25:world.update_animation(static_cast<AnimationSceneActor<Frames>&>(actor),authority,elapsed);return;
    case 28:{auto& value=static_cast<PikeSceneActor<Frames>&>(actor);
        if(authority)value.update_authority(elapsed,world.mode_flags());else value.update_presentation(elapsed);return;}
    case 29:world.update_fowl(static_cast<FowlGeneratorSceneActor<Frames>&>(actor),authority,elapsed);return;
    case 30:derived.update_dog(actor,authority,elapsed);return;
    case 37:
        if((world.mode_flags()&7u)==7u)return; //005EC060/005EC540 both have this genuine initial RET.
        throw std::runtime_error(authority?"Zone authority frame requires actual005EC060 decision receivers":"Zone presentation frame requires actual005EC540 containment receivers");
    default:throw std::runtime_error("Required Scene2 actor frame receiver is unavailable");
    }
}
} //namespace hd2::game
#endif
