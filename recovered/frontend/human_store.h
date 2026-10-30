#ifndef HD2_FRONTEND_HUMAN_STORE_H
#define HD2_FRONTEND_HUMAN_STORE_H

#include "frontend/localized_text.h"
#include "io/chunk_file.h"
#include <algorithm>
#include <memory>

namespace hd2::frontend {

// Native storage for original 0x74-byte records. Strings/vector/portrait pointer
// are deliberately not the original ABI. Unknown names retain offset labels.
struct HumanRecord {
    std::string first = "First", last = "Last", nickname;
    std::string full_name, short_name, user_name;
    std::string face = "x_f_001.bmp", eye = "e_f001.bmp";
    void* portrait = nullptr;
    std::uint32_t field24 = 0;
    std::vector<std::uint32_t> items;
    std::array<std::uint32_t, 3> fields38{};
    // Rank followed by nine IEEE single-precision words. Direct 26AD loads
    // preserve all bits, including NaN payloads. Legacy formats convert ints.
    std::array<std::uint32_t, 10> stats{{0xffffffffu}};
    std::array<std::uint32_t, 2> tail{{1, 0}}; // +6C and +70 (flag plus padding).
};

// 0069FDE0: only legacy combined-name and nickname tags rebuild these strings.
inline void derive_human_names(HumanRecord& record) {
    record.full_name.clear();
    record.short_name.clear();
    const auto full = record.first + (record.nickname.empty() ? " " : " (" + record.nickname + ") ") + record.last;
    const auto short_name = record.first.empty() ? record.last : record.first.substr(0, 1) + ". " + record.last;
    if (full.size() >= 256 || short_name.size() >= 256)
        throw std::length_error("Original HumanStore name buffer exceeded");
    record.full_name = full;
    record.short_name = short_name;
}

// 0069FEF0: lowercased first substring match, not a filename-prefix test.
// Original CRT startup C-locale case conversion is explicit here.
inline void derive_human_eye(HumanRecord& record) {
    record.eye = "e_f001.bmp";
    if (record.face.size() >= 1024) throw std::length_error("Original HumanStore face buffer exceeded");
    auto face = record.face;
    for (auto& byte : face) if (byte >= 'A' && byte <= 'Z') byte += 'a' - 'A';
    const auto at = face.find("x_f_");
    if (at != std::string::npos) record.eye = "e_f" + face.substr(at + 4);
}

// 006A0AE0/0064D0B0/0064D0E0. Preserve this older validator's quirks: ED
// surrogate encodings accepted, and F4's second byte has no 80 lower bound.
inline bool human_text_is_utf8(std::string_view text) {
    for (std::size_t at = 0; at < text.size();) {
        const auto byte = [&](std::size_t offset) { return static_cast<unsigned char>(text[at + offset]); };
        const auto lead = byte(0);
        const std::size_t count = lead < 0xc0 ? 1 : lead < 0xe0 ? 2 : lead < 0xf0 ? 3 : lead < 0xf8 ? 4 : lead < 0xfc ? 5 : 6;
        if (count > 4 || count > text.size() - at || (lead >= 0x80 && lead <= 0xc1) || lead >= 0xf5) return false;
        for (std::size_t i = 2; i < count; ++i) if (byte(i) < 0x80 || byte(i) > 0xbf) return false;
        if (count > 1) {
            const auto second = byte(1);
            const auto lower = lead == 0xe0 ? 0xa0 : lead == 0xf0 ? 0x90 : lead == 0xf4 ? 0 : 0x80;
            const auto upper = lead == 0xf4 ? 0x8f : 0xbf;
            if (second < lower || second > upper) return false;
        }
        at += count;
    }
    return true;
}

// 006A0B20 is a very narrow conversion, not a general Windows codepage codec.
inline bool convert_human_text(std::string& text) {
    std::string result;
    for (const unsigned char byte : text) {
        if (byte <= 0x7f) result += static_cast<char>(byte);
        else if (byte == 0xed) result += "\xc3\xad";
        else return false; // Leave original contents intact on failure.
    }
    text = std::move(result);
    return true;
}

inline void finalize_human_text(HumanRecord& record, bool wide) {
    if (!wide) return;
    for (auto* text : {&record.first, &record.last, &record.nickname,
                      &record.full_name, &record.short_name, &record.user_name})
        if (!human_text_is_utf8(*text)) convert_human_text(*text);
}

// 238C legacy combined name. Search pointers are captured BEFORE inserting
// terminators; skip exactly one space after ')'. Preserve untouched defaults.
inline void parse_human_combined_name(HumanRecord& record, std::string text) {
    if (text.size() >= 128) throw std::length_error("Original HumanStore legacy name buffer exceeded");
    if (!text.empty()) {
        const auto space = text.find(' '), open = text.find('('), close = text.find(')');
        if (space != std::string::npos) {
            text[space] = 0;
            record.first = text.c_str();
        } else if (open != std::string::npos) {
            text[open] = 0;
            record.first = text.c_str();
        }
        if (open != std::string::npos && close != std::string::npos) {
            text[close] = 0;
            record.nickname = text.c_str() + open + 1;
        }
        if (close != std::string::npos) {
            text[close] = 0;
            auto at = close + 1;
            if (text[at] == ' ') ++at; // std::string's end terminator is readable.
            record.last = text.c_str() + at;
        } else if (space != std::string::npos) record.last = text.c_str() + space + 1;
    }
    derive_human_names(record);
}

// Original store/reset/creation/parser/overlay 0069F930, 006A18E0/1B40/1D80,
// 006A00A0/0F30. Graphics and its original DLL must outlive this owner.
// profile_directory must come from the application's actual current-profile
// resolver (006B7100), including its trailing separator; not a guessed default.
template<class Graphics>
class HumanStore {
public:
    explicit HumanStore(Graphics& graphics) : graphics_(graphics) {}
    ~HumanStore() { clear(); }
    HumanStore(const HumanStore&) = delete;
    HumanStore& operator=(const HumanStore&) = delete;
    const auto& records() const { return records_; }
    const auto& default_names() const { return default_names_; }
    HumanRecord* record(std::int32_t index) const {
        return index >= 0 && static_cast<std::size_t>(index) < records_.size() ? records_[index].get() : nullptr;
    }
    void reset(const LocalizedTextCache& text) {
        clear();
        for (std::size_t i = 0; i < default_names_.size(); ++i) {
            default_names_[i].first = lookup_localized_text(text, 0x27a6 + static_cast<std::int32_t>(i));
            default_names_[i].second = lookup_localized_text(text, 0x2792 + static_cast<std::int32_t>(i));
        }
    }
    template<class Files>
    bool load(Files& files, const std::string& filename, const LocalizedTextCache& text,
              const std::string& profile_directory, bool wide) {
        return load_with_profile_directory(files, filename, text,
                                            [&] { return profile_directory; }, wide);
    }
    template<class Files, class ProfileDirectory>
    bool load_with_profile_directory(Files& files, const std::string& filename,
              const LocalizedTextCache& text, ProfileDirectory&& profile_directory, bool wide) {
        io::ChunkFile<Files> file(files);
        if (!file.open(filename.c_str())) return false; // No reset on missing main file.
        enter(file); // Original accepts any root tag here.
        reset(text);
        while (remaining(files, file) != 0) {
            enter(file); // Record tag is likewise not validated.
            auto value = std::make_unique<HumanRecord>();
            derive_human_eye(*value);
            derive_human_names(*value);
            records_.push_back(std::move(value)); // Append BEFORE parsing.
            parse_record(files, file, *records_.back(), wide);
            file.leave();
        }
        // Original queries the current profile here, after record finalizers.
        apply_user_names(files, profile_directory(), wide); // Optional result ignored.
        file.close();
        return true;
    }
    template<class Files>
    bool apply_user_names(Files& files, const std::string& profile_directory, bool wide) {
        io::ChunkFile<Files> file(files);
        if (!file.open((profile_directory + "usernames.sav").c_str())) return true;
        if (enter(file) != 0x029a) return false;
        while (remaining(files, file) > 0) {
            auto* value = record(enter(file)); // Child tag is a zero-based index.
            if (!value || enter(file) != 0x23b4) return false;
            auto name = read_string(files, file);
            file.leave();
            value->user_name = std::move(name);
            finalize_human_text(*value, wide); // Does not rebuild names or reload texture.
            file.leave();
        }
        file.leave();
        file.close();
        return true;
    }
private:
    void clear() {
        for (auto& value : records_) {
            if (value && value->portrait) graphics_.release_portrait(value->portrait);
            value.reset();
        }
        records_.clear(); // Keep vector capacity, original forward destruction order.
    }
    static std::int32_t signed_word(std::uint32_t value) {
        std::int32_t result;
        std::memcpy(&result, &value, 4);
        return result;
    }
    static std::uint32_t load_word(const unsigned char* bytes) {
        return std::uint32_t{bytes[0]} | (std::uint32_t{bytes[1]} << 8) |
               (std::uint32_t{bytes[2]} << 16) | (std::uint32_t{bytes[3]} << 24);
    }
    template<class Files>
    static std::int32_t remaining(Files& files, const io::ChunkFile<Files>& file) {
        const auto& state = file.state();
        if (!state.opened || !state.depth) return 0;
        const auto position = files.seek(state.handle, 0, 1);
        return position == 0xffffffffu ? 0 : signed_word(state.offsets[state.depth] - position);
    }
    template<class Files>
    static std::uint16_t enter(io::ChunkFile<Files>& file) {
        std::uint16_t id = 0;
        // Native guard: original may reuse uninitialized/stale tag after failure.
        if (!file.enter(id)) throw std::runtime_error("Truncated HumanStore chunk header");
        return id;
    }
    template<class Files>
    static std::string read_string(Files& files, io::ChunkFile<Files>& file) {
        const auto size = std::min<std::int32_t>(999, remaining(files, file));
        if (size <= 0) throw std::runtime_error("Invalid HumanStore string size");
        std::array<char, 999> bytes{};
        if (!file.read(bytes.data(), static_cast<std::uint32_t>(size)))
            throw std::runtime_error("Truncated HumanStore string");
        const auto end = std::find(bytes.begin(), bytes.begin() + size, '\0');
        if (end == bytes.begin() + size) throw std::runtime_error("Unterminated HumanStore string");
        return std::string(bytes.begin(), end);
    }
    template<std::size_t Count, class Files>
    static std::array<std::uint32_t, Count> read_words(io::ChunkFile<Files>& file) {
        std::array<unsigned char, Count * 4> bytes{};
        if (!file.read(bytes.data(), static_cast<std::uint32_t>(bytes.size())))
            throw std::runtime_error("Truncated HumanStore numeric field");
        std::array<std::uint32_t, Count> words{};
        for (std::size_t i = 0; i < Count; ++i) words[i] = load_word(bytes.data() + i * 4);
        return words;
    }
    static std::uint32_t float_word(std::uint32_t value) {
        static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559);
        const float converted = static_cast<float>(signed_word(value));
        std::uint32_t word;
        std::memcpy(&word, &converted, 4);
        return word;
    }
    template<class Files>
    void parse_record(Files& files, io::ChunkFile<Files>& file, HumanRecord& value, bool wide) {
        value.items.clear();
        while (remaining(files, file) != 0) {
            const auto id = enter(file);
            switch (id) {
            case 0x238c: {
                const auto name = read_string(files, file);
                file.leave();
                parse_human_combined_name(value, name);
                break;
            }
            case 0x2396: value.first = read_string(files, file); file.leave(); break;
            case 0x23a0: value.last = read_string(files, file); file.leave(); break;
            case 0x23aa:
                value.nickname = read_string(files, file); file.leave(); derive_human_names(value); break;
            case 0x23f0:
                value.face = read_string(files, file); file.leave(); derive_human_eye(value); break;
            case 0x26de: value.field24 = read_words<1>(file)[0]; file.leave(); break;
            case 0x26f3: case 0x26f4: case 0x26f5:
                value.fields38[id - 0x26f3] = read_words<1>(file)[0]; file.leave(); break;
            case 0x26fc: {
                const auto word = read_words<1>(file)[0]; file.leave(); value.items.push_back(word); break;
            }
            case 0x2580: value.tail = read_words<2>(file); file.leave(); break;
            case 0x26ad: value.stats = read_words<10>(file); file.leave(); break;
            case 0x26ac: {
                // Original zeroes this scratch word ONCE and ignores read results.
                std::array<unsigned char, 4> bytes{};
                for (std::size_t i = 0; i < 10; ++i) {
                    file.read(bytes.data(), 4);
                    const auto word = load_word(bytes.data());
                    value.stats[i] = i ? float_word(word) : word;
                }
                file.leave();
                break;
            }
            case 0x24b8: case 0x2648: {
                std::array<std::uint32_t, 19> words{};
                if (id == 0x24b8) {
                    const auto old = read_words<18>(file);
                    std::copy(old.begin(), old.end(), words.begin());
                } else words = read_words<19>(file);
                file.leave();
                value.stats[0] = words[0];
                constexpr std::array<std::size_t, 9> indices{{1, 2, 4, 5, 7, 9, 6, 13, 12}};
                for (std::size_t i = 0; i < 9; ++i) value.stats[i + 1] = float_word(words[indices[i]]);
                break;
            }
            default: break; // Original does NOT leave unknown chunks; preserve that traversal.
            }
        }
        finalize_human_text(value, wide);
        if (value.portrait) graphics_.release_portrait(value.portrait);
        if (graphics_.load_portrait(value.face.c_str(), "x_fc_a.bmp", 4, &value.portrait) < 0)
            value.portrait = nullptr; // Load failure does not fail the record/store.
    }
    Graphics& graphics_;
    std::vector<std::unique_ptr<HumanRecord>> records_;
    std::array<std::pair<std::string, std::string>, 8> default_names_{};
};

}
#endif
