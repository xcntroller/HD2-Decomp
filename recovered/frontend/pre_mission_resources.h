#ifndef HD2_FRONTEND_PRE_MISSION_RESOURCES_H
#define HD2_FRONTEND_PRE_MISSION_RESOURCES_H

#include "frontend/game_data.h"
#include "frontend/pre_mission_data.h"
#include <cstdio>

namespace hd2::frontend {

// 0067DBF0: four groups of six user presets. Captions and category masks are
//  real original values; empty equipment retains0067D470's -1/zero defaults.
inline std::array<PreMissionPreset, 24> default_pre_mission_user_presets() {
    std::array<PreMissionPreset, 24> result;
    constexpr std::array<const char *, 4> prefixes{
        {"Axis user preset", "Allied user preset", "Deadmatch user preset", "User preset"}};
    constexpr std::array<std::uint32_t, 4> categories{{0x12, 0x22, 0x42, 4}};
    for (std::size_t i = 0; i < result.size(); ++i) {
        auto &preset = result[i];
        preset.name = std::string(prefixes[i / 6]) + std::to_string(i % 6 + 1);
        preset.field1ac = categories[i / 6];
        if (i < 18)
            preset.field1b0 = 0x1982 + static_cast<std::int32_t>(i);
    }
    return result;
}

// 0067D930's version3 writer. Native bounds reject corrupt fixed-size arrays
//  rather than reproducing raw object overreads. The caller supplies the actual
//  selected profile directory and writable resolver; originals are never patched.
inline std::vector<unsigned char> encode_pre_mission_presets(const std::vector<PreMissionPreset> &presets) {
    if (presets.size() > std::numeric_limits<std::uint32_t>::max())
        throw std::length_error("Too many presets");
    std::vector<unsigned char> output(8 + presets.size() * 0x1b8);
    const auto word = [&](std::size_t at, std::uint32_t value) {
        for (unsigned byte = 0; byte < 4; ++byte)
            output.at(at + byte) = static_cast<unsigned char>(value >> (8 * byte));
    };
    word(0, 3);
    word(4, static_cast<std::uint32_t>(presets.size()));
    for (std::size_t index = 0; index < presets.size(); ++index) {
        const auto &value = presets[index];
        const auto base = 8 + index * 0x1b8;
        if (value.name.size() >= 32 || value.holsters.size() > 14 || value.backpack.size() > 32)
            throw std::runtime_error("Invalid serialized preset");
        std::copy(value.name.begin(), value.name.end(), output.begin() + base);
        for (std::size_t slot = 0; slot < 5; ++slot)
            word(base + 0x20 + slot * 4, static_cast<std::uint32_t>(value.equipment[slot]));
        const auto entries = [&](std::size_t offset, std::size_t capacity, const auto &values) {
            word(base + offset, static_cast<std::uint32_t>(values.size()));
            for (std::size_t i = 0; i < capacity; ++i) {
                word(base + offset + 4 + i * 8,
                     i < values.size() ? static_cast<std::uint32_t>(values[i][0]) : 0xffffffffu);
                word(base + offset + 8 + i * 8,
                     i < values.size() ? static_cast<std::uint32_t>(values[i][1]) : 0);
            }
        };
        entries(0x34, 14, value.holsters);
        entries(0xa8, 32, value.backpack);
        word(base + 0x1ac, value.field1ac);
        word(base + 0x1b0, static_cast<std::uint32_t>(value.field1b0));
        word(base + 0x1b4, value.field1b4);
    }
    return output;
}

template <class Files>
bool write_pre_mission_user_presets(Files &files, const std::string &profile_directory,
                                    const std::vector<PreMissionPreset> &presets) {
    const auto bytes = encode_pre_mission_presets(presets);
    const auto path = profile_directory + "user_presets_orig.def";
    const auto handle = files.open_write(path.c_str(), 0x10000);
    if (handle == 0xffffffffu)
        return false;
    struct Close {
        Files &files;
        std::uint32_t handle;
        ~Close() { files.close(handle); }
    } close{files, handle};
    return files.write(handle, bytes.data(), static_cast<std::uint32_t>(bytes.size())) == bytes.size();
}

// Resource composition for the actual pre-mission owner. Uses the supplied
// DTA/loose resolver for every read; no local extracted-data shortcut. Source
// archive, profile and graphics owners outlive all screens using this state.
struct PreMissionResources {
    PreMissionItemCatalogue catalogue;
    std::vector<PreMissionPreset> presets;
    std::vector<PreMissionPreset> user_presets;
    std::vector<std::optional<PreMissionEquipmentPolicy>> policies;

    template <class Files>
    bool load(Files &files, const std::string &profile_directory, const GameData &game) {
        if (!catalogue.load(files))
            return false;
        const auto preset_bytes = pre_mission_detail::read_file(files, "GameData\\presets.def");
        const auto policy_bytes = pre_mission_detail::read_file(files, "GameData\\miss_itm.def");
        if (!preset_bytes || !policy_bytes)
            return false;
        presets = decode_pre_mission_presets(*preset_bytes);
        policies.clear();
        policies.resize(120);
        for (std::uint32_t root = 0; root < std::min<std::size_t>(4, game.roots().size()); ++root) {
            const auto missions = game.mission_count(root);
            if (missions > 30)
                throw std::runtime_error("Too many missions for original equipment policies");
            for (std::int32_t mission = 0; mission < missions; ++mission)
                policies[root * 30 + mission].emplace();
        }
        // 0067E200 initializes only roots actually present; the lookup is capped
        //  at root*30+mission. Serialized active slots allocate as needed.
        decode_pre_mission_policies(*policy_bytes, policies);
        user_presets.clear();
        const auto user =
            pre_mission_detail::read_file(files, (profile_directory + "user_presets_orig.def").c_str());
        if (user) {
            const auto defaults = default_pre_mission_user_presets();
            user_presets.assign(defaults.begin(), defaults.end());
            const auto values = decode_pre_mission_presets(*user);
            const auto count = std::min<std::size_t>(24, values.size());
            const auto per_group = count / 4;
            if (count && !per_group)
                throw std::runtime_error("Invalid user preset group count");
            for (std::size_t i = 0; i < count; ++i) {
                const auto destination = i % per_group + (i / per_group) * 6;
                if (destination >= user_presets.size())
                    throw std::runtime_error("Invalid user preset group index");
                user_presets[destination] = values[i];
            }
        } else if (!write_pre_mission_user_presets(files, profile_directory, user_presets))
            return false;
        return true;
    }
    PreMissionEquipmentPolicy *policy(std::uint32_t root, std::uint32_t mission) {
        if (root >= 4 || mission >= 30)
            return nullptr;
        const auto slot = root * 30 + mission;
        return slot < policies.size() && policies[slot] ? &*policies[slot] : nullptr;
    }
};
} // namespace hd2::frontend
#endif
