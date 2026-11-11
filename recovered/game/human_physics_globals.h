#ifndef HD2_GAME_HUMAN_PHYSICS_GLOBALS_H
#define HD2_GAME_HUMAN_PHYSICS_GLOBALS_H
#include <cstdint>
#include <array>
namespace hd2::game {
//009832B0 counts actual live physical bindings across the process. Startup
//outlives every world/DD0 and must supply this same counter to all of them.
struct HumanPhysicsGlobals {
    std::uint32_t active=0;
    float motion_iterations=10.f; //0084411C, shared native multisphere budget.
    std::array<float,4> ground_normal{{0,1,0,0}}; //00983340, shared most recent ground query.
    std::uint32_t camera_projection_tick=0; //009833B0, original Game+28 key.
    float camera_projection_scale=0.f; //009833B4.
    std::uint32_t ragdoll_contacts=0; //0098AF84, shared007CD230 traversal count.
    float ragdoll_contact_normal=0; //0098AF88, accumulation and subsequent decay scratch.
    std::uint32_t reaction_sequence=0; //009831A0, shared0074D510 selection counter.
    std::int32_t last_mounted_reaction=-1; //0084E744, process lifetime.
};
}
#endif
