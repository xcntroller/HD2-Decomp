#ifndef HD2_FRONTEND_PRE_MISSION_FLOW_H
#define HD2_FRONTEND_PRE_MISSION_FLOW_H

#include "frontend/pre_mission_auto_equipment.h"
#include "frontend/pre_mission_handoff.h"
#include "frontend/pre_mission_resources.h"

namespace hd2::frontend {

enum class PreMissionScreen { none, hero, team, equipment, movie, game_unavailable };
enum class PreMissionAcceptance { accepted, missing_equipment, overloaded, confirm_unarmed };

// The live preparation owner shared by successive controller states. All
// descriptor pointers belong to game_ (or route_'s synthetic mission). Screens
// borrow this owner; a pending game request owns its payload independently.
// No screen index is used as a descriptor or successful world-load result.
template <class Humans> class PreMissionFlow {
  public:
    explicit PreMissionFlow(Humans &humans) : humans_(humans), route_(game_) {}
    PreMissionFlow(const PreMissionFlow &) = delete;
    PreMissionFlow &operator=(const PreMissionFlow &) = delete;

    template <class Files> bool load(Files &files, const std::string &profile_directory) {
        cancel();
        // 006B6530 reloads preparation rather than appending to an old tree.
        //  Route references the stable native GameData object, whose contents
        //  may be replaced only after clearing every borrowed descriptor.
        game_ = GameData{};
        if (!game_.load(files))
            return false;
        for (auto &record : discoveries_)
            record.clear();
        return resources_.load(files, profile_directory, game_);
    }
    bool begin_campaign(std::uint32_t root, std::uint32_t mode) {
        reset_selection();
        if (!route_.campaign(root, mode))
            return false;
        enter(route_.next());
        return screen_ != PreMissionScreen::none;
    }
    bool begin_standalone(std::uint32_t root, std::uint32_t mission, std::uint32_t mode) {
        reset_selection();
        if (!route_.standalone(root, mission, mode))
            return false;
        enter(route_.next());
        return screen_ != PreMissionScreen::none;
    }
    PreMissionScreen screen() const { return screen_; }
    const GameDataNode *descriptor() const { return descriptor_; }
    const PreMissionRoute &route() const { return route_; }
    GameData &game() { return game_; }
    PreMissionResources &resources() { return resources_; }
    const PreMissionResources &resources() const { return resources_; }
    // 006AE510/006A7820: the two transient profile discovery lists are
    // cleared by006B6530. Save-load suppression belongs to the unavailable receiver.
    const std::vector<std::uint32_t> &discoveries(std::uint32_t root) const {
        return discoveries_.at(root == 1 ? 1 : 0);
    }
    PreMissionHero &hero() { return hero_; }
    PreMissionTeam &team() {
        require(PreMissionScreen::team);
        return *team_;
    }
    std::vector<PreMissionSoldier> &squad() { return squad_; }
    const std::vector<PreMissionSoldier> &squad() const { return squad_; }
    PreMissionStore &store() {
        // 00675900 selects application+1C only for campaign subtype4.
        return descriptor_ && !route_.standalone() && descriptor_->kind == 6 && descriptor_->word10 == 4
                   ? secondary_store_
                   : store_;
    }
    PreMissionEquipmentPolicy *equipment_policy() {
        require(PreMissionScreen::equipment);
        return current_policy();
    }
    std::vector<const PreMissionPreset *> available_presets() const {
        std::vector<const PreMissionPreset *> result;
        for (const auto &preset : resources_.presets)
            if (preset.field1ac & 5u)
                result.push_back(&preset);
        for (const auto &preset : resources_.user_presets)
            if (preset.field1ac & 5u)
                result.push_back(&preset);
        return result;
    }
    void auto_equipment(bool initial = false) {
        require(PreMissionScreen::equipment);
        const auto *policy = current_policy();
        if (!policy)
            return;
        prepare_required_equipment();
        const auto presets = available_presets();
        std::vector<float> shooting;
        for (const auto &soldier : squad_) {
            const auto *human = humans_.record(soldier.human);
            if (!human)
                throw std::runtime_error("Missing equipment soldier record");
            shooting.push_back(pre_mission_detail::Bytes::real(human->stats[5]));
        }
        const auto order =
            assign_pre_mission_bundles(squad_, presets, resources_.catalogue, *policy, shooting);
        for (std::size_t i = squad_.size(); i > 0; --i) {
            auto &soldier = squad_[i - 1];
            if (initial && !soldier.auto_equip)
                continue;
            const auto &assignment = policy->assignments[order[i - 1]];
            if (soldier.equipment[2].item < 0 && soldier.equipment[4].item < 0) {
                PreMissionPreset preset;
                if (assignment[0] >= 0 && static_cast<std::size_t>(assignment[0]) < presets.size())
                    preset = *presets[assignment[0]];
                if (assignment[1] >= 0)
                    preset.equipment[0] = assignment[1];
                if (assignment[2] >= 0)
                    preset.equipment[3] = assignment[2];
                const bool force = soldier.auto_equip;
                clear_optional_equipment(i - 1);
                merge_preset(i - 1, preset, force);
            }
            supplement_equipment(i - 1, 0);
            soldier.auto_equip = false;
        }
        if (!initial)
            for (unsigned level : {1u, 2u})
                for (std::size_t i = 0; i < squad_.size(); ++i)
                    supplement_equipment(i, level);
        prepare_required_equipment();
    }
    std::optional<std::int32_t> missing_item() const { return missing_item_; }
    const std::optional<PreMissionGameRequest> &game_request() const { return request_; }

    void accept_hero() {
        require(PreMissionScreen::hero);
        auto *record = humans_.record(0);
        if (!record)
            throw std::runtime_error("Hero preparation requires HumanStore record0");
        hero_.accept(*record);
        enter(route_.next());
    }
    template <class Random> void auto_assign(Random &&random) {
        require(PreMissionScreen::team);
        team_->auto_assign(humans_, std::forward<Random>(random));
    }
    void accept_team() {
        require(PreMissionScreen::team);
        // Continue calls00695B30 directly.00695450 belongs to Automatic Selection;
        // it must not fill empty slots while accepting a manually chosen squad.
        if (!team_->size())
            throw std::runtime_error("No eligible soldiers for the selected mission");
        for (auto &soldier : squad_)
            if (!team_->contains(soldier.human))
                return_equipment(soldier);
        team_->reconcile(squad_, humans_, descriptor_->word10 == 1);
        enter(route_.next());
    }
    PreMissionAcceptance accept_equipment(bool unarmed_confirmed = false) {
        require(PreMissionScreen::equipment);
        missing_item_.reset();
        if (auto *policy = current_policy())
            missing_item_ = missing_pre_mission_equipment(squad_, resources_.catalogue, *policy);
        if (missing_item_)
            return PreMissionAcceptance::missing_equipment;
        if (std::any_of(squad_.begin(), squad_.end(),
                        [](const auto &soldier) { return soldier.capacity_ratio() > 1.f; }))
            return PreMissionAcceptance::overloaded;
        if (!unarmed_confirmed && !pre_mission_squad_has_firearms(squad_, resources_.catalogue))
            return PreMissionAcceptance::confirm_unarmed;
        // 00673DD0 C6 consumes the preparation flag and the SECOND store.
        // 006742EB loads008AE918, not the primary stock at008AE914.
        for (auto &soldier : squad_)
            soldier.auto_equip = false;
        secondary_store_.clear();
        enter(route_.next());
        return PreMissionAcceptance::accepted;
    }
    void complete_movie() {
        require(PreMissionScreen::movie);
        enter(route_.next());
    }
    bool back() {
        if (!descriptor_ || descriptor_->kind != 6)
            return false;
        enter(route_.previous());
        return screen_ != PreMissionScreen::none;
    }
    void cancel() {
        descriptor_ = nullptr;
        route_.clear();
        reset_selection();
    }
    bool equip(std::size_t soldier, std::uint32_t slot, std::int32_t item, std::int16_t count,
               bool replace = false) {
        require(PreMissionScreen::equipment);
        return equip_pre_mission_from_store(squad_.at(soldier), store(), resources_.catalogue, slot, item,
                                            count, false, replace);
    }
    std::int32_t unequip(std::size_t soldier, std::uint32_t slot, PreMissionEquipmentEntry item) {
        require(PreMissionScreen::equipment);
        return return_pre_mission_to_store(squad_.at(soldier), store(), resources_.catalogue, slot, item);
    }
    // 006A3BC0/00671200: transfers between owned inventory locations use
    // the same removal/compatibility/capacity rules as the live store. Restore
    // both owners when no item can be placed; stock never manufactures a unit.
    bool move_equipment(std::size_t source, unsigned source_slot, PreMissionEquipmentEntry entry,
                        std::size_t target, unsigned target_slot) {
        require(PreMissionScreen::equipment);
        if (source == target && source_slot == target_slot)
            return false;
        const auto previous = squad_;
        const auto previous_store = store();
        const auto removed = unequip(source, source_slot, entry);
        if (removed > 0 && equip(target, target_slot, entry.item, static_cast<std::int16_t>(removed), true))
            return true;
        squad_ = previous;
        store() = previous_store;
        return false;
    }
    void use_preset(std::size_t soldier, std::size_t index, bool user) {
        require(PreMissionScreen::equipment);
        const auto &presets = user ? resources_.user_presets : resources_.presets;
        clear_optional_equipment(soldier);
        merge_preset(soldier, presets.at(index), false);
    }
    void clear_optional_equipment(std::size_t index) {
        require(PreMissionScreen::equipment);
        auto &soldier = squad_.at(index);
        const auto remove = [&](unsigned slot, PreMissionEquipmentEntry entry) {
            if (entry.item < 0)
                return;
            if (const auto *policy = current_policy()) {
                const auto found = std::find_if(policy->required.begin(), policy->required.end(),
                                                [&](const auto &item) { return item[0] == entry.item; });
                if (found != policy->required.end()) {
                    int holder = -1;
                    entry.count = static_cast<std::int16_t>(
                        pre_mission_equipment_surplus(squad_, resources_.catalogue, entry.item, found->at(1),
                                                      static_cast<int>(index), holder));
                }
            }
            if (entry.count > 0)
                unequip(index, slot, entry);
        };
        // 00679490 returns only surplus required items. Pack contents precede
        // their pack so a retained required item keeps its necessary container.
        const auto holsters = soldier.holsters, pack = soldier.backpack;
        for (const auto &item : holsters)
            remove(32, item);
        for (const auto &item : pack)
            remove(64, item);
        for (const auto slot :
             std::array<std::pair<unsigned, unsigned>, 5>{{{0, 1}, {4, 16}, {2, 4}, {3, 8}, {1, 2}}})
            remove(slot.second, soldier.equipment[slot.first]);
    }
    template <class Files>
    bool save_user_preset(Files &files, const std::string &directory, std::size_t soldier, std::size_t index,
                          const std::string &name) {
        require(PreMissionScreen::equipment);
        if (name.size() >= 32)
            throw std::length_error("Original preset name exceeds its fixed field");
        auto updated = resources_.user_presets;
        auto &preset = updated.at(index);
        preset = capture_pre_mission_preset(squad_.at(soldier), std::move(preset));
        preset.name = name;
        preset.field1b0 = -1;
        if (!write_pre_mission_user_presets(files, directory, updated))
            return false;
        resources_.user_presets = std::move(updated);
        return true;
    }

  private:
    void merge_preset(std::size_t index, const PreMissionPreset &preset, bool force) {
        auto &soldier = squad_.at(index);
        for (const auto slot :
             std::array<std::pair<unsigned, unsigned>, 5>{{{0, 1}, {1, 4}, {4, 8}, {2, 16}, {3, 2}}})
            if (preset.equipment[slot.first] >= 0)
                equip_pre_mission_from_store(soldier, store(), resources_.catalogue, slot.second,
                                             preset.equipment[slot.first], 1, force);
        for (unsigned carried = 0; carried < 2; ++carried)
            for (const auto &item : carried ? preset.backpack : preset.holsters) {
                if (item[1] < 0 || item[1] > std::numeric_limits<std::int16_t>::max())
                    throw std::runtime_error("Invalid preset stack size");
                if (item[0] >= 0 && item[1])
                    equip_pre_mission_from_store(soldier, store(), resources_.catalogue, carried ? 64 : 32,
                                                 item[0], static_cast<std::int16_t>(item[1]), force);
            }
    }
    // 006777A0 supplies the original three stock-limited ammunition/utility
    // passes: level0 on preparation, then levels1/2 for explicit Auto Equip.
    void supplement_equipment(std::size_t index, unsigned level) {
        if (level > 2)
            throw std::logic_error("Invalid automatic equipment pass");
        auto &soldier = squad_.at(index);
        const auto &catalogue = resources_.catalogue;
        const auto *helmet = catalogue.item(soldier.equipment[1].item);
        const bool restricted = helmet && helmet->restricted_holder();
        const auto allowed = [&](const PreMissionItem *item) {
            return item && (!restricted || item->restricted_holder());
        };
        const auto held = [&](int id) {
            int count = 0;
            for (const auto &entry : soldier.equipment)
                if (entry.item == id)
                    count += entry.count;
            for (const auto &entry : soldier.holsters)
                if (entry.item == id)
                    count += entry.count;
            for (const auto &entry : soldier.backpack)
                if (entry.item == id)
                    count += entry.count;
            return count;
        };
        const auto carry = [&](int id, int limit) {
            const auto *item = catalogue.item(id);
            if (!allowed(item))
                return;
            int remaining = std::min(store().count(id), limit - held(id));
            if (remaining <= 0)
                return;
            for (const auto mask : {32u, 64u})
                if (item->permitted_slots() & mask) {
                    const auto request = std::min<int>(remaining, std::numeric_limits<std::int16_t>::max());
                    const auto before = held(id);
                    equip_pre_mission_from_store(soldier, store(), catalogue, mask, id,
                                                 static_cast<std::int16_t>(request), false);
                    remaining -= held(id) - before;
                    if (remaining <= 0)
                        break;
                }
        };
        if (soldier.equipment[3].item < 0)
            for (int id = 0; id < 500; ++id) {
                const auto *item = catalogue.item(id);
                if (allowed(item) && item->kind() == 17 && store().count(id) > 0 &&
                    equip_pre_mission_from_store(soldier, store(), catalogue, 8, id, 1, false))
                    break;
            }
        const int primary = (level + 6) * level + 6, secondary = (level + 1) * (level + 1);
        const auto ammo = [&](int weapon, int limit) {
            const auto *item = catalogue.item(weapon);
            if (item && item->kind() < 13)
                carry(item->ammunition(), limit);
        };
        ammo(soldier.equipment[2].item, primary);
        int carried_weapon = -1;
        for (const auto &entry : soldier.holsters) {
            const auto *item = catalogue.item(entry.item);
            if (item && item->kind() == 2)
                carried_weapon = entry.item;
        }
        ammo(carried_weapon, primary / 6);
        ammo(soldier.equipment[4].item, secondary);
        const auto *human = humans_.record(soldier.human);
        if (!human)
            throw std::runtime_error("Missing automatic equipment soldier");
        const float aid = pre_mission_detail::Bytes::real(human->stats[8]) * .02f * (level + 1);
        if (!std::isfinite(aid) || aid < 0 || aid >= float(std::numeric_limits<int>::max()))
            throw std::runtime_error("Invalid automatic first-aid capacity");
        for (int id = 0; id < 255; ++id) {
            const auto *item = catalogue.item(id);
            if (item && item->kind() == 13)
                carry(id, static_cast<int>(aid));
        }
        for (int id = 0; id < 100; ++id) {
            const auto *item = catalogue.item(id);
            if (item && item->kind() == 9)
                carry(id, secondary);
        }
        carry(9, 1);
        if (soldier.equipment[0].item < 0 && soldier.equipment[1].item != 0x79)
            for (int id = 0; id < 500; ++id) {
                const auto *item = catalogue.item(id);
                if (allowed(item) && item->kind() == 16 && store().count(id) > 0 &&
                    equip_pre_mission_from_store(soldier, store(), catalogue, 1, id, 1, false))
                    break;
            }
    }
    void prepare_required_equipment() {
        auto *policy = current_policy();
        if (!policy || squad_.empty())
            return;
        int holder = -1;
        std::size_t next = 0;
        for (auto &required : policy->required) {
            const auto *item = resources_.catalogue.item(required[0]);
            if (!item)
                throw std::runtime_error("Required mission item is absent from the catalogue");
            const auto kind = item->kind();
            if (kind == 15 || required[0] == 0xe7)
                required[1] = std::min<int>(required[1], squad_.size());
            int missing = required[1] - pre_mission_equipment_surplus(squad_, resources_.catalogue,
                                                                      required[0], required[1], -1, holder);
            if (missing <= 0)
                continue;
            if (kind == 15 || kind == 16 || kind == 20 || kind < 13)
                next = 0;
            std::size_t failures = 0;
            while (missing > 0 && failures < squad_.size()) {
                bool accepted = false;
                for (unsigned mask : {1u, 2u, 4u, 8u, 16u, 32u, 64u})
                    if ((item->permitted_slots() & mask) &&
                        equip_pre_mission_from_store(squad_[next], store(), resources_.catalogue, mask,
                                                     required[0], 1, true)) {
                        accepted = true;
                        break;
                    }
                next = (next + 1) % squad_.size();
                if (accepted) {
                    --missing;
                    failures = 0;
                } else
                    ++failures;
            }
        }
    }
    void require(PreMissionScreen expected) const {
        if (screen_ != expected)
            throw std::logic_error("Pre-mission action belongs to a different descriptor owner");
    }
    void reset_selection() {
        descriptor_ = nullptr;
        screen_ = PreMissionScreen::none;
        squad_.clear();
        store_.clear();
        secondary_store_.clear();
        team_.reset();
        hero_ = {};
        request_.reset();
        missing_item_.reset();
    }
    void return_equipment(PreMissionSoldier &soldier) {
        for (const auto &entry : soldier.equipment)
            if (entry.item >= 0)
                store_.add(entry.item, entry.count);
        for (const auto &entry : soldier.holsters)
            if (entry.item >= 0)
                store_.add(entry.item, entry.count);
        for (const auto &entry : soldier.backpack)
            if (entry.item >= 0)
                store_.add(entry.item, entry.count);
        soldier.clear_equipment();
    }
    PreMissionEquipmentPolicy *current_policy() {
        const auto location = route_.location();
        if (!location)
            throw std::logic_error("Equipment preparation has no current mission");
        return resources_.policy(route_.root_index(), location->flat_ordinal);
    }
    void enter(const GameDataNode *descriptor) {
        request_.reset();
        team_.reset();
        descriptor_ = descriptor;
        screen_ = PreMissionScreen::none;
        if (!descriptor)
            return;
        auto &discovery = discoveries_[route_.root_index() == 1 ? 1 : 0];
        discovery.insert(discovery.end(), descriptor->items.begin(), descriptor->items.end());
        if (descriptor->kind == 5) {
            screen_ = PreMissionScreen::movie;
            return;
        }
        if (descriptor->kind == 7) {
            request_ = make_pre_mission_game_request(route_, resources_.catalogue, squad_);
            screen_ = PreMissionScreen::game_unavailable;
            return;
        }
        if (descriptor->kind != 6)
            throw std::logic_error("Unexpected pre-mission descriptor kind");
        switch (descriptor->word10) {
        case 0:
            hero_ = {};
            screen_ = PreMissionScreen::hero;
            break;
        case 1:
        case 2:
            if (!route_.standalone()) {
                if (descriptor->word10 == 2) {
                    // 006908F0, table00841CB0. Twelve real rows precede the
                    // following strings; those strings are not extra roster rows.
                    static constexpr std::array<std::array<int, 10>, 12> recruits{
                        {{{-1}},
                         {{-1}},
                         {{7, 9, 11, 16, 17, -1}},
                         {{2, 5, 8, 12, 19, 22, -1}},
                         {{4, 10, 23, 24, 28, 29, -1}},
                         {{3, 6, 13, 21, 25, -1}},
                         {{14, 15, 18, 20, -1}},
                         {{0, 1, 26, 27, -1}},
                         {{-1}},
                         {{-1}},
                         {{-1}},
                         {{-1}}}};
                    const auto campaign = route_.campaign_index();
                    if (!campaign || *campaign >= recruits.size())
                        throw std::runtime_error("Campaign has no original recruitment table row");
                    for (const auto index : recruits[*campaign]) {
                        if (index < 0)
                            break;
                        auto *record = humans_.record(index);
                        if (!record)
                            throw std::runtime_error("Campaign recruit is absent from HumanStore");
                        if (!(record->tail[1] & 0xffu) &&
                            std::find(record->items.begin(), record->items.end(), 6u) == record->items.end())
                            record->items.push_back(6);
                    }
                    for (auto &soldier : squad_)
                        return_equipment(soldier);
                }
                // 00694520 removes unavailable actors on entry; their items
                //  return to the live stock before their native owner is erased.
                for (auto &soldier : squad_) {
                    const auto *record = humans_.record(soldier.human);
                    if (record && (record->tail[1] & 0xffu))
                        return_equipment(soldier);
                }
                squad_.erase(std::remove_if(squad_.begin(), squad_.end(),
                                            [&](const auto &soldier) {
                                                const auto *record = humans_.record(soldier.human);
                                                return record && (record->tail[1] & 0xffu);
                                            }),
                             squad_.end());
            }
            {
                const auto location = route_.location();
                const bool one_soldier =
                    route_.game_mode() == 2 ||
                    (route_.standalone() && route_.root_index() == 0 && location &&
                     (location->flat_ordinal == 2 || location->flat_ordinal == 15)); // 006908B0.
                team_.emplace(one_soldier);
            }
            team_->restore(squad_, descriptor->word10 == 1);
            screen_ = PreMissionScreen::team;
            break;
        case 3:
        case 4:
            prepare_pre_mission_store(store_, *descriptor, resources_.catalogue);
            screen_ = PreMissionScreen::equipment;
            break;
        default:
            throw std::logic_error("Post-mission descriptor requires actual game results");
        }
    }
    Humans &humans_;
    GameData game_;
    PreMissionRoute route_;
    PreMissionResources resources_;
    std::array<std::vector<std::uint32_t>, 2> discoveries_;
    PreMissionHero hero_;
    std::optional<PreMissionTeam> team_;
    std::vector<PreMissionSoldier> squad_;
    PreMissionStore store_, secondary_store_;
    const GameDataNode *descriptor_ = nullptr;
    PreMissionScreen screen_ = PreMissionScreen::none;
    std::optional<PreMissionGameRequest> request_;
    std::optional<std::int32_t> missing_item_;
};

} // namespace hd2::frontend
#endif
