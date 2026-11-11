#ifndef HD2_GAME_HUMAN_PHYSICS_GLOBALS_H
#define HD2_GAME_HUMAN_PHYSICS_GLOBALS_H
#include <cstdint>
namespace hd2::game {
//009832B0 counts actual live physical bindings across the process. Startup
//outlives every world/DD0 and must supply this same counter to all of them.
struct HumanPhysicsGlobals {
    std::uint32_t active=0;
    std::uint32_t camera_projection_tick=0; //009833B0, original Game+28 key.
    float camera_projection_scale=0.f; //009833B4.
};
}
#endif
