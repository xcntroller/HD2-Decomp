#ifndef HD2_FRONTEND_PROFILE_PAYLOAD_H
#define HD2_FRONTEND_PROFILE_PAYLOAD_H

#include "frontend/profile_directories.h"
#include "frontend/startup_language.h"
#include <type_traits>
#include <vector>

namespace hd2::frontend {

struct ProfileMovie {
    std::uint8_t enabled;
    std::uint32_t caption;
    const char* filename;
};

// 006A6B10 / 006A6D10 / 006A6E80, not the four-movie opening sequence itself.
class ProfileMovies {
public:
    ProfileMovies() { reset(); }
    const auto& records() const { return records_; }
    void reset() {
        records_.clear();
        if (records_.capacity() < 20) records_.reserve(20);
        constexpr std::array<const char*, 10> names{{"Intro.bik", "IntroArctic.bik", "IntroAfrica.bik",
            "IntroNorway.bik", "IntroBurma.bik", "IntroAlps.bik", "IntroNormandy.bik",
            "IntroCzech.avi", "Outro1.bik", "Outro2.bik"}};
        for (std::size_t i = 0; i < names.size(); ++i)
            records_.push_back({0, 0xa0u + static_cast<std::uint32_t>(i), names[i]});
        enable("Intro.bik", 1);
    }
    bool enable(std::string_view filename, std::uint8_t value) {
        for (auto at = records_.rbegin(); at != records_.rend(); ++at)
            if (startup_case_equal(at->filename, filename)) { at->enabled = value; return true; }
        return false;
    }
    template<class Files>
    void read(Files& files, io::ChunkFile<Files>& file) {
        while (remaining(files, file) > 0) {
            std::uint16_t tag = 0;
            if (!file.enter(tag)) throw std::runtime_error("Truncated profile movie header");
            if (tag == 0x1ec8) {
                // Original fixed ten-entry loop reads BACKWARDS into flag bytes.
                // An out-of-bound read leaves the initialized flag unchanged.
                for (std::size_t i = 10; i != 0; --i) file.read(&records_[i - 1].enabled, 1);
            }
            file.leave();
        }
    }
private:
    template<class Files>
    static std::int32_t remaining(Files& files, const io::ChunkFile<Files>& file) {
        const auto& state = file.state();
        if (!state.opened || !state.depth) return 0;
        const auto position = files.seek(state.handle, 0, 1);
        if (position == 0xffffffffu) return 0;
        const auto bits = state.offsets[state.depth] - position;
        std::int32_t value;
        std::memcpy(&value, &bits, 4);
        return value;
    }
    std::vector<ProfileMovie> records_;
};

// Persisted fields consumed by 006AFB00 and reset006AF420. Index = original
// offset / 4, never an original ABI claim. Only actual string slots are used;
// numeric words remain raw so signed IDs/float bit patterns survive unchanged.
struct ProfilePayload {
    std::array<std::uint32_t, 37> words{};
    std::array<std::string, 13> strings{};
    ProfileMovies movies;
    void reset() {
        words[0x34 / 4] = 2;
        for (std::size_t at = 0x74 / 4; at <= 0x90 / 4; ++at) words[at] = 0xffffffffu;
        strings[1].clear(); strings[2].clear();
        for (std::size_t at = 0x0c / 4; at <= 0x2c / 4; at += 2) words[at] = 0xffffffffu;
        movies.reset(); // Other fields intentionally persist across reset.
    }
};

enum class ProfilePayloadResult { missing_file, invalid_root, payload_read };

// File/reset/recognized-payload portion of 006AFB00. Completion runs before
// root leave/close, allowing the real finalizer to open nested save files.
// profile_load.h supplies the actual fallback and saved-game finalization;
// payload_read alone is not a complete profile-selection result.
template<class Files, class Finish = std::nullptr_t>
ProfilePayloadResult read_profile_payload(ProfilePayload& state, ProfileDirectories& profiles,
                                          Files& files, bool wide, Finish finish = nullptr) {
    io::ChunkFile<Files> file(files);
    const auto path = profiles.current_directory(wide) + "profile.sav";
    if (!file.open(path.c_str())) return ProfilePayloadResult::missing_file;
    state.reset(); // Original resets AFTER successful open, BEFORE root check.
    std::uint16_t tag = 0;
    const auto enter = [&] {
        if (!file.enter(tag)) throw std::runtime_error("Truncated profile payload header");
    };
    const auto remaining = [&]() -> std::int32_t {
        const auto& s = file.state();
        if (!s.opened || !s.depth) return 0;
        const auto position = files.seek(s.handle, 0, 1);
        if (position == 0xffffffffu) return 0;
        const auto bits = s.offsets[s.depth] - position;
        std::int32_t count;
        std::memcpy(&count, &bits, 4);
        return count;
    };
    const auto text = [&] {
        const auto size = std::min<std::int32_t>(999, remaining());
        if (size <= 0) throw std::runtime_error("Invalid profile payload string size");
        std::array<char, 999> bytes{};
        if (!file.read(bytes.data(), static_cast<std::uint32_t>(size))) throw std::runtime_error("Truncated profile payload string");
        const auto end = std::find(bytes.begin(), bytes.begin() + size, '\0');
        if (end == bytes.begin() + size) throw std::runtime_error("Unterminated profile payload string");
        return std::string(bytes.begin(), end);
    };
    const auto read_word = [&](std::size_t offset) {
        // Original reads directly into initialized words and ignores failures,
        // including partial overwrites. Explicit LE scratch preserves that.
        auto& value = state.words[offset / 4];
        std::array<unsigned char, 4> bytes{};
        for (unsigned i = 0; i < 4; ++i) bytes[i] = static_cast<unsigned char>(value >> (8 * i));
        file.read(bytes.data(), 4);
        value = std::uint32_t{bytes[0]} | (std::uint32_t{bytes[1]} << 8) |
                (std::uint32_t{bytes[2]} << 16) | (std::uint32_t{bytes[3]} << 24);
    };
    enter();
    if (tag != 0x1edc) return ProfilePayloadResult::invalid_root;
    bool old_name_seen = false;
    while (remaining() > 0) {
        enter();
        switch (tag) {
        case 0x1edd: if (wide) profiles.current_name = text(); break;
        case 0x1ee6:
            read_word(0x34);
            for (std::size_t at = 0x74; at <= 0x80; at += 4) read_word(at);
            break;
        case 0x1ee8:
            state.strings[old_name_seen ? 2 : 1] = text(); old_name_seen = true; break;
        case 0x1eea: state.strings[1] = text(); break;
        case 0x1eeb: state.strings[2] = text(); break;
        case 0x1ef0: state.movies.read(files, file); break;
        case 0x1ef5: for (std::size_t at = 0x0c; at <= 0x2c; at += 8) read_word(at); break;
        case 0x1efa: case 0x1efb: case 0x1efc: case 0x1efd:
            state.strings[4 + (tag - 0x1efa) * 2] = text(); break;
        case 0x1efe: state.strings[0x30 / 4] = text(); break;
        case 0x1eff: read_word(0x34); break;
        case 0x1f05: for (std::size_t at = 0x74; at <= 0x80; at += 4) read_word(at); break;
        case 0x1f06: for (std::size_t at = 0x84; at <= 0x90; at += 4) read_word(at); break;
        default: break;
        }
        file.leave();
    }
    if constexpr (!std::is_same_v<Finish, std::nullptr_t>) finish();
    file.leave();
    file.close();
    return ProfilePayloadResult::payload_read;
}

}
#endif
