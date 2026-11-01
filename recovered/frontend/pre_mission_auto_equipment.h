#ifndef HD2_FRONTEND_PRE_MISSION_AUTO_EQUIPMENT_H
#define HD2_FRONTEND_PRE_MISSION_AUTO_EQUIPMENT_H
#include "frontend/pre_mission_inventory.h"

namespace hd2::frontend {

// 0067A0F0: assign the policy's equipment bundles to live soldiers. Bundle
// indices address006755F0's mask5 preset view, rather than the unfiltered file.
// Shooting chooses the best and second-best soldiers; remaining bundles use
// weight and firearm/rifle preference. Original comparisons are strict.
inline std::vector<std::size_t>
assign_pre_mission_bundles(const std::vector<PreMissionSoldier> &squad,
                           const std::vector<const PreMissionPreset *> &presets,
                           const PreMissionItemCatalogue &catalogue, const PreMissionEquipmentPolicy &policy,
                           const std::vector<float> &shooting) {
    const auto count = squad.size();
    if (count > policy.assignments.size() || shooting.size() != count)
        throw std::runtime_error("Invalid policy squad size");
    struct Bundle {
        float weight = 0;
        unsigned flags = 0;
        bool used = false;
    };
    std::vector<Bundle> bundles(count);
    unsigned rifles = 0;
    const auto add = [&](Bundle &bundle, int item, int amount = 1) {
        if (const auto *value = catalogue.item(item))
            bundle.weight += value->weight * amount;
    };
    for (std::size_t i = 0; i < count; ++i) {
        const auto &assignment = policy.assignments[i];
        auto &bundle = bundles[i];
        const auto *preset = assignment[0] >= 0 && static_cast<std::size_t>(assignment[0]) < presets.size()
                                 ? presets[assignment[0]]
                                 : nullptr;
        add(bundle, assignment[1] >= 0 ? assignment[1] : preset ? preset->equipment[0] : -1);
        add(bundle, assignment[2] >= 0 ? assignment[2] : preset ? preset->equipment[3] : -1);
        if (!preset)
            continue;
        for (unsigned slot : {1u, 2u}) {
            add(bundle, preset->equipment[slot]);
            if (const auto *weapon = catalogue.item(preset->equipment[slot]); weapon && weapon->kind() == 4) {
                bundle.flags |= 2;
                const auto &payload = weapon->payloads[0];
                // Serialized tag5 maps to runtime type4; its first payload
                // word is the +0C rifle subtype read by0067A0F0.
                if (payload.type == 5 && payload.bytes.size() >= 4 &&
                    pre_mission_detail::Bytes::word(payload.bytes.data()) < 4) {
                    bundle.flags |= 1;
                    ++rifles;
                }
            }
        }
        add(bundle, preset->equipment[4]);
        for (const auto &entry : preset->holsters)
            add(bundle, entry[0], entry[1]);
        for (const auto &entry : preset->backpack)
            add(bundle, entry[0], entry[1]);
    }
    std::size_t best = count, second = count;
    int best_score = -1, second_score = -1;
    for (std::size_t i = 0; i < count; ++i) {
        if (!std::isfinite(shooting[i]) || shooting[i] >= float(std::numeric_limits<int>::max()) ||
            shooting[i] < float(std::numeric_limits<int>::min()))
            throw std::runtime_error("Invalid equipment assignment score");
        if (shooting[i] > best_score) {
            second = best;
            second_score = best_score;
            best = i;
            best_score = static_cast<int>(shooting[i]);
        } else if (shooting[i] > second_score) {
            second = i;
            second_score = static_cast<int>(shooting[i]);
        }
    }
    std::vector<std::size_t> order(count);
    for (std::size_t soldier = 0; soldier < count; ++soldier) {
        std::size_t selected = count;
        int selected_score = -1;
        for (std::size_t candidate = 0; candidate < count; ++candidate) {
            const auto &bundle = bundles[candidate];
            if (bundle.used)
                continue;
            if (selected == count) {
                selected = candidate;
                selected_score = 0;
            }
            const bool fits = bundle.weight <= squad[soldier].capacity;
            if (soldier == best && ((bundle.flags & 1) || ((bundle.flags & 2) && !rifles)) && fits) {
                selected = candidate;
                selected_score = 100;
            } else if (soldier == second && (bundle.flags & 2) && (!(bundle.flags & 1) || rifles > 1) &&
                       fits) {
                selected = candidate;
                selected_score = 90;
            } else if (fits && !(bundle.flags & 1)) {
                const int score = static_cast<int>(shooting[soldier]) - ((bundle.flags & 2) ? 30 : 0);
                if (selected_score < score) {
                    selected = candidate;
                    selected_score = score;
                }
            }
        }
        if (selected == count)
            throw std::logic_error("Equipment bundle assignment exhausted");
        bundles[selected].used = true;
        order[soldier] = count == 1 ? 0 : selected;
    }
    return order;
}
} // namespace hd2::frontend
#endif
