#ifndef HD2_GAME_HUMAN_EQUIPMENT_OFFSETS_H
#define HD2_GAME_HUMAN_EQUIPMENT_OFFSETS_H

#include <array>

namespace hd2::game {
//Actual static initializers, never reads of BSS. Normal loading values;
//offline action position-saving producers are outside this interface.
inline constexpr std::array<float,4> human_equipment_offset20{{0x1.679cc80000000p-3f,0x1.71c2a00000000p-3f,-0x1.1107740000000p-3f,0}}; //0078C900.
inline constexpr std::array<float,4> human_equipment_offset24{{0x1.122e000000000p-6f,0x1.1b530c0000000p-3f,-0x1.1edb220000000p-6f,0}}; //0078C930.
inline constexpr std::array<float,4> human_equipment_offset28{{-0x1.f3c8640000000p-11f,0x1.41db880000000p-3f,-0x1.0f0f160000000p-3f,0}}; //0078C960.
} //namespace hd2::game
#endif
