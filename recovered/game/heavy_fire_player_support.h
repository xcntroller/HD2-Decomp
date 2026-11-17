#ifndef HD2_GAME_HEAVY_FIRE_PLAYER_SUPPORT_H
#define HD2_GAME_HEAVY_FIRE_PLAYER_SUPPORT_H
#include "game/heavy_fire_requests.h"
#include "game/human_packet_commands.h"
namespace hd2::game {
inline std::int32_t heavy_fire_signed(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
//005467C0: successful inventory/ray admission returns1 regardless of native
//module16 busy/result code. It does not consume the support item or use Mine12.
template<class State,class World>bool request_human_heavy_fire(State& state,const HumanPacketCommand& command,World& world){
    auto* human=state.human();if(!human)throw std::logic_error("Player support requires actual bound Human");
    const auto action=command.fields.template get<std::uint32_t>(0x40);
    if(action!=33&&action!=34)return false;
    if(!world.heavy_fire_has_definition(*human,action==33?243:244))return false;
    auto validate=world.human_packet_action_validator(state);
    const auto start=command.fields.template get<HumanVector>(0x10),direction=command.fields.template get<HumanVector>(0x30);
    auto displacement=direction;for(unsigned a=0;a!=3;++a)displacement[a]=static_cast<float>(static_cast<long double>(displacement[a])*500.f);
    const auto hit=world.heavy_fire_target_ray(start,displacement);if(!hit)return false;
    validate();const auto side=world.human_specialization(*human,false);
    if(action==34){
        (void)world.heavy_fire_configuration_count(side); //Native reads it even though00472CE0 ignores this word.
        HeavyFireMortarRequest request;request.position=*hit;request.side=heavy_fire_signed(side);request.human=human;
        world.heavy_fire_request_mortar(request);
    }else{
        HeavyFireAirRequest request;request.position=*hit;request.direction={{-direction[0],0,-direction[2],0}};
        request.side=heavy_fire_signed(side);request.human=human;
        request.count=(world.mode_flags()&7u)==7u?static_cast<std::int32_t>(side!=0):world.heavy_fire_configuration_count(side);
        request.type=heavy_fire_signed(human_packet_ftol(static_cast<long double>(world.human_sound_random())*2.L+1.f));
        request.position[1]=static_cast<float>(static_cast<long double>((*hit)[1])+60.f);
        world.heavy_fire_request_air(request);
    }
    return true;
}
} //namespace hd2::game
#endif
