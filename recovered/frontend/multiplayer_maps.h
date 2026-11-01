#ifndef HD2_FRONTEND_MULTIPLAYER_MAPS_H
#define HD2_FRONTEND_MULTIPLAYER_MAPS_H

#include "frontend/multiplayer_settings.h"
#include <charconv>
#include <map>
#include <optional>

namespace hd2::frontend {

enum class MultiplayerMapSide { outside, available, selected };
//0062E920/0062EA90, doubles00813AC8..00813AF0. The pointer's normalized
// menu Y is negative; do not substitute a half-screen or screen-pixel test.
inline MultiplayerMapSide multiplayer_map_side(const std::array<float,3>& point) {
    const double x=point[0],y=point[1];
    if (!(y>-.68 && y<-.3)) return MultiplayerMapSide::outside;
    if (x>.08 && x<.26) return MultiplayerMapSide::available;
    if (x>.28 && x<.46) return MultiplayerMapSide::selected;
    return MultiplayerMapSide::outside;
}

//006197B0 removes and reinserts, preserving the intervening order. This is
// not the team screen's swap operation. Reject stale native indices safely.
inline bool move_multiplayer_map(std::vector<std::string>& maps,std::size_t from,std::size_t to) {
    if (from>=maps.size() || to>=maps.size() || from==to) return false;
    auto value=std::move(maps[from]);
    maps.erase(maps.begin()+from); maps.insert(maps.begin()+to,std::move(value));
    return true;
}

struct MultiplayerMap {
    std::string name, directory;
    std::int32_t description = -1;
    unsigned style = 0;
};

//006189C0's base-client allowlist at0083EC48. The installed catalogue also
// includes expansion maps; their presence must not expose expansion gameplay.
inline bool base_multiplayer_map(std::string_view directory) {
    static constexpr const char* names[]{
        "Africa1_mp", "Africa3_mp", "Africa5_mp", "Arctic1_mp", "Arctic2_mp", "Burma1_mp",
        "Czech3_mp", "Normandy_mp", "africa2_mp_zone", "Africa5_mp_zone", "Alps1_mp_zone",
        "Arctic4_mp_zone", "Burma1_mp_zone", "Czech2_mp_zone", "Czech4_mp_zone", "Czech5_mp_zone",
        "Czech6_mp_zone", "Normandy2_mp_zone", "NORMANDY2B_MP_ZONE", "Normandy3_mp", "Africa1_obj",
        "Africa3_obj", "Africa4_obj", "Arctic1_obj", "Arctic3_obj", "Burma1_obj", "Burma2_obj",
        "Czech1_obj", "Czech2_obj", "Czech3_obj"
    };
    const auto equal = [](std::string_view a, std::string_view b) {
        if (a.size() != b.size()) return false;
        for (std::size_t i = 0; i < a.size(); ++i) {
            const auto lower = [](unsigned char c) { return c >= 'A' && c <= 'Z' ? c + ('a'-'A') : c; };
            if (lower(a[i]) != lower(b[i])) return false;
        }
        return true;
    };
    return std::any_of(std::begin(names), std::end(names), [&](const char* name) { return equal(directory, name); });
}

// Frontend portion of00618A00: catalogue identity, display names, mode and
// description. Objective/equipment/zone metadata remains in the source file
// for the future game loader; it is not replaced with default gameplay data.
// This bounded scanner accepts the original markup's quoted attributes and
// comments, and validates nesting rather than interpreting arbitrary XML/DTDs.
inline std::vector<MultiplayerMap> decode_multiplayer_maps(std::string_view source) {
    if (source.size() > 4 * 1024 * 1024) throw std::runtime_error("Multiplayer map catalogue too large");
    std::vector<MultiplayerMap> result;
    std::vector<std::string> stack;
    std::optional<unsigned> style;
    std::size_t at = 0;
    bool root = false;
    const auto space = [](char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; };
    const auto name_char = [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '_' || c == '-'; };
    while ((at = source.find('<', at)) != std::string_view::npos) {
        if (source.substr(at, 4) == "<!--") {
            const auto end = source.find("-->", at + 4);
            if (end == std::string_view::npos) throw std::runtime_error("Unterminated multiplayer map comment");
            at = end + 3; continue;
        }
        ++at;
        bool closing = false, empty = false;
        if (at < source.size() && source[at] == '/') { closing = true; ++at; }
        const auto start = at;
        while (at < source.size() && name_char(source[at])) ++at;
        if (start == at) throw std::runtime_error("Invalid multiplayer map tag");
        const std::string tag(source.substr(start, at - start));
        std::map<std::string, std::string> attributes;
        for (;;) {
            while (at < source.size() && space(source[at])) ++at;
            if (at == source.size()) throw std::runtime_error("Truncated multiplayer map tag");
            if (source[at] == '/') { empty = true; ++at; }
            if (at < source.size() && source[at] == '>') { ++at; break; }
            if (closing || empty) throw std::runtime_error("Invalid multiplayer map closing tag");
            const auto key_at = at;
            while (at < source.size() && name_char(source[at])) ++at;
            const std::string key(source.substr(key_at, at - key_at));
            while (at < source.size() && space(source[at])) ++at;
            if (key.empty() || at == source.size() || source[at++] != '=')
                throw std::runtime_error("Invalid multiplayer map attribute");
            while (at < source.size() && space(source[at])) ++at;
            if (at == source.size() || (source[at] != '\'' && source[at] != '"'))
                throw std::runtime_error("Unquoted multiplayer map attribute");
            const char quote = source[at++];
            const auto end = source.find(quote, at);
            if (end == std::string_view::npos || end - at > 4096)
                throw std::runtime_error("Invalid multiplayer map attribute length");
            if (!attributes.emplace(key, std::string(source.substr(at, end-at))).second)
                throw std::runtime_error("Duplicate multiplayer map attribute");
            at = end + 1;
        }
        if (closing) {
            if (stack.empty() || stack.back() != tag) throw std::runtime_error("Mismatched multiplayer map tag");
            stack.pop_back();
            if (tag == "GAMESTYLE") style.reset();
            continue;
        }
        if (stack.empty()) {
            if (root || tag != "MAP_LIST") throw std::runtime_error("Invalid multiplayer map root");
            root = true;
        } else if (tag == "GAMESTYLE" && stack.size() == 1) {
            const auto found = attributes.find("type");
            style.reset();
            if (found != attributes.end()) {
                if (found->second == "deathmatch") style = 0;
                else if (found->second == "teamplay") style = 1;
                else if (found->second == "hd2multiplayer") style = 2;
                // The base00618A00 explicitly skips cooperative subtrees.
            }
        } else if (tag == "MAP" && stack.size() == 2 && stack.back() == "GAMESTYLE" && style) {
            const auto name = attributes.find("name"), directory = attributes.find("dir");
            if (name != attributes.end() && directory != attributes.end() && base_multiplayer_map(directory->second)) {
                MultiplayerMap map{name->second, directory->second, -1, *style};
                const auto description = attributes.find("desc");
                if (description != attributes.end()) {
                    const auto& text = description->second;
                    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), map.description);
                    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size())
                        throw std::runtime_error("Invalid multiplayer map description");
                }
                if (result.size() == 1024) throw std::runtime_error("Too many multiplayer maps");
                result.push_back(std::move(map));
            }
        }
        if (!empty) {
            if (stack.size() == 64) throw std::runtime_error("Multiplayer map nesting too deep");
            stack.push_back(tag);
        }
    }
    if (!root || !stack.empty()) throw std::runtime_error("Incomplete multiplayer map catalogue");
    return result;
}

} // namespace hd2::frontend
#endif
