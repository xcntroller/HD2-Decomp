#ifndef HD2_HOST_PROFILE_DIRECTORIES_H
#define HD2_HOST_PROFILE_DIRECTORIES_H

#include "frontend/profile_directories.h"
#include "host/resource_directories.h"
#include <direct.h>
#include <cstdio>

namespace hd2::host {

// 006ED900/910/920 wrap the old CRT FindFirstFileA/FindNextFileA/FindClose
// implementation. Scanner uses only its directory bit and filename, so the
// existing Win32 search boundary supplies the same consumed values. Other CRT
// timestamp/errno outputs are outside this bounded source contract.
template<class Directory>
class ProfileDirectoryPlatform {
public:
    explicit ProfileDirectoryPlatform(Directory& directory) : directory_(directory) {}
    std::intptr_t find_first(const std::string& pattern, io::ResourceDirectoryEntry& output) {
        return search_.find_first(pattern, output);
    }
    bool find_next(std::intptr_t handle, io::ResourceDirectoryEntry& output) {
        return search_.find_next(handle, output);
    }
    void find_close(std::intptr_t handle) { search_.find_close(handle); }
    bool restore_executable_directory() { return _chdir(directory_.get().c_str()) != -1; }
    bool change_to_directory(const std::string& relative) {
        return _chdir((directory_.get() + "\\" + relative).c_str()) != -1;
    }
    bool probe_save_file(const std::string& path) {
        auto* file = std::fopen(path.c_str(), "r");
        if (!file) return false;
        std::fclose(file);
        return true;
    }
private:
    Directory& directory_;
    ResourceDirectoryPlatform search_;
};

}
#endif
