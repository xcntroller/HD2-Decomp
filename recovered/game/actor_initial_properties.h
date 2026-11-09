#ifndef HD2_GAME_ACTOR_INITIAL_PROPERTIES_H
#define HD2_GAME_ACTOR_INITIAL_PROPERTIES_H

#include "game/actor_properties.h"

namespace hd2::game {

//The parsers' value types are not complete actor constructors. Apply the proven
//constructor/initialization defaults before reading optional/versioned fields;
//the containing concrete actors still own their control/frame/resource state.
inline HumanActorProperties initial_human_properties(std::uint32_t mode_flags) { //0059BF00.
    HumanActorProperties result;
    result.stats_ec={{800,80,80,80,80,80,50,30,20,50,0,80}};
    result.stats_11c=result.stats_ec;
    result.flags1a4[1]=(mode_flags&7)!=7;
    return result;
}
inline LadderActorProperties initial_ladder_properties() { //005EA3D0.
    LadderActorProperties result;result.settings170={{0xffffffffu,0xffffffffu,0xffffffffu,1}};return result;
}
inline PikeActorProperties initial_pike_properties() { //005B8D00.
    PikeActorProperties result;result.field18c=result.field190=0x3fc90fdb;
    result.vector90={{0,0x3f800000,0}};return result;
}
inline DogActorProperties initial_dog_properties() { //005D4B00.
    DogActorProperties result;result.fields4a0={{250,50,20,100}};return result;
}
inline FowlGeneratorActorProperties initial_fowl_generator_properties() { //005BA4D0.
    FowlGeneratorActorProperties result;result.fielda8=0x40000000;result.fieldac=0x40a00000;
    result.name="ptacekletacek";return result;
}
inline AnimationActorProperties initial_animation_properties() { //005B3740/005B4790.
    AnimationActorProperties result;result.fieldd0=0;
    result.fielda8=result.fieldac=result.fieldb0=0xffffffffu;return result;
}
inline ExplosivePlaceActorProperties initial_explosive_place_properties() { //0060C230.
    ExplosivePlaceActorProperties result;result.flag98=1;result.field9c=0x1bf9;
    result.fielda0=0x44;result.fieldsa8[0]=result.fieldsa8[1]=0xffffffffu;return result;
}
inline DoorActorProperties initial_door_properties() { //005FF210.
    DoorActorProperties result;result.fields220={{0,0,0x3fc90fdb,0x3fc90fdb}};
    result.vector210={{0,0xbf800000,0}};result.field114=0x1bf1;return result;
}
inline SupplyBoxActorProperties initial_supply_box_properties() { //005FA6A0/005F98E0.
    SupplyBoxActorProperties result;result.field210=0x1bfa;return result;
}

} //namespace hd2::game
#endif
