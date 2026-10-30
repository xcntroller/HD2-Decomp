#ifndef HD2_FRONTEND_PROFILE_DIRECTORIES_H
#define HD2_FRONTEND_PROFILE_DIRECTORIES_H

#include "io/chunk_file.h"
#include "io/resource_directories.h"
#include <algorithm>
#include <map>
#include <string>

namespace hd2::frontend {

// Original profile-name map: displayed name -> physical directory spelling.
// Replaces its old string/tree ABI, not the profile/configuration data owner.
class ProfileDirectories {
public:
    using Callback = void (*)(const char*, void*);
    std::string current_name; // Original profile owner +00.
    const auto& names() const { return names_; }

    // 006AE530 / 006B7100. No case folding or fallback to displayed name when
    // wide-mode mapping is absent. Empty name yields PlayersProfiles\\.
    std::string resolve(const std::string& name, bool wide) const {
        const std::string value(name.c_str());
        if (!wide) return value;
        const auto found = names_.find(value);
        return found == names_.end() ? std::string{} : found->second;
    }
    std::string current_directory(bool wide) const {
        return "PlayersProfiles\\" + resolve(current_name, wide) + "\\";
    }

    // 006AF730: metadata lookup always initializes output to physical name;
    // only a first 1EDD field inside root1EDC replaces it, including empty text.
    template<class Files>
    static bool display_name(Files& files, const std::string& directory, std::string& output) {
        output = directory.c_str();
        io::ChunkFile<Files> file(files);
        const auto path = "PlayersProfiles\\" + std::string(directory.c_str()) + "\\profile.sav";
        if (!file.open(path.c_str())) return false;
        if (enter(file) != 0x1edc) return false;
        while (remaining(files, file) > 0) {
            if (enter(file) == 0x1edd) {
                output = text(files, file);
                file.leave();
                file.close(); // Do not leave root before original early close.
                return true;
            }
            file.leave();
        }
        return false;
    }

    // 006AEF50: clear map even on failed search; enumerate unsorted directory
    // entries, metadata/map only in wide mode, callback for each accepted name.
    // Platform restores the actual executable cwd on normal completion, even
    // on a failed initial search; its result is ignored by original scan.
    template<class Platform, class Files>
    void scan(Platform& platform, Files& files, bool wide,
              Callback callback = nullptr, void* user = nullptr) {
        names_.clear();
        io::ResourceDirectoryEntry entry{};
        const auto handle = platform.find_first("PlayersProfiles\\*", entry);
        if (handle != -1) {
            try {
                do {
                    if (!(entry.attributes & 0x10) || entry.name == "." || entry.name == "..") continue;
                    auto name = entry.name;
                    if (wide) {
                        display_name(files, entry.name, name); // Boolean deliberately ignored.
                        names_.emplace(name, entry.name); // First exact duplicate wins.
                    }
                    if (callback) callback(name.c_str(), user); // Original ECX name / EDX user.
                } while (platform.find_next(handle, entry));
            } catch (...) {
                platform.find_close(handle); // Native exception cleanup.
                throw;
            }
            platform.find_close(handle);
        }
        platform.restore_executable_directory();
    }
private:
    template<class Files>
    static std::uint16_t enter(io::ChunkFile<Files>& file) {
        std::uint16_t tag = 0;
        if (!file.enter(tag)) throw std::runtime_error("Truncated profile-name chunk header");
        return tag;
    }
    template<class Files>
    static std::int32_t remaining(Files& files, const io::ChunkFile<Files>& file) {
        const auto& state = file.state();
        if (!state.opened || !state.depth) return 0;
        const auto pos = files.seek(state.handle, 0, 1);
        if (pos == 0xffffffffu) return 0;
        const auto bits = state.offsets[state.depth] - pos;
        std::int32_t result;
        std::memcpy(&result, &bits, 4);
        return result;
    }
    template<class Files>
    static std::string text(Files& files, io::ChunkFile<Files>& file) {
        const auto size = std::min<std::int32_t>(999, remaining(files, file));
        if (size <= 0) throw std::runtime_error("Invalid profile-name string size");
        std::array<char, 999> bytes{};
        if (!file.read(bytes.data(), static_cast<std::uint32_t>(size))) throw std::runtime_error("Truncated profile name");
        const auto end = std::find(bytes.begin(), bytes.begin() + size, '\0');
        if (end == bytes.begin() + size) throw std::runtime_error("Unterminated profile name");
        return std::string(bytes.begin(), end);
    }
    std::map<std::string, std::string> names_;
};

}
#endif
