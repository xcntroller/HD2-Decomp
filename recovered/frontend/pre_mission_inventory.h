#ifndef HD2_FRONTEND_PRE_MISSION_INVENTORY_H
#define HD2_FRONTEND_PRE_MISSION_INVENTORY_H

#include "frontend/game_data.h"
#include "frontend/human_store.h"
#include "frontend/pre_mission_data.h"
#include <cmath>

namespace hd2::frontend {

// Original006739F0's soldier and six-byte inventory records, represented with
// native ownership. Store/grid coordinates are independent of item identity.
struct PreMissionEquipmentEntry {
    std::int16_t position = -1, item = -1, count = 0;
};
struct PreMissionSoldier {
    std::string name;
    std::int32_t human = -1, specialization = -1;
    bool active = true, auto_equip = false;
    std::array<PreMissionEquipmentEntry, 5> equipment{}; // masks1,2,4,8,16
    std::vector<PreMissionEquipmentEntry> holsters, backpack;
    std::int32_t capacity = 0, pack_volume = 0, holster_volume = 0;
    float load = 0, pack_capacity = 0, pack_load = 0, holster_load = 0;

    // 006A2340: the live soldier borrows its actual HumanStore identity and
    //  truncates Might(+4C) to its integer carrying capacity.
    void assign(std::int32_t index, const HumanRecord &record) {
        human = index;
        name = record.user_name.empty() ? record.full_name : record.user_name;
        const auto might = pre_mission_detail::Bytes::real(record.stats[2]);
        if (!std::isfinite(might) || might < 0 || might >= float(std::numeric_limits<std::int32_t>::max()))
            throw std::runtime_error("Invalid HumanStore carrying capacity");
        capacity = static_cast<std::int32_t>(might);
    }
    void clear_equipment() { // 006A27C0
        equipment = {};
        holsters.clear();
        backpack.clear();
        load = pack_capacity = pack_load = holster_load = 0;
        pack_volume = holster_volume = 0;
    }
    float capacity_ratio() const { // 006A2330: signed integer+8C, float+90.
        return std::fabs(load / static_cast<float>(capacity));
    }
};

class PreMissionStore {
  public:
    std::int32_t count(std::int32_t id) const { // 006A5C30
        return id >= 0 && static_cast<std::size_t>(id) < counts_.size() ? counts_[id] : 0;
    }
    void add(std::int32_t id, std::int32_t count) { // 006A5AA0
        if (id < 0)
            return;
        if (id >= 500)
            throw std::runtime_error("Invalid store item index");
        if (static_cast<std::size_t>(id) >= counts_.size())
            counts_.resize(id + 1);
        const auto sum = std::int64_t{counts_[id]} + count;
        if (sum < std::numeric_limits<std::int32_t>::min() || sum > std::numeric_limits<std::int32_t>::max())
            throw std::overflow_error("Store quantity overflow");
        counts_[id] = static_cast<std::int32_t>(sum);
    }
    std::int32_t take(std::int32_t id, std::int32_t requested) { // 006A5B20
        if (id < 0 || static_cast<std::size_t>(id) >= counts_.size())
            return 0;
        const auto accepted = std::min(counts_[id], requested);
        counts_[id] -= accepted;
        return accepted;
    }
    void clear() { counts_.clear(); }

  private:
    std::vector<std::int32_t> counts_;
};

// 006736D0: only subtype3 replaces stock; subtype4 retains it. Entry byte12
//  selects the associated ammunition ID rather than the item's own ID.
inline void prepare_pre_mission_store(PreMissionStore &store, const GameDataNode &descriptor,
                                      const PreMissionItemCatalogue &catalogue) {
    if (descriptor.kind != 6)
        throw std::logic_error("Equipment preparation needs a setup descriptor");
    if (descriptor.word10 != 3)
        return;
    store.clear();
    for (const auto &entry : descriptor.entries) {
        const auto identifier =
            pre_mission_detail::Bytes::integer(pre_mission_detail::Bytes::word(entry.data()));
        const auto count =
            pre_mission_detail::Bytes::integer(pre_mission_detail::Bytes::word(entry.data() + 4));
        if (!entry[12])
            store.add(identifier, count);
        else {
            const auto *item = catalogue.item(identifier);
            if (item)
                store.add(item->ammunition(), count);
        }
    }
}

// 006A3080's native transfer. Caller owns slot compatibility and store balance
//  (00671200/0067BDD0/BDA0). Return is the unchanged, residual or swapped entry.
inline PreMissionEquipmentEntry transfer_pre_mission_equipment(PreMissionSoldier &soldier,
                                                               const PreMissionItemCatalogue &catalogue,
                                                               std::uint32_t mask,
                                                               PreMissionEquipmentEntry incoming) {
    const auto *item = catalogue.item(incoming.item);
    if (!item || incoming.count <= 0)
        return incoming;
    if ((item->permitted_slots() & mask) == 0)
        return incoming;
    const auto fixed_index = [](std::uint32_t slot) -> int {
        switch (slot) {
        case 1:
            return 0;
        case 2:
            return 1;
        case 4:
            return 2;
        case 8:
            return 3;
        case 16:
            return 4;
        default:
            return -1;
        }
    };
    const auto index = fixed_index(mask);
    if (index >= 0) {
        auto &destination = soldier.equipment[index];
        const auto *old = catalogue.item(destination.item);
        const float old_weight = old ? old->weight : 0;
        if (!((soldier.load + item->weight) - old_weight <= static_cast<float>(soldier.capacity)))
            return incoming;
        float pack_capacity = 0;
        std::int32_t pack_volume = 0;
        if (mask == 8) {
            const auto &payload = item->payloads[0];
            if (payload.type == 8 && payload.bytes.size() >= 20) {
                pack_capacity = pre_mission_detail::Bytes::real(
                    pre_mission_detail::Bytes::word(payload.bytes.data() + 12));
                const float volume = pre_mission_detail::Bytes::real(
                    pre_mission_detail::Bytes::word(payload.bytes.data() + 16));
                if (!std::isfinite(volume) || volume < 0 ||
                    volume >= float(std::numeric_limits<std::int32_t>::max()))
                    throw std::runtime_error("Invalid backpack volume");
                pack_volume = static_cast<std::int32_t>(volume);
            }
            if (!(soldier.pack_load <= pack_capacity) || soldier.pack_volume > pack_volume)
                return incoming;
        }
        const auto previous = destination;
        destination = incoming;
        soldier.load = (item->weight - old_weight) + soldier.load;
        if (mask == 8) {
            soldier.pack_capacity = pack_capacity;
        }
        // Original+98 stores available pack volume, while+ A0 stores used volume.
        // Native pack_volume is used volume; keep the available limit on item.
        return previous;
    }
    if (mask != 0x20 && mask != 0x40)
        return incoming;
    auto &entries = mask == 0x20 ? soldier.holsters : soldier.backpack;
    const std::int32_t units = item->category > 0 ? static_cast<std::int32_t>(item->category * 2) : 1;
    auto found = std::find_if(entries.begin(), entries.end(),
                              [&](const auto &entry) { return entry.item == incoming.item; });
    std::size_t selected =
        found == entries.end() ? entries.size() : static_cast<std::size_t>(found - entries.begin());
    std::int32_t pack_limit = 0;
    if (mask == 0x40) {
        const auto *pack = catalogue.item(soldier.equipment[3].item);
        if (pack && pack->payloads[0].type == 8 && pack->payloads[0].bytes.size() >= 20) {
            const auto value = pre_mission_detail::Bytes::real(
                pre_mission_detail::Bytes::word(pack->payloads[0].bytes.data() + 16));
            if (!std::isfinite(value) || value < 0 ||
                value >= float(std::numeric_limits<std::int32_t>::max()))
                throw std::runtime_error("Invalid backpack volume");
            pack_limit = static_cast<std::int32_t>(value);
        }
    }
    while (incoming.count > 0) {
        if (!(soldier.load + item->weight <= static_cast<float>(soldier.capacity)))
            break;
        if (mask == 0x20) {
            if (std::int64_t{soldier.holster_volume} + units > 12)
                break; // 00863758
        } else if (!(soldier.pack_load + item->weight <= soldier.pack_capacity) ||
                   std::int64_t{soldier.pack_volume} + units > pack_limit)
            break;
        if (selected == entries.size())
            entries.push_back({-1, incoming.item, 0});
        if (entries[selected].count == std::numeric_limits<std::int16_t>::max())
            break;
        ++entries[selected].count;
        --incoming.count;
        soldier.load += item->weight;
        if (mask == 0x20) {
            soldier.holster_load += item->weight;
            soldier.holster_volume += units;
        } else {
            soldier.pack_load += item->weight;
            soldier.pack_volume += units;
        }
    }
    if (!incoming.count)
        incoming.item = -1;
    return incoming;
}

// 006A3BC0/006A3990: remove up to the requested quantity, updating the
//  caller's record to the actual removed count. Pack removal requires an empty
//  pack. Empty soldiers clear all derived load/capacity bookkeeping.
inline std::int32_t remove_pre_mission_equipment(PreMissionSoldier &soldier,
                                                 const PreMissionItemCatalogue &catalogue, std::uint32_t mask,
                                                 PreMissionEquipmentEntry &requested) {
    const auto *item = catalogue.item(requested.item);
    if (!item || requested.count <= 0)
        return 0;
    PreMissionEquipmentEntry *entry = nullptr;
    std::vector<PreMissionEquipmentEntry> *entries = nullptr;
    switch (mask) {
    case 1:
        entry = &soldier.equipment[0];
        break;
    case 2:
        entry = &soldier.equipment[1];
        break;
    case 4:
        entry = &soldier.equipment[2];
        break;
    case 8:
        if (!soldier.backpack.empty())
            return 0;
        entry = &soldier.equipment[3];
        break;
    case 16:
        entry = &soldier.equipment[4];
        break;
    case 32:
        entries = &soldier.holsters;
        break;
    case 64:
        entries = &soldier.backpack;
        break;
    default:
        return 0;
    }
    if (entries) {
        const auto found = std::find_if(entries->begin(), entries->end(),
                                        [&](const auto &value) { return value.item == requested.item; });
        if (found == entries->end())
            return 0;
        entry = &*found;
    }
    if (entry->item != requested.item)
        return 0;
    requested.count = std::min(requested.count, entry->count);
    entry->count -= requested.count;
    if (!entry->count)
        *entry = {};
    const auto units = item->category > 0 ? static_cast<std::int32_t>(item->category * 2) : 1;
    if (mask == 32) {
        soldier.holster_load -= requested.count * item->weight;
        soldier.holster_volume -= requested.count * units;
    } else if (mask == 64) {
        soldier.pack_load -= requested.count * item->weight;
        soldier.pack_volume -= requested.count * units;
    } else if (mask == 8) {
        soldier.pack_capacity = 0;
    }
    if (entries)
        entries->erase(std::remove_if(entries->begin(), entries->end(),
                                      [](const auto &value) { return value.item == -1; }),
                       entries->end());
    soldier.load = std::fabs(soldier.load - requested.count * item->weight);
    if (soldier.holsters.empty()) {
        soldier.holster_load = 0;
        soldier.holster_volume = 0;
    }
    if (soldier.backpack.empty()) {
        soldier.pack_load = 0;
        soldier.pack_volume = 0;
    }
    const auto fixed_empty = std::all_of(soldier.equipment.begin(), soldier.equipment.end(),
                                         [](const auto &value) { return value.item == -1; });
    if (fixed_empty && soldier.holsters.empty() && soldier.backpack.empty()) {
        soldier.load = soldier.pack_capacity = soldier.pack_load = soldier.holster_load = 0;
        soldier.pack_volume = soldier.holster_volume = 0;
    }
    return requested.count;
}

// 0067BDD0/00677620/0067BDA0: withdrawals are balanced even when a transfer
//  rejects an item, accepts only part of a stack or swaps an equipped item.
inline bool equip_pre_mission_from_store(PreMissionSoldier &soldier, PreMissionStore &store,
                                         const PreMissionItemCatalogue &catalogue, std::uint32_t mask,
                                         std::int32_t identifier, std::int16_t requested, bool force,
                                         bool replace = false) {
    if (!catalogue.item(identifier) || requested <= 0)
        return false;
    if (force && store.count(identifier) < requested)
        store.add(identifier, requested - store.count(identifier));
    const auto taken = store.take(identifier, requested);
    if (taken <= 0)
        return false;
    const PreMissionEquipmentEntry incoming{-1, static_cast<std::int16_t>(identifier),
                                            static_cast<std::int16_t>(taken)};
    const auto fixed_index = mask == 1    ? 0
                             : mask == 2  ? 1
                             : mask == 4  ? 2
                             : mask == 8  ? 3
                             : mask == 16 ? 4
                                          : -1;
    auto residual = incoming;
    if (replace || fixed_index < 0 || soldier.equipment[fixed_index].item == -1)
        residual = transfer_pre_mission_equipment(soldier, catalogue, mask, incoming);
    if (residual.item != -1 && residual.count != 0)
        store.add(residual.item, residual.count);
    return residual.item != identifier || residual.count < taken;
}

inline std::int32_t return_pre_mission_to_store(PreMissionSoldier &soldier, PreMissionStore &store,
                                                const PreMissionItemCatalogue &catalogue, std::uint32_t mask,
                                                PreMissionEquipmentEntry requested) {
    const auto removed = remove_pre_mission_equipment(soldier, catalogue, mask, requested);
    if (removed > 0)
        store.add(requested.item, requested.count);
    return removed;
}

// 00677D00's preset fields follow serialization order rather than native slot
//  mask order: uniform, primary, secondary, headgear, backpack. Keep the two
//  carried vectors independent and retain the preset's category/name metadata.
inline PreMissionPreset capture_pre_mission_preset(const PreMissionSoldier &soldier,
                                                   PreMissionPreset preset) {
    constexpr std::array<std::size_t, 5> slots{{0, 2, 4, 1, 3}};
    for (std::size_t i = 0; i < slots.size(); ++i)
        preset.equipment[i] = soldier.equipment[slots[i]].item;
    preset.holsters.clear();
    preset.backpack.clear();
    for (const auto &entry : soldier.holsters)
        preset.holsters.push_back({entry.item, entry.count});
    for (const auto &entry : soldier.backpack)
        preset.backpack.push_back({entry.item, entry.count});
    if (preset.holsters.size() > 14 || preset.backpack.size() > 32)
        throw std::runtime_error("Equipment exceeds original preset storage");
    return preset;
}

inline void apply_pre_mission_preset(PreMissionSoldier &soldier, PreMissionStore &store,
                                     const PreMissionItemCatalogue &catalogue, const PreMissionPreset &preset,
                                     bool force = false) {
    // 00679490 returns the old inventory before applying00677620 transfers.
    //  Return the pack contents before the pack itself; all withdrawals retain
    //  the same compatibility, stock, quantity and capacity checks as dragging.
    for (const auto &entry : soldier.equipment)
        if (entry.item >= 0)
            store.add(entry.item, entry.count);
    for (const auto &entry : soldier.holsters)
        if (entry.item >= 0)
            store.add(entry.item, entry.count);
    for (const auto &entry : soldier.backpack)
        if (entry.item >= 0)
            store.add(entry.item, entry.count);
    soldier.clear_equipment();
    for (const auto &slot :
         std::array<std::pair<std::size_t, unsigned>, 5>{{{0, 1}, {3, 2}, {1, 4}, {4, 8}, {2, 16}}})
        if (preset.equipment[slot.first] >= 0)
            equip_pre_mission_from_store(soldier, store, catalogue, slot.second, preset.equipment[slot.first],
                                         1, force);
    const auto carried = [&](const auto &entries, unsigned mask) {
        for (const auto &entry : entries) {
            if (entry[1] < 0 || entry[1] > std::numeric_limits<std::int16_t>::max())
                throw std::runtime_error("Invalid preset carried quantity");
            if (entry[0] >= 0 && entry[1])
                equip_pre_mission_from_store(soldier, store, catalogue, mask, entry[0],
                                             static_cast<std::int16_t>(entry[1]), force);
        }
    };
    carried(preset.holsters, 32);
    carried(preset.backpack, 64);
}

// 00679060. The restricted holder survives across required-item queries in a
//  validation pass (00840D54); it is reset once by00678AC0, not once per item.
inline std::int32_t pre_mission_equipment_surplus(const std::vector<PreMissionSoldier> &squad,
                                                  const PreMissionItemCatalogue &catalogue,
                                                  std::int32_t item_id, std::int32_t required,
                                                  std::int32_t requested_holder,
                                                  std::int32_t &restricted_holder) {
    const auto *item = catalogue.item(item_id);
    if (!item)
        throw std::runtime_error("Missing required mission item");
    std::int32_t total = 0, held = 0;
    for (std::size_t i = 0; i != squad.size(); ++i) {
        const auto &soldier = squad[i];
        if (!soldier.active)
            continue;
        const auto count = [&](const PreMissionEquipmentEntry &value, bool fixed) {
            if (value.item != item_id)
                return;
            if (item->restricted_holder()) {
                if (restricted_holder != -1 && restricted_holder != static_cast<std::int32_t>(i))
                    return;
                restricted_holder = static_cast<std::int32_t>(i);
            }
            total += fixed ? 1 : value.count;
            if (requested_holder == static_cast<std::int32_t>(i))
                held += value.count;
        };
        // Original fixed-slot order differs from native slot-mask order.
        for (const auto slot : {0, 1, 3, 4, 2})
            count(soldier.equipment[slot], true);
        for (const auto &value : soldier.holsters)
            count(value, false);
        for (const auto &value : soldier.backpack)
            count(value, false);
    }
    if (total < required)
        return 0;
    if (requested_holder < 0)
        return total;
    return std::min(total - required, held);
}

// 00678AC0: return the first missing item for the actual localized failure
//  dialog. The original clamps the stored requirement in place for kind15/E7.
inline std::optional<std::int32_t> missing_pre_mission_equipment(std::vector<PreMissionSoldier> &squad,
                                                                 const PreMissionItemCatalogue &catalogue,
                                                                 PreMissionEquipmentPolicy &policy) {
    std::int32_t holder = -1;
    for (auto &required : policy.required) {
        const auto *item = catalogue.item(required[0]);
        if (!item)
            throw std::runtime_error("Missing required mission item");
        if (item->kind() == 15 || required[0] == 0xe7)
            required[1] = std::min(required[1], static_cast<std::int32_t>(squad.size()));
        if (!pre_mission_equipment_surplus(squad, catalogue, required[0], required[1], -1, holder))
            return required[0];
    }
    return std::nullopt;
}

// 006743F0: every active soldier needs a class1 item in the weapon slots or
//  either carried vector. Inactive soldiers are excluded from the check.
inline bool pre_mission_squad_has_firearms(const std::vector<PreMissionSoldier> &squad,
                                           const PreMissionItemCatalogue &catalogue) {
    const auto firearm = [&](const PreMissionEquipmentEntry &entry) {
        const auto *item = catalogue.item(entry.item);
        return item && item->item_class == 1;
    };
    for (const auto &soldier : squad)
        if (soldier.active) {
            if (firearm(soldier.equipment[2]) || firearm(soldier.equipment[4]))
                continue;
            if (std::any_of(soldier.holsters.begin(), soldier.holsters.end(), firearm) ||
                std::any_of(soldier.backpack.begin(), soldier.backpack.end(), firearm))
                continue;
            return false;
        }
    return true;
}
} // namespace hd2::frontend
#endif
