#ifndef HD2_FRONTEND_STARTUP_LANGUAGE_H
#define HD2_FRONTEND_STARTUP_LANGUAGE_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

namespace hd2::frontend {

// Original CRT 007e88f0's default C-locale equality path. Locale-specific CRT
// tables/locking are separate; non-ASCII bytes are preserved, not case folded.
inline bool startup_case_equal(std::string_view left, std::string_view right) {
    left = left.substr(0, left.find('\0'));
    right = right.substr(0, right.find('\0'));
    if (left.size() != right.size()) return false;
    const auto fold = [](unsigned char c) { return c >= 'A' && c <= 'Z' ? c + 32u : c; };
    for (std::size_t index = 0; index < left.size(); ++index)
        if (fold(static_cast<unsigned char>(left[index])) !=
            fold(static_cast<unsigned char>(right[index]))) return false;
    return true;
}

struct ApplicationLanguageState {
    std::string cached; // Original application +2c retained string.
    bool wide_characters = false; // +30, global 008ae92c in normal application.
};

// 006b67b0 plus normal self-query helpers 006b6750/0048fbc0. Platform supplies
// actual registry operations and the fopen("r")/fclose font probe. Normal
// application's virtual language query calls get() again after caching.
template<class Platform>
class ApplicationLanguage {
public:
    explicit ApplicationLanguage(Platform& platform) : platform_(platform) {}
    ApplicationLanguageState& state() { return state_; }
    const ApplicationLanguageState& state() const { return state_; }

    std::string get() {
        if (!state_.cached.empty()) return state_.cached;
        if (initializing_) throw std::logic_error("Recursive empty application language");
        struct Guard {
            bool& active;
            explicit Guard(bool& flag) : active(flag) { active = true; }
            ~Guard() { active = false; }
        } guard(initializing_);
        std::uintptr_t key = 0;
        if (platform_.open_registry(0x80000002u,
                "SOFTWARE\\Illusion Softworks\\Hidden & Dangerous 2", 0, 1, key) == 0) {
            try {
                std::array<char, 128> bytes{};
                std::uint32_t size = 128;
                if (platform_.query_registry(key, "language", bytes.data(), size) == 0) {
                    const auto end = std::find(bytes.begin(), bytes.end(), '\0');
                    if (end == bytes.end())
                        throw std::length_error("Unterminated installed language");
                    state_.cached.assign(bytes.begin(), end);
                } else state_.cached = "english";
            } catch (...) {
                platform_.close_registry(key);
                throw;
            }
            // Cache installation precedes close, matching original callback order.
            platform_.close_registry(key);
        } else state_.cached = "english";
        if (matches("Japan") || matches("Russian")) state_.wide_characters = true;
        else state_.wide_characters = platform_.probe_text_file(text_directory() + "\\FontConfig.hxml");
        return state_.cached;
    }

    bool matches(std::string_view name) { return startup_case_equal(get(), name); }
    std::string text_directory() { return "Text\\" + std::string(get().c_str()) + "\\"; }

private:
    Platform& platform_;
    ApplicationLanguageState state_;
    bool initializing_ = false;
};

// 006e8ed0: cache the executable path, truncate at LAST backslash only. An
// empty result is queried again on the next call. Original ignores API result.
template<class Platform>
class ExecutableDirectory {
public:
    explicit ExecutableDirectory(Platform& platform) : platform_(platform) {}
    std::string get() {
        if (cached_.empty()) {
            std::array<char, 260> bytes{};
            platform_.module_filename(bytes.data(), 260);
            const auto end = std::find(bytes.begin(), bytes.end(), '\0');
            if (end == bytes.end()) throw std::length_error("Unterminated executable path");
            cached_.assign(bytes.begin(), end);
            const auto slash = cached_.rfind('\\');
            if (slash != std::string::npos) cached_.resize(slash);
        }
        return cached_;
    }
private:
    Platform& platform_;
    std::string cached_;
};

}
#endif
