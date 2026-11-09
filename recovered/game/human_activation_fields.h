#ifndef HD2_GAME_HUMAN_ACTIVATION_FIELDS_H
#define HD2_GAME_HUMAN_ACTIVATION_FIELDS_H

#include "game/human_joint_math.h"
#include <cstdint>

namespace hd2::game {
//Known scalar/vector writes in0059F2C0, without native object layout or pointer
//overlays. Zero guards for previously unwritten lanes earn no constructor credit.
struct HumanActivationFields {
    std::uint32_t e4=0,e8=0,field154=0,field158=0,field15c=0,field1ac=0,field200=0,field210=0,field254=0;
    float field14c=0,field16c=0;
    bool flag150=false,flag178=false,flag20c=false,flag214=false;
    std::array<float,3> rotation17c{};
    HumanVector vector1b0{},vector1c0{},position1d0{},colour1e0{};
};
} //namespace hd2::game
#endif
