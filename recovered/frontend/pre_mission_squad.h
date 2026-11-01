#ifndef HD2_FRONTEND_PRE_MISSION_SQUAD_H
#define HD2_FRONTEND_PRE_MISSION_SQUAD_H

#include "frontend/pre_mission_inventory.h"
#include <functional>

namespace hd2::frontend {

// 00697A50/00650350/0064FEA0/0064FF80: native hero value storage. Row
//  adjustments spend/refund the shared20-point counter through its linked
//  messages. The nine fields accepted by00696DC0 retain their original mapping.
class PreMissionHero {
  public:
    static constexpr std::array<std::int32_t, 18> bases{
        {80, 75, 60, 50, 75, 30, 80, 40, 75, 70, 80, 50, 80, 30, 70, 40, 50, 60}};
    std::int32_t remaining() const { return points_; }
    std::int32_t value(std::size_t row) const { return bases.at(row) + increments_.at(row); }
    void values(const std::array<std::int32_t, 18> &values) {
        std::array<std::int32_t, 18> increments{};
        std::int32_t spent = 0;
        for (std::size_t row = 0; row < values.size(); ++row) {
            if (values[row] < bases[row] || values[row] > 100)
                throw std::runtime_error("Invalid hero stat value");
            increments[row] = values[row] - bases[row];
            spent += increments[row];
        }
        if (spent > 20)
            throw std::runtime_error("Hero stat point budget exceeded");
        increments_ = increments;
        points_ = 20 - spent;
    }
    bool adjust(std::size_t row, bool increase) {
        auto &increment = increments_.at(row);
        if (increase) {
            if (!points_ || value(row) >= 100)
                return false;
            ++increment;
            --points_;
        } else {
            if (!increment)
                return false;
            --increment;
            ++points_;
        }
        return true;
    }
    void accept(HumanRecord &hero) const {
        constexpr std::array<std::pair<std::size_t, std::size_t>, 9> fields{
            {{0, 1}, {1, 2}, {4, 3}, {5, 7}, {6, 5}, {9, 6}, {10, 4}, {13, 9}, {14, 8}}};
        for (const auto &field : fields) {
            const float number = static_cast<float>(value(field.first));
            std::memcpy(&hero.stats[field.second], &number, 4);
        }
    }

  private:
    std::array<std::int32_t, 18> increments_{};
    std::int32_t points_ = 20;
};

struct PreMissionTeamSlot {
    std::int32_t human = -1;
    bool selectable = true;
};

// 00694C40's ordered selection,00695450 auto selection and00695B30 live
//  reconciliation. HumanStore owns candidate data/portraits; these slots retain
//  original indices. A hidden/locked soldier does not become selectable merely
//  because a native slot can hold an index.
class PreMissionTeam {
  public:
    explicit PreMissionTeam(bool single = false) : single_(single) {}
    const auto &slots() const { return slots_; }
    std::size_t size() const {
        std::size_t count = 0;
        while (count < slots_.size() && slots_[count].human >= 0)
            ++count;
        return count;
    }
    bool contains(std::int32_t human) const {
        return std::any_of(slots_.begin(), slots_.end(),
                           [&](const auto &slot) { return slot.human == human; });
    }
    template <class Humans> bool toggle(std::int32_t human, const Humans &humans) {
        const auto *record = humans.record(human);
        if (!record || human >= 30 || (record->tail[1] & 0xffu))
            return false;
        const auto count = size();
        auto found =
            std::find_if(slots_.begin(), slots_.end(), [&](const auto &slot) { return slot.human == human; });
        if (found != slots_.end()) {
            if (!found->selectable)
                return false;
            const auto offset = static_cast<std::size_t>(found - slots_.begin());
            for (std::size_t i = offset; i + 1 < slots_.size(); ++i)
                slots_[i] = slots_[i + 1];
            slots_.back() = {};
            return true;
        }
        const auto limit = single_ ? 1u : 4u;
        if (count < limit) {
            slots_[count] = {human, true};
            return true;
        }
        if (!slots_[limit - 1].selectable)
            return false;
        slots_[limit - 1] = {human, true};
        return true;
    }
    // 00693C10 swaps complete slot records, including their replacement lock.
    bool reorder(std::size_t from, std::size_t to) {
        if (from >= size() || to >= size() || from == to)
            return false;
        std::swap(slots_[from], slots_[to]);
        return true;
    }
    void restore(const std::vector<PreMissionSoldier> &squad, bool replacement) {
        slots_ = {};
        for (const auto &soldier : squad) {
            if (soldier.human < 0 || contains(soldier.human))
                continue;
            const auto count = size();
            if (count >= (single_ ? 1u : 4u))
                break;
            slots_[count] = {soldier.human, replacement ? !soldier.active : true};
        }
    }
    template <class Humans, class Random> void auto_assign(const Humans &humans, Random &&random) {
        if (single_) {
            if (size())
                return;
            std::vector<std::int32_t> candidates;
            for (std::int32_t i = 0; i < 30; ++i)
                if (available(i, humans))
                    candidates.push_back(i);
            if (candidates.empty())
                throw std::runtime_error("No eligible single-player soldier");
            slots_[0] = {candidates[random() % candidates.size()], true};
            return;
        }
        for (auto slot = size(); slot < 4; ++slot) {
            // 006953E0 retains the first five candidates then replaces the
            //  lowest strictly inferior score. Randomize among those retained.
            std::vector<std::pair<std::int32_t, std::int32_t>> candidates;
            for (std::int32_t i = 0; i < 30; ++i) {
                if (!available(i, humans))
                    continue;
                const auto &record = *humans.record(i);
                const float shooting = pre_mission_detail::Bytes::real(record.stats[5]);
                float score = shooting;
                if (slot == 1)
                    score = pre_mission_detail::Bytes::real(record.stats[8]);
                else if (slot == 3)
                    score = (pre_mission_detail::Bytes::real(record.stats[2]) - shooting * 0.1f) * 10.0f;
                if (!std::isfinite(score) || score >= float(std::numeric_limits<std::int32_t>::max()) ||
                    score < float(std::numeric_limits<std::int32_t>::min()))
                    throw std::runtime_error("Invalid HumanStore assignment score");
                const auto entry = std::make_pair(i, static_cast<std::int32_t>(score));
                if (candidates.size() < 5)
                    candidates.push_back(entry);
                else {
                    auto worst = candidates.end();
                    for (auto at = candidates.begin(); at != candidates.end(); ++at)
                        if (at->second < entry.second &&
                            (worst == candidates.end() || at->second < worst->second))
                            worst = at;
                    if (worst != candidates.end())
                        *worst = entry;
                }
            }
            if (!candidates.empty())
                slots_[size()] = {candidates[random() % candidates.size()].first, true};
        }
    }
    template <class Humans>
    void reconcile(std::vector<PreMissionSoldier> &squad, const Humans &humans, bool replacement) const {
        std::vector<PreMissionSoldier> ordered;
        for (const auto &slot : slots_) {
            if (slot.human < 0)
                break;
            const auto *record = humans.record(slot.human);
            if (!record)
                throw std::runtime_error("Missing selected HumanStore soldier");
            auto found = std::find_if(squad.begin(), squad.end(),
                                      [&](const auto &soldier) { return soldier.human == slot.human; });
            if (found != squad.end()) {
                ordered.push_back(std::move(*found));
                ordered.back().name = record->user_name.empty() ? record->full_name : record->user_name;
            } else {
                PreMissionSoldier soldier;
                soldier.assign(slot.human, *record);
                soldier.auto_equip = replacement;
                ordered.push_back(std::move(soldier));
            }
        }
        for (auto &soldier : ordered)
            if (soldier.specialization < 0) {
                soldier.specialization = -1;
                for (std::int32_t role = 1; role <= 4; ++role)
                    if (std::none_of(ordered.begin(), ordered.end(),
                                     [&](const auto &other) { return other.specialization == role; })) {
                        soldier.specialization = role;
                        break;
                    }
            }
        squad = std::move(ordered);
    }

  private:
    template <class Humans> bool available(std::int32_t index, const Humans &humans) const {
        const auto *record = humans.record(index);
        return record && !(record->tail[1] & 0xffu) && !contains(index);
    }
    std::array<PreMissionTeamSlot, 4> slots_{};
    bool single_;
};
} // namespace hd2::frontend
#endif
