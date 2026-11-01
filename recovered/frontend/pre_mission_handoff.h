#ifndef HD2_FRONTEND_PRE_MISSION_HANDOFF_H
#define HD2_FRONTEND_PRE_MISSION_HANDOFF_H

#include "frontend/pre_mission_route.h"
#include "frontend/pre_mission_squad.h"

namespace hd2::frontend {

// Owned request at006C0EF0/006B9190's kind7 boundary. This is a validated
// description of the selected mission and live squad; it never acknowledges
// world loading. Original00698510/006989B0 world receiver remains unavailable.
struct PreMissionGameRequest {
    std::uint32_t root = 0, mode = 0, mission_caption = 0;
    bool standalone = false;
    std::string mission_resource, screenshot;
    std::array<std::uint32_t, 4> mission_words{};
    std::vector<PreMissionSoldier> squad;
    static constexpr bool game_entry_available = false;
};

inline PreMissionGameRequest make_pre_mission_game_request(const PreMissionRoute &route,
                                                           const PreMissionItemCatalogue &catalogue,
                                                           const std::vector<PreMissionSoldier> &squad) {
    const auto *descriptor = route.current();
    const auto *mission = route.mission();
    if (!descriptor || descriptor->kind != 7 || !mission || mission->kind != 3 || mission->text24.empty())
        throw std::logic_error("Game handoff requires an actual selected mission/game descriptor");
    if (!catalogue.loaded())
        throw std::logic_error("Game handoff requires the actual item catalogue");
    if (squad.empty())
        throw std::logic_error("Game handoff requires an actual squad");
    bool active = false;
    for (const auto &soldier : squad) {
        if (soldier.human < 0 || soldier.name.empty())
            throw std::logic_error("Game handoff has an unassigned soldier");
        active = active || soldier.active;
        const auto validate = [&](const PreMissionEquipmentEntry &value) {
            if (value.item == -1) {
                if (value.count)
                    throw std::logic_error("Empty equipment has a nonzero quantity");
                return;
            }
            if (!catalogue.item(value.item) || value.count <= 0)
                throw std::logic_error("Game handoff has invalid equipment");
        };
        for (const auto &value : soldier.equipment)
            validate(value);
        for (const auto &value : soldier.holsters)
            validate(value);
        for (const auto &value : soldier.backpack)
            validate(value);
    }
    if (!active)
        throw std::logic_error("Game handoff has no active soldier");
    return {route.root_index(), route.game_mode(), mission->word8,   route.standalone(),
            mission->text24,    mission->text28,   mission->words40, squad};
}
} // namespace hd2::frontend
#endif
