#ifndef HD2_FRONTEND_PRE_MISSION_ROUTE_H
#define HD2_FRONTEND_PRE_MISSION_ROUTE_H

#include "frontend/game_data.h"
#include <cstddef>
#include <utility>

namespace hd2::frontend {

// Native descriptor traversal for original container vtable18/1C/20/24.
// Nodes retain the original cursor sentinel, including the container itself at
// entry/end. Movies and setup markers are leaves; related objective nodes are
// metadata and never inserted into the traversal.
inline bool advance_pre_mission_node(GameDataNode &node) {
    if (node.kind < 1 || node.kind > 3)
        return false;
    if (node.word20 == 0xffffffffu) {
        node.word20 = 0;
        return true;
    }
    if (node.word20 < node.children.size()) {
        if (advance_pre_mission_node(*node.children[node.word20]))
            return true;
        ++node.word20;
        return node.word20 < node.children.size();
    }
    return false;
}
inline bool reverse_pre_mission_node(GameDataNode &node) {
    if (node.kind < 1 || node.kind > 3 || node.word20 == 0xffffffffu)
        return false;
    if (node.word20 > node.children.size())
        throw std::runtime_error("Invalid GameData traversal cursor");
    if (node.word20 == node.children.size()) {
        if (node.children.empty()) {
            node.word20 = 0xffffffffu;
            return true;
        }
        --node.word20;
        reverse_pre_mission_node(*node.children[node.word20]);
        return true;
    }
    if (!reverse_pre_mission_node(*node.children[node.word20])) {
        --node.word20;
        if (node.word20 != 0xffffffffu)
            reverse_pre_mission_node(*node.children[node.word20]);
    }
    return true;
}
inline GameDataNode &current_pre_mission_node(GameDataNode &node) {
    if (node.kind >= 1 && node.kind <= 3 && node.word20 < node.children.size())
        return current_pre_mission_node(*node.children[node.word20]);
    return node;
}

inline std::unique_ptr<GameDataNode> copy_pre_mission_node(const GameDataNode &source) {
    auto result = std::make_unique<GameDataNode>(source.kind);
    result->word8 = source.word8;
    result->word10 = source.word10;
    result->word20 = source.word20;
    result->flag14 = source.flag14;
    result->flag15 = source.flag15;
    result->flag18 = source.flag18;
    result->flag2c = source.flag2c;
    result->text10 = source.text10;
    result->text14 = source.text14;
    result->text24 = source.text24;
    result->text28 = source.text28;
    result->words40 = source.words40;
    result->entries = source.entries;
    result->items = source.items;
    for (const auto &child : source.children)
        result->children.push_back(copy_pre_mission_node(*child));
    for (const auto &child : source.related)
        result->related.push_back(copy_pre_mission_node(*child));
    return result;
}

struct PreMissionLocation {
    GameDataNode *campaign = nullptr;
    GameDataNode *mission = nullptr;
    std::uint32_t campaign_ordinal = 0, mission_ordinal = 0, flat_ordinal = 0;
};

// 006B8660/006B8510/006B8420: count only actual campaign/mission nodes, even
//  when movie/setup nodes precede them. Returned pointers are borrowed from the
//  GameData owner, which must outlive the route and every pending descriptor.
inline std::optional<PreMissionLocation> find_pre_mission_location(GameData &game, std::uint32_t root,
                                                                   std::uint32_t ordinal) {
    if (root >= game.roots().size())
        return std::nullopt;
    std::uint32_t campaign_ordinal = 0, flat = 0;
    for (const auto &campaign : game.roots()[root]->children) {
        if (campaign->kind != 2)
            continue;
        std::uint32_t local = 0;
        for (const auto &mission : campaign->children) {
            if (mission->kind != 3)
                continue;
            if (flat == ordinal)
                return PreMissionLocation{campaign.get(), mission.get(), campaign_ordinal, local, flat};
            ++flat;
            ++local;
        }
        ++campaign_ordinal;
    }
    return std::nullopt;
}

class PreMissionRoute {
  public:
    explicit PreMissionRoute(GameData &game) : game_(game) {}
    bool standalone() const { return synthetic_ != nullptr; }
    std::uint32_t root_index() const { return root_index_; }
    std::uint32_t game_mode() const { return game_mode_; }
    const GameDataNode *current() const { return root_ ? &current_pre_mission_node(*root_) : nullptr; }
    std::optional<PreMissionLocation> location() const {
        if (synthetic_)
            return selected_;
        const auto *selected = mission();
        const auto count = game_.mission_count(root_index_);
        for (std::int32_t i = 0; selected && i < count; ++i) {
            auto location = find_pre_mission_location(game_, root_index_, i);
            if (location && location->mission == selected)
                return location;
        }
        return std::nullopt;
    }
    std::optional<std::uint32_t> campaign_index() const {
        if (synthetic_)
            return selected_.campaign_ordinal;
        if (!root_ || root_->word20 >= root_->children.size() || root_->children[root_->word20]->kind != 2)
            return std::nullopt;
        std::uint32_t count = 0;
        for (std::size_t i = 0; i < root_->word20; ++i)
            if (root_->children[i]->kind == 2)
                ++count;
        return count;
    }

    // Start from the real loaded cursors. Caller00638D30 owns resource/profile
    // preparation and any required gameplay-save load; this service neither
    // fabricates progress nor silently positions a later campaign mission.
    bool campaign(std::uint32_t root, std::uint32_t mode) {
        clear();
        if (root >= game_.roots().size() || mode < 1 || mode > 3)
            return false;
        root_index_ = root;
        game_mode_ = mode;
        root_ = game_.roots()[root].get();
        // 006B8A20: seek the next campaign/mission container, leaving it current.
        while (advance_pre_mission_node(*root_)) {
            const auto kind = current_pre_mission_node(*root_).kind;
            if (kind == 2 || kind == 3)
                return true;
            if (kind < 2)
                break;
        }
        clear();
        return false;
    }

    // 006B9680: copied mission metadata/objectives, eligible mission movies,
    //  fresh team/equipment markers, game entry, debrief and terminal marker.
    //  The source GameData is unchanged. Standalone equipment accumulates all
    //  kind6 records from the selected mission backwards within its campaign.
    bool standalone(std::uint32_t root, std::uint32_t ordinal, std::uint32_t mode) {
        clear();
        const auto location = find_pre_mission_location(game_, root, ordinal);
        if (!location || (mode != 0 && mode != 3))
            return false;
        root_index_ = root;
        game_mode_ = mode;
        selected_ = *location;
        auto node = std::make_unique<GameDataNode>(3);
        node->word8 = location->mission->word8;
        node->text24 = location->mission->text24;
        node->text28 = location->mission->text28;
        node->flag2c = location->mission->flag2c;
        node->words40 = location->mission->words40;
        for (const auto &objective : location->mission->related)
            node->related.push_back(copy_pre_mission_node(*objective));
        for (const auto &step : location->mission->children)
            if (step->kind == 5 && step->flag18)
                node->children.push_back(copy_pre_mission_node(*step));
        auto team = std::make_unique<GameDataNode>(6);
        team->word10 = 2;
        node->children.push_back(std::move(team));
        GameDataNode *equipment = node->children.back().get();
        if (ordinal != 10) {
            auto items = std::make_unique<GameDataNode>(6);
            items->word10 = 3;
            items->flag14 = 1;
            equipment = items.get();
            node->children.push_back(std::move(items));
        }
        std::vector<GameDataNode *> missions;
        for (const auto &mission : location->campaign->children)
            if (mission->kind == 3)
                missions.push_back(mission.get());
        for (std::size_t i = location->mission_ordinal + 1; i != 0; --i)
            for (const auto &step : missions.at(i - 1)->children)
                if (step->kind == 6)
                    equipment->entries.insert(equipment->entries.end(), step->entries.begin(),
                                              step->entries.end());
        node->children.push_back(std::make_unique<GameDataNode>(7));
        auto debrief = std::make_unique<GameDataNode>(6);
        debrief->word10 = 5;
        node->children.push_back(std::move(debrief));
        node->children.push_back(std::make_unique<GameDataNode>(0));
        synthetic_ = std::move(node);
        root_ = synthetic_.get();
        return true;
    }

    // 006B8AA0 plus006C20F0's Lone Wolf subtype1 skip. Containers are state
    //  boundaries, not screens. End-of-tree returns null, never a game success.
    const GameDataNode *next() {
        if (!root_)
            return nullptr;
        while (advance_pre_mission_node(*root_)) {
            auto &node = current_pre_mission_node(*root_);
            if (node.kind == 2 || node.kind == 3)
                continue;
            if (node.kind != 5 && node.kind != 6 && node.kind != 7)
                return nullptr;
            if (game_mode_ == 2 && node.kind == 6 && node.word10 == 1)
                continue;
            return &node;
        }
        return nullptr;
    }
    // 006B8B20/006C20F0: current kind6's flag14 controls crossing a container.
    const GameDataNode *previous() {
        if (!root_)
            return nullptr;
        const auto *from = &current_pre_mission_node(*root_);
        if (from->kind != 6)
            return nullptr;
        while (reverse_pre_mission_node(*root_)) {
            auto &node = current_pre_mission_node(*root_);
            if (node.kind == 6) {
                if (game_mode_ == 2 && node.word10 == 1) {
                    from = &node;
                    continue;
                }
                return &node;
            }
            if ((node.kind == 2 || node.kind == 3) && from->flag14)
                continue;
            if (node.kind == 4 || node.kind == 5 || node.kind == 7)
                continue;
            return nullptr;
        }
        return nullptr;
    }
    // 006B9190's mission handoff, including the synthetic standalone mission.
    const GameDataNode *mission() const {
        if (!root_)
            return nullptr;
        if (synthetic_)
            return synthetic_.get();
        auto *node = root_;
        while (node->kind >= 1 && node->kind <= 3 && node->word20 < node->children.size()) {
            if (node->kind == 3)
                return node;
            node = node->children[node->word20].get();
        }
        return node->kind == 3 ? node : nullptr;
    }
    void clear() {
        root_ = nullptr;
        synthetic_.reset();
        selected_ = {};
        root_index_ = game_mode_ = 0;
    }

  private:
    GameData &game_;
    GameDataNode *root_ = nullptr;
    std::unique_ptr<GameDataNode> synthetic_;
    PreMissionLocation selected_;
    std::uint32_t root_index_ = 0, game_mode_ = 0;
};

} // namespace hd2::frontend
#endif
