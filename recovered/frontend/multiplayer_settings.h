#ifndef HD2_FRONTEND_MULTIPLAYER_SETTINGS_H
#define HD2_FRONTEND_MULTIPLAYER_SETTINGS_H

#include "frontend/multiplayer_server.h"
#include <string>

namespace hd2::frontend {

//0062D880 refreshes base-mode controls;0062D3C0 repeats the two toggle
//dependencies after edits. Disabled rules retain their stored values.
inline bool multiplayer_rule_enabled(unsigned style,unsigned row,bool respawn,bool friendly) {
    if (style>=3) throw std::out_of_range("Unsupported base multiplayer style");
    switch (row) {
    case 5: return style!=2; //Point limit.
    case 9: return style==2; //Allow respawn.
    case 10: return respawn; //Respawn time.
    case 12: return friendly; //Inverse damage.
    case 13: case 14: return style!=0; //Friendly fire / automatic team balance.
    default: return true;
    }
}

// The four 0x58-byte setting blocks at original options+200/258/2B0/308.
// Field order is persisted verbatim by00617DF0; never serialize native padding.
struct MultiplayerRules {
    std::array<std::int32_t, 22> values{};
    static MultiplayerRules defaults(unsigned style) {
        //00618A00. These defaults differ from the temporary builder captions.
        static constexpr std::int32_t original[4][22] = {
            {32,20,0,3,10,5,1,0,0,0,5,100,1,1,1,1,0,3,-1,1,0,1},
            {32,0,30,3,10,30,1,1,1,0,5,100,1,1,1,1,0,3,-1,1,0,1},
            {32,0,5,3,10,0,0,1,1,0,5,100,1,1,1,1,0,3,-1,1,0,1},
            {6,0,0,3,10,5,1,1,0,0,5,100,1,1,1,1,0,3,30,1,0,1}
        };
        if (style >= 4) throw std::out_of_range("Multiplayer rule style");
        MultiplayerRules result;
        std::copy(std::begin(original[style]), std::end(original[style]), result.values.begin());
        return result;
    }
};

// MPSetingsOrig.sav is separate from the ordinary 60-byte profile settings.
// The original options reader accepts historical tags and leaves absent fields
// unchanged. Native decode is transactional and rejects corrupt chunk bounds.
struct MultiplayerSettings {
    using Rotation = std::array<std::vector<std::string>, 4>;
    std::array<Rotation, 5> rotations;
    std::array<MultiplayerRules, 4> rules{{MultiplayerRules::defaults(0), MultiplayerRules::defaults(1),
                                        MultiplayerRules::defaults(2), MultiplayerRules::defaults(3)}};
    std::string hostname, password;
    std::uint32_t port = 11001, voice = 0, style = 0, preset = 0;
    std::uint8_t internet = 0, dedicated = 0;
    MultiplayerFilters filters{-1, -1, false, false, false, false, false, false};
    std::uint8_t legacy_filter = 0, sort = 0, descending = 0;

    static MultiplayerSettings decode(const unsigned char* data, std::size_t size,
                                      MultiplayerSettings initial);
    std::vector<unsigned char> encode() const;

private:
    static std::string string(multiplayer_archive::Reader value) {
        if (value.remaining() > 4096) throw std::runtime_error("Oversized multiplayer setting string");
        std::string result;
        while (value.remaining()) {
            const auto ch = static_cast<char>(value.word(1));
            if (!ch) return result;
            result.push_back(ch);
        }
        throw std::runtime_error("Unterminated multiplayer setting string");
    }
    static void string(std::vector<unsigned char>& bytes, std::uint16_t tag, const std::string& value) {
        if (value.size() > 4095 || value.find('\0') != std::string::npos)
            throw std::runtime_error("Invalid multiplayer setting string");
        const auto start = multiplayer_archive::begin(bytes, tag);
        bytes.insert(bytes.end(), value.begin(), value.end()); bytes.push_back(0);
        multiplayer_archive::end(bytes, start);
    }
    void read_options(multiplayer_archive::Reader wrapper) {
        using namespace multiplayer_archive;
        std::uint16_t tag = 0;
        if (!wrapper.remaining()) return;
        auto block = wrapper.chunk(tag);
        if (tag != 0x4b0 || !block.remaining()) return;
        auto version = block.chunk(tag);
        if (tag != 0x514 || version.word(4) != 0x42) return;
        while (block.remaining()) {
            auto field = block.chunk(tag);
            if (tag == 0x4b3 || tag == 0x4b1) {
                std::uint32_t ping = static_cast<std::uint32_t>(filters.maximum_ping);
                std::uint32_t mode = static_cast<std::uint32_t>(filters.style);
                field.optional(ping); field.optional(mode);
                filters.maximum_ping = static_cast<std::int32_t>(ping);
                filters.style = static_cast<std::int32_t>(mode);
                std::array<std::uint8_t, 7> flags{{filters.responding, filters.dedicated, filters.not_empty,
                    filters.not_full, filters.no_password, legacy_filter, filters.current_version}};
                for (unsigned i = 0; i < flags.size(); ++i) {
                    if (tag == 0x4b1 && i == 5) { flags[i] = 0; continue; }
                    field.optional(flags[i]);
                }
                filters.responding = flags[0] != 0; filters.dedicated = flags[1] != 0;
                filters.not_empty = flags[2] != 0; filters.not_full = flags[3] != 0;
                filters.no_password = flags[4] != 0; legacy_filter = flags[5];
                filters.current_version = flags[6] != 0;
            } else if (tag == 0x4b2) { field.optional(sort); field.optional(descending); }
        }
    }
    void read_host(multiplayer_archive::Reader block) {
        while (block.remaining()) {
            std::uint16_t tag = 0;
            auto field = block.chunk(tag);
            switch (tag) {
            case 0x1ab: hostname = string(field); break;
            case 0x1ac: password = string(field); break;
            case 0x1af: field.optional(port); break;
            case 0x1ad: preset = 0; [[fallthrough]];
            case 0x1b7:
                if (tag == 0x1b7) field.optional(preset);
                field.optional(style); field.optional(voice); field.optional(internet); field.optional(dedicated);
                break;
            case 0x1aa: case 0x1ae: case 0x1b0: case 0x1b4: case 0x1b8: {
                const unsigned count = tag == 0x1aa ? 13 : tag == 0x1ae ? 14 : tag == 0x1b8 ? 22 : 17;
                const unsigned groups = tag == 0x1b4 || tag == 0x1b8 ? 4 : 3;
                for (unsigned g = 0; g < groups; ++g)
                    if (field.remaining() >= count * 4)
                        for (unsigned i = 0; i < count; ++i)
                            rules[g].values[i] = static_cast<std::int32_t>(field.word(4));
                break;
            }
            case 0x1b6: {
                if (!field.remaining()) break;
                std::uint16_t child = 0;
                auto index = field.chunk(child);
                if (child != 0x1b5) break;
                const auto slot = index.word(4);
                if (slot >= rotations.size()) throw std::runtime_error("Invalid multiplayer map preset");
                while (field.remaining()) {
                    auto list = field.chunk(child);
                    const unsigned group = child == 0x1a4 ? 0 : child == 0x1a6 ? 1 : child == 0x1a8 ? 2 : child == 0x1b2 ? 3 : 4;
                    if (group == 4) continue;
                    auto& maps = rotations[slot][group]; maps.clear();
                    while (list.remaining()) {
                        auto name = list.chunk(child);
                        if (child == 0x4d2) {
                            if (maps.size() == 50) throw std::runtime_error("Multiplayer map rotation exceeds 50");
                            maps.push_back(string(name));
                        }
                    }
                }
                break;
            }
            default: break;
            }
        }
        //00618130, including old files with style values outside the base modes.
        style %= 3;
    }
};

inline MultiplayerSettings MultiplayerSettings::decode(const unsigned char* data, std::size_t size,
                                                        MultiplayerSettings initial) {
    multiplayer_archive::Reader file(data, size);
    std::uint16_t tag = 0;
    auto root = file.chunk(tag);
    while (root.remaining()) {
        auto group = root.chunk(tag);
        if (tag == 0x44d) initial.read_host(group);
        else if (tag == 0x4b0) initial.read_options(group);
    }
    return initial;
}

inline std::vector<unsigned char> MultiplayerSettings::encode() const {
    using namespace multiplayer_archive;
    std::vector<unsigned char> bytes;
    const auto root = begin(bytes, 0x4e6), host = begin(bytes, 0x44d);
    for (unsigned slot = 0; slot < rotations.size(); ++slot) {
        const auto entry = begin(bytes, 0x1b6), index = begin(bytes, 0x1b5);
        word(bytes, slot); end(bytes, index);
        constexpr std::uint16_t tags[]{0x1a4, 0x1a6, 0x1a8, 0x1b2};
        for (unsigned group = 0; group < 4; ++group) {
            const auto list = begin(bytes, tags[group]);
            if (rotations[slot][group].size() > 50) throw std::runtime_error("Multiplayer map rotation exceeds 50");
            for (const auto& name : rotations[slot][group]) string(bytes, 0x4d2, name);
            end(bytes, list);
        }
        end(bytes, entry);
    }
    const auto modes = begin(bytes, 0x1b7);
    word(bytes, preset); word(bytes, style); word(bytes, voice); word(bytes, internet, 1); word(bytes, dedicated, 1);
    end(bytes, modes);
    const auto blocks = begin(bytes, 0x1b8);
    for (const auto& rule : rules) for (auto value : rule.values) word(bytes, static_cast<std::uint32_t>(value));
    end(bytes, blocks);
    string(bytes, 0x1ab, hostname); string(bytes, 0x1ac, password);
    const auto number = begin(bytes, 0x1af); word(bytes, port); end(bytes, number); end(bytes, host);
    //0061AC10 wraps00616C80's own 4B0 chunk in another4B0 chunk.
    const auto wrapper = begin(bytes, 0x4b0), options = begin(bytes, 0x4b0), version = begin(bytes, 0x514);
    word(bytes, 0x42); end(bytes, version);
    const auto filter = begin(bytes, 0x4b3);
    word(bytes, static_cast<std::uint32_t>(filters.maximum_ping)); word(bytes, static_cast<std::uint32_t>(filters.style));
    for (std::uint8_t flag : {std::uint8_t(filters.responding), std::uint8_t(filters.dedicated),
            std::uint8_t(filters.not_empty), std::uint8_t(filters.not_full), std::uint8_t(filters.no_password),
            legacy_filter, std::uint8_t(filters.current_version)}) word(bytes, flag, 1);
    end(bytes, filter);
    const auto order = begin(bytes, 0x4b2); word(bytes, sort, 1); word(bytes, descending, 1);
    end(bytes, order); end(bytes, options); end(bytes, wrapper); end(bytes, root);
    return bytes;
}

} // namespace hd2::frontend
#endif
