#ifndef HD2_FRONTEND_LEGACY_TEXT_H
#define HD2_FRONTEND_LEGACY_TEXT_H

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace hd2::frontend {

// 006E9230: append nonempty raw CR/LF-separated lines. The caller supplies the
// path; game.cfg and profile.pr share this body and its signed-handle close rule.
template<class Files>
bool load_legacy_text_lines(Files& files, const char* path, std::vector<std::string>& lines) {
    const auto handle = files.open(path, 0);
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
// A key mentioned in a comment or value can match. No trimming or key anchoring.
inline const char* legacy_text_value(const std::vector<std::string>& lines, const char* key) {
    for (const auto& stored : lines) {
        const std::string_view line(stored.c_str());
        if (line.find(key) != std::string_view::npos) {
            const auto equal = line.find('=');
            if (equal == std::string_view::npos) throw std::runtime_error("Original legacy text value pointer is invalid");
            return stored.c_str() + equal + 1;
        }
    }
    return nullptr;
}

// 006E9690 / string forwarding thunk006E9830: replace the entire first line
// containing key, or append. Later duplicates and unrelated lines survive.
inline bool set_legacy_text_value(std::vector<std::string>& lines, const char* key, const char* value) {
    const std::string replacement = std::string(key) + "=" + value;
    for (auto& stored : lines) {
        if (std::string_view(stored.c_str()).find(key) != std::string_view::npos) {
            stored = replacement;
            return true;
        }
    }
    lines.push_back(replacement);
    return true;
}

// 006E94B0: DTA writer, raw stored length followed by a separate CRLF write.
// Every successful handle, including signed-negative ones, closes here. Close
// errors do not alter the result. Failure retains any already-written prefix.
template<class Files>
bool save_legacy_text_lines(Files& files, const char* path, const std::vector<std::string>& lines) {
    const auto handle = files.open_write(path, 0x10000);
    if (handle == 0xffffffffu) return false;
    try {
        for (const auto& line : lines) {
            if (line.size() > 0x7fffffffu) throw std::length_error("Legacy text line exceeds original count");
            const auto size = static_cast<std::uint32_t>(line.size());
            if (files.write(handle, line.data(), size) != size || files.write(handle, "\r\n", 2) != 2) {
                files.close(handle);
                return false;
            }
        }
    } catch (...) { files.close(handle); throw; }
    files.close(handle);
    return true;
}

}
#endif
