#ifndef HD2_GAME_HUMAN_PLAYER_PICKUP_H
#define HD2_GAME_HUMAN_PLAYER_PICKUP_H
#include "game/human_packet_commands.h"
namespace hd2::game {
//Player virtual6C /0053E440, canonical command9 for the existing authority receiver.
inline HumanPacketCommand human_player_pickup_request(std::int32_t actor,std::int32_t slot){
    HumanPacketCommand command;command.kind=9;command.fields.set(8,actor);command.fields.set(0xc,slot);return command;
}
//Player virtual7C /0053E270. Canonical command11; the native constructor
//uses shared tag9 plus a DISTINCT vtable00810C94 to choose this receiver.
inline HumanPacketCommand human_player_supply_request(std::int32_t actor,std::int32_t row){
    HumanPacketCommand command;command.kind=11;command.fields.set(8,actor);command.fields.set(0xc,row);
    command.fields.set(0x10,std::int32_t{-1});command.fields.set(0x14,7u);return command;
}
//005A45A0→0053F1B0, FIRST80 request. Authority command21 publishes20.
inline HumanPacketCommand human_player_inventory_request(std::int32_t actor){
    HumanPacketCommand command;command.kind=21;command.fields.set(8,actor);return command;
}
} //namespace hd2::game
#endif
