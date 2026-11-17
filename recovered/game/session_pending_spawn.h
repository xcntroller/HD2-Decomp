#ifndef HD2_GAME_SESSION_PENDING_SPAWN_H
#define HD2_GAME_SESSION_PENDING_SPAWN_H
#include "frontend/pre_mission_inventory.h"
#include "game/human_joint_math.h"
#include <memory>
namespace hd2::game {
//0069D9C0's transferred record and two allocations. The game9C/A0 queue owns
//the payload until0072A0B0 kind3 accepts it; the SAME process FIFO then owns it.
//No native address is truncated into a scalar command or packet word.
struct SessionPendingSpawn {
    HumanVector position{},direction{};
    std::unique_ptr<frontend::PreMissionSoldier> soldier;
    std::unique_ptr<std::array<std::uint32_t,12>> stats;
    std::uint32_t peer=0;
    bool serialized=false;
};
}
#endif
