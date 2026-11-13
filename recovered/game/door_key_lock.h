#ifndef HD2_GAME_DOOR_KEY_LOCK_H
#define HD2_GAME_DOOR_KEY_LOCK_H
#include "game/actor_properties.h"
#include "game/door_loading_packets.h"
namespace hd2::game {
//005FF650/005FF5F0. Resolve the actual linked Door only on the successful-key
//mutation path, after the native self flag store. The caller ignores status.
template<class Linked>bool change_door_key_lock(DoorActorProperties& properties,std::int32_t identity,
        std::uint32_t key,bool lock,SessionBroadcastPackets& packets,Linked&& linked){
    if(lock){if(properties.flag234)return true;if(properties.fields220[1])return false;}
    else if(!properties.flag234)return true;
    if(key!=properties.field230){write_door_loading_command(packets,identity,5);return lock?properties.flag234!=0:properties.flag234==0;}
    properties.flag234=lock?1:0;
    if(auto* other=linked())other->flag234=properties.flag234;
    write_door_loading_command(packets,identity,lock?4:3);return lock?properties.flag234!=0:properties.flag234==0;
}
}
#endif
