#ifndef HD2_GAME_MISSION_MODULE_DISPATCH_H
#define HD2_GAME_MISSION_MODULE_DISPATCH_H

#include <cstddef>
#include <cstdint>

namespace hd2::game {

//0048DBF0 /006CADE0. Read each registration afresh; a callback may replace
//later slots. The containing runtime supplies the actual registered receiver,
//including its original empty methods where verified, with no default handler.
template<class Registry,class Services>
void finish_mission_module_pass(Registry& registry,Services& services,bool authority) {
    for(std::size_t kind=0;kind!=registry.size();++kind) {
        auto* module=registry[kind];
        if(module)services.finish_mission_module(*module,authority);
    }
}
template<class Registry,class Services>
void finish_mission_modules(Registry& registry,Services& services,const std::uint32_t& mode_flags) {
    if(mode_flags&1)finish_mission_module_pass(registry,services,true);
    if(mode_flags&2)finish_mission_module_pass(registry,services,false);
}

//0048DB00: same fresh ascending traversal, but actual virtual+1C receivers.
//006989B0 invokes the true authority pass then false presentation pass according
//to mode bits. Ordering of HUD/overlays between these passes stays with caller.
template<class Registry,class Services>
void activate_mission_module_pass(Registry& registry,Services& services,bool authority) {
    for(std::size_t kind=0;kind!=registry.size();++kind) {
        auto* module=registry[kind];
        if(module)services.activate_mission_module(*module,authority);
    }
}

//0048DB90: finish(+20) is REVERSE, unlike loading/activation/file unload.
//Read each slot after the preceding receiver; never snapshot registrations.
template<class Registry,class Services>
void deactivate_mission_module_pass(Registry& registry,Services& services,bool authority) {
    for(std::size_t kind=registry.size();kind;) {
        auto* module=registry[--kind];
        if(module)services.deactivate_mission_module(*module,authority);
    }
}
//0048DC20: file-stage unload(+18) is ascending and distinct from finish(+20).
template<class Registry,class Services>
void unload_mission_module_pass(Registry& registry,Services& services,bool authority) {
    for(std::size_t kind=0;kind!=registry.size();++kind) {
        auto* module=registry[kind];
        if(module)services.unload_mission_module(*module,authority);
    }
}

} //namespace hd2::game
#endif
