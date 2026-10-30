#ifndef HD2_FRONTEND_LOCALIZED_TEXT_H
#define HD2_FRONTEND_LOCALIZED_TEXT_H

#include "frontend/startup_language.h"
#include <limits>
#include <map>
#include <utility>
#include <vector>

namespace hd2::frontend {

struct LocalizedTextEntry {
    std::string text;
    std::uint8_t flag;
};
using LocalizedTextCache = std::map<std::int32_t, LocalizedTextEntry>;

// 006E4FB0, shared by startup data and profile/menu consumers.
inline std::string lookup_localized_text(const LocalizedTextCache& cache, std::int32_t id) {
    const auto found = cache.find(id);
    return found == cache.end() ? "__undefined index <" + std::to_string(id) + "> in all_txt __" : found->second.text;
}

// 006e5060: signed ID ordering, reject -1 and duplicates; first text wins.
inline bool add_localized_text(LocalizedTextCache& cache, std::int32_t id,
                                std::string text, std::uint8_t flag) {
    if (id == -1 || cache.find(id) != cache.end()) return false;
    cache.emplace(id, LocalizedTextEntry{std::move(text), flag});
    return true;
}

// 006e5240 uses first and LAST quote on one C-string line, with sscanf("%d")
// of its beginning. No escape decoding, multiline join or comment-quote rule.
// Native numeric overflow throws rather than relying on CRT overflow behavior.
inline void parse_localized_text_record(LocalizedTextCache& cache, std::string_view line) {
    line = line.substr(0, line.find('\0'));
    const auto first = line.find('"'), last = line.rfind('"');
    if (first == std::string_view::npos || first == last) return;
    std::size_t index = 0;
    while (index < line.size() && (line[index] == ' ' ||
            (line[index] >= '\t' && line[index] <= '\r'))) ++index;
    bool negative = false;
    if (index < line.size() && (line[index] == '+' || line[index] == '-'))
        negative = line[index++] == '-';
    const auto digits = index;
    const std::uint32_t limit = negative ? 0x80000000u : 0x7fffffffu;
    std::uint32_t value = 0;
    while (index < line.size() && line[index] >= '0' && line[index] <= '9') {
        const auto digit = static_cast<std::uint32_t>(line[index++] - '0');
        if (value > (limit - digit) / 10) throw std::out_of_range("Localization ID overflow");
        value = value * 10 + digit;
    }
    if (index == digits) return;
    const auto id = static_cast<std::int32_t>(negative ? -static_cast<std::int64_t>(value) : value);
    add_localized_text(cache, id, std::string(line.substr(first + 1, last - first - 1)), 0);
}

// 006e5300/006e5320: skip CR/LF runs, copy each bounded line for record parsing.
inline void parse_localized_text(LocalizedTextCache& cache, std::string_view bytes) {
    std::size_t begin = 0;
    while (begin < bytes.size()) {
        while (begin < bytes.size() && (bytes[begin] == '\r' || bytes[begin] == '\n')) ++begin;
        if (begin == bytes.size()) return;
        const auto end = bytes.find_first_of("\r\n", begin);
        const auto length = (end == std::string_view::npos ? bytes.size() : end) - begin;
        if (!length) return;
        parse_localized_text_record(cache, bytes.substr(begin, length));
        begin += length;
    }
}

// 006e4cf0 delegates dtaOpen/Seek/Read/Close. Preserve its surprising close
// condition: only handle ZERO is closed on normal return, including failures.
// UTF16LE is rejected; UTF8 BOM is skipped without decoding remaining bytes.
template<class Files>
bool load_localized_text_file(LocalizedTextCache& cache, Files& files, const std::string& path) {
    const std::uint32_t handle = files.open(path.c_str(), 0);
    bool success = false;
    if (handle != 0xffffffffu) {
        const std::uint32_t size = files.seek(handle, 0, 2);
        if (size && size <= static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max())) {
            std::vector<char> bytes(size);
            if (files.seek(handle, 0, 0) == 0 && files.read(handle, bytes.data(), size) == size) {
                const auto byte = [&](std::size_t i) { return static_cast<unsigned char>(bytes[i]); };
                if (size >= 2 && byte(0) == 0xff && byte(1) == 0xfe) {
                    files.text_diagnostic(path, true);
                } else {
                    const auto skip = size >= 3 && byte(0) == 0xef && byte(1) == 0xbb && byte(2) == 0xbf ? 3u : 0u;
                    parse_localized_text(cache, std::string_view(bytes.data() + skip, size - skip));
                    success = true;
                }
            }
        }
    }
    if (!success) files.text_diagnostic(path, false);
    if (handle == 0) files.close(handle);
    return success;
}

// 006b1090 enumerates loose names under the executable directory, loads through
// the original DTA boundary using relative paths, and ignores individual failures.
template<class Platform, class Files, class Directory>
void preload_language_text(LocalizedTextCache& cache, Platform& platform, Files& files,
                            Directory& directory, const std::string& language) {
    const std::string name(language.c_str());
    const auto pattern = directory.get() + "\\text\\" + name + "\\*.txt";
    std::string filename;
    const auto handle = platform.find_first(pattern, filename);
    if (handle == -1) return;
    try {
        do {
            if (!startup_case_equal(filename, ".") && !startup_case_equal(filename, "..") &&
                !startup_case_equal(filename, "dialogs.txt"))
                load_localized_text_file(cache, files, "text\\" + name + "\\" + filename);
        } while (platform.find_next(handle, filename) != -1);
    } catch (...) {
        platform.find_close(handle); // Native exception cleanup beyond original.
        throw;
    }
    platform.find_close(handle);
}

}
#endif
