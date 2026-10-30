#ifndef HD2_IO_RESOURCE_DIRECTORIES_H
#define HD2_IO_RESOURCE_DIRECTORIES_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace hd2::io {

inline std::string directory_c_string(const std::string& value) {
    return value.substr(0, value.find('\0'));
}

// Client cache +18 insertion / +1c removal: 006d7590/7770, 006d8550/8730,
// 006d9640/9820, 006d6b80/6d60. Normal original derived slots dispatch to the
// matching removal body BEFORE append. Only the last exact duplicate moves.
// This recovers the directory list, not the cache's separate loaded objects.
class ResourceDirectoryList {
public:
    const std::vector<std::string>& paths() const { return paths_; }
    void add(const std::string& path) {
        remove(path);
        paths_.push_back(normalize(path));
    }
    void remove(const std::string& path) {
        const auto normalized = normalize(path);
        for (auto count = paths_.size(); count != 0; --count) {
            if (paths_[count - 1] == normalized) {
                paths_.erase(paths_.begin() + static_cast<std::ptrdiff_t>(count - 1));
                break;
            }
        }
    }
    void clear() { paths_.clear(); }
private:
    static std::string normalize(const std::string& value) {
        auto path = directory_c_string(value);
        // Original reads length-1 even for empty strings. Reject that case
        // explicitly, rather than depend on a refcounted allocation header.
        if (path.empty()) throw std::invalid_argument("Empty resource directory");
        if (path.back() != '\\') path += '\\';
        return path;
    }
    std::vector<std::string> paths_;
};

struct ResourceDirectoryEntry {
    std::uint32_t attributes = 0;
    std::string name;
};

// Original CRT _strlwr's default C locale branch (007e5c20). Other locale
// mappings are outside this source checkpoint; bytes >=80 are left untouched.
inline void lowercase_resource_path(std::string& path) {
    for (auto& byte : path)
        if (byte >= 'A' && byte <= 'Z') byte = static_cast<char>(byte + ('a' - 'A'));
}

// Five DISTINCT client cache owners; categories 1 and 5 share code, not state.
// Graphics category zero is still a real engine service supplied by the caller.
template<class Graphics>
class ResourceDirectoryCaches {
public:
    explicit ResourceDirectoryCaches(Graphics& graphics) : graphics_(graphics) {}
    ResourceDirectoryList& client(std::uint32_t category) {
        if (category == 0 || category > 5)
            throw std::out_of_range("Invalid client resource directory category");
        return clients_[category - 1];
    }
    const ResourceDirectoryList& client(std::uint32_t category) const {
        if (category == 0 || category > 5)
            throw std::out_of_range("Invalid client resource directory category");
        return clients_[category - 1];
    }
    void add(std::uint32_t category, const std::string& path) {
        if (category == 0) graphics_.add_graphics_directory(path.c_str(), 0);
        else if (category <= 5) client(category).add(path);
        // Original switch has no default registration, but still enumerates.
    }
private:
    Graphics& graphics_;
    std::array<ResourceDirectoryList, 5> clients_;
};

// 006ed6b0/006ed570: depth-first preorder, in FindFirst/Next order. Original
// follows directory attributes (including reparse directories), no sorting or
// dedup/cycle policy. Keep each parent search open across child recursion.
template<class Caches, class Platform>
void enumerate_resource_directories(Caches& caches, Platform& platform,
                                     const std::string& directory, std::uint32_t category) {
    if (directory.size() + 4 >= 512)
        throw std::length_error("Resource directory search exceeds original buffer");
    ResourceDirectoryEntry entry;
    const auto handle = platform.find_first(directory + "\\*.*", entry);
    if (handle == -1) return;
    try {
        do {
            auto name = directory_c_string(entry.name);
            // Original constructs/lowers every composite before filtering.
            auto path = directory + "\\" + name;
            if (path.size() >= 512)
                throw std::length_error("Resource directory exceeds original buffer");
            lowercase_resource_path(path);
            if ((entry.attributes & 0x10) == 0 || name == "." || name == "..") continue;
            caches.add(category, path);
            enumerate_resource_directories(caches, platform, path, category);
        } while (platform.find_next(handle, entry));
    } catch (...) {
        platform.find_close(handle); // Native exception cleanup, not SEH parity.
        throw;
    }
    platform.find_close(handle);
}

// 006ed760: root spelling is preserved; descendants above are lowercased.
// Register the root even when enumeration fails. No existence prerequisite.
template<class Caches, class Platform>
void add_resource_directory_tree(Caches& caches, Platform& platform,
                                  const std::string& input, std::uint32_t category) {
    const auto directory = directory_c_string(input);
    if (directory.size() >= 1004)
        throw std::length_error("Resource root exceeds original buffer");
    caches.add(category, directory);
    enumerate_resource_directories(caches, platform, directory, category);
}

// Eight directory calls of 006dbe60, BEFORE its unresolved protection postlude.
// This function claims only the ordinary directory registration region.
template<class Caches, class Platform>
void setup_resource_directory_trees(Caches& caches, Platform& platform) {
    add_resource_directory_tree(caches, platform, "Maps", 0);
    add_resource_directory_tree(caches, platform, "Maps_C", 0);
    add_resource_directory_tree(caches, platform, "Maps_U", 0);
    add_resource_directory_tree(caches, platform, "Models", 1);
    add_resource_directory_tree(caches, platform, "Sounds", 2);
    add_resource_directory_tree(caches, platform, "Models", 3);
    add_resource_directory_tree(caches, platform, "Models", 4);
    add_resource_directory_tree(caches, platform, "Models", 5);
}

}
#endif
