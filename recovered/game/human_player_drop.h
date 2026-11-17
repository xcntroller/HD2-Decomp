#ifndef HD2_GAME_HUMAN_PLAYER_DROP_H
#define HD2_GAME_HUMAN_PLAYER_DROP_H
#include "game/human_packet_commands.h"
#include "game/human_ai_shot_direction.h"
namespace hd2::game {
//005A2C50: three process RNG draws; preserve each native FLOAT spill.
template<class Random>HumanVector human_requested_drop_velocity(const HumanVector& direction,Random&& random){
    const float x=static_cast<float>(static_cast<long double>(random())*.20000000298023224f);
    const float y=static_cast<float>(static_cast<long double>(random())*.10000000149011612f);
    const auto z=static_cast<long double>(random());
    const float sx=static_cast<float>(static_cast<long double>(x)+direction[0]);
    const float sy=static_cast<float>(static_cast<long double>(y)+direction[1]);
    const float sz=static_cast<float>(z*.20000000298023224f+direction[2]);
    return {{sx*2.5f,sy*2.5f,sz*2.5f,0}};
}
//0053E9D0, Player virtual70: append owned command10 to FIRST80 queue.
//The original copies the fourth vector word unchanged while normalizing XYZ.
inline HumanPacketCommand human_player_drop_request(std::int32_t identity,const HumanVector& velocity){
    auto direction=velocity;
    const auto square=(static_cast<long double>(velocity[2])*velocity[2]+static_cast<long double>(velocity[1])*velocity[1])+static_cast<long double>(velocity[0])*velocity[0];
    normalize_human_ai_shot(direction,square);
    const float stored=static_cast<float>((static_cast<long double>(velocity[0])*velocity[0]+static_cast<long double>(velocity[1])*velocity[1])+static_cast<long double>(velocity[2])*velocity[2]);
    std::int32_t bits;std::memcpy(&bits,&stored,4);
    const float speed=bits<0x322bcc77?0.f:std::abs(static_cast<long double>(stored)-1)<9.99999993922529e-09f?1.f:static_cast<float>(std::sqrt(static_cast<long double>(stored)));
    HumanPacketCommand command;command.kind=10;
    command.fields.set(8,identity);command.fields.set(0x10,direction);
    command.fields.set(0x20,speed);command.fields.set(0x24,std::int32_t{-1});return command;
}
} //namespace hd2::game
#endif
