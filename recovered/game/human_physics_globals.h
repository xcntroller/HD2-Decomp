#ifndef HD2_GAME_HUMAN_PHYSICS_GLOBALS_H
#define HD2_GAME_HUMAN_PHYSICS_GLOBALS_H
#include <cstdint>
namespace hd2::game {
//009832B0 counts actual live physical bindings across the process. Startup
//outlives every world/DD0 and must supply this same counter to all of them.
struct HumanPhysicsGlobals {std::uint32_t active=0;};
}
#endif
