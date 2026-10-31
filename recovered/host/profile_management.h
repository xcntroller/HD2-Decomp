#ifndef HD2_HOST_PROFILE_MANAGEMENT_H
#define HD2_HOST_PROFILE_MANAGEMENT_H
#include "frontend/profile_management.h"
#include <string>
#include <windows.h>
namespace hd2::host {
// 006AE630/006AE990/006AEC90, English physical-directory branch. Keep all
// operations below the staged executable's PlayersProfiles; never change cwd.
class ProfileFilesystem {
  public:
    explicit ProfileFilesystem(std::string root) : root_(std::move(root)) {}
    int create(const std::string &name) const {
        if (!frontend::profile_component(name))
            return -2;
        const auto base = root_ + "\\PlayersProfiles";
        if (!CreateDirectoryA(base.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
            return -2;
        if (CreateDirectoryA(path(name).c_str(), nullptr))
            return 1;
        return GetLastError() == ERROR_ALREADY_EXISTS ? -1 : -2;
    }
    int rename(const std::string &from, const std::string &to) const {
        if (!directory_component(from) || !frontend::profile_component(to))
            return -2;
        if (MoveFileA(path(from).c_str(), path(to).c_str()))
            return 1;
        return GetLastError() == ERROR_ALREADY_EXISTS ? -1 : -2;
    }
    bool remove(const std::string &name) const {
        return directory_component(name) && remove_tree(path(name));
    }

  private:
    static bool directory_component(const std::string &name) {
        return !name.empty() && name != "." && name != ".." && name.find('\0') == std::string::npos &&
               name.find_first_of("/\\:") == std::string::npos;
    }
    std::string path(const std::string &name) const { return root_ + "\\PlayersProfiles\\" + name; }
    static bool remove_tree(const std::string &directory) {
        const auto attributes = GetFileAttributesA(directory.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY) ||
            (attributes & FILE_ATTRIBUTE_REPARSE_POINT))
            return false;
        WIN32_FIND_DATAA entry{};
        const auto handle = FindFirstFileA((directory + "\\*.*").c_str(), &entry);
        if (handle == INVALID_HANDLE_VALUE)
            return false;
        bool result = true;
        do {
            const std::string name = entry.cFileName;
            if (name == "." || name == "..")
                continue;
            const auto child = directory + "\\" + name;
            if (entry.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
                result = false;
                break;
            }
            if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (!remove_tree(child)) {
                    result = false;
                    break;
                }
            } else if (!DeleteFileA(child.c_str())) {
                // 006E8DA0 retries after resetting attributes. The original
                // ignores a second file-unlink failure, but rmdir reports it.
                SetFileAttributesA(child.c_str(), FILE_ATTRIBUTE_NORMAL);
                DeleteFileA(child.c_str());
            }
        } while (FindNextFileA(handle, &entry));
        FindClose(handle);
        if (!result)
            return false;
        if (RemoveDirectoryA(directory.c_str()))
            return true;
        SetFileAttributesA(directory.c_str(), FILE_ATTRIBUTE_NORMAL);
        return RemoveDirectoryA(directory.c_str()) != 0;
    }
    std::string root_;
};
} // namespace hd2::host
#endif
