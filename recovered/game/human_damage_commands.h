#ifndef HD2_GAME_HUMAN_DAMAGE_COMMANDS_H
#define HD2_GAME_HUMAN_DAMAGE_COMMANDS_H
#include "game/human_packet_commands.h"
namespace hd2::game {
struct HumanDropCommand {std::int32_t item=-1,actor=-1;HumanVector direction{};float speed=0;};
struct HumanDamageCommand {std::uint32_t kind=0,hit_kind=0,health=0;std::int32_t source=-1;HumanVector point{},direction{};};
inline HumanDamageCommand human_damage_command_value(const HumanPacketCommand& command){
    const auto& f=command.fields;return {f.get<std::uint32_t>(8),f.get<std::uint32_t>(0xc),f.get<std::uint32_t>(0x10),
        f.get<std::int32_t>(0x14),f.get<HumanVector>(0x20),f.get<HumanVector>(0x30)};
}
inline HumanDropCommand human_drop_command_value(const HumanPacketCommand& command){
    const auto& f=command.fields;return {f.get<std::int32_t>(8),f.get<std::int32_t>(0x24),f.get<HumanVector>(0x10),f.get<float>(0x20)};
}
} //namespace hd2::game
#endif
