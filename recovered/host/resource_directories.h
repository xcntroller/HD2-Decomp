#ifndef HD2_HOST_RESOURCE_DIRECTORIES_H
#define HD2_HOST_RESOURCE_DIRECTORIES_H

#include "io/resource_directories.h"
#include "host/menu_scene.h"

namespace hd2::host {

// Win32 Find APIs, not CRT _findfirst: matches 006ed6b0's attributes/order.
class ResourceDirectoryPlatform {
public:
    std::intptr_t find_first(const std::string& pattern, io::ResourceDirectoryEntry& output) {
        WIN32_FIND_DATAA entry{};
        const auto handle = FindFirstFileA(pattern.c_str(), &entry);
        if (handle != INVALID_HANDLE_VALUE) {
            try {
                output = {entry.dwFileAttributes, entry.cFileName};
            } catch (...) {
                FindClose(handle);
                throw;
            }
        }
        return reinterpret_cast<std::intptr_t>(handle);
    }
    bool find_next(std::intptr_t handle, io::ResourceDirectoryEntry& output) {
        WIN32_FIND_DATAA entry{};
        if (!FindNextFileA(reinterpret_cast<HANDLE>(handle), &entry)) return false;
        output = {entry.dwFileAttributes, entry.cFileName};
        return true;
    }
    void find_close(std::intptr_t handle) { FindClose(reinterpret_cast<HANDLE>(handle)); }
};

// Borrowed LS3DF module. Resolve GetIGraph EACH call, as original category0
// routing does. The engine owns its graphics paths; client caches are native.
class GraphicsResourceDirectories {
public:
    explicit GraphicsResourceDirectories(HMODULE module) : module_(module) {
        static_assert(sizeof(void*) == 4, "Original resource ABI requires i386");
        const auto entry = module ? GetProcAddress(module, "GetIGraph") : nullptr;
        if (!entry || reinterpret_cast<std::uintptr_t>(entry) !=
                reinterpret_cast<std::uintptr_t>(module) + 0x68f90)
            throw std::runtime_error("Unsupported resource GetIGraph export");
        static_assert(sizeof(get_graphics_) == sizeof(entry));
        std::memcpy(&get_graphics_, &entry, sizeof(entry));
    }
    std::int32_t add_graphics_directory(const char* path, std::uint32_t recurse) const {
        using Add = std::int32_t (WINAPI *)(EngineFrame*, const char*, std::uint32_t);
        auto* graphics = get_graphics_();
        return scene_method<Add>(graphics, module_, 0x110, 0x6ff80)(graphics, path, recurse);
    }
private:
    HMODULE module_;
    using GetGraphics = EngineFrame* (__cdecl *)();
    GetGraphics get_graphics_ = nullptr;
};

}
#endif
