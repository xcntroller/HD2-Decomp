#ifndef HD2_FRONTEND_PROFILE_LOAD_H
#define HD2_FRONTEND_PROFILE_LOAD_H

#include "frontend/game_data.h"
#include "frontend/localized_text.h"
#include "frontend/profile_payload.h"

namespace hd2::frontend {

inline std::int32_t profile_signed_word(std::uint32_t word) {
    std::int32_t value;
    std::memcpy(&value, &word, 4);
    return value;
}

// 006E9230: append nonempty CR/LF-separated raw lines. This older reader closes
// only nonnegative signed handles; preserve that distinction from ChunkFile.
template<class Files>
bool load_profile_pr_lines(Files& files, std::vector<std::string>& lines) {
    const auto handle = files.open("profile.pr", 0);
    if (handle == 0xffffffffu) return false;
    const auto close = [&] { if (handle <= 0x7fffffffu) files.close(handle); };
    try {
        const auto size = files.seek(handle, 0, 2);
        if (!size || size > 0x7fffffffu) { close(); return false; }
        std::vector<char> bytes(size);
        files.seek(handle, 0, 0); // Original ignores seek result.
        if (files.read(handle, bytes.data(), size) != size) { close(); return false; }
        std::size_t at = 0;
        while (at < bytes.size()) {
            const auto begin = at;
            while (at < bytes.size() && bytes[at] != '\r' && bytes[at] != '\n') ++at;
            if (at != begin) lines.emplace_back(bytes.data() + begin, at - begin);
            ++at;
        }
    } catch (...) { close(); throw; }
    close();
    return true;
}

// 006E9840: first case-sensitive SUBSTRING match, then first '=' on that line.
// Neither key anchoring, comment handling, quoting nor whitespace trimming.
inline const char* profile_pr_value(const std::vector<std::string>& lines, const char* key) {
    for (const auto& stored : lines) {
        const std::string_view line(stored.c_str());
        if (line.find(key) != std::string_view::npos) {
            const auto equal = line.find('=');
            if (equal == std::string_view::npos) throw std::runtime_error("Original profile.pr value pointer is invalid");
            return stored.c_str() + equal + 1;
        }
    }
    return nullptr;
}

// 006E9920: sscanf %s then strtol(base10), whole token consumed. Saturate as
// original 32-bit strtol does on overflow. Native guards its 20-byte scratch.
inline bool profile_pr_number(const std::vector<std::string>& lines, const char* key, std::uint32_t& output) {
    const auto* value = profile_pr_value(lines, key);
    if (!value) return false;
    const auto space = [](char c) { return c == ' ' || (c >= '\t' && c <= '\r'); };
    while (space(*value)) ++value;
    std::string_view token(value);
    std::size_t end = 0;
    while (end < token.size() && !space(token[end])) ++end;
    token = token.substr(0, end);
    if (token.empty() || token.size() >= 20) throw std::length_error("Original profile.pr number buffer exceeded");
    std::size_t at = 0;
    bool negative = false;
    if (token[at] == '+' || token[at] == '-') negative = token[at++] == '-';
    const auto digits = at;
    const std::uint32_t limit = negative ? 0x80000000u : 0x7fffffffu;
    std::uint32_t number = 0;
    for (; at < token.size(); ++at) {
        if (token[at] < '0' || token[at] > '9') return false;
        const auto digit = static_cast<std::uint32_t>(token[at] - '0');
        number = number > (limit - digit) / 10 ? limit : number * 10 + digit;
    }
    if (at == digits) return false;
    output = negative ? 0u - number : number;
    return true;
}

// Data portion of fallback006AF490. Caller resets and restores executable cwd
// before this call. Its first numeric scratch is undefined on failed LASTGAME;
// guard that case, then retain scratch across the other ignored lookup failures.
template<class Files>
bool read_legacy_profile_fields(ProfilePayload& state, Files& files) {
    std::vector<std::string> lines;
    if (!load_profile_pr_lines(files, lines)) return false;
    profile_pr_number(lines, "GAME_DIFFICULTY", state.words[0x34 / 4]);
    std::uint32_t scratch = 0;
    if (!profile_pr_number(lines, "LASTGAME", scratch))
        throw std::runtime_error("Undefined original profile.pr numeric scratch");
    state.words[0x74 / 4] = scratch;
    constexpr std::array<const char*, 3> keys{{"LASTCAMPAIGN", "LASTLONEWOLF", "LASTCARNAGE"}};
    for (std::size_t i = 0; i < keys.size(); ++i) {
        profile_pr_number(lines, keys[i], scratch);
        state.words[0x78 / 4 + i] = scratch;
    }
    std::string text;
    if (const auto* value = profile_pr_value(lines, "LASTSAVE")) text = value;
    if (text.size() >= 1000) throw std::length_error("Original profile.pr text buffer exceeded");
    state.strings[1] = text;
    if (const auto* value = profile_pr_value(lines, "LASTSAVETEXT")) text = value;
    if (text.size() >= 1000) throw std::length_error("Original profile.pr text buffer exceeded");
    state.strings[2] = text;
    if (profile_signed_word(state.words[0x34 / 4]) > 4) state.words[0x34 / 4] = 3;
    return true;
}

// 006C47C0, including surprising signed comparison and output-on-invalid-marker
// behavior. Lowercase "quick" substring required anywhere in original name.
template<class Files>
bool validate_profile_quick_save(const GameData& game, const ProfileDirectories& profiles,
                                  Files& files, bool wide, const std::string& stored_name,
                                  std::int32_t* mission_output = nullptr) {
    const std::string name(stored_name.c_str());
    if (name.empty() || name.find("quick") == std::string::npos) return false;
    io::ChunkFile<Files> file(files);
    if (!file.open((profiles.current_directory(wide) + name).c_str())) return false;
    std::uint16_t root = 0, tag = 0;
    if (!file.enter(root)) throw std::runtime_error("Truncated quick-save root");
    const auto pos = files.seek(file.state().handle, 0, 1);
    if (pos == 0xffffffffu) return false;
    if (profile_signed_word(file.state().offsets[file.state().depth] - pos) <= 0) return false;
    if (!file.enter(tag)) throw std::runtime_error("Truncated quick-save marker");
    const bool marker = root > 0x4e8b && tag == 0x0bb6;
    const auto count = game.mission_count(0);
    // Original scratch already holds tag + low sixteen size bits on read failure.
    const auto payload_position = files.seek(file.state().handle, 0, 1);
    const auto total = file.state().offsets[file.state().depth] - payload_position + 6u;
    const auto initial = std::uint32_t{tag} | (total << 16);
    std::array<unsigned char, 4> bytes{};
    for (unsigned i = 0; i < 4; ++i) bytes[i] = static_cast<unsigned char>(initial >> (8 * i));
    file.read(bytes.data(), 4); // Ignore result, preserve partial overwrite.
    const auto mission = profile_signed_word(std::uint32_t{bytes[0]} | (std::uint32_t{bytes[1]} << 8) |
                                             (std::uint32_t{bytes[2]} << 16) | (std::uint32_t{bytes[3]} << 24));
    file.leave();
    if (mission_output) *mission_output = mission;
    return marker && count > mission; // Original does NOT reject negative mission.
}

// Native guarded equivalent of the three-string localized save-caption format.
// Original translations use %s and %% (English2051: %s$After %s complete$%s).
inline std::string format_profile_save_caption(const std::string& format,
                                               const std::string& mode, const std::string& mission) {
    const std::array<std::string, 3> values{{mode, mission, ""}};
    std::size_t argument = 0;
    std::string result;
    for (std::size_t at = 0; at < format.size(); ++at) {
        if (format[at] != '%') result += format[at];
        else if (++at < format.size() && format[at] == '%') result += '%';
        else if (at < format.size() && format[at] == 's' && argument < values.size()) result += values[argument++];
        else throw std::runtime_error("Unsupported original profile save-caption format");
    }
    return result;
}

// 006B0520: replace invalid quick-save name with prior-mission completed save.
// Physical existence uses CRT fopen("r"), separate from the DTA file boundary.
template<class Platform>
bool promote_profile_save(ProfilePayload& state, const GameData& game,
                           const ProfileDirectories& profiles, Platform& platform,
                           const LocalizedTextCache& text, bool wide, std::int32_t mission) {
    if (mission >= game.mission_count(0) || mission <= 0) return false;
    const auto previous = mission - 1;
    const auto caption = profile_signed_word(game.mission_caption(0, previous));
    if (caption < 0) return false;
    constexpr std::array<const char*, 3> prefixes{{"CAM_", "LOW_", "CAR_"}};
    std::size_t mode = 0;
    while (mode < prefixes.size() && state.strings[1].find(prefixes[mode]) != 0) ++mode;
    if (mode == prefixes.size()) return false;
    char name[64];
    const auto size = std::snprintf(name, sizeof(name), "%s%02d%02d.sav", prefixes[mode], 0, previous);
    if (size < 0 || size >= static_cast<int>(sizeof(name))) throw std::length_error("Profile save name overflow");
    state.strings[1] = name; // Mutation precedes existence/format checks.
    const auto mode_text = lookup_localized_text(text, 0x89a + static_cast<std::int32_t>(mode));
    if (!platform.probe_save_file(profiles.current_directory(wide) + state.strings[1])) return false;
    const auto format = lookup_localized_text(text, 0x803);
    const auto mission_text = lookup_localized_text(text, caption);
    if (format.empty() || mode_text.empty() || mission_text.empty()) return false;
    state.strings[2] = format_profile_save_caption(format, mode_text, mission_text);
    return true;
}

// 006B03B0. Null game represents missing original application singleton; skip
// ALL changes and return false, rather than pretending initialization succeeded.
template<class Files, class Platform>
bool finalize_profile(ProfilePayload& state, const GameData* game,
                       const ProfileDirectories& profiles, Files& files, Platform& platform,
                       const LocalizedTextCache& text, bool wide) {
    if (!game) return false;
    for (std::size_t slot = 0; slot < 5; ++slot) {
        auto& id = state.words[0x0c / 4 + slot * 2];
        auto& name = state.strings[0x10 / 4 + slot * 2];
        if (id != 0xffffffffu && !validate_profile_quick_save(*game, profiles, files, wide, name)) {
            id = 0xffffffffu;
            name.clear();
        }
    }
    if (!state.strings[1].empty() && state.strings[1].find("quick") != std::string::npos) {
        std::int32_t mission = -1;
        if (!validate_profile_quick_save(*game, profiles, files, wide, state.strings[1], &mission)) {
            if (mission >= 0 && promote_profile_save(state, *game, profiles, platform, text, wide, mission)) return true;
            state.strings[1].clear(); state.strings[2].clear();
        }
    }
    return true;
}

// Actual 006AFB00 file/payload/finalize path and 006AF490 missing-file fallback.
// Options application and outer profile-selection006AF1D0 remain separate.
template<class Files, class Platform>
bool load_profile(ProfilePayload& state, ProfileDirectories& profiles, Files& files,
                   Platform& platform, const GameData* game, const LocalizedTextCache& text, bool wide) {
    const auto result = read_profile_payload(state, profiles, files, wide, [&] {
        finalize_profile(state, game, profiles, files, platform, text, wide);
    });
    if (result == ProfilePayloadResult::invalid_root) return false;
    if (result == ProfilePayloadResult::payload_read) return true;
    state.reset();
    platform.restore_executable_directory();
    const auto legacy = read_legacy_profile_fields(state, files);
    platform.restore_executable_directory();
    finalize_profile(state, game, profiles, files, platform, text, wide);
    return legacy;
}

}
#endif
