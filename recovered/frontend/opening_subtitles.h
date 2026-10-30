#ifndef HD2_FRONTEND_OPENING_SUBTITLES_H
#define HD2_FRONTEND_OPENING_SUBTITLES_H

#include "frontend/localized_text.h"
#include "frontend/text_object.h"
#include <array>
#include <limits>
#include <stdexcept>

namespace hd2::frontend {

// Defined-range007E6190/atoi behavior, including no digits=>0 and numeric prefix.
// Native overflow throws instead of depending on CRT overflow/undefined access.
inline std::uint32_t subtitle_integer(const char* text) {
    while (*text == ' ' || (*text >= '\t' && *text <= '\r')) ++text;
    const bool negative = *text == '-';
    if (*text == '+' || *text == '-') ++text;
    const std::uint32_t limit = negative ? 0x80000000u : 0x7fffffffu;
    std::uint32_t value = 0;
    while (*text >= '0' && *text <= '9') {
        const auto digit = static_cast<unsigned>(*text++ - '0');
        if (value > (limit - digit) / 10) throw std::out_of_range("Subtitle integer overflow");
        value = value * 10 + digit;
    }
    return negative ? 0u - value : value;
}

inline std::string opening_subtitle_path(const std::string& executable_directory,
                                       const std::string& language, const char* filename) {
    std::string stem(filename);
    const auto dot = stem.rfind('.');
    if (dot != std::string::npos) stem.erase(dot);
    return executable_directory + "\\text\\" + language + "\\subtitles\\" + stem + ".txt";
}

struct OpeningSubtitle {
    std::uint32_t start, end; // Original units100ms; multiplication wraps32 bits.
    SharedText text;
};

// Native owning subset of00641E30/006438D0/00641CC0. The reached ordinary movie
// needs styles0/8. Other catalog entries/credits registration remain separate.
// Retain the actual zero-initialized14 style handles, not fictitious font refs.
class OpeningTextStyles {
public:
    OpeningTextStyles() {
        styles_.emplace(0, TextStyle{2, .03f, 0xff000000u, 1, 2});
        styles_.emplace(8, TextStyle{3, .035f, 0xffffffffu, 2, 1});
    }
    const TextStyle& get(std::uint8_t id) const {
        const auto found = styles_.find(id);
        if (found == styles_.end()) throw std::logic_error("Unsupported or removed opening text style");
        return found->second;
    }
    void clear_movie_styles() noexcept {
        for (auto& id : movie_styles_) {
            if (id >= 0) styles_.erase(static_cast<std::uint32_t>(id));
            id = -1;
        }
    }
    const auto& movie_styles() const { return movie_styles_; }
private:
    std::map<std::uint32_t, TextStyle> styles_;
    std::array<std::int32_t, 14> movie_styles_{};
};

// Ordinary00689970 and the text/audio tail of00688C10. Backend provides actual
// sample ownership, voice calls and rendered text, not successful empty services.
template<class Backend>
class OpeningSubtitles {
public:
    using Sample = typename Backend::Sample;
    struct AudioCue { std::uint32_t start; Sample sample; };
    explicit OpeningSubtitles(Backend& backend) : backend_(backend) {}
    ~OpeningSubtitles() { clear_records(); }
    OpeningSubtitles(const OpeningSubtitles&) = delete;
    OpeningSubtitles& operator=(const OpeningSubtitles&) = delete;

    void parse(LocalizedTextCache& input, bool text_enabled) {
        if (!text_.empty() || !audio_.empty()) throw std::logic_error("Subtitle records already owned");
        for (const auto& entry : input) {
            std::string line(entry.second.text.c_str());
            const auto left = line.find('<'), right = line.find('>');
            if (!line.empty() && line.front() == 'S') {
                auto sample = backend_.create_sample(); // Original allocates before delimiter checks.
                if (left == std::string::npos || right == std::string::npos) continue;
                line[right] = '\0';
                const auto start = subtitle_integer(line.c_str() + left + 1);
                const std::string name(line.c_str() + right + 1);
                if (backend_.load_sample(sample, ("Sounds\\" + name).c_str(), 0) != 0)
                    backend_.load_sample(sample, ("sounds\\DATADISK\\" + name).c_str(), 0);
                audio_.push_back({start, std::move(sample)});
            } else if (text_enabled) {
                const auto comma = line.find(',');
                if (left == std::string::npos || right == std::string::npos || comma == std::string::npos)
                    continue;
                line[comma] = '\0'; line[right] = '\0';
                const auto start = subtitle_integer(line.c_str() + left + 1);
                const auto end = subtitle_integer(line.c_str() + comma + 1);
                text_.push_back({start, end, std::make_shared<const std::string>(line.c_str() + right + 1)});
            }
        }
        input.clear(); //006E4C80: this is the movie-local cache, not startup captions.
    }

    void update() {
        backend_.read_movie_time(movie_time_); // Error result leaves the previous value intact.
        if (backend_.has_text()) {
            if (active_ != -1) {
                if (static_cast<std::size_t>(active_) >= text_.size())
                    throw std::logic_error("Subtitle index belongs to an unsupported reused video state");
                if (text_[active_].end * 100u < movie_time_) {
                    backend_.time_marker();
                    backend_.clear_text();
                    active_ = -1;
                }
            }
            if (active_ == -1) {
                for (std::size_t i = 0; i < text_.size(); ++i) {
                    const auto& cue = text_[i];
                    if (cue.start * 100u < movie_time_ && movie_time_ < cue.end * 100u) {
                        backend_.time_marker();
                        backend_.set_text(cue.text, 8, 0);
                        active_ = static_cast<std::int32_t>(i); // Last overlap wins; no break.
                    }
                }
            }
        }
        if (backend_.has_voice() && next_audio_ < audio_.size() &&
                audio_[next_audio_].start * 100u < movie_time_) {
            backend_.set_driver_volume();
            backend_.bind_sample(audio_[next_audio_].sample, 0);
            backend_.voice_volume(1);
            backend_.play_voice(0, 0);
            ++next_audio_; // Exactly one per update, independent of call results.
        }
    }
    void clear_records() noexcept {
        while (!text_.empty()) text_.pop_back();
        while (!audio_.empty()) audio_.pop_back();
    }
    const auto& text() const { return text_; }
    const auto& audio() const { return audio_; }
    std::uint32_t movie_time() const { return movie_time_; }
    std::int32_t active() const { return active_; }
    std::size_t next_audio() const { return next_audio_; }
private:
    Backend& backend_;
    std::vector<OpeningSubtitle> text_;
    std::vector<AudioCue> audio_;
    std::uint32_t movie_time_ = 0;
    std::int32_t active_ = -1;
    std::size_t next_audio_ = 0;
};

} // namespace hd2::frontend
#endif
